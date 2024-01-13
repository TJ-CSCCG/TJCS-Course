
#include <NTL/lzz_p.h>


NTL_START_IMPL


NTL_TLS_GLOBAL_DECL(SmartPtr<zz_pInfoT>, zz_pInfo_stg)

NTL_CHEAP_THREAD_LOCAL zz_pInfoT *zz_pInfo = 0;



SmartPtr<zz_pInfoT> Build_zz_pInfo(FFTPrimeInfo *info)
{
   return MakeSmart<zz_pInfoT>(INIT_FFT, info);
}


zz_pInfoT::zz_pInfoT(long NewP, long maxroot)
{
   if (maxroot < 0) LogicError("zz_pContext: maxroot may not be negative");

   if (NewP <= 1) LogicError("zz_pContext: p must be > 1");
   if (NumBits(NewP) > NTL_SP_NBITS) ResourceError("zz_pContext: modulus too big");

   ZZ P, B, M, M1, MinusM;
   long n, i;
   long q, t;
   mulmod_t qinv;

   p = NewP;
   pinv = PrepMulMod(p);
   red_struct = sp_PrepRem(p);
   ll_red_struct = make_sp_ll_reduce_struct(p);
   ZZ_red_struct.build(p);

   p_info = 0;

   conv(P, p);

   sqr(B, P);
   LeftShift(B, B, maxroot+NTL_FFTFudge);

   set(M);
   n = 0;
   while (M <= B) {
      UseFFTPrime(n);
      q = GetFFTPrime(n);
      n++;
      mul(M, M, q);
   }

   if (n > 4) LogicError("zz_pInit: too many primes");

   NumPrimes = n;
   PrimeCnt = n;
   MaxRoot = CalcMaxRoot(q);

   if (maxroot < MaxRoot)
      MaxRoot = maxroot;

   negate(MinusM, M);
   MinusMModP = rem(MinusM, p);
   MinusMModPpinv = PrepMulModPrecon(MinusMModP, p, pinv);

   CoeffModP.SetLength(n);
   CoeffModPpinv.SetLength(n);
   x.SetLength(n);
   u.SetLength(n);
   uqinv.SetLength(n);

   for (i = 0; i < n; i++) {
      q = GetFFTPrime(i);
      qinv = GetFFTPrimeInv(i);

      div(M1, M, q);
      t = rem(M1, q);
      t = InvMod(t, q);
      CoeffModP[i] = rem(M1, p);
      CoeffModPpinv[i] = PrepMulModPrecon(CoeffModP[i], p, pinv); 
      x[i] = ((double) t)/((double) q);
      u[i] = t;
      uqinv[i] = PrepMulModPrecon(t, q, qinv);
   }
}

zz_pInfoT::zz_pInfoT(INIT_FFT_TYPE, FFTPrimeInfo *info)
{
   p = info->q;
   pinv = info->qinv;
   red_struct = sp_PrepRem(p);
   ll_red_struct = make_sp_ll_reduce_struct(p);
   ZZ_red_struct.build(p);


   p_info = info;

   NumPrimes = 1;
   PrimeCnt = 0;

   MaxRoot = CalcMaxRoot(p);
}

// FIXME: we could make bigtab an optional argument

zz_pInfoT::zz_pInfoT(INIT_USER_FFT_TYPE, long q)
{
   long w;
   if (!IsFFTPrime(q, w)) LogicError("invalid user supplied prime");

   p = q;
   pinv = PrepMulMod(p);
   red_struct = sp_PrepRem(p);
   ll_red_struct = make_sp_ll_reduce_struct(p);
   ZZ_red_struct.build(p);


   p_info_owner.make();
   p_info = p_info_owner.get();

   long bigtab_index = -1;
#ifdef NTL_FFT_BIGTAB
   bigtab_index = 0;
#endif
   InitFFTPrimeInfo(*p_info, q, w, bigtab_index); 

   NumPrimes = 1;
   PrimeCnt = 0;

   MaxRoot = CalcMaxRoot(p);
}


void zz_p::init(long p, long maxroot)
{
   zz_pContext c(p, maxroot);
   c.restore();

}

void zz_p::FFTInit(long index)
{
   zz_pContext c(INIT_FFT, index);
   c.restore();
}

void zz_p::UserFFTInit(long q)
{
   zz_pContext c(INIT_USER_FFT, q);
   c.restore();
}

zz_pContext::zz_pContext(long p, long maxroot) : 
   ptr(MakeSmart<zz_pInfoT>(p, maxroot)) 
{ }

zz_pContext::zz_pContext(INIT_FFT_TYPE, long index)
{
   if (index < 0)
      LogicError("bad FFT prime index");

   UseFFTPrime(index);

   ptr =  FFTTables[index]->zz_p_context;
}

zz_pContext::zz_pContext(INIT_USER_FFT_TYPE, long q) :
   ptr(MakeSmart<zz_pInfoT>(INIT_USER_FFT, q))
{ }


void zz_pContext::save()
{
   NTL_TLS_GLOBAL_ACCESS(zz_pInfo_stg);
   ptr = zz_pInfo_stg;
}

void zz_pContext::restore() const
{
   NTL_TLS_GLOBAL_ACCESS(zz_pInfo_stg);
   zz_pInfo_stg = ptr;
   zz_pInfo = zz_pInfo_stg.get();
}



zz_pBak::~zz_pBak()
{
   if (MustRestore) c.restore();
}

void zz_pBak::save()
{
   c.save();
   MustRestore = true;
}


void zz_pBak::restore()
{
   c.restore();
   MustRestore = false;
}







istream& operator>>(istream& s, zz_p& x)
{
   NTL_ZZRegister(y);
   NTL_INPUT_CHECK_RET(s, s >> y);
   conv(x, y);

   return s;
}

ostream& operator<<(ostream& s, zz_p a)
{
   NTL_ZZRegister(y);
   y = rep(a);
   s << y;

   return s;
}



// ***********************************************************************


#ifdef NTL_HAVE_LL_TYPE


// NOTE: the following code sequence will generate imulq 
// instructions on x86_64 machines, which empirically is faster
// than using the mulq instruction or even the mulxq instruction,
// (tested on a Haswell machine).

long 
InnerProd_LL(const long *ap, const zz_p *bp, long n, long d, 
          sp_ll_reduce_struct dinv)
{
   const long BLKSIZE = (1L << min(20, 2*(NTL_BITS_PER_LONG-NTL_SP_NBITS)));

   unsigned long acc0 = 0;
   ll_type acc21;
   ll_init(acc21, 0);

   long i;
   for (i = 0; i <= n-BLKSIZE; i += BLKSIZE, ap += BLKSIZE, bp += BLKSIZE) {
      // sum ap[j]*rep(bp[j]) for j in [0..BLKSIZE)

      ll_type sum;
      ll_init(sum, 0);
      for (long j = 0; j < BLKSIZE; j += 4) {
         ll_imul_add(sum, ap[j+0], rep(bp[j+0]));
         ll_imul_add(sum, ap[j+1], rep(bp[j+1]));
         ll_imul_add(sum, ap[j+2], rep(bp[j+2]));
         ll_imul_add(sum, ap[j+3], rep(bp[j+3]));
      }

      ll_add(sum, acc0); 
      acc0 = ll_get_lo(sum);
      ll_add(acc21, ll_get_hi(sum));
   }

   if (i < n) {
      // sum ap[i]*rep(bp[j]) for j in [0..n-i)

      ll_type sum; 
      ll_init(sum, 0);
      long j = 0;
      for (; j <= n-i-4; j += 4) {
         ll_imul_add(sum, ap[j+0], rep(bp[j+0]));
         ll_imul_add(sum, ap[j+1], rep(bp[j+1]));
         ll_imul_add(sum, ap[j+2], rep(bp[j+2]));
         ll_imul_add(sum, ap[j+3], rep(bp[j+3]));
      }

      for (; j < n-i; j++)
         ll_imul_add(sum, ap[j], rep(bp[j]));


      ll_add(sum, acc0); 
      acc0 = ll_get_lo(sum);
      ll_add(acc21, ll_get_hi(sum));
   }

   if (dinv.nbits == NTL_SP_NBITS) 
      return sp_ll_red_31_normalized(ll_get_hi(acc21), ll_get_lo(acc21), acc0, d, dinv);
   else
      return sp_ll_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, d, dinv);
}


long 
InnerProd_LL(const zz_p *ap, const zz_p *bp, long n, long d, 
          sp_ll_reduce_struct dinv)
{
   const long BLKSIZE = (1L << min(20, 2*(NTL_BITS_PER_LONG-NTL_SP_NBITS)));

   unsigned long acc0 = 0;
   ll_type acc21;
   ll_init(acc21, 0);

   long i;
   for (i = 0; i <= n-BLKSIZE; i += BLKSIZE, ap += BLKSIZE, bp += BLKSIZE) {
      // sum ap[j]*rep(bp[j]) for j in [0..BLKSIZE)

      ll_type sum;
      ll_init(sum, 0);
      for (long j = 0; j < BLKSIZE; j += 4) {
         ll_imul_add(sum, rep(ap[j+0]), rep(bp[j+0]));
         ll_imul_add(sum, rep(ap[j+1]), rep(bp[j+1]));
         ll_imul_add(sum, rep(ap[j+2]), rep(bp[j+2]));
         ll_imul_add(sum, rep(ap[j+3]), rep(bp[j+3]));
      }

      ll_add(sum, acc0); 
      acc0 = ll_get_lo(sum);
      ll_add(acc21, ll_get_hi(sum));
   }

   if (i < n) {
      // sum ap[i]*rep(bp[j]) for j in [0..n-i)

      ll_type sum; 
      ll_init(sum, 0);
      long j = 0;
      for (; j <= n-i-4; j += 4) {
         ll_imul_add(sum, rep(ap[j+0]), rep(bp[j+0]));
         ll_imul_add(sum, rep(ap[j+1]), rep(bp[j+1]));
         ll_imul_add(sum, rep(ap[j+2]), rep(bp[j+2]));
         ll_imul_add(sum, rep(ap[j+3]), rep(bp[j+3]));
      }

      for (; j < n-i; j++)
         ll_imul_add(sum, rep(ap[j]), rep(bp[j]));


      ll_add(sum, acc0); 
      acc0 = ll_get_lo(sum);
      ll_add(acc21, ll_get_hi(sum));
   }

   if (dinv.nbits == NTL_SP_NBITS) 
      return sp_ll_red_31_normalized(ll_get_hi(acc21), ll_get_lo(acc21), acc0, d, dinv);
   else
      return sp_ll_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, d, dinv);
}


long 
InnerProd_L(const long *ap, const zz_p *bp, long n, long d, 
          sp_reduce_struct dinv, long bound)
{
   unsigned long sum = 0;
   long j = 0;

   if (n <= bound) {
      for (; j <= n-4; j += 4) {
	 sum += (ap[j+0]) * rep(bp[j+0]);
	 sum += (ap[j+1]) * rep(bp[j+1]);
	 sum += (ap[j+2]) * rep(bp[j+2]);
	 sum += (ap[j+3]) * rep(bp[j+3]);
      }

      for (; j < n; j++)
	 sum += (ap[j]) * rep(bp[j]);

      return rem(sum, d, dinv);
   }

   for (; j <= n-bound; j += bound) {

      long i = 0;
      for (; i <= bound-4; i += 4) {
         sum += (ap[j+i+0]) * rep(bp[j+i+0]);
         sum += (ap[j+i+1]) * rep(bp[j+i+1]);
         sum += (ap[j+i+2]) * rep(bp[j+i+2]);
         sum += (ap[j+i+3]) * rep(bp[j+i+3]);
      }

      for (; i < bound; i++) {
         sum += (ap[j+i]) * rep(bp[j+i]);
      }

      sum = rem(sum, d, dinv);
   }

   if (j >= n) return sum;

   for (; j <= n-4; j += 4) {
      sum += (ap[j+0]) * rep(bp[j+0]);
      sum += (ap[j+1]) * rep(bp[j+1]);
      sum += (ap[j+2]) * rep(bp[j+2]);
      sum += (ap[j+3]) * rep(bp[j+3]);
   }

   for (; j < n; j++)
      sum += (ap[j]) * rep(bp[j]);

   return rem(sum, d, dinv);
}

long 
InnerProd_L(const zz_p *ap, const zz_p *bp, long n, long d, 
          sp_reduce_struct dinv, long bound)
{
   unsigned long sum = 0;
   long j = 0;

   if (n <= bound) {
      for (; j <= n-4; j += 4) {
	 sum += rep(ap[j+0]) * rep(bp[j+0]);
	 sum += rep(ap[j+1]) * rep(bp[j+1]);
	 sum += rep(ap[j+2]) * rep(bp[j+2]);
	 sum += rep(ap[j+3]) * rep(bp[j+3]);
      }

      for (; j < n; j++)
	 sum += rep(ap[j]) * rep(bp[j]);

      return rem(sum, d, dinv);
   }

   for (; j <= n-bound; j += bound) {

      long i = 0;
      for (; i <= bound-4; i += 4) {
         sum += rep(ap[j+i+0]) * rep(bp[j+i+0]);
         sum += rep(ap[j+i+1]) * rep(bp[j+i+1]);
         sum += rep(ap[j+i+2]) * rep(bp[j+i+2]);
         sum += rep(ap[j+i+3]) * rep(bp[j+i+3]);
      }

      for (; i < bound; i++) {
         sum += rep(ap[j+i]) * rep(bp[j+i]);
      }

      sum = rem(sum, d, dinv);
   }

   if (j >= n) return sum;

   for (; j <= n-4; j += 4) {
      sum += rep(ap[j+0]) * rep(bp[j+0]);
      sum += rep(ap[j+1]) * rep(bp[j+1]);
      sum += rep(ap[j+2]) * rep(bp[j+2]);
      sum += rep(ap[j+3]) * rep(bp[j+3]);
   }

   for (; j < n; j++)
      sum += rep(ap[j]) * rep(bp[j]);

   return rem(sum, d, dinv);
}

#endif



NTL_END_IMPL

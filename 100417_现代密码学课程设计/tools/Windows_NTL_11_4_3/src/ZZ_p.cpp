

#include <NTL/ZZ_p.h>
#include <NTL/FFT.h>



NTL_START_IMPL



NTL_TLS_GLOBAL_DECL(SmartPtr<ZZ_pInfoT>, ZZ_pInfo_stg)
NTL_TLS_GLOBAL_DECL(SmartPtr<ZZ_pTmpSpaceT>, ZZ_pTmpSpace_stg)

NTL_CHEAP_THREAD_LOCAL ZZ_pInfoT *ZZ_pInfo = 0;
NTL_CHEAP_THREAD_LOCAL ZZ_pTmpSpaceT *ZZ_pTmpSpace = 0;
NTL_CHEAP_THREAD_LOCAL bool ZZ_pInstalled = false;



ZZ_pInfoT::ZZ_pInfoT(const ZZ& NewP)
{
   if (NewP <= 1) LogicError("ZZ_pContext: p must be > 1");

   p = NewP;
   size = p.size();

   ExtendedModulusSize = 2*size + 
                 (NTL_BITS_PER_LONG + NTL_ZZ_NBITS - 1)/NTL_ZZ_NBITS;

}



// we use a lazy strategy for initializing and installing
// FFTInfo and TmpSpace related to a ZZ_p modulus.  
// The routines GetFFTInfo and GetTmpSpace make sure this process 
// is complete.

void ZZ_p::DoInstall()
{
   SmartPtr<ZZ_pTmpSpaceT> tmps; 

   do { // NOTE: thread safe lazy init 
      Lazy<ZZ_pFFTInfoT>::Builder builder(ZZ_pInfo->FFTInfo);
      if (!builder()) break;

      UniquePtr<ZZ_pFFTInfoT> FFTInfo;
      FFTInfo.make();

      ZZ B, M, M1, M2, M3;
      long n, i;
      long q, t;
      mulmod_t qinv;

      sqr(B, ZZ_pInfo->p);

      LeftShift(B, B, NTL_FFTMaxRoot+NTL_FFTFudge);

      // FIXME: the following is quadratic time...would
      // be nice to get a faster solution...
      // One could estimate the # of primes by summing logs,
      // then multiply using a tree-based multiply, then 
      // adjust up or down...

      // Assuming IEEE floating point, the worst case estimate
      // for error guarantees a correct answer +/- 1 for
      // numprimes up to 2^25...for sure we won't be
      // using that many primes...we can certainly put in 
      // a sanity check, though. 

      // If I want a more accuaruate summation (with using Kahan,
      // which has some portability issues), I could represent 
      // numbers as x = a + f, where a is integer and f is the fractional
      // part.  Summing in this representation introduces an *absolute*
      // error of 2 epsilon n, which is just as good as Kahan 
      // for this application.

      // same strategy could also be used in the ZZX HomMul routine,
      // if we ever want to make that subquadratic

      set(M);
      n = 0;
      while (M <= B) {
         UseFFTPrime(n);
         q = GetFFTPrime(n);
         n++;
         mul(M, M, q);
      }

      FFTInfo->NumPrimes = n;
      FFTInfo->MaxRoot = CalcMaxRoot(q);


      double fn = double(n);

      // NOTE: the following checks is somewhat academic,
      // but the implementation relies on it

      if (8.0*fn*(fn+48) > NTL_FDOUBLE_PRECISION)
         ResourceError("modulus too big");


      FFTInfo->rem_struct.init(n, ZZ_pInfo->p, GetFFTPrime);
      FFTInfo->crt_struct.init(n, ZZ_pInfo->p, GetFFTPrime);

      if (!FFTInfo->crt_struct.special()) {
         FFTInfo->prime.SetLength(n);
         FFTInfo->prime_recip.SetLength(n);
         FFTInfo->u.SetLength(n);
         FFTInfo->uqinv.SetLength(n);

         // montgomery
         FFTInfo->reduce_struct.init(ZZ_pInfo->p, ZZ(n) << NTL_SP_NBITS);

         ZZ qq, rr;

         DivRem(qq, rr, M, ZZ_pInfo->p);

         NegateMod(FFTInfo->MinusMModP, rr, ZZ_pInfo->p);

         // montgomery
         FFTInfo->reduce_struct.adjust(FFTInfo->MinusMModP);

         for (i = 0; i < n; i++) {
            q = GetFFTPrime(i);
            qinv = GetFFTPrimeInv(i);

            long tt = rem(qq, q);

            mul(M2, ZZ_pInfo->p, tt);
            add(M2, M2, rr); 
            div(M2, M2, q);  // = (M/q) rem p
            

            div(M1, M, q);
            t = rem(M1, q);
            t = InvMod(t, q);

            // montgomery
            FFTInfo->reduce_struct.adjust(M2);

            FFTInfo->crt_struct.insert(i, M2);

            FFTInfo->prime[i] = q;
            FFTInfo->prime_recip[i] = 1/double(q);
            FFTInfo->u[i] = t;
            FFTInfo->uqinv[i] = PrepMulModPrecon(FFTInfo->u[i], q, qinv);
         }

      }

      tmps = MakeSmart<ZZ_pTmpSpaceT>();
      tmps->crt_tmp_vec.fetch(FFTInfo->crt_struct);
      tmps->rem_tmp_vec.fetch(FFTInfo->rem_struct);

      builder.move(FFTInfo);
   } while (0);

   if (!tmps) {
      const ZZ_pFFTInfoT *FFTInfo = ZZ_pInfo->FFTInfo.get();
      tmps = MakeSmart<ZZ_pTmpSpaceT>();
      tmps->crt_tmp_vec.fetch(FFTInfo->crt_struct);
      tmps->rem_tmp_vec.fetch(FFTInfo->rem_struct);
   }

   NTL_TLS_GLOBAL_ACCESS(ZZ_pTmpSpace_stg);
   ZZ_pTmpSpace_stg = tmps; 
   ZZ_pTmpSpace = ZZ_pTmpSpace_stg.get();
}




void ZZ_p::init(const ZZ& p)
{
   ZZ_pContext c(p);
   c.restore();
}


void ZZ_pContext::save() 
{ 
   NTL_TLS_GLOBAL_ACCESS(ZZ_pInfo_stg);
   ptr = ZZ_pInfo_stg; 
}

void ZZ_pContext::restore() const
{
   if (ZZ_pInfo == ptr.get()) return; 
   // NOTE: this simple optimization could be useful in some situations,
   //    for example, a worker thread re-setting the current modulus
   //    in a multi-threaded build

   NTL_TLS_GLOBAL_ACCESS(ZZ_pInfo_stg);
   ZZ_pInfo_stg = ptr;
   ZZ_pInfo = ZZ_pInfo_stg.get();

   NTL_TLS_GLOBAL_ACCESS(ZZ_pTmpSpace_stg);
   ZZ_pTmpSpace_stg = 0;
   ZZ_pTmpSpace = 0;

   ZZ_pInstalled = false;
}



ZZ_pBak::~ZZ_pBak()
{
   if (MustRestore) c.restore();
}

void ZZ_pBak::save()
{
   c.save();
   MustRestore = true;
}


void ZZ_pBak::restore()
{
   c.restore();
   MustRestore = false;
}


const ZZ_p& ZZ_p::zero()
{
   static const ZZ_p z(INIT_NO_ALLOC); // GLOBAL (assumes C++11 thread-safe init)
   return z;
}

NTL_CHEAP_THREAD_LOCAL
ZZ_p::DivHandlerPtr ZZ_p::DivHandler = 0;

   

ZZ_p::ZZ_p(INIT_VAL_TYPE, const ZZ& a)  // NO_ALLOC
{
   conv(*this, a);
} 

ZZ_p::ZZ_p(INIT_VAL_TYPE, long a) // NO_ALLOC
{
   conv(*this, a);
}


void conv(ZZ_p& x, long a)
{
   if (a == 0)
      clear(x);
   else if (a == 1)
      set(x);
   else {
      NTL_ZZRegister(y);

      conv(y, a);
      conv(x, y);
   }
}

istream& operator>>(istream& s, ZZ_p& x)
{
   NTL_ZZRegister(y);

   NTL_INPUT_CHECK_RET(s, s >> y);
   conv(x, y);

   return s;
}

void div(ZZ_p& x, const ZZ_p& a, const ZZ_p& b)
{
   NTL_ZZ_pRegister(T);

   inv(T, b);
   mul(x, a, T);
}

void inv(ZZ_p& x, const ZZ_p& a)
{
   NTL_ZZRegister(T);

   if (InvModStatus(T, a._ZZ_p__rep, ZZ_p::modulus())) {
      if (!IsZero(a._ZZ_p__rep) && ZZ_p::DivHandler)
         (*ZZ_p::DivHandler)(a);

      InvModError("ZZ_p: division by non-invertible element",
                   a._ZZ_p__rep, ZZ_p::modulus());
   }

   x._ZZ_p__rep = T;
}

long operator==(const ZZ_p& a, long b)
{
   if (b == 0)
      return IsZero(a);

   if (b == 1)
      return IsOne(a);

   NTL_ZZ_pRegister(T);
   conv(T, b);
   return a == T;
}



void add(ZZ_p& x, const ZZ_p& a, long b)
{
   NTL_ZZ_pRegister(T);
   conv(T, b);
   add(x, a, T);
}

void sub(ZZ_p& x, const ZZ_p& a, long b)
{
   NTL_ZZ_pRegister(T);
   conv(T, b);
   sub(x, a, T);
}

void sub(ZZ_p& x, long a, const ZZ_p& b)
{
   NTL_ZZ_pRegister(T);
   conv(T, a);
   sub(x, T, b);
}

void mul(ZZ_p& x, const ZZ_p& a, long b)
{
   NTL_ZZ_pRegister(T);
   conv(T, b);
   mul(x, a, T);
}

void div(ZZ_p& x, const ZZ_p& a, long b)
{
   NTL_ZZ_pRegister(T);
   conv(T, b);
   div(x, a, T);
}

void div(ZZ_p& x, long a, const ZZ_p& b)
{
   if (a == 1) {
      inv(x, b);
   }
   else {
      NTL_ZZ_pRegister(T);
      conv(T, a);
      div(x, T, b);
   }
}

NTL_END_IMPL

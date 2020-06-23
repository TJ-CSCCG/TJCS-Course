

#include <NTL/ZZX.h>
#include <NTL/BasicThreadPool.h>


NTL_START_IMPL




struct NewFastCRTHelperScratch {
   Vec<ZZ> tmp_vec;        // length == nlevels+1
   ZZ tmp1, tmp2, tmp3;
};


struct NewFastCRTHelper {

   ZZ prod;
   ZZ prod_half;
   
   long nprimes;

   long nlevels;
   long veclen;
   long nblocks;   // number of nodes in the last level
   long start_last_level;   // index of first item in last level

   Vec<long> nprimes_vec;  // length == veclen
   Vec<long> first_vec;    // length == nblocks
   Vec<ZZ> prod_vec;       // length == veclen

   Vec<long> coeff_vec;    // length == nprimes, coeff_vec[i] = (prod/p_i)^{-1} mod p_i

   Vec<long> prime_vec;   // length == nprimes
   Vec<const sp_ZZ_reduce_struct*> red_struct_vec; // length == nprimes
   Vec<mulmod_precon_t> coeffpinv_vec; // length == nprimes
   Vec<ZZVec> ppvec;       // length == nblocks
   
   long GetNumPrimes() const { return nprimes; }

   NewFastCRTHelper(long bound); 

   void fill_nprimes_vec(long index); 
   void fill_prod_vec(long index);

   void reduce_aux(const ZZ& value, long *remainders, NewFastCRTHelperScratch& scratch,
                   long index, long level) const;

   void reconstruct_aux(ZZ& value, const long* remainders, NewFastCRTHelperScratch& scratch,
                        long index, long level) const;


   void reduce(const ZZ& value, long *remainders, NewFastCRTHelperScratch& scratch) const;
   void reconstruct(ZZ& value, const long *remainders, NewFastCRTHelperScratch& scratch) const;

   void init_scratch(NewFastCRTHelperScratch& scratch) const;

};

void NewFastCRTHelper::init_scratch(NewFastCRTHelperScratch& scratch) const
{
   scratch.tmp_vec.SetLength(nlevels+1);
}

void NewFastCRTHelper::fill_nprimes_vec(long index) 
{
   long left, right;
   left = 2*index + 1;
   right = 2*index + 2;
   if (left >= veclen) return;

   nprimes_vec[left] = nprimes_vec[index]/2;
   nprimes_vec[right] = nprimes_vec[index] - nprimes_vec[left];
   fill_nprimes_vec(left);
   fill_nprimes_vec(right);
}

void NewFastCRTHelper::fill_prod_vec(long index)
{
   long left, right;
   left = 2*index + 1;
   right = 2*index + 2;
   if (left >= veclen) return;

   fill_prod_vec(left);
   fill_prod_vec(right);
   mul(prod_vec[index], prod_vec[left], prod_vec[right]);
}

NewFastCRTHelper::NewFastCRTHelper(long bound) 
{
   long thresh = 96;
   bound += 2; // extra 2 bits ensures correct results

   // assumes bound >= 1, thresh >= 1

   prod = 1;
   for (nprimes = 0; NumBits(prod) <= bound; nprimes++) {
      UseFFTPrime(nprimes);
      prod *= GetFFTPrime(nprimes);
   }

   RightShift(prod_half, prod, 1);

   long sz = nprimes;
   nlevels = 1;
   while (sz > thresh) {
      sz = sz/2;
      nlevels++;
   }

   veclen = (1L << nlevels) - 1;
   nblocks = 1L << (nlevels-1);
   start_last_level = (1L << (nlevels-1)) - 1;

   nprimes_vec.SetLength(veclen);
   nprimes_vec[0] = nprimes;

   fill_nprimes_vec(0);

   first_vec.SetLength(nblocks+1);

   first_vec[0] = 0;
   for (long k = 1; k <= nblocks; k++)
      first_vec[k] = first_vec[k-1] + nprimes_vec[start_last_level + k-1];

   prod_vec.SetLength(veclen);

   // fill product leaves
   for (long k = 0; k < nblocks; k++) {
      prod_vec[start_last_level + k] = 1;
      for (long i = first_vec[k]; i < first_vec[k+1]; i++) {
         prod_vec[start_last_level + k] *= GetFFTPrime(i);
      }
   }

   // fill rest of product trees
   fill_prod_vec(0);

   ZZ t1;

   coeff_vec.SetLength(nprimes);
   prime_vec.SetLength(nprimes);
   red_struct_vec.SetLength(nprimes);
   coeffpinv_vec.SetLength(nprimes);

   for (long i = 0; i < nprimes; i++) {
      long p = GetFFTPrime(i);
      div(t1, prod, p);
      long tt = rem(t1, p);
      tt = InvMod(tt, p);
      coeff_vec[i] = tt;
      prime_vec[i] = p;
      red_struct_vec[i] = &GetFFT_ZZ_red_struct(i);
      coeffpinv_vec[i] = PrepMulModPrecon(tt, p);
   }

   ppvec.SetLength(nblocks);
   for (long k = 0; k < nblocks; k++) {
      const ZZ& block_prod = prod_vec[start_last_level + k];
      ppvec[k].SetSize(first_vec[k+1]-first_vec[k], block_prod.size());
      for (long i = first_vec[k]; i < first_vec[k+1]; i++) {
         div(t1, block_prod, prime_vec[i]);
         ppvec[k][i-first_vec[k]] = t1;
      }
   }

}

void NewFastCRTHelper::reduce_aux(const ZZ& value, long *remainders, 
                                  NewFastCRTHelperScratch& scratch,
                                  long index, long level) const
{
   long left, right;
   left = 2*index + 1;
   right = 2*index + 2;

   ZZ& result = scratch.tmp_vec[level];

   if (NumBits(value) <= NumBits(prod_vec[index]))
      result = value;
   else {
      rem(scratch.tmp1, value, prod_vec[index]);
      sub(scratch.tmp2, scratch.tmp1, prod_vec[index]);
      if (NumBits(scratch.tmp2) < NumBits(scratch.tmp1))
         result = scratch.tmp2;
      else
         result = scratch.tmp1;
   }

   if (left < veclen) {
      reduce_aux(result, remainders, scratch, left, level+1);
      reduce_aux(result, remainders, scratch, right, level+1);
   }
   else {
      long k = index - start_last_level;
      long i_begin = first_vec[k];
      long i_end = first_vec[k+1];

      for (long i = i_begin; i < i_end; i++) {
         remainders[i] = red_struct_vec[i]->rem(result);
      }
   }
}

void NewFastCRTHelper::reduce(const ZZ& value, long *remainders, 
                              NewFastCRTHelperScratch& scratch) const
{
   reduce_aux(value, remainders, scratch, 0, 0);
}

void NewFastCRTHelper::reconstruct_aux(ZZ& value, const long* remainders, 
                                       NewFastCRTHelperScratch& scratch,
                                       long index, long level) const
{
   long left, right;
   left = 2*index + 1;
   right = 2*index + 2;

   if (left >= veclen) {
      long k = index - start_last_level;
      long i_begin = first_vec[k];
      long i_end = first_vec[k+1];
      const ZZ* ppv = ppvec[k].elts();
      ZZ& acc = scratch.tmp1;

      QuickAccumBegin(acc, prod_vec[index].size());
      for (long i = i_begin; i < i_end; i++) {
         long p = prime_vec[i];
         long tt = coeff_vec[i];
         mulmod_precon_t ttpinv = coeffpinv_vec[i];
         long s = MulModPrecon(remainders[i], tt, p, ttpinv);
         QuickAccumMulAdd(acc, ppv[i-i_begin], s);
      }
      QuickAccumEnd(acc);

      value = acc;
      return;
   }

   reconstruct_aux(scratch.tmp_vec[level], remainders, scratch, left, level+1);
   reconstruct_aux(scratch.tmp1, remainders, scratch, right, level+1);

   mul(scratch.tmp2, scratch.tmp_vec[level], prod_vec[right]);
   mul(scratch.tmp3, scratch.tmp1, prod_vec[left]);
   add(value, scratch.tmp2, scratch.tmp3);
}

void NewFastCRTHelper::reconstruct(ZZ& value, const long *remainders, 
                                   NewFastCRTHelperScratch& scratch) const
{
   reconstruct_aux(scratch.tmp1, remainders, scratch, 0, 0);
   rem(scratch.tmp2, scratch.tmp1, prod);
   if (scratch.tmp2 > prod_half)
      sub(scratch.tmp2, scratch.tmp2, prod);

   value = scratch.tmp2;
}




#define CRT_BLK (8)

void HomMul(ZZX& x, const ZZX& a, const ZZX& b)
{
   if (&a == &b) {
      HomSqr(x, a);
      return;
   }

   long da = deg(a);
   long db = deg(b);

   if (da < 0 || db < 0) {
      clear(x);
      return;
   }

   long dc = da + db;

   zz_pBak bak;
   bak.save();

   long bound = NumBits(min(da, db)+1) + MaxBits(a) + MaxBits(b);

   NewFastCRTHelper H(bound);

   long nprimes = H.GetNumPrimes();

   if (NTL_OVERFLOW(nprimes, CRT_BLK, 0))
      ResourceError("overflow"); // this is pretty academic


   Vec< zz_pX > A, B, C;

   A.SetLength(nprimes);
   for (long i = 0; i < nprimes; i++) A[i].SetLength(da+1);

   NTL_EXEC_RANGE(da+1, first, last)
   {
      Vec<long> remainders_store;
      remainders_store.SetLength(nprimes*CRT_BLK); 
      long *remainders = remainders_store.elts();

      NewFastCRTHelperScratch scratch;
      H.init_scratch(scratch);

      long jj = first;
      for (; jj <= last-CRT_BLK; jj += CRT_BLK) {
	 for (long j = 0; j < CRT_BLK; j++) 
	    H.reduce(a[jj+j], remainders + j*nprimes, scratch);
         for (long i = 0; i < nprimes; i++) {
            zz_p *Ai = A[i].rep.elts();
            for (long j = 0; j < CRT_BLK; j++)
               Ai[jj+j].LoopHole() = remainders[j*nprimes+i];
         }
      }
      if (jj < last) {
	 for (long j = 0; j < last-jj; j++) 
	    H.reduce(a[jj+j], remainders + j*nprimes, scratch);
	 for (long i = 0; i < nprimes; i++) {
            zz_p *Ai = A[i].rep.elts();
	    for (long j = 0; j < last-jj; j++)
	       Ai[jj+j].LoopHole() = remainders[j*nprimes+i];
	 }
      }
   }
   NTL_EXEC_RANGE_END

   B.SetLength(nprimes);
   for (long i = 0; i < nprimes; i++) B[i].SetLength(db+1);

   NTL_EXEC_RANGE(db+1, first, last)
   {
      Vec<long> remainders_store;
      remainders_store.SetLength(nprimes*CRT_BLK); 
      long *remainders = remainders_store.elts();

      NewFastCRTHelperScratch scratch;
      H.init_scratch(scratch);

      long jj = first;
      for (; jj <= last-CRT_BLK; jj += CRT_BLK) {
	 for (long j = 0; j < CRT_BLK; j++) 
	    H.reduce(b[jj+j], remainders + j*nprimes, scratch);
         for (long i = 0; i < nprimes; i++) {
            zz_p *Bi = B[i].rep.elts();
            for (long j = 0; j < CRT_BLK; j++)
               Bi[jj+j].LoopHole() = remainders[j*nprimes+i];
         }
      }
      if (jj < last) {
	 for (long j = 0; j < last-jj; j++) 
	    H.reduce(b[jj+j], remainders + j*nprimes, scratch);
	 for (long i = 0; i < nprimes; i++) {
            zz_p *Bi = B[i].rep.elts();
	    for (long j = 0; j < last-jj; j++)
	       Bi[jj+j].LoopHole() = remainders[j*nprimes+i];
	 }
      }
   }
   NTL_EXEC_RANGE_END
         

   C.SetLength(nprimes);
   for (long i = 0; i < nprimes; i++) C[i].SetMaxLength(dc+1);

   NTL_EXEC_RANGE(nprimes, first, last)
   for (long i = first; i < last; i++) {
      zz_p::FFTInit(i);
      A[i].normalize();
      B[i].normalize();
      mul(C[i], A[i], B[i]);
      long dci = deg(C[i]);
      C[i].SetLength(dc+1);
      if (dci < dc) {
         zz_p *Ci = C[i].rep.elts();
         for (long j = dci+1; j <= dc; j++) Ci[j] = 0;
      }
   }
   NTL_EXEC_RANGE_END

   ZZVec xx;
   xx.SetSize(dc+1, (bound+NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS);
   // NOTE: we pre-allocate all the storage we
   // need to collect the result.  Based on my experience,
   // too many calls to malloc in a multi-threaded setting
   // can lead to significant performance degredation

   NTL_EXEC_RANGE(dc+1, first, last)
   {
      Vec<long> remainders_store;
      remainders_store.SetLength(nprimes*CRT_BLK); 
      long *remainders = remainders_store.elts();

      NewFastCRTHelperScratch scratch;
      H.init_scratch(scratch);

      long jj = first;
      for (; jj <= last-CRT_BLK; jj += CRT_BLK) {
         for (long i = 0; i < nprimes; i++) {
            zz_p *Ci = C[i].rep.elts();
            for (long j = 0; j < CRT_BLK; j++)
               remainders[j*nprimes+i] = rep(Ci[jj+j]);
         }
	 for (long j = 0; j < CRT_BLK; j++) 
	    H.reconstruct(xx[jj+j], remainders + j*nprimes, scratch);
      }
      if (jj < last) {
	 for (long i = 0; i < nprimes; i++) {
            zz_p *Ci = C[i].rep.elts();
	    for (long j = 0; j < last-jj; j++)
               remainders[j*nprimes+i] = rep(Ci[jj+j]);
	 }
	 for (long j = 0; j < last-jj; j++) 
	    H.reconstruct(xx[jj+j], remainders + j*nprimes, scratch);
      }
   }
   NTL_EXEC_RANGE_END

   x.SetLength(dc+1);
   for (long j = 0; j <=dc; j++)
      x[j] = xx[j];
   x.normalize();
}

void HomSqr(ZZX& x, const ZZX& a)
{
   long da = deg(a);

   if (da < 0) {
      clear(x);
      return;
   }

   long dc = da + da;

   zz_pBak bak;
   bak.save();

   long bound = NumBits(da+1) + 2*MaxBits(a);

   NewFastCRTHelper H(bound);

   long nprimes = H.GetNumPrimes();

   if (NTL_OVERFLOW(nprimes, CRT_BLK, 0))
      ResourceError("overflow"); // this is pretty academic


   Vec< zz_pX > A, C;

   A.SetLength(nprimes);
   for (long i = 0; i < nprimes; i++) A[i].SetLength(da+1);

   NTL_EXEC_RANGE(da+1, first, last)
   {
      Vec<long> remainders_store;
      remainders_store.SetLength(nprimes*CRT_BLK); 
      long *remainders = remainders_store.elts();

      NewFastCRTHelperScratch scratch;
      H.init_scratch(scratch);

      long jj = first;
      for (; jj <= last-CRT_BLK; jj += CRT_BLK) {
	 for (long j = 0; j < CRT_BLK; j++) 
	    H.reduce(a[jj+j], remainders + j*nprimes, scratch);
         for (long i = 0; i < nprimes; i++) {
            zz_p *Ai = A[i].rep.elts();
            for (long j = 0; j < CRT_BLK; j++)
               Ai[jj+j].LoopHole() = remainders[j*nprimes+i];
         }
      }
      if (jj < last) {
	 for (long j = 0; j < last-jj; j++) 
	    H.reduce(a[jj+j], remainders + j*nprimes, scratch);
	 for (long i = 0; i < nprimes; i++) {
            zz_p *Ai = A[i].rep.elts();
	    for (long j = 0; j < last-jj; j++)
	       Ai[jj+j].LoopHole() = remainders[j*nprimes+i];
	 }
      }
   }
   NTL_EXEC_RANGE_END


   C.SetLength(nprimes);
   for (long i = 0; i < nprimes; i++) C[i].SetMaxLength(dc+1);

   NTL_EXEC_RANGE(nprimes, first, last)
   for (long i = first; i < last; i++) {
      zz_p::FFTInit(i);
      A[i].normalize();
      sqr(C[i], A[i]);
      long dci = deg(C[i]);
      C[i].SetLength(dc+1);
      if (dci < dc) {
         zz_p *Ci = C[i].rep.elts();
         for (long j = dci+1; j <= dc; j++) Ci[j] = 0;
      }
   }
   NTL_EXEC_RANGE_END

   ZZVec xx;
   xx.SetSize(dc+1, (bound+NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS);
   // NOTE: we pre-allocate all the storage we
   // need to collect the result.  Based on my experience,
   // too many calls to malloc in a multi-threaded setting
   // can lead to significant performance degredation

   NTL_EXEC_RANGE(dc+1, first, last)
   {
      Vec<long> remainders_store;
      remainders_store.SetLength(nprimes*CRT_BLK); 
      long *remainders = remainders_store.elts();

      NewFastCRTHelperScratch scratch;
      H.init_scratch(scratch);

      long jj = first;
      for (; jj <= last-CRT_BLK; jj += CRT_BLK) {
         for (long i = 0; i < nprimes; i++) {
            zz_p *Ci = C[i].rep.elts();
            for (long j = 0; j < CRT_BLK; j++)
               remainders[j*nprimes+i] = rep(Ci[jj+j]);
         }
	 for (long j = 0; j < CRT_BLK; j++) 
	    H.reconstruct(xx[jj+j], remainders + j*nprimes, scratch);
      }
      if (jj < last) {
	 for (long i = 0; i < nprimes; i++) {
            zz_p *Ci = C[i].rep.elts();
	    for (long j = 0; j < last-jj; j++)
               remainders[j*nprimes+i] = rep(Ci[jj+j]);
	 }
	 for (long j = 0; j < last-jj; j++) 
	    H.reconstruct(xx[jj+j], remainders + j*nprimes, scratch);
      }
   }
   NTL_EXEC_RANGE_END

   x.SetLength(dc+1);
   for (long j = 0; j <=dc; j++)
      x[j] = xx[j];
   x.normalize();
}





static
long MaxSize(const ZZX& a)
{
   long res = 0;
   long n = a.rep.length();

   long i;
   for (i = 0; i < n; i++) {
      long t = a.rep[i].size();
      if (t > res)
         res = t;
   }

   return res;
}



void conv(zz_pX& x, const ZZX& a)
{
   conv(x.rep, a.rep);
   x.normalize();
}


void conv(ZZX& x, const zz_pX& a)
{
   conv(x.rep, a.rep);
   x.normalize();
}


long CRT(ZZX& gg, ZZ& a, const zz_pX& G)
{
   long n = gg.rep.length();

   long p = zz_p::modulus();

   ZZ new_a;
   mul(new_a, a, p);

   long a_inv;
   a_inv = rem(a, p);
   a_inv = InvMod(a_inv, p);

   long p1;
   p1 = p >> 1;

   ZZ a1;
   RightShift(a1, a, 1);

   long p_odd = (p & 1);

   long modified = 0;

   long h;

   long m = G.rep.length();

   long max_mn = max(m, n);

   gg.rep.SetLength(max_mn);

   ZZ g;
   long i;

   for (i = 0; i < n; i++) {
      if (!CRTInRange(gg.rep[i], a)) {
         modified = 1;
         rem(g, gg.rep[i], a);
         if (g > a1) sub(g, g, a);
      }
      else
         g = gg.rep[i];
   
      h = rem(g, p);

      if (i < m)
         h = SubMod(rep(G.rep[i]), h, p);
      else
         h = NegateMod(h, p);

      h = MulMod(h, a_inv, p);
      if (h > p1)
         h = h - p;
   
      if (h != 0) {
         modified = 1;

         if (!p_odd && g > 0 && (h == p1))
            MulSubFrom(g, a, h);
         else
            MulAddTo(g, a, h);
      }

      gg.rep[i] = g;
   }


   for (; i < m; i++) {
      h = rep(G.rep[i]);
      h = MulMod(h, a_inv, p);
      if (h > p1)
         h = h - p;
   
      modified = 1;
      mul(g, a, h);
      gg.rep[i] = g;
   }

   gg.normalize();
   a = new_a;

   return modified;
}

long CRT(ZZX& gg, ZZ& a, const ZZ_pX& G)
{
   long n = gg.rep.length();

   const ZZ& p = ZZ_p::modulus();

   ZZ new_a;
   mul(new_a, a, p);

   ZZ a_inv;
   rem(a_inv, a, p);
   InvMod(a_inv, a_inv, p);

   ZZ p1;
   RightShift(p1, p, 1);

   ZZ a1;
   RightShift(a1, a, 1);

   long p_odd = IsOdd(p);

   long modified = 0;

   ZZ h;
   ZZ ah;

   long m = G.rep.length();

   long max_mn = max(m, n);

   gg.rep.SetLength(max_mn);

   ZZ g;
   long i;

   for (i = 0; i < n; i++) {
      if (!CRTInRange(gg.rep[i], a)) {
         modified = 1;
         rem(g, gg.rep[i], a);
         if (g > a1) sub(g, g, a);
      }
      else
         g = gg.rep[i];
   
      rem(h, g, p);

      if (i < m)
         SubMod(h, rep(G.rep[i]), h, p);
      else
         NegateMod(h, h, p);

      MulMod(h, h, a_inv, p);
      if (h > p1)
         sub(h, h, p);
   
      if (h != 0) {
         modified = 1;
         mul(ah, a, h);
   
         if (!p_odd && g > 0 && (h == p1))
            sub(g, g, ah);
         else
            add(g, g, ah);
      }

      gg.rep[i] = g;
   }


   for (; i < m; i++) {
      h = rep(G.rep[i]);
      MulMod(h, h, a_inv, p);
      if (h > p1)
         sub(h, h, p);
   
      modified = 1;
      mul(g, a, h);
      gg.rep[i] = g;
   }

   gg.normalize();
   a = new_a;

   return modified;
}


#define SS_PAR_THRESH (2000.0)
//#define SS_PAR_THRESH (10.0) 
// For testing


static inline bool SS_BelowThresh(long n, long k)
{
   return double(n)*double(k) < SS_PAR_THRESH;
}


static void
SS_AddMod(ZZ& x, const ZZ& a, const ZZ& b, const ZZ& p, long n)
// x = a + b mod p, where p = 2^n+1,  a, b in [0, p).
// x may not alias p.
{
#ifndef NTL_PROVIDES_SS_LIP_IMPL
   add(x, a, b);
   if (x >= p) {
      x--; SwitchBit(x, n); // x -= p
   }
#else
   SS_AddMod_lip_impl(x, a, b, p, n);
#endif
}

static void
SS_SubMod(ZZ& x, const ZZ& a, const ZZ& b, const ZZ& p, long n)
// x = a - b mod p, where p = 2^n+1,  a, b in [0, p).
// x may not alias b or p.
{
#ifndef NTL_PROVIDES_SS_LIP_IMPL
   if (a < b) {
      add(x, a, p);
      SubPos(x, x, b);
   }
   else {
      SubPos(x, a, b);
   }
#else
   SS_SubMod_lip_impl(x, a, b, p, n);
#endif
}



/* Compute a = b * 2^e mod p, where p = 2^n+1. 0<=e<n and 0<b<p are
   assumed. */

static void 
LeftRotate(ZZ& a, const ZZ& b, long e, const ZZ& p, long n, ZZ& scratch)
{
#ifndef NTL_PROVIDES_SS_LIP_IMPL
  if (e == 0) {
    if (&a != &b) {
      a = b;
    }
    return;
  }

  /* scratch := upper e bits of b */
  RightShift(scratch, b, n - e);
  /* a := 2^e * lower n - e bits of b */
  trunc(a, b, n - e);
  LeftShift(a, a, e);
  /* a -= scratch */
  SS_SubMod(a, a, scratch, p, n);
#else
   LeftRotate_lip_impl(a, b, e, p, n, scratch);
#endif
}


#define SS_FFT_THRESH (4)
#define SS_NTEMPS (3)
#define SS_FFT_RDUP (3)

static long 
SS_FFTRoundUp(long xn, long k)
// Assumes k >= 0.
// Returns an integer m such that 1 <= m <= n = 2^k and 
// m divsisible my 2^SS_FFT_RDUP.
// Also, if xn <= n, then m >= xn.
{
   long n = 1L << k;
   if (xn <= 0) xn = 1;

   xn = ((xn+((1L << SS_FFT_RDUP)-1)) >> SS_FFT_RDUP) << SS_FFT_RDUP; 

   if (xn > n - (n >> 4)) xn = n;

   return xn;
}



// p = 2^n+1, where n = r*2^{l-1}, so 2^r is primitive 2^l-th root 
// of unity mod p.

// j in [0, 2^{level-1})
// a = b*2^{j*r*2^{l-level}}
static void
Rotate(ZZ& a, const ZZ& b, long j, long level,
       long r, long l, const ZZ& p, long n, ZZ* tmp)
{
   if (l-level >= 0) 
      LeftRotate(a, b, (j*r) << (l-level), p, n, tmp[0]);
   else if (((j*r) & 1) == 0)
      LeftRotate(a, b, (j*r) >> 1, p, n, tmp[0]);
   else {
      // use sqrt(2) = 2^{3n/4} - 2^{n/4}

      long k = (j*r) >> 1; // j*r = 2*k + 1

      // now compute a = b*2^{k+1/2} mod p

      // a = b*{2^k} mod p
      LeftRotate(a, b, k, p, n, tmp[0]);

      // tmp[1] = a*2^{n/4} mod p
      LeftRotate(tmp[1], a, n >> 2, p, n, tmp[0]);

      // a = a*2^{3n/4} mod p
      LeftRotate(a, a, 3*(n >> 2), p, n, tmp[0]);

      // a -= tmp[1] mod p
      SS_SubMod(a, a, tmp[1], p, n);
   }
}


static void
SS_butterfly(ZZ& x, ZZ& y, const ZZ& p, long n, ZZ* tmp)
// (x, y) := (x+y, x-y)
{
  /* tmp[0] = x - y mod p */
  SS_SubMod(tmp[0], x, y, p, n);

  /* x += y mod p */
  SS_AddMod(x, x, y, p, n);

  y = tmp[0];
}

static void
SS_fwd_butterfly(ZZ& x, ZZ& y, long j, long level, 
                long r, long l, const ZZ& p, long n, 
                ZZ* tmp)

//         ( x, y ) *= ( 1  2^{j*r*2^{l-level}} )
//                     ( 1 -2^{j*r*2^{l-level}} ) 

{
  /* tmp[0] = x - y mod p */
  SS_SubMod(tmp[0], x, y, p, n);

  /* x += y mod p */
  SS_AddMod(x, x, y, p, n);

  /* y = tmp[0] * 2^{j*r*2^{l-level}} mod p */
  Rotate(y, tmp[0], j, level, r, l, p, n, tmp+1);
}

static void
SS_inv_butterfly(ZZ& x, ZZ& y, long j, long level, 
                long r, long l, const ZZ& p, long n, 
                ZZ* tmp)

//         ( x, y ) *= ( 1                     1                    )
//                     ( 2^{-j*r*2^{l-level}} -2^{-j*r*2^{l-level}} ) 

// *** should not be called with j == 0 
//     call SS_butterfly instead

{
  /* tmp[0] = y * 2^{(2^{level-1}-j)*r*2^{l-level}} mod p */
  Rotate(tmp[0], y, (1L<<(level-1))-j, level, r, l, p, n, tmp+1);

  /* y = x + tmp[0] mod p */
  SS_AddMod(y, x, tmp[0], p, n);  // NEGATED

  /* x = x - tmp[0] mod p */
  SS_SubMod(x, x, tmp[0], p, n);  // NEGATED
}


// Much of the following logic is taken from the code in FFT.cpp
// for single-precision modular FFT's, which itself is adapted
// from code originally written by David Harvey.
// See copyright notice in FFT.cpp.

// size == 2^level
static void
fft_layer(ZZ* xp, long blocks, long size, long level, long r, long l,
          const ZZ& p, long n, ZZ* tmp)
{
   size /= 2;
 
   do {
      ZZ *xp0 = xp;
      ZZ *xp1 = xp + size;

      for (long j = 0; j < size; j++)
         SS_fwd_butterfly(xp0[j], xp1[j], j, level, r, l, p, n, tmp);

      xp += 2*size;
   } while (--blocks != 0);
}

static void 
fft_base(ZZ* xp, long lgN, long r, long l, const ZZ& p, long n,
         ZZ* tmp)
{
  long N = 1L << lgN;

  for (long j = lgN, size = N, blocks = 1; 
       j >= 1; j--, blocks <<= 1, size >>= 1)
    fft_layer(xp, blocks, size, j, r, l, p, n, tmp);
}


static void 
fft_rec(ZZ* xp, long lgN, long r, long l, const ZZ& p, long n,
         ZZ* tmp)
{
   if (lgN <= SS_FFT_THRESH) {
      fft_base(xp, lgN, r, l, p, n, tmp);
      return;
   }

   long N = 1L << lgN;
   long half = N >> 1;

   ZZ *xp0 = xp;
   ZZ *xp1 = xp + half;

   for (long j = 0; j < half; j++) 
      SS_fwd_butterfly(xp0[j], xp1[j], j, lgN, r, l, p, n, tmp);

   fft_rec(xp0, lgN-1, r, l, p, n, tmp);
   fft_rec(xp1, lgN-1, r, l, p, n, tmp);
}


static void 
fft_short(ZZ* xp, long yn, long xn, long lgN, 
         long r, long l, const ZZ& p, long n,
         ZZ* tmp, RecursiveThreadPool *pool)
{
  Vec<ZZ> alt_tmp;
  if (!tmp) {
    alt_tmp.SetLength(SS_NTEMPS);
    tmp = &alt_tmp[0];
  }

  long N = 1L << lgN;

  if (yn == N)
    {
      if (xn == N && lgN <= SS_FFT_THRESH)
	{
	  // no truncation
	  fft_base(xp, lgN, r, l, p, n, tmp);
	  return;
	}
    }


  // divide-and-conquer algorithm

  long half = N >> 1;

  if (yn <= half)
    {
      if (xn <= half)
	{
	  fft_short(xp, yn, xn, lgN-1, r, l, p, n, tmp, pool);
	}
      else
	{
	  xn -= half;

	  // (X, Y) -> X + Y
	  for (long j = 0; j < xn; j++)
	    SS_AddMod(xp[j], xp[j], xp[j + half], p, n);

	  fft_short(xp, yn, half, lgN-1, r, l, p, n, tmp, pool);
	}
    }
  else
    {
      yn -= half;
      
      ZZ *xp0 = xp;
      ZZ *xp1 = xp + half;

      if (xn <= half)
	{
	  // X -> (X, w*X)
	  for (long j = 0; j < xn; j++)
	    Rotate(xp1[j], xp0[j], j, lgN, r, l, p, n, tmp);

          
          bool seq = SS_BelowThresh(half+yn, p.size());
          NTL_EXEC_DIVIDE(seq, pool, helper, double(half)/double(half+yn),
	     fft_short(xp0, half, xn, lgN-1,  r, l, p, n, tmp, helper.subpool(0)),
	     fft_short(xp1, yn, xn, lgN-1, r, l, p, n, 
                       (helper.concurrent() ? 0 : tmp), helper.subpool(1)))
	}
      else
	{
	  xn -= half;

	  // (X, Y) -> (X + Y, w*(X - Y))
	  for (long j = 0; j < xn; j++) 
            SS_fwd_butterfly(xp0[j], xp1[j], j, lgN, r, l, p, n, tmp);

	  // X -> (X, w*X)
	  for (long j = xn; j < half; j++)
            Rotate(xp1[j], xp0[j], j, lgN, r, l, p, n, tmp);

          bool seq = SS_BelowThresh(half+yn, p.size());
          NTL_EXEC_DIVIDE(seq, pool, helper, double(half)/double(half+yn),
	     fft_short(xp0, half, half, lgN-1, r, l, p, n, tmp, helper.subpool(0)),
	     fft_short(xp1, yn, half, lgN-1, r, l, p, n, 
                       (helper.concurrent() ? 0 : tmp), helper.subpool(1)))
	}
    }
}



static void 
fft(ZZVec& a, long r, long l, const ZZ& p, long n)
{
   ZZ tmp[SS_NTEMPS];
   fft_rec(&a[0], l, r, l, p, n, &tmp[0]);
}

static void 
fft1(ZZVec& a, long r, long l, long l1, const ZZ& p, long n)
{
   ZZ tmp[SS_NTEMPS];
   fft_rec(&a[0], l, r, l1, p, n, &tmp[0]);
}

static void 
fft_trunc(ZZVec& a, long yn, long xn, 
          long r, long l, long l1, const ZZ& p, long n)
{
   ZZ tmp[SS_NTEMPS];
   fft_short(&a[0], yn, xn, l, r, l1, p, n, &tmp[0], NTL_INIT_DIVIDE);
}

static void 
fft_trunc_pair(ZZVec& a_0, ZZVec& a_1, long yn, long xn_0, long xn_1, 
          long r, long l, long l1, const ZZ& p, long n, 
          RecursiveThreadPool* pool)
{
   ZZ tmp_0[SS_NTEMPS];
   ZZ tmp_1[SS_NTEMPS];
   bool seq = SS_BelowThresh(yn, p.size());
   NTL_EXEC_DIVIDE(seq, pool, helper, 0.5,
     fft_short(&a_0[0], yn, xn_0, l, r, l1, p, n, &tmp_0[0], helper.subpool(0)),
     fft_short(&a_1[0], yn, xn_1, l, r, l1, p, n, &tmp_1[0], helper.subpool(1)))
}

static void
ifft_layer(ZZ* xp, long blocks, long size, long level, long r, long l,
          const ZZ& p, long n, ZZ* tmp)
{
   size /= 2;
 
   do {
      ZZ *xp0 = xp;
      ZZ *xp1 = xp + size;

      SS_butterfly(xp0[0], xp1[0], p, n, tmp);
      for (long j = 1; j < size; j++)
         SS_inv_butterfly(xp0[j], xp1[j], j, level, r, l, p, n, tmp);

      xp += 2*size;
   } while (--blocks != 0);
}

static void 
ifft_base(ZZ* xp, long lgN, long r, long l, const ZZ& p, long n,
         ZZ* tmp)
{
  long N = 1L << lgN;

  for (long j = 1, size = 2, blocks = N/2; 
       j <= lgN; j++, blocks >>= 1, size <<= 1)
    ifft_layer(xp, blocks, size, j, r, l, p, n, tmp);
}


static void 
ifft_rec(ZZ* xp, long lgN, long r, long l, const ZZ& p, long n,
         ZZ* tmp)
{
   if (lgN <= SS_FFT_THRESH) {
      ifft_base(xp, lgN, r, l, p, n, tmp);
      return;
   }

   long N = 1L << lgN;
   long half = N >> 1;

   ZZ *xp0 = xp;
   ZZ *xp1 = xp + half;

   ifft_rec(xp0, lgN-1, r, l, p, n, tmp);
   ifft_rec(xp1, lgN-1, r, l, p, n, tmp);

   SS_butterfly(xp0[0], xp1[0], p, n, tmp);
   for (long j = 1; j < half; j++) 
      SS_inv_butterfly(xp0[j], xp1[j], j, lgN, r, l, p, n, tmp);
}

static void 
ifft_short2(ZZ* xp, long yn, long lgN, 
           long r, long l, const ZZ& p, long n, ZZ* tmp, RecursiveThreadPool* pool);

static void 
ifft_short0(ZZ* xp, long lgN, 
           long r, long l, const ZZ& p, long n, ZZ* tmp, RecursiveThreadPool* pool)

{
  Vec<ZZ> alt_tmp;
  if (!tmp) {
    alt_tmp.SetLength(SS_NTEMPS);
    tmp = &alt_tmp[0];
  }


  long N = 1L << lgN;

  if (lgN <= SS_FFT_THRESH)
    {
      // no truncation
      ifft_base(xp, lgN, r, l, p, n, tmp);
      return;
    }

  // divide-and-conquer algorithm

  long half = N >> 1;

  ZZ *xp0 = xp;
  ZZ *xp1 = xp + half;

  bool seq = SS_BelowThresh(N, p.size());
  NTL_EXEC_DIVIDE(seq, pool, helper, 0.5,
     ifft_short0(xp0, lgN-1, r, l, p, n, tmp, helper.subpool(0)),
     ifft_short0(xp1, lgN-1, r, l, p, n, 
                 (helper.concurrent() ? 0 : tmp), helper.subpool(1)))

  // (X, Y) -> (X + Y/w, X - Y/w)
  SS_butterfly(xp0[0], xp1[0], p, n, tmp);
  for (long j = 1; j < half; j++) 
    SS_inv_butterfly(xp0[j], xp1[j], j, lgN, r, l, p, n, tmp);
}

static void 
ifft_short1(ZZ* xp, long yn, long lgN, 
           long r, long l, const ZZ& p, long n, ZZ* tmp, RecursiveThreadPool* pool)

{
  long N = 1L << lgN;

  if (yn == N) {
    // no truncation
    ifft_short0(xp, lgN, r, l, p, n, tmp, pool);
    return;
  }

  // divide-and-conquer algorithm

  long half = N >> 1;

  if (yn <= half)
    {
      // X -> 2X
      for (long j = 0; j < yn; j++)
      	SS_AddMod(xp[j], xp[j], xp[j], p, n);

      ifft_short1(xp, yn, lgN-1, r, l, p, n, tmp, pool);
    }
  else
    {
      ZZ *xp0 = xp;
      ZZ *xp1 = xp + half;

      ifft_short0(xp0, lgN-1, r, l, p, n, tmp, pool);

      yn -= half;

      // X -> (2X, w*X)
      for (long j = yn; j < half; j++)
	{
	  tmp[0] = xp0[j];
          SS_AddMod(xp0[j], xp0[j], xp0[j], p, n);
          Rotate(xp1[j], tmp[0], j, lgN, r, l, p, n, tmp+1);
	}

      ifft_short2(xp1, yn, lgN-1, r, l, p, n, tmp, pool);

      // (X, Y) -> (X + Y/w, X - Y/w)
      SS_butterfly(xp0[0], xp1[0], p, n, tmp);
      for (long j = 1; j < yn; j++) 
        SS_inv_butterfly(xp0[j], xp1[j], j, lgN, r, l, p, n, tmp);
    }
}


static void 
ifft_short2(ZZ* xp, long yn, long lgN, 
           long r, long l, const ZZ& p, long n, ZZ* tmp, RecursiveThreadPool* pool)

{
  long N = 1L << lgN;

  if (yn == N) {
    // no truncation
    ifft_short0(xp, lgN, r, l, p, n, tmp, pool);
    return;
  }

  // divide-and-conquer algorithm

  long half = N >> 1;

  if (yn <= half)
    {
      // X -> 2X
      for (long j = 0; j < yn; j++)
      	SS_AddMod(xp[j], xp[j], xp[j], p, n);

      // (X, Y) -> X + Y
      for (long j = yn; j < half; j++)
	SS_AddMod(xp[j], xp[j], xp[j + half], p, n);

      ifft_short2(xp, yn, lgN-1, r, l, p, n, tmp, pool);

      // (X, Y) -> X - Y
      for (long j = 0; j < yn; j++)
	SS_SubMod(xp[j], xp[j], xp[j + half], p, n);
    }
  else
    {
      ZZ *xp0 = xp;
      ZZ *xp1 = xp + half;

      ifft_short0(xp0, lgN-1, r, l, p, n, tmp, pool);

      yn -= half;

      // (X, Y) -> (2X - Y, w*(X - Y))
      for (long j = yn; j < half; j++)
	{
          SS_SubMod(tmp[0], xp0[j], xp1[j], p, n);
          SS_AddMod(xp0[j], xp0[j], tmp[0], p, n);
          Rotate(xp1[j], tmp[0], j, lgN, r, l, p, n, tmp+1);
	}


      ifft_short2(xp1, yn, lgN-1, r, l, p, n, tmp, pool);

      // (X, Y) -> (X + Y/w, X - Y/w)
      SS_butterfly(xp0[0], xp1[0], p, n, tmp);
      for (long j = 1; j < yn; j++) 
        SS_inv_butterfly(xp0[j], xp1[j], j, lgN, r, l, p, n, tmp);
    }
}


static void 
ifft(ZZVec& a, long r, long l, const ZZ& p, long n)
{
   ZZ tmp[SS_NTEMPS];
   ifft_rec(&a[0], l, r, l, p, n, &tmp[0]);
}

static void 
ifft1(ZZVec& a, long r, long l, long l1, const ZZ& p, long n)
{
   ZZ tmp[SS_NTEMPS];
   ifft_rec(&a[0], l, r, l1, p, n, &tmp[0]);
}

static void 
ifft_trunc(ZZVec& a, long yn, long r, long l, long l1, const ZZ& p, long n)
{
   ZZ tmp[SS_NTEMPS];
   ifft_short1(&a[0], yn, l, r, l1, p, n, &tmp[0], NTL_INIT_DIVIDE);
}





/* Multiplication a la Schoenhage & Strassen, modulo a "Fermat" number
   p = 2^{mr}+1, where m is a power of two and r is odd. Then w = 2^r
   is a primitive 2mth root of unity, i.e., polynomials whose product
   has degree less than 2m can be multiplied, provided that the
   coefficients of the product polynomial are at most 2^{mr-1} in
   absolute value. The algorithm is not called recursively;
   coefficient arithmetic is done directly.*/

// The original version of SSMUl was written by Juergen Gerhard.
// However, it has been almost completely re-written so as
// to provide the following improvements:
//   * uses truncated FFT and Inverse FFT algorithms,
//     for better performance between powers of 2
//   * better cache locality because of divide and conquer structure
//   * better performance because of sqrt(2) trick

void SSMul(ZZX& c, const ZZX& a, const ZZX& b)
{
  if (&a == &b) {
    SSSqr(c, a);
    return;
  }

  long na = deg(a);
  long nb = deg(b);

  if (na <= 0 || nb <= 0) {
    PlainMul(c, a, b);
    return;
  }

  long n = na + nb; /* degree of the product */


  /* Choose m and r suitably */
  long l = NextPowerOfTwo(n + 1) - 1; /* 2^l <= n < 2^{l+1} */
  long N = 1L << (l + 1); /* N = 2^{l+1} */
  /* Bitlength of the product: if the coefficients of a are absolutely less
     than 2^ka and the coefficients of b are absolutely less than 2^kb, then
     the coefficients of ab are absolutely less than
     (min(na,nb)+1)2^{ka+kb} <= 2^bound. */
  long bound = 2 + NumBits(min(na, nb)) + MaxBits(a) + MaxBits(b);
  /* Let r be minimal so that mr > bound */
  long r = (bound >> l) + 1;
  long mr = r << l;

  // sqrt(2) trick
  long l1 = l;
  if (l1 >= 3) {
    long alt_l1 = l-1;
    long alt_r = (bound >> alt_l1) + 1;
    long alt_mr = alt_r << alt_l1;

    if (alt_mr < mr - mr/8) {
      l1 = alt_l1;
      r = alt_r;
      mr = alt_mr;
    }
  }

  /* p := 2^{mr}+1 */
  ZZ p;
  set(p);
  LeftShift(p, p, mr);
  add(p, p, 1);

  /* Make coefficients of a and b positive */
  ZZVec aa, bb;
  aa.SetSize(N, p.size());
  bb.SetSize(N, p.size());

  for (long i = 0; i <= deg(a); i++) {
    if (sign(a.rep[i]) >= 0) {
      aa[i] = a.rep[i];
    } else {
      add(aa[i], a.rep[i], p);
    }
  }

  for (long i = 0; i <= deg(b); i++) {
    if (sign(b.rep[i]) >= 0) {
      bb[i] = b.rep[i];
    } else {
      add(bb[i], b.rep[i], p);
    }
  }

  long yn = SS_FFTRoundUp(n+1, l+1);

  /* N-point FFT's mod p */
  fft_trunc_pair(aa, bb, yn, SS_FFTRoundUp(na+1, l+1), SS_FFTRoundUp(nb+1, l+1),
                 r, l+1, l1+1, p, mr, NTL_INIT_DIVIDE);
  //fft_trunc(aa, yn, SS_FFTRoundUp(na+1, l+1), r, l+1, l1+1, p, mr);
  //fft_trunc(bb, yn, SS_FFTRoundUp(nb+1, l+1), r, l+1, l1+1, p, mr);


  /* Pointwise multiplication aa := aa * bb mod p */
  bool seq = SS_BelowThresh(yn, p.size());
  NTL_GEXEC_RANGE(seq, yn, first, last)
  ZZ tmp, ai;
  for (long i = first; i < last; i++) {
    mul(ai, aa[i], bb[i]);
    if (NumBits(ai) > mr) {
      RightShift(tmp, ai, mr);
      trunc(ai, ai, mr);
      sub(ai, ai, tmp);
      if (sign(ai) < 0) {
        add(ai, ai, p);
      }
    }
    aa[i] = ai;
  }
  NTL_GEXEC_RANGE_END

  ifft_trunc(aa, yn, r, l+1, l1+1, p, mr);

  /* Retrieve c, dividing by N, and subtracting p where necessary */

  c.rep.SetLength(n + 1);
  ZZ ai, tmp, scratch;
  for (long i = 0; i <= n; i++) {
    ai = aa[i];
    ZZ& ci = c.rep[i];
    if (!IsZero(ai)) {
      /* ci = -ai * 2^{mr-l-1} = ai * 2^{-l-1} = ai / N mod p */
      LeftRotate(ai, ai, mr - l - 1, p, mr, scratch);
      sub(tmp, p, ai);
      if (NumBits(tmp) >= mr) { /* ci >= (p-1)/2 */
        negate(ci, ai); /* ci = -ai = ci - p */
      }
      else
        ci = tmp;
    } 
    else
       clear(ci);
  }
}

void SSMul(ZZ_pX& c, const ZZ_pX& a, const ZZ_pX& b)
{
  if (&a == &b) {
    SSSqr(c, a);
    return;
  }

  long na = deg(a);
  long nb = deg(b);

  if (na <= 0 || nb <= 0) {
    PlainMul(c, a, b);
    return;
  }

  long n = na + nb; /* degree of the product */


  /* Choose m and r suitably */
  long l = NextPowerOfTwo(n + 1) - 1; /* 2^l <= n < 2^{l+1} */
  long N = 1L << (l + 1); /* N = 2^{l+1} */
  /* Bitlength of the product: if the coefficients of a are absolutely less
     than 2^ka and the coefficients of b are absolutely less than 2^kb, then
     the coefficients of ab are absolutely less than
     (min(na,nb)+1)2^{ka+kb} <= 2^bound. */
  long bound = 2 + NumBits(min(na, nb)) + 2*NumBits(ZZ_p::modulus());
  /* Let r be minimal so that mr > bound */
  long r = (bound >> l) + 1;
  long mr = r << l;

  // sqrt(2) trick
  long l1 = l;
  if (l1 >= 3) {
    long alt_l1 = l-1;
    long alt_r = (bound >> alt_l1) + 1;
    long alt_mr = alt_r << alt_l1;

    if (alt_mr < mr - mr/8) {
      l1 = alt_l1;
      r = alt_r;
      mr = alt_mr;
    }
  }

  /* p := 2^{mr}+1 */
  ZZ p;
  set(p);
  LeftShift(p, p, mr);
  add(p, p, 1);

  ZZVec aa, bb;
  aa.SetSize(N, p.size());
  bb.SetSize(N, p.size());

  for (long i = 0; i <= deg(a); i++) {
      aa[i] = rep(a.rep[i]);
  }

  for (long i = 0; i <= deg(b); i++) {
    bb[i] = rep(b.rep[i]);
  }

  long yn = SS_FFTRoundUp(n+1, l+1);

  /* N-point FFT's mod p */
  fft_trunc_pair(aa, bb, yn, SS_FFTRoundUp(na+1, l+1), SS_FFTRoundUp(nb+1, l+1),
                 r, l+1, l1+1, p, mr, NTL_INIT_DIVIDE);
  //fft_trunc(aa, yn, SS_FFTRoundUp(na+1, l+1), r, l+1, l1+1, p, mr);
  //fft_trunc(bb, yn, SS_FFTRoundUp(nb+1, l+1), r, l+1, l1+1, p, mr);


  /* Pointwise multiplication aa := aa * bb mod p */
  bool seq = SS_BelowThresh(yn, p.size());
  NTL_GEXEC_RANGE(seq, yn, first, last)
  ZZ tmp, ai;
  for (long i = first; i < last; i++) {
    mul(ai, aa[i], bb[i]);
    if (NumBits(ai) > mr) {
      RightShift(tmp, ai, mr);
      trunc(ai, ai, mr);
      sub(ai, ai, tmp);
      if (sign(ai) < 0) {
        add(ai, ai, p);
      }
    }
    aa[i] = ai;
  }
  NTL_GEXEC_RANGE_END

  ifft_trunc(aa, yn, r, l+1, l1+1, p, mr);

  /* Retrieve c, dividing by N, and subtracting p where necessary */

  c.rep.SetLength(n+1);
  bool seq1 = SS_BelowThresh(n+1, p.size());
  ZZ_pContext context;
  context.save();
  NTL_GEXEC_RANGE(seq1, n+1, first, last)
  context.restore();
  ZZ ai, tmp, scratch;
  for (long i = first; i < last; i++) {
    ai = aa[i];
    ZZ_p& ci = c.rep[i];
    if (!IsZero(ai)) {
      /* ci = -ai * 2^{mr-l-1} = ai * 2^{-l-1} = ai / N mod p */
      LeftRotate(ai, ai, mr - l - 1, p, mr, scratch);
      sub(tmp, p, ai);
      conv(ci, tmp);
    } 
    else
       clear(ci);
  }
  NTL_GEXEC_RANGE_END

  c.normalize();
}



// SSRatio computes how much bigger the SS modulus must be
// to accomodate the necessary roots of unity.
// This is useful in determining algorithm crossover points.

double SSRatio(long na, long maxa, long nb, long maxb)
{
  if (na <= 0 || nb <= 0) return 0;

  long n = na + nb; /* degree of the product */


  long l = NextPowerOfTwo(n + 1) - 1; /* 2^l <= n < 2^{l+1} */
  long bound = 2 + NumBits(min(na, nb)) + maxa + maxb;
  long r = (bound >> l) + 1;
  long mr = r << l;

  // sqrt(2) trick
  long l1 = l;
  if (l1 >= 3) {
    long alt_l1 = l-1;
    long alt_r = (bound >> alt_l1) + 1;
    long alt_mr = alt_r << alt_l1;

    if (alt_mr < mr - mr/8) {
      l1 = alt_l1;
      r = alt_r;
      mr = alt_mr;
    }
  }

  return double(mr + 1)/double(bound);
}



static
void conv(vec_zz_p& x, const ZZVec& a)
{
   long i, n;

   n = a.length();
   x.SetLength(n);

   VectorConv(n, x.elts(), a.elts());
}



// Decide to use SSMul.  
static bool ChooseSS(long da, long maxbitsa, long db, long maxbitsb)
{
   long k = ((maxbitsa+maxbitsb+NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS)/2;
   double rat = SSRatio(da, maxbitsa, db, maxbitsb);

#if 1
   // I've made SSMul fully thread boosted, so I'm using
   // just one set of crossovers...FIXME: may need to tune this.
   return (k >= 13  && rat < 1.15) ||
	  (k >= 26  && rat < 1.30) ||
	  (k >= 53  && rat < 1.60) ||
	  (k >= 106 && rat < 1.80) ||
	  (k >= 212 && rat < 2.00);

#else
   // This old code was based on the fact that SSMul was not 
   // full thread boosted

   long nt = AvailableThreads();

   if (nt == 1) {

      return (k >= 13  && rat < 1.15) ||
             (k >= 26  && rat < 1.30) ||
             (k >= 53  && rat < 1.60) ||
             (k >= 106 && rat < 1.80) ||
             (k >= 212 && rat < 2.00);

   }
   else if (nt == 2) {

      return (k >= 53  && rat < 1.10) ||
             (k >= 106 && rat < 1.10) ||
             (k >= 212 && rat < 1.40);

   }
   else if (nt == 3) {

      return (k >= 106 && rat < 1.05) ||
             (k >= 212 && rat < 1.20);

   }
   else if (nt == 4) {

      return (k >= 106 && rat < 1.04) ||
             (k >= 212 && rat < 1.10);

   }
   else if (nt <= 8) {

      return (k >= 212 && rat < 1.01);

   }
   else {

      return false;

   }
#endif
 
}


void mul(ZZX& c, const ZZX& a, const ZZX& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(c);
      return;
   }

   if (&a == &b) {
      sqr(c, a);
      return;
   }

   long maxa = MaxSize(a);
   long maxb = MaxSize(b);

   long k = min(maxa, maxb);
   long s = min(deg(a), deg(b)) + 1;

   // FIXME: I should have a way of setting all these crossovers
   // automatically

   if (s == 1 || (k == 1 && s < 40) || (k == 2 && s < 20) || 
                 (k == 3 && s < 10)) {

      PlainMul(c, a, b);
      return;
   }

   if (s < 80 || (k < 30 && s < 150))  {
      KarMul(c, a, b);
      return;
   }



   if (ChooseSS(deg(a), MaxBits(a), deg(b), MaxBits(b))) {
      SSMul(c, a, b);
   }
   else {
      HomMul(c, a, b);
   }
}

void SSSqr(ZZX& c, const ZZX& a)

{
  long na = deg(a);

  if (na <= 0) {
    PlainSqr(c, a);
    return;
  }

  long n = na + na; /* degree of the product */


  /* Choose m and r suitably */
  long l = NextPowerOfTwo(n + 1) - 1; /* 2^l <= n < 2^{l+1} */
  long N = 1L << (l + 1); /* N = 2^{l+1} */
  long bound = 2 + NumBits(na) + 2*MaxBits(a);
  /* Let r be minimal so that mr > bound */
  long r = (bound >> l) + 1;
  long mr = r << l;

  // sqrt(2) trick
  long l1 = l;
  if (l1 >= 3) {
    long alt_l1 = l-1;
    long alt_r = (bound >> alt_l1) + 1;
    long alt_mr = alt_r << alt_l1;

    if (alt_mr < mr - mr/8) {
      l1 = alt_l1;
      r = alt_r;
      mr = alt_mr;
    }
  }

  /* p := 2^{mr}+1 */
  ZZ p;
  set(p);
  LeftShift(p, p, mr);
  add(p, p, 1);

  /* Make coefficients of a and b positive */
  ZZVec aa;
  aa.SetSize(N, p.size());

  for (long i = 0; i <= deg(a); i++) {
    if (sign(a.rep[i]) >= 0) {
      aa[i] = a.rep[i];
    } else {
      add(aa[i], a.rep[i], p);
    }
  }

  long yn = SS_FFTRoundUp(n+1, l+1);

  /* N-point FFT's mod p */
  fft_trunc(aa, yn, SS_FFTRoundUp(na+1, l+1), r, l+1, l1+1, p, mr);


  /* Pointwise multiplication aa := aa * bb mod p */
  bool seq = SS_BelowThresh(yn, p.size());
  NTL_GEXEC_RANGE(seq, yn, first, last)
  ZZ tmp, ai;
  for (long i = first; i < last; i++) {
    sqr(ai, aa[i]);
    if (NumBits(ai) > mr) {
      RightShift(tmp, ai, mr);
      trunc(ai, ai, mr);
      sub(ai, ai, tmp);
      if (sign(ai) < 0) {
        add(ai, ai, p);
      }
    }
    aa[i] = ai;
  }
  NTL_GEXEC_RANGE_END

  ifft_trunc(aa, yn, r, l+1, l1+1, p, mr);

  /* Retrieve c, dividing by N, and subtracting p where necessary */
  c.rep.SetLength(n + 1);
  ZZ ai, tmp, scratch;
  for (long i = 0; i <= n; i++) {
    ai = aa[i];
    ZZ& ci = c.rep[i];
    if (!IsZero(ai)) {
      /* ci = -ai * 2^{mr-l-1} = ai * 2^{-l-1} = ai / N mod p */
      LeftRotate(ai, ai, mr - l - 1, p, mr, scratch);
      sub(tmp, p, ai);
      if (NumBits(tmp) >= mr) { /* ci >= (p-1)/2 */
        negate(ci, ai); /* ci = -ai = ci - p */
      }
      else
        ci = tmp;
    } 
    else
       clear(ci);
  }
}

void SSSqr(ZZ_pX& c, const ZZ_pX& a)

{
  long na = deg(a);

  if (na <= 0) {
    PlainSqr(c, a);
    return;
  }

  long n = na + na; /* degree of the product */


  /* Choose m and r suitably */
  long l = NextPowerOfTwo(n + 1) - 1; /* 2^l <= n < 2^{l+1} */
  long N = 1L << (l + 1); /* N = 2^{l+1} */
  long bound = 2 + NumBits(na) + 2*NumBits(ZZ_p::modulus());
  /* Let r be minimal so that mr > bound */
  long r = (bound >> l) + 1;
  long mr = r << l;

  // sqrt(2) trick
  long l1 = l;
  if (l1 >= 3) {
    long alt_l1 = l-1;
    long alt_r = (bound >> alt_l1) + 1;
    long alt_mr = alt_r << alt_l1;

    if (alt_mr < mr - mr/8) {
      l1 = alt_l1;
      r = alt_r;
      mr = alt_mr;
    }
  }

  /* p := 2^{mr}+1 */
  ZZ p;
  set(p);
  LeftShift(p, p, mr);
  add(p, p, 1);

  ZZVec aa;
  aa.SetSize(N, p.size());

  for (long i = 0; i <= deg(a); i++) {
      aa[i] = rep(a.rep[i]);
  }

  long yn = SS_FFTRoundUp(n+1, l+1);

  /* N-point FFT's mod p */
  fft_trunc(aa, yn, SS_FFTRoundUp(na+1, l+1), r, l+1, l1+1, p, mr);


  /* Pointwise multiplication aa := aa * bb mod p */
  bool seq = SS_BelowThresh(yn, p.size());
  NTL_GEXEC_RANGE(seq, yn, first, last)
  ZZ tmp, ai;
  for (long i = first; i < last; i++) {
    sqr(ai, aa[i]);
    if (NumBits(ai) > mr) {
      RightShift(tmp, ai, mr);
      trunc(ai, ai, mr);
      sub(ai, ai, tmp);
      if (sign(ai) < 0) {
        add(ai, ai, p);
      }
    }
    aa[i] = ai;
  }
  NTL_GEXEC_RANGE_END

  ifft_trunc(aa, yn, r, l+1, l1+1, p, mr);

  /* Retrieve c, dividing by N, and subtracting p where necessary */
  c.rep.SetLength(n+1);
  bool seq1 = SS_BelowThresh(n+1, p.size());
  ZZ_pContext context;
  context.save();
  NTL_GEXEC_RANGE(seq1, n+1, first, last)
  context.restore();
  ZZ ai, tmp, scratch;
  for (long i = first; i < last; i++) {
    ai = aa[i];
    ZZ_p& ci = c.rep[i];
    if (!IsZero(ai)) {
      /* ci = -ai * 2^{mr-l-1} = ai * 2^{-l-1} = ai / N mod p */
      LeftRotate(ai, ai, mr - l - 1, p, mr, scratch);
      sub(tmp, p, ai);
      conv(ci, tmp);
    } 
    else
       clear(ci);
  }
  NTL_GEXEC_RANGE_END

  c.normalize();
}



void sqr(ZZX& c, const ZZX& a)
{
   if (IsZero(a)) {
      clear(c);
      return;
   }

   long maxa = MaxSize(a);

   long k = maxa;
   long s = deg(a) + 1;

   if (s == 1 || (k == 1 && s < 50) || (k == 2 && s < 25) || 
                 (k == 3 && s < 25) || (k == 4 && s < 10)) {

      PlainSqr(c, a);
      return;
   }

   if (s < 80 || (k < 30 && s < 150))  {
      KarSqr(c, a);
      return;
   }

   if (ChooseSS(deg(a), MaxBits(a), deg(a), MaxBits(a))) {
      SSSqr(c, a);
   }
   else {
      HomSqr(c, a);
   }
}


void mul(ZZX& x, const ZZX& a, const ZZ& b)
{
   ZZ t;
   long i, da;

   const ZZ *ap;
   ZZ* xp;

   if (IsZero(b)) {
      clear(x);
      return;
   }

   t = b;
   da = deg(a);
   x.rep.SetLength(da+1);
   ap = a.rep.elts();
   xp = x.rep.elts();

   for (i = 0; i <= da; i++) 
      mul(xp[i], ap[i], t);
}

void mul(ZZX& x, const ZZX& a, long b)
{
   long i, da;

   const ZZ *ap;
   ZZ* xp;

   if (b == 0) {
      clear(x);
      return;
   }

   da = deg(a);
   x.rep.SetLength(da+1);
   ap = a.rep.elts();
   xp = x.rep.elts();

   for (i = 0; i <= da; i++) 
      mul(xp[i], ap[i], b);
}




void diff(ZZX& x, const ZZX& a)
{
   long n = deg(a);
   long i;

   if (n <= 0) {
      clear(x);
      return;
   }

   if (&x != &a)
      x.rep.SetLength(n);

   for (i = 0; i <= n-1; i++) {
      mul(x.rep[i], a.rep[i+1], i+1);
   }

   if (&x == &a)
      x.rep.SetLength(n);

   x.normalize();
}

void HomPseudoDivRem(ZZX& q, ZZX& r, const ZZX& a, const ZZX& b)
{
   if (IsZero(b)) ArithmeticError("division by zero");

   long da = deg(a);
   long db = deg(b);

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   ZZ LC;
   LC = LeadCoeff(b);

   ZZ LC1;

   power(LC1, LC, da-db+1);

   long a_bound = NumBits(LC1) + MaxBits(a);

   LC1.kill();

   long b_bound = MaxBits(b);

   zz_pBak bak;
   bak.save();

   ZZX qq, rr;

   ZZ prod, t;
   set(prod);

   clear(qq);
   clear(rr);

   long i;
   long Qinstable, Rinstable;

   Qinstable = 1;
   Rinstable = 1;

   for (i = 0; ; i++) {
      zz_p::FFTInit(i);
      long p = zz_p::modulus();


      if (divide(LC, p)) continue;

      zz_pX A, B, Q, R;

      conv(A, a);
      conv(B, b);
      
      if (!IsOne(LC)) {
         zz_p y;
         conv(y, LC);
         power(y, y, da-db+1);
         mul(A, A, y);
      }

      if (!Qinstable) {
         conv(Q, qq);
         mul(R, B, Q);
         sub(R, A, R);

         if (deg(R) >= db)
            Qinstable = 1;
         else
            Rinstable = CRT(rr, prod, R);
      }

      if (Qinstable) {
         DivRem(Q, R, A, B);
         t = prod;
         Qinstable = CRT(qq, t, Q);
         Rinstable =  CRT(rr, prod, R);
      }

      if (!Qinstable && !Rinstable) {
         // stabilized...check if prod is big enough

         long bound1 = b_bound + MaxBits(qq) + NumBits(min(db, da-db)+1);
         long bound2 = MaxBits(rr);
         long bound = max(bound1, bound2);

         if (a_bound > bound)
            bound = a_bound;

         bound += 4;

         if (NumBits(prod) > bound)
            break;
      }
   }

   bak.restore();

   q = qq;
   r = rr;
}




void HomPseudoDiv(ZZX& q, const ZZX& a, const ZZX& b)
{
   ZZX r;
   HomPseudoDivRem(q, r, a, b);
}

void HomPseudoRem(ZZX& r, const ZZX& a, const ZZX& b)
{
   ZZX q;
   HomPseudoDivRem(q, r, a, b);
}

void PlainPseudoDivRem(ZZX& q, ZZX& r, const ZZX& a, const ZZX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ *bp;
   ZZ *qp;
   ZZ *xp;


   ZZ  s, t;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZX: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   ZZX lb;

   if (&q == &b) {
      lb = b;
      bp = lb.rep.elts();
   }
   else
      bp = b.rep.elts();

   ZZ LC = bp[db];
   LCIsOne = IsOne(LC);


   vec_ZZ x;

   x = a.rep;
   xp = x.elts();

   dq = da - db;
   q.rep.SetLength(dq+1);
   qp = q.rep.elts();

   if (!LCIsOne) {
      t = LC;
      for (i = dq-1; i >= 0; i--) {
         mul(xp[i], xp[i], t);
         if (i > 0) mul(t, t, LC);
      }
   }

   for (i = dq; i >= 0; i--) {
      t = xp[i+db];
      qp[i] = t;

      for (j = db-1; j >= 0; j--) {
         mul(s, t, bp[j]);
         if (!LCIsOne) mul(xp[i+j], xp[i+j], LC);
         sub(xp[i+j], xp[i+j], s);
      }
   }

   if (!LCIsOne) {
      t = LC;
      for (i = 1; i <= dq; i++) {
         mul(qp[i], qp[i], t);
         if (i < dq) mul(t, t, LC);
      }
   }
      

   r.rep.SetLength(db);
   for (i = 0; i < db; i++)
      r.rep[i] = xp[i];
   r.normalize();
}


void PlainPseudoDiv(ZZX& q, const ZZX& a, const ZZX& b)
{
   ZZX r;
   PlainPseudoDivRem(q, r, a, b);
}

void PlainPseudoRem(ZZX& r, const ZZX& a, const ZZX& b)
{
   ZZX q;
   PlainPseudoDivRem(q, r, a, b);
}

void div(ZZX& q, const ZZX& a, long b)
{
   if (b == 0) ArithmeticError("div: division by zero");

   if (!divide(q, a, b)) ArithmeticError("DivRem: quotient undefined over ZZ");
}

void div(ZZX& q, const ZZX& a, const ZZ& b)
{
   if (b == 0) ArithmeticError("div: division by zero");

   if (!divide(q, a, b)) ArithmeticError("DivRem: quotient undefined over ZZ");
}

static
void ConstDivRem(ZZX& q, ZZX& r, const ZZX& a, const ZZ& b)
{
   if (b == 0) ArithmeticError("DivRem: division by zero");

   if (!divide(q, a, b)) ArithmeticError("DivRem: quotient undefined over ZZ");

   r = 0;
}

static
void ConstRem(ZZX& r, const ZZX& a, const ZZ& b)
{
   if (b == 0) ArithmeticError("rem: division by zero");

   r = 0;
}



void DivRem(ZZX& q, ZZX& r, const ZZX& a, const ZZX& b)
{
   long da = deg(a);
   long db = deg(b);

   if (db < 0) ArithmeticError("DivRem: division by zero");

   if (da < db) {
      r = a;
      q = 0;
   }
   else if (db == 0) {
      ConstDivRem(q, r, a, ConstTerm(b));
   }
   else if (IsOne(LeadCoeff(b))) {
      PseudoDivRem(q, r, a, b);
   }
   else if (LeadCoeff(b) == -1) {
      ZZX b1;
      negate(b1, b);
      PseudoDivRem(q, r, a, b1);
      negate(q, q);
   }
   else if (divide(q, a, b)) {
      r = 0;
   }
   else {
      ZZX q1, r1;
      ZZ m;
      PseudoDivRem(q1, r1, a, b);
      power(m, LeadCoeff(b), da-db+1);
      if (!divide(q, q1, m)) ArithmeticError("DivRem: quotient not defined over ZZ");
      if (!divide(r, r1, m)) ArithmeticError("DivRem: remainder not defined over ZZ");
   }
}

void div(ZZX& q, const ZZX& a, const ZZX& b)
{
   long da = deg(a);
   long db = deg(b);

   if (db < 0) ArithmeticError("div: division by zero");

   if (da < db) {
      q = 0;
   }
   else if (db == 0) {
      div(q, a, ConstTerm(b));
   }
   else if (IsOne(LeadCoeff(b))) {
      PseudoDiv(q, a, b);
   }
   else if (LeadCoeff(b) == -1) {
      ZZX b1;
      negate(b1, b);
      PseudoDiv(q, a, b1);
      negate(q, q);
   }
   else if (divide(q, a, b)) {

      // nothing to do
      
   }
   else {
      ZZX q1;
      ZZ m;
      PseudoDiv(q1, a, b);
      power(m, LeadCoeff(b), da-db+1);
      if (!divide(q, q1, m)) ArithmeticError("div: quotient not defined over ZZ");
   }
}

void rem(ZZX& r, const ZZX& a, const ZZX& b)
{
   long da = deg(a);
   long db = deg(b);

   if (db < 0) ArithmeticError("rem: division by zero");

   if (da < db) {
      r = a;
   }
   else if (db == 0) {
      ConstRem(r, a, ConstTerm(b));
   }
   else if (IsOne(LeadCoeff(b))) {
      PseudoRem(r, a, b);
   }
   else if (LeadCoeff(b) == -1) {
      ZZX b1;
      negate(b1, b);
      PseudoRem(r, a, b1);
   }
   else if (divide(a, b)) {
      r = 0;
   }
   else {
      ZZX r1;
      ZZ m;
      PseudoRem(r1, a, b);
      power(m, LeadCoeff(b), da-db+1);
      if (!divide(r, r1, m)) ArithmeticError("rem: remainder not defined over ZZ");
   }
}

long HomDivide(ZZX& q, const ZZX& a, const ZZX& b)
{
   if (IsZero(b)) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   if (IsZero(a)) {
      clear(q);
      return 1;
   }

   if (deg(b) == 0) {
      return divide(q, a, ConstTerm(b));
   }

   if (deg(a) < deg(b)) return 0;

   ZZ ca, cb, cq;

   content(ca, a);
   content(cb, b);

   if (!divide(cq, ca, cb)) return 0;

   ZZX aa, bb;

   divide(aa, a, ca);
   divide(bb, b, cb);

   if (!divide(LeadCoeff(aa), LeadCoeff(bb)))
      return 0;

   if (!divide(ConstTerm(aa), ConstTerm(bb)))
      return 0;

   zz_pBak bak;
   bak.save();

   ZZX qq;

   ZZ prod;
   set(prod);

   clear(qq);
   long res = 1;
   long Qinstable = 1;


   long a_bound = MaxBits(aa);
   long b_bound = MaxBits(bb);


   long i;
   for (i = 0; ; i++) {
      zz_p::FFTInit(i);
      long p = zz_p::modulus();

      if (divide(LeadCoeff(bb), p)) continue;

      zz_pX A, B, Q, R;

      conv(A, aa);
      conv(B, bb);

      if (!Qinstable) {
         conv(Q, qq);
         mul(R, B, Q);
         sub(R, A, R);

         if (deg(R) >= deg(B))
            Qinstable = 1;
         else if (!IsZero(R)) {
            res = 0;
            break;
         }
         else
            mul(prod, prod, p);
      }

      if (Qinstable) {
         if (!divide(Q, A, B)) {
            res = 0;
            break;
         }

         Qinstable = CRT(qq, prod, Q);
      }

      if (!Qinstable) {
         // stabilized...check if prod is big enough

         long bound = b_bound + MaxBits(qq) + 
                     NumBits(min(deg(bb), deg(qq)) + 1);

         if (a_bound > bound)
            bound = a_bound;

         bound += 3;

         if (NumBits(prod) > bound) 
            break;
      }
   }

   bak.restore();

   if (res) mul(q, qq, cq);
   return res;

}


long HomDivide(const ZZX& a, const ZZX& b)
{
   if (deg(b) == 0) {
      return divide(a, ConstTerm(b));
   }
   else {
      ZZX q;
      return HomDivide(q, a, b);
   }
}

long PlainDivide(ZZX& qq, const ZZX& aa, const ZZX& bb)
{
   if (IsZero(bb)) {
      if (IsZero(aa)) {
         clear(qq);
         return 1;
      }
      else
         return 0;
   }

   if (deg(bb) == 0) {
      return divide(qq, aa, ConstTerm(bb));
   }

   long da, db, dq, i, j, LCIsOne;
   const ZZ *bp;
   ZZ *qp;
   ZZ *xp;


   ZZ  s, t;

   da = deg(aa);
   db = deg(bb);

   if (da < db) {
      return 0;
   }

   ZZ ca, cb, cq;

   content(ca, aa);
   content(cb, bb);

   if (!divide(cq, ca, cb)) {
      return 0;
   } 


   ZZX a, b, q;

   divide(a, aa, ca);
   divide(b, bb, cb);

   if (!divide(LeadCoeff(a), LeadCoeff(b)))
      return 0;

   if (!divide(ConstTerm(a), ConstTerm(b)))
      return 0;

   long coeff_bnd = MaxBits(a) + (NumBits(da+1)+1)/2 + (da-db);

   bp = b.rep.elts();

   ZZ LC;
   LC = bp[db];

   LCIsOne = IsOne(LC);

   xp = a.rep.elts();

   dq = da - db;
   q.rep.SetLength(dq+1);
   qp = q.rep.elts();

   for (i = dq; i >= 0; i--) {
      if (!LCIsOne) {
         if (!divide(t, xp[i+db], LC))
            return 0;
      }
      else
         t = xp[i+db];

      if (NumBits(t) > coeff_bnd) return 0;

      qp[i] = t;

      for (j = db-1; j >= 0; j--) {
         mul(s, t, bp[j]);
         sub(xp[i+j], xp[i+j], s);
      }
   }

   for (i = 0; i < db; i++)
      if (!IsZero(xp[i]))
         return 0;

   mul(qq, q, cq);
   return 1;
}

long PlainDivide(const ZZX& a, const ZZX& b)
{
   if (deg(b) == 0) 
      return divide(a, ConstTerm(b));
   else {
      ZZX q;
      return PlainDivide(q, a, b);
   }
}


long divide(ZZX& q, const ZZX& a, const ZZX& b)
{
   long da = deg(a);
   long db = deg(b);

   if (db <= 8 || da-db <= 8)
      return PlainDivide(q, a, b);
   else
      return HomDivide(q, a, b);
}

long divide(const ZZX& a, const ZZX& b)
{
   long da = deg(a);
   long db = deg(b);

   if (db <= 8 || da-db <= 8)
      return PlainDivide(a, b);
   else
      return HomDivide(a, b);
}







long divide(ZZX& q, const ZZX& a, const ZZ& b)
{
   if (IsZero(b)) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   if (IsOne(b)) {
      q = a;
      return 1;
   }

   if (b == -1) {
      negate(q, a);
      return 1;
   }

   long n = a.rep.length();
   vec_ZZ res(INIT_SIZE, n);
   long i;

   for (i = 0; i < n; i++) {
      if (!divide(res[i], a.rep[i], b))
         return 0;
   }

   q.rep = res;
   return 1;
}

long divide(const ZZX& a, const ZZ& b)
{
   if (IsZero(b)) return IsZero(a);

   if (IsOne(b) || b == -1) {
      return 1;
   }

   long n = a.rep.length();
   long i;

   for (i = 0; i < n; i++) {
      if (!divide(a.rep[i], b))
         return 0;
   }

   return 1;
}

long divide(ZZX& q, const ZZX& a, long b)
{
   if (b == 0) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   if (b == 1) {
      q = a;
      return 1;
   }

   if (b == -1) {
      negate(q, a);
      return 1;
   }

   long n = a.rep.length();
   vec_ZZ res(INIT_SIZE, n);
   long i;

   for (i = 0; i < n; i++) {
      if (!divide(res[i], a.rep[i], b))
         return 0;
   }

   q.rep = res;
   return 1;
}

long divide(const ZZX& a, long b)
{
   if (b == 0) return IsZero(a);
   if (b == 1 || b == -1) {
      return 1;
   }

   long n = a.rep.length();
   long i;

   for (i = 0; i < n; i++) {
      if (!divide(a.rep[i], b))
         return 0;
   }

   return 1;
}

   

void content(ZZ& d, const ZZX& f)
{
   ZZ res;
   long i;

   clear(res);
   for (i = 0; i <= deg(f); i++) {
      GCD(res, res, f.rep[i]);
      if (IsOne(res)) break;
   }

   if (sign(LeadCoeff(f)) < 0) negate(res, res);
   d = res;
}

void PrimitivePart(ZZX& pp, const ZZX& f)
{
   if (IsZero(f)) {
      clear(pp);
      return;
   }
 
   ZZ d;

   content(d, f);
   divide(pp, f, d);
}


static
void BalCopy(ZZX& g, const zz_pX& G)
{
   long p = zz_p::modulus();
   long p2 = p >> 1;
   long n = G.rep.length();
   long i;
   long t;

   g.rep.SetLength(n);
   for (i = 0; i < n; i++) {
      t = rep(G.rep[i]);
      if (t > p2) t = t - p;
      conv(g.rep[i], t);
   }
}


   
void GCD(ZZX& d, const ZZX& a, const ZZX& b)
{
   if (IsZero(a)) {
      d = b;
      if (sign(LeadCoeff(d)) < 0) negate(d, d);
      return;
   }

   if (IsZero(b)) {
      d = a;
      if (sign(LeadCoeff(d)) < 0) negate(d, d);
      return;
   }

   ZZ c1, c2, c;
   ZZX f1, f2;

   content(c1, a);
   divide(f1, a, c1);

   content(c2, b);
   divide(f2, b, c2);

   GCD(c, c1, c2);

   ZZ ld;
   GCD(ld, LeadCoeff(f1), LeadCoeff(f2));

   ZZX g, h, res;

   ZZ prod;
   set(prod);

   zz_pBak bak;
   bak.save();


   long FirstTime = 1;

   long i;
   for (i = 0; ;i++) {
      zz_p::FFTInit(i);
      long p = zz_p::modulus();

      if (divide(LeadCoeff(f1), p) || divide(LeadCoeff(f2), p)) continue;

      zz_pX G, F1, F2;
      zz_p  LD;

      conv(F1, f1);
      conv(F2, f2);
      conv(LD, ld);

      GCD(G, F1, F2);
      mul(G, G, LD);


      if (deg(G) == 0) { 
         set(res);
         break;
      }

      if (FirstTime || deg(G) < deg(g)) {
         FirstTime = 0;
         conv(prod, p);
         BalCopy(g, G);
      }
      else if (deg(G) > deg(g)) 
         continue;
      else if (!CRT(g, prod, G)) {
         PrimitivePart(res, g);
         if (divide(f1, res) && divide(f2, res))
            break;
      }

   }

   bak.restore();

   mul(d, res, c);
   if (sign(LeadCoeff(d)) < 0) negate(d, d);
}

void trunc(ZZX& x, const ZZX& a, long m)

// x = a % X^m, output may alias input

{
   if (m < 0) LogicError("trunc: bad args");

   if (&x == &a) {
      if (x.rep.length() > m) {
         x.rep.SetLength(m);
         x.normalize();
      }
   }
   else {
      long n;
      long i;
      ZZ* xp;
      const ZZ* ap;

      n = min(a.rep.length(), m);
      x.rep.SetLength(n);

      xp = x.rep.elts();
      ap = a.rep.elts();

      for (i = 0; i < n; i++) xp[i] = ap[i];

      x.normalize();
   }
}



void LeftShift(ZZX& x, const ZZX& a, long n)
{
   if (IsZero(a)) {
      clear(x);
      return;
   }

   if (n < 0) {
      if (n < -NTL_MAX_LONG)  
         clear(x);
      else
         RightShift(x, a, -n);
      return;
   }

   if (NTL_OVERFLOW(n, 1, 0))
      ResourceError("overflow in LeftShift");

   long m = a.rep.length();

   x.rep.SetLength(m+n);

   long i;
   for (i = m-1; i >= 0; i--)
      x.rep[i+n] = a.rep[i];

   for (i = 0; i < n; i++)
      clear(x.rep[i]);
}


void RightShift(ZZX& x, const ZZX& a, long n)
{
   if (IsZero(a)) {
      clear(x);
      return;
   }

   if (n < 0) {
      if (n < -NTL_MAX_LONG) ResourceError("overflow in RightShift");
      LeftShift(x, a, -n);
      return;
   }

   long da = deg(a);
   long i;

   if (da < n) {
      clear(x);
      return;
   }

   if (&x != &a)
      x.rep.SetLength(da-n+1);

   for (i = 0; i <= da-n; i++)
      x.rep[i] = a.rep[i+n];

   if (&x == &a)
      x.rep.SetLength(da-n+1);

   x.normalize();
}


void TraceVec(vec_ZZ& S, const ZZX& ff)
{
   if (!IsOne(LeadCoeff(ff)))
      LogicError("TraceVec: bad args");

   ZZX f;
   f = ff;

   long n = deg(f);

   S.SetLength(n);

   if (n == 0)
      return;

   long k, i;
   ZZ acc, t;

   S[0] = n;

   for (k = 1; k < n; k++) {
      mul(acc, f.rep[n-k], k);

      for (i = 1; i < k; i++) {
         mul(t, f.rep[n-i], S[k-i]);
         add(acc, acc, t);
      }

      negate(S[k], acc);
   }

}

static
void EuclLength(ZZ& l, const ZZX& a)
{
   long n = a.rep.length();
   long i;
 
   ZZ sum, t;

   clear(sum);
   for (i = 0; i < n; i++) {
      sqr(t, a.rep[i]);
      add(sum, sum, t);
   }

   if (sum > 1) {
      SqrRoot(l, sum);
      add(l, l, 1);
   }
   else
      l = sum;
}



static
long ResBound(const ZZX& a, const ZZX& b)
{
   if (IsZero(a) || IsZero(b)) 
      return 0;

   ZZ t1, t2, t;
   EuclLength(t1, a);
   EuclLength(t2, b);
   power(t1, t1, deg(b));
   power(t2, t2, deg(a));
   mul(t, t1, t2);
   return NumBits(t);
}



void resultant(ZZ& rres, const ZZX& a, const ZZX& b, long deterministic)
{
   if (IsZero(a) || IsZero(b)) {
      clear(rres);
      return;
   }

   zz_pBak zbak;
   zbak.save();

   ZZ_pBak Zbak;
   Zbak.save();

   long instable = 1;

   long bound = 2+ResBound(a, b);

   long gp_cnt = 0;

   ZZ res, prod;

   clear(res);
   set(prod);


   long i;
   for (i = 0; ; i++) {
      if (NumBits(prod) > bound)
         break;

      if (!deterministic &&
          !instable && bound > 1000 && NumBits(prod) < 0.25*bound) {

         ZZ P;


         long plen = 90 + NumBits(max(bound, NumBits(res)));

         do {
            GenPrime(P, plen, 90 + 2*NumBits(gp_cnt++));
         }
         while (divide(LeadCoeff(a), P) || divide(LeadCoeff(b), P));

         ZZ_p::init(P);

         ZZ_pX A, B;
         conv(A, a);
         conv(B, b);

         ZZ_p t;
         resultant(t, A, B);

         if (CRT(res, prod, rep(t), P))
            instable = 1;
         else
            break;
      }


      zz_p::FFTInit(i);
      long p = zz_p::modulus();
      if (divide(LeadCoeff(a), p) || divide(LeadCoeff(b), p))
         continue;

      zz_pX A, B;
      conv(A, a);
      conv(B, b);

      zz_p t;
      resultant(t, A, B);

      instable = CRT(res, prod, rep(t), p);
   }

   rres = res;

   zbak.restore();
   Zbak.restore();
}




void MinPolyMod(ZZX& gg, const ZZX& a, const ZZX& f)

{
   if (!IsOne(LeadCoeff(f)) || deg(f) < 1 || deg(a) >= deg(f))
      LogicError("MinPolyMod: bad args");

   if (IsZero(a)) {
      SetX(gg);
      return;
   }

   ZZ_pBak Zbak;
   Zbak.save();
   zz_pBak zbak;
   zbak.save();

   long n = deg(f);

   long instable = 1;

   long gp_cnt = 0;

   ZZ prod;
   ZZX g;

   clear(g);
   set(prod);

   long bound = -1;

   long i;
   for (i = 0; ; i++) {
      if (deg(g) == n) {
         if (bound < 0)
            bound = 2+CharPolyBound(a, f);

         if (NumBits(prod) > bound)
            break;
      }

      if (!instable && 
         (deg(g) < n || 
         (deg(g) == n && bound > 1000 && NumBits(prod) < 0.75*bound))) {

         // guarantees 2^{-80} error probability
         long plen = 90 + max( 2*NumBits(n) + NumBits(MaxBits(f)),
                         max( NumBits(n) + NumBits(MaxBits(a)),
                              NumBits(MaxBits(g)) ));

         ZZ P;
         GenPrime(P, plen, 90 + 2*NumBits(gp_cnt++));
         ZZ_p::init(P);


         ZZ_pX A, F, G;
         conv(A, a);
         conv(F, f);
         conv(G, g);

         ZZ_pXModulus FF;
         build(FF, F);

         ZZ_pX H;
         CompMod(H, G, A, FF);
         
         if (IsZero(H))
            break;

         instable = 1;
      } 
         
      zz_p::FFTInit(i);

      zz_pX A, F;
      conv(A, a);
      conv(F, f);

      zz_pXModulus FF;
      build(FF, F);

      zz_pX G;
      MinPolyMod(G, A, FF);

      if (deg(G) < deg(g))
         continue;

      if (deg(G) > deg(g)) {
         clear(g);
         set(prod);
      }

      instable = CRT(g, prod, G);
   }

   gg = g;

   Zbak.restore();
   zbak.restore();
}


void XGCD(ZZ& rr, ZZX& ss, ZZX& tt, const ZZX& a, const ZZX& b, 
          long deterministic)
{
   ZZ r;

   resultant(r, a, b, deterministic);

   if (IsZero(r)) {
      clear(rr);
      return;
   }

   zz_pBak bak;
   bak.save();

   long i;
   long instable = 1;

   ZZ tmp;
   ZZ prod;
   ZZX s, t;

   set(prod);
   clear(s);
   clear(t);

   for (i = 0; ; i++) {
      zz_p::FFTInit(i);
      long p = zz_p::modulus();

      if (divide(LeadCoeff(a), p) || divide(LeadCoeff(b), p) || divide(r, p))
         continue;

      zz_p R;
      conv(R, r);

      zz_pX D, S, T, A, B;
      conv(A, a);
      conv(B, b);

      if (!instable) {
         conv(S, s);
         conv(T, t);
         zz_pX t1, t2;
         mul(t1, A, S); 
         mul(t2, B, T);
         add(t1, t1, t2);

         if (deg(t1) == 0 && ConstTerm(t1) == R)
            mul(prod, prod, p);
         else
            instable = 1;
      }

      if (instable) {
         XGCD(D, S, T, A, B);
   
         mul(S, S, R);
         mul(T, T, R);
   
         tmp = prod;
         long Sinstable = CRT(s, tmp, S);
         long Tinstable = CRT(t, prod, T);
   
         instable = Sinstable || Tinstable;
      }

      if (!instable) {
         long bound1 = NumBits(min(deg(a), deg(s)) + 1) 
                      + MaxBits(a) + MaxBits(s);
         long bound2 = NumBits(min(deg(b), deg(t)) + 1) 
                      + MaxBits(b) + MaxBits(t);

         long bound = 4 + max(NumBits(r), max(bound1, bound2));

         if (NumBits(prod) > bound)
            break;
      }
   }

   rr = r;
   ss = s;
   tt = t;

   bak.restore();
}

void NormMod(ZZ& x, const ZZX& a, const ZZX& f, long deterministic)
{
   if (!IsOne(LeadCoeff(f)) || deg(a) >= deg(f) || deg(f) <= 0)
      LogicError("norm: bad args");

   if (IsZero(a)) {
      clear(x);
      return;
   }

   resultant(x, f, a, deterministic);
}

void TraceMod(ZZ& res, const ZZX& a, const ZZX& f)
{
   if (!IsOne(LeadCoeff(f)) || deg(a) >= deg(f) || deg(f) <= 0)
      LogicError("trace: bad args");

   vec_ZZ S;

   TraceVec(S, f);

   InnerProduct(res, S, a.rep);
}


void discriminant(ZZ& d, const ZZX& a, long deterministic)
{
   long m = deg(a);

   if (m < 0) {
      clear(d);
      return;
   }

   ZZX a1;
   ZZ res;

   diff(a1, a);
   resultant(res, a, a1, deterministic);
   if (!divide(res, res, LeadCoeff(a)))
      LogicError("discriminant: inexact division");

   m = m & 3;
   if (m >= 2)
      negate(res, res);

   d = res;
}


void MulMod(ZZX& x, const ZZX& a, const ZZX& b, const ZZX& f)
{
   if (deg(a) >= deg(f) || deg(b) >= deg(f) || deg(f) == 0 || 
       !IsOne(LeadCoeff(f)))
      LogicError("MulMod: bad args");

   ZZX t;
   mul(t, a, b);
   rem(x, t, f);
}

void SqrMod(ZZX& x, const ZZX& a, const ZZX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0 || !IsOne(LeadCoeff(f)))
      LogicError("MulMod: bad args");

   ZZX t;
   sqr(t, a);
   rem(x, t, f);
}



static
void MulByXModAux(ZZX& h, const ZZX& a, const ZZX& f)
{
   long i, n, m;
   ZZ* hh;
   const ZZ *aa, *ff;

   ZZ t, z;


   n = deg(f);
   m = deg(a);

   if (m >= n || n == 0 || !IsOne(LeadCoeff(f)))
      LogicError("MulByXMod: bad args");

   if (m < 0) {
      clear(h);
      return;
   }

   if (m < n-1) {
      h.rep.SetLength(m+2);
      hh = h.rep.elts();
      aa = a.rep.elts();
      for (i = m+1; i >= 1; i--)
         hh[i] = aa[i-1];
      clear(hh[0]);
   }
   else {
      h.rep.SetLength(n);
      hh = h.rep.elts();
      aa = a.rep.elts();
      ff = f.rep.elts();
      negate(z, aa[n-1]);
      for (i = n-1; i >= 1; i--) {
         mul(t, z, ff[i]);
         add(hh[i], aa[i-1], t);
      }
      mul(hh[0], z, ff[0]);
      h.normalize();
   }
}

void MulByXMod(ZZX& h, const ZZX& a, const ZZX& f)
{
   if (&h == &f) {
      ZZX hh;
      MulByXModAux(hh, a, f);
      h = hh;
   }
   else
      MulByXModAux(h, a, f);
}

static
void EuclLength1(ZZ& l, const ZZX& a)
{
   long n = a.rep.length();
   long i;
 
   ZZ sum, t;

   clear(sum);
   for (i = 0; i < n; i++) {
      sqr(t, a.rep[i]);
      add(sum, sum, t);
   }

   abs(t, ConstTerm(a));
   mul(t, t, 2);
   add(t, t, 1);
   add(sum, sum, t);

   if (sum > 1) {
      SqrRoot(l, sum);
      add(l, l, 1);
   }
   else
      l = sum;
}


long CharPolyBound(const ZZX& a, const ZZX& f)
// This computes a bound on the size of the
// coefficients of the characterstic polynomial.
// It uses the characterization of the char poly as
// resultant_y(f(y), x-a(y)), and then interpolates this
// through complex primimitive (deg(f)+1)-roots of unity.

{
   if (IsZero(a) || IsZero(f))
      LogicError("CharPolyBound: bad args");

   ZZ t1, t2, t;
   EuclLength1(t1, a);
   EuclLength(t2, f);
   power(t1, t1, deg(f));
   power(t2, t2, deg(a));
   mul(t, t1, t2);
   return NumBits(t);
}


void SetCoeff(ZZX& x, long i, long a)
{
   if (a == 1) 
      SetCoeff(x, i);
   else {
      NTL_ZZRegister(aa);
      conv(aa, a);
      SetCoeff(x, i, aa);
   }
}


void CopyReverse(ZZX& x, const ZZX& a, long hi)

   // x[0..hi] = reverse(a[0..hi]), with zero fill
   // input may not alias output

{
   long i, j, n, m;

   n = hi+1;
   m = a.rep.length();

   x.rep.SetLength(n);

   const ZZ* ap = a.rep.elts();
   ZZ* xp = x.rep.elts();

   for (i = 0; i < n; i++) {
      j = hi-i;
      if (j < 0 || j >= m)
         clear(xp[i]);
      else
         xp[i] = ap[j];
   }

   x.normalize();
}

void reverse(ZZX& x, const ZZX& a, long hi)
{
   if (hi < 0) { clear(x); return; }
   if (NTL_OVERFLOW(hi, 1, 0))
      ResourceError("overflow in reverse");

   if (&x == &a) {
      ZZX tmp;
      CopyReverse(tmp, a, hi);
      x = tmp;
   }
   else
      CopyReverse(x, a, hi);
}

void MulTrunc(ZZX& x, const ZZX& a, const ZZX& b, long n)
{
   ZZX t;
   mul(t, a, b);
   trunc(x, t, n);
}

void SqrTrunc(ZZX& x, const ZZX& a, long n)
{
   ZZX t;
   sqr(t, a);
   trunc(x, t, n);
}


void NewtonInvTrunc(ZZX& c, const ZZX& a, long e)
{
   ZZ x;

   if (ConstTerm(a) == 1)
      x = 1;
   else if (ConstTerm(a) == -1)
      x = -1;
   else
      ArithmeticError("InvTrunc: non-invertible constant term");

   if (e == 1) {
      conv(c, x);
      return;
   }

   vec_long E;
   E.SetLength(0);
   append(E, e);
   while (e > 1) {
      e = (e+1)/2;
      append(E, e);
   }

   long L = E.length();

   ZZX g, g0, g1, g2;


   g.rep.SetMaxLength(E[0]);
   g0.rep.SetMaxLength(E[0]);
   g1.rep.SetMaxLength((3*E[0]+1)/2);
   g2.rep.SetMaxLength(E[0]);

   conv(g, x);

   long i;

   for (i = L-1; i > 0; i--) {
      // lift from E[i] to E[i-1]

      long k = E[i];
      long l = E[i-1]-E[i];

      trunc(g0, a, k+l);

      mul(g1, g0, g);
      RightShift(g1, g1, k);
      trunc(g1, g1, l);

      mul(g2, g1, g);
      trunc(g2, g2, l);
      LeftShift(g2, g2, k);

      sub(g, g, g2);
   }

   c = g;
}


void InvTrunc(ZZX& c, const ZZX& a, long e)
{
   if (e < 0) LogicError("InvTrunc: bad args");

   if (e == 0) {
      clear(c);
      return;
   }

   if (NTL_OVERFLOW(e, 1, 0))
      ResourceError("overflow in InvTrunc");

   NewtonInvTrunc(c, a, e);
}

NTL_END_IMPL

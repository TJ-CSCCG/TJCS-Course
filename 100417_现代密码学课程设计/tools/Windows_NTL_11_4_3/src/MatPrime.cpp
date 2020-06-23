
#include <NTL/MatPrime.h>


NTL_START_IMPL


MatPrimeTablesType MatPrimeTables;
// a truly GLOBAL variable, shared among all threads



// For now, we use the same logic as for IsFFTPrime, 
// which is good enough

static
long IsMatPrime(long n)
{
   long  m, x, y, z;
   long j, k;


   if (n <= 1 || n >= NTL_SP_BOUND) return 0;

   if (n % 2 == 0) return 0;

   if (n % 3 == 0) return 0;

   if (n % 5 == 0) return 0;

   if (n % 7 == 0) return 0;
   
   m = n - 1;
   k = 0;
   while ((m & 1) == 0) {
      m = m >> 1;
      k++;
   }

   for (;;) {
      x = RandomBnd(n);

      if (x == 0) continue;
      z = PowerMod(x, m, n);
      if (z == 1) continue;

      x = z;
      j = 0;
      do {
         y = z;
         z = MulMod(y, y, n);
         j++;
      } while (j != k && z != 1);

      if (z != 1 || y !=  n-1) return 0;

      if (j == k) 
         break;
   }

   /* x^{2^k} = 1 mod n, x^{2^{k-1}} = -1 mod n */

   long TrialBound;

   TrialBound = m >> k;
   if (TrialBound > 0) {
      if (!ProbPrime(n, 5)) return 0;
   
      /* we have to do trial division by special numbers */
   
      TrialBound = SqrRoot(TrialBound);
   
      long a, b;
   
      for (a = 1; a <= TrialBound; a++) {
         b = (a << k) + 1;
         if (n % b == 0) return 0; 
      }
   }

   return 1;
}


static
void NextMatPrime(long& q, long index)
{
   static long m = NTL_MatPrime_NBITS-1;
   static long k = 0;
   // m and k are truly GLOBAL variables, shared among
   // all threads.  Access is protected by a critical section
   // guarding MatPrimeTables

   static long last_index = -1;
   static long last_m = 0;
   static long last_k = 0;

   if (index == last_index) {
      // roll back m and k...part of a simple error recovery
      // strategy if an exception was thrown in the last 
      // invocation of UseMatPrime...probably of academic 
      // interest only

      m = last_m;
      k = last_k;
   }
   else {
      last_index = index;
      last_m = m;
      last_k = k;
   }

   long cand;

   for (;;) {
      if (k == 0) {
         m--;
         if (m < 3) ResourceError("ran out of matrix primes");
         k = 1L << (NTL_MatPrime_NBITS-m-2);
      }

      k--;

      cand = (1L << (NTL_MatPrime_NBITS-1)) + (k << (m+1)) + (1L << m) + 1;

      if (!IsMatPrime(cand)) continue;
      q = cand;
      return;
   }
}



void InitMatPrimeInfo(MatPrimeInfo& info, long q)
{
   info.q = q;
   info.context = zz_pContext(q);
}


void UseMatPrime(long index)
{
   if (index < 0) LogicError("invalid matrix prime index");
   if (index >= NTL_MAX_MATPRIMES) ResourceError("matrix prime index too large");

   if (index+1 >= NTL_NSP_BOUND) ResourceError("matrix prime index too large");
   // largely acacedemic, but it is a convenient assumption

   do {  // NOTE: thread safe lazy init
      MatPrimeTablesType::Builder bld(MatPrimeTables, index+1);
      long amt = bld.amt();
      if (!amt) break;

      long first = index+1-amt;
      // initialize entries first..index

      long i;
      for (i = first; i <= index; i++) {
         UniquePtr<MatPrimeInfo> info;
         info.make();

         long q, w;
         NextMatPrime(q, i);

         InitMatPrimeInfo(*info, q);
         bld.move(info);
      }

   } while (0);
}


#ifndef NTL_MatPrime_HALF_SIZE_STRATEGY


void build(MatPrime_crt_helper& H, const ZZ& P)
{
   ZZ B, M, M1, M2, M3;
   long n, i;
   long q, t;
   mulmod_t qinv;

   sqr(B, P);
   mul(B, B, NTL_MatPrimeLimit);
   LeftShift(B, B, NTL_MatPrimeFudge);

   set(M);
   n = 0;
   while (M <= B) {
      UseMatPrime(n);
      q = GetMatPrime(n);
      n++;
      mul(M, M, q);
   }


   double fn = double(n);

   if (8.0*fn*(fn+48) > NTL_FDOUBLE_PRECISION)
      ResourceError("modulus too big");

   H.NumPrimes = n;
   H.sz = P.size();
   H.prime.SetLength(n);
   H.prime_recip.SetLength(n);
   H.u.SetLength(n);
   H.uqinv.SetLength(n);
   H.ZZ_red_struct.SetLength(n);

   H.coeff.SetSize(n, P.size());

   H.montgomery_struct.init(P, ZZ(n) << NTL_MatPrime_NBITS);

   ZZ qq, rr;

   DivRem(qq, rr, M, P);

   NegateMod(H.MinusMModP, rr, P);

   H.montgomery_struct.adjust(H.MinusMModP);

   for (i = 0; i < n; i++) {
      q = GetMatPrime(i);
      qinv = MatPrimeTables[i]->context.ModulusInverse();

      long tt = rem(qq, q);

      mul(M2, P, tt);
      add(M2, M2, rr); 
      div(M2, M2, q);  // = (M/q) rem p
      

      div(M1, M, q);
      t = rem(M1, q);
      t = InvMod(t, q);

      // montgomery
      H.montgomery_struct.adjust(M2);


      H.prime[i] = q;
      H.prime_recip[i] = 1/double(q);
      H.u[i] = t;
      H.uqinv[i] = PrepMulModPrecon(H.u[i], q, qinv);
      H.ZZ_red_struct[i] = &MatPrimeTables[i]->context.ZZ_red_struct();
      H.coeff[i] = M2;
   }

   H.cost = double(H.sz)*double(n);

}


void reduce(const MatPrime_crt_helper& H, const ZZ& value, MatPrime_residue_t *remainders,
            MatPrime_crt_helper_scratch& scratch)
{
   long n = H.NumPrimes;
   const sp_ZZ_reduce_struct *const *red_struct = H.ZZ_red_struct.elts();

   for (long i = 0; i < n; i++)
      remainders[i] = red_struct[i]->rem(value);
}



void reconstruct(const MatPrime_crt_helper& H, ZZ& value, const MatPrime_residue_t *remainders,
                 MatPrime_crt_helper_scratch& scratch)
{
   ZZ& t = scratch.t;

   long nprimes = H.NumPrimes;
   const long *u = H.u.elts();
   const long *prime = H.prime.elts();
   const mulmod_precon_t  *uqinv = H.uqinv.elts();
   const double *prime_recip = H.prime_recip.elts();

   double y = 0.0;

   QuickAccumBegin(t, H.sz);
   for (long i = 0; i < nprimes; i++) {
      long r = MulModPrecon(remainders[i], u[i], prime[i], uqinv[i]);
      y += double(r)*prime_recip[i];
      QuickAccumMulAdd(t, H.coeff[i], r);
   }

   long q = long(y + 0.5);
   QuickAccumMulAdd(t, H.MinusMModP, q);

   QuickAccumEnd(t);

   // montgomery
   H.montgomery_struct.eval(value, t);

}



#else



void build(MatPrime_crt_helper& H, const ZZ& P)
{
   ZZ B, M, M1, M2, M3;
   long n, i, j;
   long q, t;
   mulmod_t qinv;

   sqr(B, P);
   mul(B, B, NTL_MatPrimeLimit);
   LeftShift(B, B, NTL_MatPrimeFudge);

   set(M);
   n = 0;
   while (M <= B) {
      UseMatPrime(n);
      q = GetMatPrime(n);
      n++;
      mul(M, M, q);
   }


   double fn = double(n);

   if (8.0*fn*(fn+48) > NTL_FDOUBLE_PRECISION)
      ResourceError("modulus too big");

   long n_half_ceil = (n+1)/2;


   H.NumPrimes = n;
   H.sz = P.size();
   H.prime.SetLength(n);
   H.prime_recip.SetLength(n);
   H.u.SetLength(n);
   H.uqinv.SetLength(n);
   H.red_struct.SetLength(n);

   H.ZZ_red_struct.SetLength(n_half_ceil);
   H.coeff.SetSize(n_half_ceil, P.size());

   H.montgomery_struct.init(P, ZZ(n) << (2*NTL_MatPrime_NBITS));


   for (i = 0; i < n; i++) {
      q = GetMatPrime(i);
      qinv = MatPrimeTables[i]->context.ModulusInverse();

      div(M1, M, q);
      t = rem(M1, q);
      t = InvMod(t, q); // = (M/q)^{-1} rem q 

      H.prime[i] = q;
      H.prime_recip[i] = 1/double(q);
      H.u[i] = t;
      H.uqinv[i] = PrepMulModPrecon(H.u[i], q, qinv);
      H.red_struct[i] = MatPrimeTables[i]->context.red_struct();

   }

   ZZ qq, rr;
   DivRem(qq, rr, M, P);
   NegateMod(H.MinusMModP, rr, P);
   H.montgomery_struct.adjust(H.MinusMModP);

   for (i = 0, j = 0; i < n; i += 2, j++) {
      q = GetMatPrime(i);
      if (i+1 < n) q *= GetMatPrime(i+1);

      long tt = rem(qq, q);

      mul(M2, P, tt);
      add(M2, M2, rr); 
      div(M2, M2, q);  // = (M/q) rem p

      // montgomery
      H.montgomery_struct.adjust(M2);

      H.ZZ_red_struct[j].build(q);
      H.coeff[j] = M2;
   }

   H.cost = double(H.sz)*double(n_half_ceil);
}


void reduce(const MatPrime_crt_helper& H, const ZZ& value, MatPrime_residue_t *remainders,
            MatPrime_crt_helper_scratch& scratch)
{
   long n = H.NumPrimes;
   const sp_ZZ_reduce_struct *ZZ_red_struct = H.ZZ_red_struct.elts();
   const sp_reduce_struct *red_struct = H.red_struct.elts();
   const long *prime = H.prime.elts();

   long i = 0, j = 0;
   for (; i <= n-2; i += 2, j++) {
      unsigned long t = ZZ_red_struct[j].rem(value); 
      remainders[i] = rem(t, prime[i], red_struct[i]);
      remainders[i+1] = rem(t, prime[i+1], red_struct[i+1]);
   }
   if (i < n) {
      remainders[i] = ZZ_red_struct[j].rem(value);
   }
}



void reconstruct(const MatPrime_crt_helper& H, ZZ& value, const MatPrime_residue_t *remainders,
                 MatPrime_crt_helper_scratch& scratch)
{
   ZZ& t = scratch.t;

   long nprimes = H.NumPrimes;
   const long *u = H.u.elts();
   const long *prime = H.prime.elts();
   const mulmod_precon_t  *uqinv = H.uqinv.elts();
   const double *prime_recip = H.prime_recip.elts();

   double y = 0.0;

   QuickAccumBegin(t, H.sz);

   long i = 0, j = 0;
   for (; i <= nprimes-2; i += 2, j++) {
      long r0 = MulModPrecon(remainders[i], u[i], prime[i], uqinv[i]);
      long r1 = MulModPrecon(remainders[i+1], u[i+1], prime[i+1], uqinv[i+1]);
      y += double(r0)*prime_recip[i] + double(r1)*prime_recip[i+1];
      long r = r0*prime[i+1] + r1*prime[i];
      QuickAccumMulAdd(t, H.coeff[j], r);
   }

   if (i < nprimes) {
      long r = MulModPrecon(remainders[i], u[i], prime[i], uqinv[i]);
      y += double(r)*prime_recip[i];
      QuickAccumMulAdd(t, H.coeff[j], r);
   }

   long q = long(y + 0.5);
   QuickAccumMulAdd(t, H.MinusMModP, q);

   QuickAccumEnd(t);

   // montgomery
   H.montgomery_struct.eval(value, t);

}

#endif


// Facillitates PIMPL in ZZ_p.h
void MatPrime_crt_helper_deleter(MatPrime_crt_helper* p)
{
   delete p;
}



NTL_END_IMPL

#if 0

NTL_CLIENT

int main()
{
   ZZ P;
   RandomLen(P, 8000);

   MatPrime_crt_helper H;
   build(H, P);


   MatPrime_crt_helper_scratch scratch;

   long nprimes = H.GetNumPrimes();

   cerr << nprimes << "\n";

   ZZ a, b, c, d, e;
   RandomBnd(a, P);
   RandomBnd(b, P);
   RandomBnd(c, P);
   RandomBnd(d, P);

   e = (a*b + c*d) % P;


   Vec<MatPrime_residue_t> avec, bvec, cvec, dvec, evec;

   avec.SetLength(nprimes);
   bvec.SetLength(nprimes);
   cvec.SetLength(nprimes);
   dvec.SetLength(nprimes);
   evec.SetLength(nprimes);

   reduce(H, a, avec.elts(), scratch);
   reduce(H, b, bvec.elts(), scratch);
   reduce(H, c, cvec.elts(), scratch);
   reduce(H, d, dvec.elts(), scratch);

   for (long i = 0; i < nprimes; i++) {
      long q = GetMatPrime(i);
      evec[i] = AddMod(MulMod(avec[i], bvec[i], q), MulMod(cvec[i], dvec[i], q), q);
   }

   ZZ e1;

   reconstruct(H, e1, evec.elts(), scratch);

   if (e == e1) 
      cerr << "PASS\n";
   else
      cerr << "FAIL\n";
}


#endif



#include <NTL/LLL.h>
#include <NTL/fileio.h>


NTL_START_IMPL




static void RowTransform(vec_ZZ& A, vec_ZZ& B, const ZZ& MU1)
// x = x - y*MU
{
   NTL_ZZRegister(T);
   NTL_ZZRegister(MU);
   long k;

   long n = A.length();
   long i;

   MU = MU1;

   if (MU == 1) {
      for (i = 1; i <= n; i++)
         sub(A(i), A(i), B(i));

      return;
   }

   if (MU == -1) {
      for (i = 1; i <= n; i++)
         add(A(i), A(i), B(i));

      return;
   }

   if (MU == 0) return;

   if (NumTwos(MU) >= NTL_ZZ_NBITS) 
      k = MakeOdd(MU);
   else
      k = 0;


   if (MU.WideSinglePrecision()) {
      long mu1;
      conv(mu1, MU);

      for (i = 1; i <= n; i++) {
         mul(T, B(i), mu1);
         if (k > 0) LeftShift(T, T, k);
         sub(A(i), A(i), T);
      }
   }
   else {
      for (i = 1; i <= n; i++) {
         mul(T, B(i), MU);
         if (k > 0) LeftShift(T, T, k);
         sub(A(i), A(i), T);
      }
   }
}

static void RowTransform2(vec_ZZ& A, vec_ZZ& B, const ZZ& MU1)
// x = x + y*MU
{
   NTL_ZZRegister(T);
   NTL_ZZRegister(MU);
   long k;

   long n = A.length();
   long i;

   MU = MU1;

   if (MU == 1) {
      for (i = 1; i <= n; i++)
         add(A(i), A(i), B(i));

      return;
   }

   if (MU == -1) {
      for (i = 1; i <= n; i++)
         sub(A(i), A(i), B(i));

      return;
   }

   if (MU == 0) return;

   if (NumTwos(MU) >= NTL_ZZ_NBITS) 
      k = MakeOdd(MU);
   else
      k = 0;

   if (MU.WideSinglePrecision()) {
      long mu1;
      conv(mu1, MU);

      for (i = 1; i <= n; i++) {
         mul(T, B(i), mu1);
         if (k > 0) LeftShift(T, T, k);
         add(A(i), A(i), T);
      }
   }
   else {
      for (i = 1; i <= n; i++) {
         mul(T, B(i), MU);
         if (k > 0) LeftShift(T, T, k);
         add(A(i), A(i), T);
      }
   }
}

void ComputeGS(const mat_ZZ& B, mat_RR& B1, 
               mat_RR& mu, vec_RR& b, 
               vec_RR& c, long k, const RR& bound, long st, 
               vec_RR& buf, const RR& bound2)
{
   long i, j;
   RR s, t, t1;
   ZZ T1;

   if (st < k) {
      for (i = 1; i < st; i++)
         mul(buf(i), mu(k,i), c(i));
   }

   for (j = st; j <= k-1; j++) {
      InnerProduct(s, B1(k), B1(j));

      sqr(t1, s);
      mul(t1, t1, bound);
      mul(t, b(k), b(j));

      if (t >= bound2 && t >= t1) {
         InnerProduct(T1, B(k), B(j));
         conv(s, T1);
      }

      clear(t1);
      for (i = 1; i <= j-1; i++) {
         mul(t, mu(j, i), buf(i));
         add(t1, t1, t);
      }

      sub(t, s, t1);
      buf(j) = t;
      div(mu(k, j), t, c(j));
   }


   clear(s);
   for (j = 1; j <= k-1; j++) {
      mul(t, mu(k, j), buf(j));
      add(s, s, t);
   }

   sub(c(k), b(k), s);
}

NTL_TLS_GLOBAL_DECL(RR, red_fudge)

static NTL_CHEAP_THREAD_LOCAL long log_red = 0;

static void init_red_fudge()
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);

   log_red = long(0.50*RR::precision());

   power2(red_fudge, -log_red);
}

static void inc_red_fudge()
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);


   mul(red_fudge, red_fudge, 2);
   log_red--;

   cerr << "LLL_RR: warning--relaxing reduction (" << log_red << ")\n";

   if (log_red < 4)
      ResourceError("LLL_RR: can not continue...sorry");
}




static NTL_CHEAP_THREAD_LOCAL long verbose = 0;
static NTL_CHEAP_THREAD_LOCAL unsigned long NumSwaps = 0;
static NTL_CHEAP_THREAD_LOCAL double StartTime = 0;
static NTL_CHEAP_THREAD_LOCAL double LastTime = 0;



static void LLLStatus(long max_k, double t, long m, const mat_ZZ& B)
{
   cerr << "---- LLL_RR status ----\n";
   cerr << "elapsed time: ";
   PrintTime(cerr, t-StartTime);
   cerr << ", stage: " << max_k;
   cerr << ", rank: " << m;
   cerr << ", swaps: " << NumSwaps << "\n";

   ZZ t1;
   long i;
   double prodlen = 0;

   for (i = 1; i <= m; i++) {
      InnerProduct(t1, B(i), B(i));
      if (!IsZero(t1))
         prodlen += log(t1);
   }

   cerr << "log of prod of lengths: " << prodlen/(2.0*log(2.0)) << "\n";

   if (LLLDumpFile) {
      cerr << "dumping to " << LLLDumpFile << "...";

      ofstream f;
      OpenWrite(f, LLLDumpFile);
      
      f << "[";
      for (i = 1; i <= m; i++) {
         f << B(i) << "\n";
      }
      f << "]\n";

      f.close();

      cerr << "\n";
   }

   LastTime = t;
   
}



static
long ll_LLL_RR(mat_ZZ& B, mat_ZZ* U, const RR& delta, long deep, 
           LLLCheckFct check, mat_RR& B1, mat_RR& mu, 
           vec_RR& b, vec_RR& c, long m, long init_k, long &quit)
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);

   long n = B.NumCols();

   long i, j, k, Fc1;
   ZZ MU;
   RR mu1, t1, t2, cc;
   ZZ T1;

   RR bound;

      // we tolerate a 15% loss of precision in computing
      // inner products in ComputeGS.

   power2(bound, 2*long(0.15*RR::precision()));


   RR bound2;

   power2(bound2, 2*RR::precision());


   quit = 0;
   k = init_k;

   vec_long st_mem;
   st_mem.SetLength(m+2);
   long *st = st_mem.elts();

   for (i = 1; i < k; i++)
      st[i] = i;

   for (i = k; i <= m+1; i++)
      st[i] = 1;

   vec_RR buf;
   buf.SetLength(m);

   long rst;
   long counter;

   long trigger_index;
   long small_trigger;
   long cnt;

   RR half;
   conv(half,  0.5);
   RR half_plus_fudge;
   add(half_plus_fudge, half, red_fudge);

   long max_k = 0;
   double tt;

   while (k <= m) {

      if (k > max_k) {
         max_k = k;
      }

      if (verbose) {
         tt = GetTime();

         if (tt > LastTime + LLLStatusInterval)
            LLLStatus(max_k, tt, m, B);
      }


      if (st[k] == k)
         rst = 1;
      else
         rst = k;

      if (st[k] < st[k+1]) st[k+1] = st[k];
      ComputeGS(B, B1, mu, b, c, k, bound, st[k], buf, bound2);
      st[k] = k;

      counter = 0;
      trigger_index = k;
      small_trigger = 0;
      cnt = 0;

      do {
         // size reduction

         counter++;
         if (counter > 10000) {
            cerr << "LLL_XD: warning--possible infinite loop\n";
            counter = 0;
         }


         Fc1 = 0;

         for (j = rst-1; j >= 1; j--) {
            abs(t1, mu(k,j));
            if (t1 > half_plus_fudge) {

               if (!Fc1) {
                  if (j > trigger_index ||
                      (j == trigger_index && small_trigger)) {

                     cnt++;

                     if (cnt > 10) {
                        inc_red_fudge();
                        add(half_plus_fudge, half, red_fudge);
                        cnt = 0;
                     }
                  }

                  trigger_index = j;
                  small_trigger = (t1 < 4);
               }

               Fc1 = 1;
   
               mu1 = mu(k,j);
               if (sign(mu1) >= 0) {
                  sub(mu1, mu1, half);
                  ceil(mu1, mu1);
               }
               else {
                  add(mu1, mu1, half);
                  floor(mu1, mu1);
               }

               if (mu1 == 1) {
                  for (i = 1; i <= j-1; i++)
                     sub(mu(k,i), mu(k,i), mu(j,i));
               }
               else if (mu1 == -1) {
                  for (i = 1; i <= j-1; i++)
                     add(mu(k,i), mu(k,i), mu(j,i));
               }
               else {
                  for (i = 1; i <= j-1; i++) {
                     mul(t2, mu1, mu(j,i));
                     sub(mu(k,i), mu(k,i), t2);
                  }
               }

   
               conv(MU, mu1);

               sub(mu(k,j), mu(k,j), mu1);
   
               RowTransform(B(k), B(j), MU);
               if (U) RowTransform((*U)(k), (*U)(j), MU);
            }
         }

         if (Fc1) {
            for (i = 1; i <= n; i++)
               conv(B1(k, i), B(k, i));
   
            InnerProduct(b(k), B1(k), B1(k));
            ComputeGS(B, B1, mu, b, c, k, bound, 1, buf, bound2);
         }
      } while (Fc1);

      if (check && (*check)(B(k))) 
         quit = 1;

      if (IsZero(b(k))) {
         for (i = k; i < m; i++) {
            // swap i, i+1
            swap(B(i), B(i+1));
            swap(B1(i), B1(i+1));
            swap(b(i), b(i+1));
            if (U) swap((*U)(i), (*U)(i+1));
         }

         for (i = k; i <= m+1; i++) st[i] = 1;

         m--;
         if (quit) break;
         continue;
      }

      if (quit) break;

      if (deep > 0) {
         // deep insertions
   
         cc = b(k);
         long l = 1;
         while (l <= k-1) { 
            mul(t1, delta, c(l));
            if (t1 > cc) break;
            sqr(t1, mu(k,l));
            mul(t1, t1, c(l));
            sub(cc, cc, t1);
            l++;
         }
   
         if (l <= k-1 && (l <= deep || k-l <= deep)) {
            // deep insertion at position l
   
            for (i = k; i > l; i--) {
               // swap rows i, i-1
               swap(B(i), B(i-1));
               swap(B1(i), B1(i-1));
               swap(mu(i), mu(i-1));
               swap(b(i), b(i-1));
               if (U) swap((*U)(i), (*U)(i-1));
            }
   
            k = l;
            continue;
         }
      } // end deep insertions

      // test LLL reduction condition

      if (k <= 1) {
         k++;
      }
      else {
         sqr(t1, mu(k,k-1));
         mul(t1, t1, c(k-1));
         add(t1, t1, c(k));
         mul(t2, delta, c(k-1));
         if (t2 > t1) {
            // swap rows k, k-1
            swap(B(k), B(k-1));
            swap(B1(k), B1(k-1));
            swap(mu(k), mu(k-1));
            swap(b(k), b(k-1));
            if (U) swap((*U)(k), (*U)(k-1));
   
            k--;
            NumSwaps++;
         }
         else {
            k++;
         }
      }
   }

   if (verbose) {
      LLLStatus(m+1, GetTime(), m, B);
   }


   return m;
}

static
long LLL_RR(mat_ZZ& B, mat_ZZ* U, const RR& delta, long deep, 
           LLLCheckFct check)
{
   long m = B.NumRows();
   long n = B.NumCols();

   long i, j;
   long new_m, dep, quit;
   RR s;
   ZZ MU;
   RR mu1;

   RR t1;
   ZZ T1;

   init_red_fudge();

   if (U) ident(*U, m);

   mat_RR B1;  // approximates B
   B1.SetDims(m, n);


   mat_RR mu;
   mu.SetDims(m, m);

   vec_RR c;  // squared lengths of Gramm-Schmidt basis vectors
   c.SetLength(m);

   vec_RR b; // squared lengths of basis vectors
   b.SetLength(m);


   for (i = 1; i <=m; i++)
      for (j = 1; j <= n; j++) 
         conv(B1(i, j), B(i, j));


         
   for (i = 1; i <= m; i++) {
      InnerProduct(b(i), B1(i), B1(i));
   }


   new_m = ll_LLL_RR(B, U, delta, deep, check, B1, mu, b, c, m, 1, quit);
   dep = m - new_m;
   m = new_m;

   if (dep > 0) {
      // for consistency, we move all of the zero rows to the front

      for (i = 0; i < m; i++) {
         swap(B(m+dep-i), B(m-i));
         if (U) swap((*U)(m+dep-i), (*U)(m-i));
      }
   }


   return m;
}

         

long LLL_RR(mat_ZZ& B, double delta, long deep, 
            LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("LLL_RR: bad delta");
   if (deep < 0) LogicError("LLL_RR: bad deep");
   RR Delta;
   conv(Delta, delta);
   return LLL_RR(B, 0, Delta, deep, check);
}

long LLL_RR(mat_ZZ& B, mat_ZZ& U, double delta, long deep, 
           LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("LLL_RR: bad delta");
   if (deep < 0) LogicError("LLL_RR: bad deep");
   RR Delta;
   conv(Delta, delta);
   return LLL_RR(B, &U, Delta, deep, check);
}



NTL_TLS_GLOBAL_DECL(vec_RR, BKZConstant)

static
void ComputeBKZConstant(long beta, long p)
{
   NTL_TLS_GLOBAL_ACCESS(BKZConstant);

   RR c_PI;
   ComputePi(c_PI);

   RR LogPI = log(c_PI);

   BKZConstant.SetLength(beta-1);

   vec_RR Log;
   Log.SetLength(beta);


   long i, j, k;
   RR x, y;

   for (j = 1; j <= beta; j++)
      Log(j) = log(to_RR(j));

   for (i = 1; i <= beta-1; i++) {
      // First, we compute x = gamma(i/2)^{2/i}

      k = i/2;

      if ((i & 1) == 0) { // i even
         x = 0;
         for (j = 1; j <= k; j++)
            x += Log(j);
          
         x = exp(x/k);

      }
      else { // i odd
         x = 0;
         for (j = k + 2; j <= 2*k + 2; j++)
            x += Log(j);

         x += 0.5*LogPI - 2*(k+1)*Log(2);

         x = exp(2*x/i);
      }

      // Second, we compute y = 2^{2*p/i}

      y = exp(-(2*p/to_RR(i))*Log(2));

      BKZConstant(i) = x*y/c_PI;
   }

}

NTL_TLS_GLOBAL_DECL(vec_RR, BKZThresh)

static 
void ComputeBKZThresh(RR *c, long beta)
{
   NTL_TLS_GLOBAL_ACCESS(BKZConstant);
   NTL_TLS_GLOBAL_ACCESS(BKZThresh);

   BKZThresh.SetLength(beta-1);

   long i;
   RR x;
   RR t1;

   x = 0;

   for (i = 1; i <= beta-1; i++) {
      log(t1, c[i-1]);
      add(x, x, t1);
      div(t1, x, i);
      exp(t1, t1);
      mul(BKZThresh(i), t1, BKZConstant(i));
   }
}




static 
void BKZStatus(double tt, double enum_time, unsigned long NumIterations, 
               unsigned long NumTrivial, unsigned long NumNonTrivial, 
               unsigned long NumNoOps, long m, 
               const mat_ZZ& B)
{
   cerr << "---- BKZ_RR status ----\n";
   cerr << "elapsed time: ";
   PrintTime(cerr, tt-StartTime);
   cerr << ", enum time: ";
   PrintTime(cerr, enum_time);
   cerr << ", iter: " << NumIterations << "\n";
   cerr << "triv: " << NumTrivial;
   cerr << ", nontriv: " << NumNonTrivial;
   cerr << ", no ops: " << NumNoOps;
   cerr << ", rank: " << m;
   cerr << ", swaps: " << NumSwaps << "\n";



   ZZ t1;
   long i;
   double prodlen = 0;

   for (i = 1; i <= m; i++) {
      InnerProduct(t1, B(i), B(i));
      if (!IsZero(t1))
         prodlen += log(t1);
   }

   cerr << "log of prod of lengths: " << prodlen/(2.0*log(2.0)) << "\n";


   if (LLLDumpFile) {
      cerr << "dumping to " << LLLDumpFile << "...";

      ofstream f;
      OpenWrite(f, LLLDumpFile);
      
      f << "[";
      for (i = 1; i <= m; i++) {
         f << B(i) << "\n";
      }
      f << "]\n";

      f.close();

      cerr << "\n";
   }

   LastTime = tt;
   
}




static
long BKZ_RR(mat_ZZ& BB, mat_ZZ* UU, const RR& delta, 
         long beta, long prune, LLLCheckFct check)
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);
   NTL_TLS_GLOBAL_ACCESS(BKZThresh);


   long m = BB.NumRows();
   long n = BB.NumCols();
   long m_orig = m;
   
   long i, j;
   ZZ MU;

   RR t1, t2;
   ZZ T1;

   init_red_fudge();

   mat_ZZ B;
   B = BB;

   B.SetDims(m+1, n);


   mat_RR B1;
   B1.SetDims(m+1, n);

   mat_RR mu;
   mu.SetDims(m+1, m);

   vec_RR c;
   c.SetLength(m+1);

   vec_RR b;
   b.SetLength(m+1);

   RR cbar;

   vec_RR ctilda;
   ctilda.SetLength(m+1);

   vec_RR vvec;
   vvec.SetLength(m+1);

   vec_RR yvec;
   yvec.SetLength(m+1);

   vec_RR uvec;
   uvec.SetLength(m+1);

   vec_RR utildavec;
   utildavec.SetLength(m+1);

   vec_long Deltavec;
   Deltavec.SetLength(m+1);

   vec_long deltavec;
   deltavec.SetLength(m+1);

   mat_ZZ Ulocal;
   mat_ZZ *U;

   if (UU) {
      Ulocal.SetDims(m+1, m);
      for (i = 1; i <= m; i++)
         conv(Ulocal(i, i), 1);
      U = &Ulocal;
   }
   else
      U = 0;

   long quit;
   long new_m;
   long z, jj, kk;
   long s, t;
   long h;


   for (i = 1; i <=m; i++)
      for (j = 1; j <= n; j++) 
         conv(B1(i, j), B(i, j));

         
   for (i = 1; i <= m; i++) {
      InnerProduct(b(i), B1(i), B1(i));
   }

   // cerr << "\n";
   // cerr << "first LLL\n";

   m = ll_LLL_RR(B, U, delta, 0, check, B1, mu, b, c, m, 1, quit);

   double tt;

   double enum_time = 0;
   unsigned long NumIterations = 0;
   unsigned long NumTrivial = 0;
   unsigned long NumNonTrivial = 0;
   unsigned long NumNoOps = 0;

   long verb = verbose;

   verbose = 0;


   if (m < m_orig) {
      for (i = m_orig+1; i >= m+2; i--) {
         // swap i, i-1

         swap(B(i), B(i-1));
         if (U) swap((*U)(i), (*U)(i-1));
      }
   }

   long clean = 1;

   if (!quit && m > 1) {
      // cerr << "continuing\n";

      if (beta > m) beta = m;

      if (prune > 0)
         ComputeBKZConstant(beta, prune);

      z = 0;
      jj = 0;
   
      while (z < m-1) {
         jj++;
         kk = min(jj+beta-1, m);
   
         if (jj == m) {
            jj = 1;
            kk = beta;
            clean = 1;
         }

         if (verb) {
            tt = GetTime();
            if (tt > LastTime + LLLStatusInterval)
               BKZStatus(tt, enum_time, NumIterations, NumTrivial,
                         NumNonTrivial, NumNoOps, m, B);
         }

         // ENUM

         double tt1;

         if (verb) {
            tt1 = GetTime();
         }

         if (prune > 0)
            ComputeBKZThresh(&c(jj), kk-jj+1);

         cbar = c(jj);
         conv(utildavec(jj), 1);
         conv(uvec(jj), 1);
   
         conv(yvec(jj), 0);
         conv(vvec(jj), 0);
         Deltavec(jj) = 0;
   
   
         s = t = jj;
         deltavec(jj) = 1;
   
         for (i = jj+1; i <= kk+1; i++) {
            conv(ctilda(i), 0);
            conv(uvec(i), 0);
            conv(utildavec(i), 0);
            conv(yvec(i), 0);
            Deltavec(i) = 0;
            conv(vvec(i), 0);
            deltavec(i) = 1;
         }

         long enum_cnt = 0;
   
         while (t <= kk) {
            if (verb) {
               enum_cnt++;
               if (enum_cnt > 100000) {
                  enum_cnt = 0;
                  tt = GetTime();
                  if (tt > LastTime + LLLStatusInterval) {
                     enum_time += tt - tt1;
                     tt1 = tt;
                     BKZStatus(tt, enum_time, NumIterations, NumTrivial,
                               NumNonTrivial, NumNoOps, m, B);
                  }
               }
            }


            add(t1, yvec(t), utildavec(t));
            sqr(t1, t1);
            mul(t1, t1, c(t));
            add(ctilda(t), ctilda(t+1), t1);

            if (prune > 0 && t > jj) 
               sub(t1, cbar, BKZThresh(t-jj));
            else
               t1 = cbar;

   
            if (ctilda(t) <t1) {
               if (t > jj) {
                  t--;
                  clear(t1);
                  for (i = t+1; i <= s; i++) {
                     mul(t2, utildavec(i), mu(i,t));
                     add(t1, t1, t2);
                  }

                  yvec(t) = t1;
                  negate(t1, t1);
                  if (sign(t1) >= 0) {
                     sub(t1, t1, 0.5);
                     ceil(t1, t1);
                  }
                  else {
                     add(t1, t1, 0.5);
                     floor(t1, t1);
                  }

                  utildavec(t) = t1;
                  vvec(t) = t1;
                  Deltavec(t) = 0;

                  negate(t1, t1);

                  if (t1 < yvec(t)) 
                     deltavec(t) = -1;
                  else
                     deltavec(t) = 1;
               }
               else {
                  cbar = ctilda(jj);
                  for (i = jj; i <= kk; i++) {
                     uvec(i) = utildavec(i);
                  }
               }
            }
            else {
               t++;
               s = max(s, t);
               if (t < s) Deltavec(t) = -Deltavec(t);
               if (Deltavec(t)*deltavec(t) >= 0) Deltavec(t) += deltavec(t);
               add(utildavec(t), vvec(t), Deltavec(t));
            }
         }
         
         if (verb) {
            tt1 = GetTime() - tt1;
            enum_time += tt1;
         }

         NumIterations++;
   
         h = min(kk+1, m);

         mul(t1, red_fudge, -8);
         add(t1, t1, delta);
         mul(t1, t1, c(jj));
   
         if (t1 > cbar) {
 
            clean = 0;

            // we treat the case that the new vector is b_s (jj < s <= kk)
            // as a special case that appears to occur most of the time.
   
            s = 0;
            for (i = jj+1; i <= kk; i++) {
               if (uvec(i) != 0) {
                  if (s == 0)
                     s = i;
                  else
                     s = -1;
               }
            }
   
            if (s == 0) LogicError("BKZ_RR: internal error");
   
            if (s > 0) {
               // special case
               // cerr << "special case\n";

               NumTrivial++;
   
               for (i = s; i > jj; i--) {
                  // swap i, i-1
                  swap(B(i-1), B(i));
                  swap(B1(i-1), B1(i));
                  swap(b(i-1), b(i));
                  if (U) swap((*U)(i-1), (*U)(i));
               }
   
               new_m = ll_LLL_RR(B, U, delta, 0, check, 
                                B1, mu, b, c, h, jj, quit);
               if (new_m != h) LogicError("BKZ_RR: internal error");
               if (quit) break;
            }
            else {
               // the general case

               NumNonTrivial++;
   
               for (i = 1; i <= n; i++) conv(B(m+1, i), 0);

               if (U) {
                  for (i = 1; i <= m_orig; i++)
                     conv((*U)(m+1, i), 0);
               }

               for (i = jj; i <= kk; i++) {
                  if (uvec(i) == 0) continue;
                  conv(MU, uvec(i));
                  RowTransform2(B(m+1), B(i), MU);
                  if (U) RowTransform2((*U)(m+1), (*U)(i), MU);
               }
      
               for (i = m+1; i >= jj+1; i--) {
                  // swap i, i-1
                  swap(B(i-1), B(i));
                  swap(B1(i-1), B1(i));
                  swap(b(i-1), b(i));
                  if (U) swap((*U)(i-1), (*U)(i));
               }
      
               for (i = 1; i <= n; i++)
                  conv(B1(jj, i), B(jj, i));
      
               InnerProduct(b(jj), B1(jj), B1(jj));
      
               if (b(jj) == 0) LogicError("BKZ_RR: internal error"); 
      
               // remove linear dependencies
   
               // cerr << "general case\n";
               new_m = ll_LLL_RR(B, U, delta, 0, 0, B1, mu, b, c, kk+1, jj, quit);
              
               if (new_m != kk) LogicError("BKZ_RR: internal error"); 

               // remove zero vector
      
               for (i = kk+2; i <= m+1; i++) {
                  // swap i, i-1
                  swap(B(i-1), B(i));
                  swap(B1(i-1), B1(i));
                  swap(b(i-1), b(i));
                  if (U) swap((*U)(i-1), (*U)(i));
               }
      
               quit = 0;
               if (check) {
                  for (i = 1; i <= kk; i++)
                     if ((*check)(B(i))) {
                        quit = 1;
                        break;
                     }
               }

               if (quit) break;
   
               if (h > kk) {
                  // extend reduced basis
   
                  new_m = ll_LLL_RR(B, U, delta, 0, check, 
                                   B1, mu, b, c, h, h, quit);
   
                  if (new_m != h) LogicError("BKZ_RR: internal error");
                  if (quit) break;
               }
            }
   
            z = 0;
         }
         else {
            // LLL_RR
            // cerr << "progress\n";

            NumNoOps++;

            if (!clean) {
               new_m = 
                  ll_LLL_RR(B, U, delta, 0, check, B1, mu, b, c, h, h, quit);
               if (new_m != h) LogicError("BKZ_RR: internal error");
               if (quit) break;
            }
   
            z++;
         }
      }
   }

   if (verb) {
      BKZStatus(GetTime(), enum_time, NumIterations, NumTrivial, NumNonTrivial,
                NumNoOps, m, B);
   }


   // clean up

   if (m_orig > m) {
      // for consistency, we move zero vectors to the front

      for (i = m+1; i <= m_orig; i++) {
         swap(B(i), B(i+1));
         if (U) swap((*U)(i), (*U)(i+1));
      }

      for (i = 0; i < m; i++) {
         swap(B(m_orig-i), B(m-i));
         if (U) swap((*U)(m_orig-i), (*U)(m-i));
      }
   }

   B.SetDims(m_orig, n);
   BB = B;

   if (U) {
      U->SetDims(m_orig, m_orig);
      *UU = *U;
   }

   return m;
}

long BKZ_RR(mat_ZZ& BB, mat_ZZ& UU, double delta, 
         long beta, long prune, LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("BKZ_RR: bad delta");
   if (beta < 2) LogicError("BKZ_RR: bad block size");

   RR Delta;
   conv(Delta, delta);

   return BKZ_RR(BB, &UU, Delta, beta, prune, check);
}

long BKZ_RR(mat_ZZ& BB, double delta, 
         long beta, long prune, LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("BKZ_RR: bad delta");
   if (beta < 2) LogicError("BKZ_RR: bad block size");

   RR Delta;
   conv(Delta, delta);

   return BKZ_RR(BB, 0, Delta, beta, prune, check);
}




void NearVector(vec_ZZ& ww, const mat_ZZ& BB, const vec_ZZ& a)
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);

   long n = BB.NumCols();

   if (n != BB.NumRows())
      LogicError("NearVector: matrix must be square");

   if (n != a.length())
      LogicError("NearVector: dimension mismatch");

   long i, j;
   mat_ZZ B;

   B.SetDims(n+1, n);
   for (i = 1; i <= n; i++)
      B(i) = BB(i);

   B(n+1) = a;

   mat_RR B1, mu;
   vec_RR b, c;

   B1.SetDims(n+1, n);
   mu.SetDims(n+1, n+1);
   b.SetLength(n+1);
   c.SetLength(n+1);

   vec_RR buf;
   buf.SetLength(n+1);


   for (i = 1; i <= n+1; i++)
      for (j = 1; j <= n; j++)
         conv(B1(i, j), B(i, j));

   for (i = 1; i <= n+1; i++)
      InnerProduct(b(i), B1(i), B1(i));

   

   RR bound;
   power2(bound, 2*long(0.15*RR::precision()));

   RR bound2;
   power2(bound2, 2*RR::precision());


   for (i = 1; i <= n+1; i++)
      ComputeGS(B, B1, mu, b, c, i, bound, 1, buf, bound2);

   init_red_fudge();

   RR half;
   conv(half,  0.5);
   RR half_plus_fudge;
   add(half_plus_fudge, half, red_fudge);

   RR t1, t2, mu1;
   ZZ MU;

   long trigger_index = n+1;
   long small_trigger = 0;
   long cnt = 0;

   long Fc1;

   vec_ZZ w;
   w.SetLength(n);
   clear(w);

   do {
      Fc1 = 0;

      for (j = n; j >= 1; j--) {
         abs(t1, mu(n+1,j));
         if (t1 > half_plus_fudge) {

            if (!Fc1) {
               if (j > trigger_index ||
                   (j == trigger_index && small_trigger)) {

                  cnt++;

                  if (cnt > 10) {
                     inc_red_fudge();
                     add(half_plus_fudge, half, red_fudge);
                     cnt = 0;
                  }
               }

               trigger_index = j;
               small_trigger = (t1 < 4);
            }

            Fc1 = 1;

            mu1 = mu(n+1,j);
            if (sign(mu1) >= 0) {
               sub(mu1, mu1, half);
               ceil(mu1, mu1);
            }
            else {
               add(mu1, mu1, half);
               floor(mu1, mu1);
            }

            if (mu1 == 1) {
               for (i = 1; i <= j-1; i++)
                  sub(mu(n+1,i), mu(n+1,i), mu(j,i));
            }
            else if (mu1 == -1) {
               for (i = 1; i <= j-1; i++)
                  add(mu(n+1,i), mu(n+1,i), mu(j,i));
            }
            else {
               for (i = 1; i <= j-1; i++) {
                  mul(t2, mu1, mu(j,i));
                  sub(mu(n+1,i), mu(n+1,i), t2);
               }
            }


            conv(MU, mu1);

            sub(mu(n+1,j), mu(n+1,j), mu1);

            RowTransform(B(n+1), B(j), MU);
            RowTransform2(w, B(j), MU);
         }
      }

      if (Fc1) {
         for (i = 1; i <= n; i++)
            conv(B1(n+1, i), B(n+1, i));

         InnerProduct(b(n+1), B1(n+1), B1(n+1));
         ComputeGS(B, B1, mu, b, c, n+1, bound, 1, buf, bound2);
      }
   } while (Fc1);

   ww = w;
}

NTL_END_IMPL

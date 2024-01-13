
#include <NTL/LLL.h>
#include <NTL/fileio.h>
#include <NTL/vec_xdouble.h>
#include <NTL/vec_double.h>


NTL_START_IMPL


static xdouble InnerProduct(xdouble *a, xdouble *b, long n)
{
   xdouble s;
   long i;

   s = 0;
   for (i = 1; i <= n; i++) 
      MulAdd(s, s, a[i], b[i]);

   return s;
}


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

      if (k > 0) {

         for (i = 1; i <= n; i++) {
            mul(T, B(i), mu1);
            LeftShift(T, T, k);
            sub(A(i), A(i), T);
         }

      }
      else {

         for (i = 1; i <= n; i++) {
            MulSubFrom(A(i), B(i), mu1);
         }

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

static
void ComputeGS(mat_ZZ& B, xdouble **B1, xdouble **mu, xdouble *b, 
               xdouble *c, long k, xdouble bound, long st, xdouble *buf)
{
   long n = B.NumCols();
   long i, j;
   xdouble s, t1, y, t;
   ZZ T1;

   xdouble *mu_k = mu[k];

   if (st < k) {
      for (i = 1; i < st; i++)
         buf[i] = mu_k[i]*c[i];
   }

   for (j = st; j <= k-1; j++) {
      if (b[k]*b[j] < NTL_FDOUBLE_PRECISION*NTL_FDOUBLE_PRECISION) {
         double z = 0;
         xdouble *B1_k = B1[k];
         xdouble *B1_j = B1[j];

         for (i = 1; i <= n; i++)
            z += B1_k[i].x * B1_j[i].x;

         s = z;
      }
      else {
         s = InnerProduct(B1[k], B1[j], n);
   
         if (s*s <= b[k]*b[j]/bound) {
            InnerProduct(T1, B(k), B(j));
            conv(s, T1);
         }
      }

      xdouble *mu_j = mu[j];

      t1 = 0;
      for (i = 1; i <= j-1; i++)
         MulAdd(t1, t1, mu_j[i], buf[i]);

      mu_k[j] = (buf[j] = (s - t1))/c[j];
   }

   s = 0;
   for (j = 1; j <= k-1; j++)
      MulAdd(s, s, mu_k[j], buf[j]);

   c[k] = b[k] - s;
}

NTL_TLS_GLOBAL_DECL_INIT(xdouble, red_fudge, (to_xdouble(0)))


static NTL_CHEAP_THREAD_LOCAL long log_red = 0;

static void init_red_fudge()
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);

   long i;

   log_red = long(0.50*NTL_DOUBLE_PRECISION);
   red_fudge = 1;

   for (i = log_red; i > 0; i--)
      red_fudge = red_fudge*0.5;
}

static void inc_red_fudge()
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);


   red_fudge = red_fudge * 2;
   log_red--;

   cerr << "LLL_XD: warning--relaxing reduction (" << log_red << ")\n";

   if (log_red < 4)
      ResourceError("LLL_XD: can not continue...sorry");
}



static NTL_CHEAP_THREAD_LOCAL long verbose = 0;
static NTL_CHEAP_THREAD_LOCAL unsigned long NumSwaps = 0;
static NTL_CHEAP_THREAD_LOCAL double StartTime = 0;
static NTL_CHEAP_THREAD_LOCAL double LastTime = 0;



static void LLLStatus(long max_k, double t, long m, const mat_ZZ& B)
{
   cerr << "---- LLL_XD status ----\n";
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
long ll_LLL_XD(mat_ZZ& B, mat_ZZ* U, xdouble delta, long deep, 
           LLLCheckFct check, xdouble **B1, xdouble **mu, 
           xdouble *b, xdouble *c,
           long m, long init_k, long &quit)
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);

   long n = B.NumCols();

   long i, j, k, Fc1;
   ZZ MU;
   xdouble mu1;

   xdouble t1;
   ZZ T1;
   xdouble *tp;


   NTL_TLS_LOCAL_INIT(xdouble, bound, (to_xdouble(0)));


   if (bound == 0) {
      // we tolerate a 15% loss of precision in computing
      // inner products in ComputeGS.

      bound = 1;
      for (i = 2*long(0.15*NTL_DOUBLE_PRECISION); i > 0; i--) {
         bound = bound * 2;
      }
   }


   xdouble half = to_xdouble(0.5);
   xdouble half_plus_fudge = 0.5 + red_fudge;

   quit = 0;
   k = init_k;

   vec_long st_mem;
   st_mem.SetLength(m+2);
   long *st = st_mem.elts();

   for (i = 1; i < k; i++)
      st[i] = i;

   for (i = k; i <= m+1; i++)
      st[i] = 1;

   UniqueArray<xdouble> buf_store;
   buf_store.SetLength(m+1);
   xdouble *buf = buf_store.get();

   long rst;
   long counter;

   long trigger_index;
   long small_trigger;
   long cnt;

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
      ComputeGS(B, B1, mu, b, c, k, bound, st[k], buf);
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
            t1 = fabs(mu[k][j]);
            if (t1 > half_plus_fudge) {

               if (!Fc1) {
                  if (j > trigger_index ||
                      (j == trigger_index && small_trigger)) {

                     cnt++;

                     if (cnt > 10) {
                        inc_red_fudge();
                        half_plus_fudge = 0.5 + red_fudge;
                        cnt = 0;
                     }
                  }

                  trigger_index = j;
                  small_trigger = (t1 < 4);
               }


               Fc1 = 1;
   
               mu1 = mu[k][j];
               if (mu1 >= 0)
                  mu1 = ceil(mu1-half);
               else
                  mu1 = floor(mu1+half);
   
   
               xdouble *mu_k = mu[k];
               xdouble *mu_j = mu[j];
  
               if (mu1 == 1) {
                  for (i = 1; i <= j-1; i++)
                     mu_k[i] -= mu_j[i];
               }
               else if (mu1 == -1) {
                  for (i = 1; i <= j-1; i++)
                     mu_k[i] += mu_j[i];
               }
               else {
                  for (i = 1; i <= j-1; i++)
                     MulSub(mu_k[i], mu_k[i], mu1, mu_j[i]);
               }
  
               mu_k[j] -= mu1;

               conv(MU, mu1);

               // cout << j << " " << MU << "\n";
   
               RowTransform(B(k), B(j), MU);
               if (U) RowTransform((*U)(k), (*U)(j), MU);
            }
         }

         if (Fc1) {
            for (i = 1; i <= n; i++)
               conv(B1[k][i], B(k, i));
   
            b[k] = InnerProduct(B1[k], B1[k], n);
            ComputeGS(B, B1, mu, b, c, k, bound, 1, buf);
         }
      } while (Fc1);

      if (check && (*check)(B(k))) 
         quit = 1;

      if (b[k] == 0) {
         for (i = k; i < m; i++) {
            // swap i, i+1
            swap(B(i), B(i+1));
            tp = B1[i]; B1[i] = B1[i+1]; B1[i+1] = tp;
            t1 = b[i]; b[i] = b[i+1]; b[i+1] = t1;
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
   
         xdouble cc = b[k];
         long l = 1;
         while (l <= k-1 && delta*c[l] <= cc) {
            cc = cc - mu[k][l]*mu[k][l]*c[l];
            l++;
         }
   
         if (l <= k-1 && (l <= deep || k-l <= deep)) {
            // deep insertion at position l
   
            for (i = k; i > l; i--) {
               // swap rows i, i-1
               swap(B(i), B(i-1));
               tp = B1[i]; B1[i] = B1[i-1]; B1[i-1] = tp;
               tp = mu[i]; mu[i] = mu[i-1]; mu[i-1] = tp;
               t1 = b[i]; b[i] = b[i-1]; b[i-1] = t1;
               if (U) swap((*U)(i), (*U)(i-1));
            }
   
            k = l;
            continue;
         }
      } // end deep insertions

      // test LLL reduction condition

      if (k > 1 && delta*c[k-1] > c[k] + mu[k][k-1]*mu[k][k-1]*c[k-1]) {
         // swap rows k, k-1
         swap(B(k), B(k-1));
         tp = B1[k]; B1[k] = B1[k-1]; B1[k-1] = tp;
         tp = mu[k]; mu[k] = mu[k-1]; mu[k-1] = tp;
         t1 = b[k]; b[k] = b[k-1]; b[k-1] = t1;
         if (U) swap((*U)(k), (*U)(k-1));

         k--;
         NumSwaps++;

         // cout << "- " << k << "\n";
      }
      else {
         k++;
         // cout << "+ " << k << "\n";
      }
   }

   if (verbose) {
      LLLStatus(m+1, GetTime(), m, B);
   }


   return m;
}




static
long LLL_XD(mat_ZZ& B, mat_ZZ* U, xdouble delta, long deep, 
           LLLCheckFct check)
{
   long m = B.NumRows();
   long n = B.NumCols();

   long i, j;
   long new_m, dep, quit;
   xdouble s;
   ZZ MU;
   xdouble mu1;

   xdouble t1;
   ZZ T1;

   init_red_fudge();

   if (U) ident(*U, m);

   Unique2DArray<xdouble> B1_store;
   B1_store.SetDimsFrom1(m+1, n+1);
   xdouble **B1 = B1_store.get();  // approximates B


   Unique2DArray<xdouble> mu_store;
   mu_store.SetDimsFrom1(m+1, m+1);
   xdouble **mu = mu_store.get();

   UniqueArray<xdouble> c_store;
   c_store.SetLength(m+1);
   xdouble *c = c_store.get(); // squared lengths of Gramm-Schmidt basis vectors

   UniqueArray<xdouble> b_store;
   b_store.SetLength(m+1);
   xdouble *b = b_store.get(); // squared lengths of basis vectors

   for (i = 1; i <=m; i++)
      for (j = 1; j <= n; j++) 
         conv(B1[i][j], B(i, j));


         
   for (i = 1; i <= m; i++) {
      b[i] = InnerProduct(B1[i], B1[i], n);
   }


   new_m = ll_LLL_XD(B, U, delta, deep, check, B1, mu, b, c, m, 1, quit);
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

         

long LLL_XD(mat_ZZ& B, double delta, long deep, 
            LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("LLL_XD: bad delta");
   if (deep < 0) LogicError("LLL_XD: bad deep");
   return LLL_XD(B, 0, to_xdouble(delta), deep, check);
}

long LLL_XD(mat_ZZ& B, mat_ZZ& U, double delta, long deep, 
           LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }


   if (delta < 0.50 || delta >= 1) LogicError("LLL_XD: bad delta");
   if (deep < 0) LogicError("LLL_XD: bad deep");
   return LLL_XD(B, &U, to_xdouble(delta), deep, check);
}



NTL_TLS_GLOBAL_DECL(vec_xdouble, BKZConstant)

static
void ComputeBKZConstant(long beta, long p)
{
   NTL_TLS_GLOBAL_ACCESS(BKZConstant);

   const double c_PI = 3.14159265358979323846264338328;
   const double LogPI = 1.14472988584940017414342735135;

   BKZConstant.SetLength(beta-1);

   vec_double Log;
   Log.SetLength(beta);


   long i, j, k;
   double x, y;

   for (j = 1; j <= beta; j++)
      Log(j) = log(double(j));

   for (i = 1; i <= beta-1; i++) {
      // First, we compute x = gamma(i/2)^{2/i}

      k = i/2;

      if ((i & 1) == 0) { // i even
         x = 0;
         for (j = 1; j <= k; j++)
            x = x + Log(j);
          
         x = x * (1/double(k));

         x = exp(x);
      }
      else { // i odd
         x = 0;
         for (j = k + 2; j <= 2*k + 2; j++)
            x = x + Log(j);

         x = 0.5*LogPI + x - 2*(k+1)*Log(2);

         x = x * (2.0/double(i));

         x = exp(x);
      }

      // Second, we compute y = 2^{2*p/i}

      y = -(2*p/double(i))*Log(2);
      y = exp(y);

      BKZConstant(i) = x*y/c_PI;
   }
}

NTL_TLS_GLOBAL_DECL(vec_xdouble, BKZThresh)

static
void ComputeBKZThresh(xdouble *c, long beta)
{
   NTL_TLS_GLOBAL_ACCESS(BKZConstant);
   NTL_TLS_GLOBAL_ACCESS(BKZThresh);

   BKZThresh.SetLength(beta-1);

   long i;
   double x;

   x = 0;

   for (i = 1; i <= beta-1; i++) {
      x += log(c[i-1]);
      BKZThresh(i) = xexp(x/double(i))*BKZConstant(i);
   }
}


static 
void BKZStatus(double tt, double enum_time, unsigned long NumIterations, 
               unsigned long NumTrivial, unsigned long NumNonTrivial, 
               unsigned long NumNoOps, long m, 
               const mat_ZZ& B)
{
   cerr << "---- BKZ_XD status ----\n";
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
long BKZ_XD(mat_ZZ& BB, mat_ZZ* UU, xdouble delta, 
         long beta, long prune, LLLCheckFct check)
{
   NTL_TLS_GLOBAL_ACCESS(red_fudge);
   NTL_TLS_GLOBAL_ACCESS(BKZThresh);


   long m = BB.NumRows();
   long n = BB.NumCols();
   long m_orig = m;
   
   long i, j;
   ZZ MU;

   xdouble t1;
   ZZ T1;
   xdouble *tp;

   init_red_fudge();

   mat_ZZ B;
   B = BB;

   B.SetDims(m+1, n);

   Unique2DArray<xdouble> B1_store;
   B1_store.SetDimsFrom1(m+2, n+1);
   xdouble **B1 = B1_store.get();  // approximates B


   Unique2DArray<xdouble> mu_store;
   mu_store.SetDimsFrom1(m+2, m+1);
   xdouble **mu = mu_store.get();

   UniqueArray<xdouble> c_store;
   c_store.SetLength(m+2);
   xdouble *c = c_store.get(); // squared lengths of Gramm-Schmidt basis vectors

   UniqueArray<xdouble> b_store;
   b_store.SetLength(m+2);
   xdouble *b = b_store.get(); // squared lengths of basis vectors

   xdouble cbar;


   UniqueArray<xdouble> ctilda_store;
   ctilda_store.SetLength(m+2);
   xdouble *ctilda = ctilda_store.get();


   UniqueArray<xdouble> vvec_store;
   vvec_store.SetLength(m+2);
   xdouble *vvec = vvec_store.get();

   UniqueArray<xdouble> yvec_store;
   yvec_store.SetLength(m+2);
   xdouble *yvec = yvec_store.get();

   UniqueArray<xdouble> uvec_store;
   uvec_store.SetLength(m+2);
   xdouble *uvec = uvec_store.get();

   UniqueArray<xdouble> utildavec_store;
   utildavec_store.SetLength(m+2);
   xdouble *utildavec = utildavec_store.get();

   UniqueArray<long> Deltavec_store;
   Deltavec_store.SetLength(m+2);
   long *Deltavec = Deltavec_store.get();

   UniqueArray<long> deltavec_store;
   deltavec_store.SetLength(m+2);
   long *deltavec = deltavec_store.get();;



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
   xdouble eta;


   for (i = 1; i <=m; i++)
      for (j = 1; j <= n; j++) 
         conv(B1[i][j], B(i, j));

         
   for (i = 1; i <= m; i++) {
      b[i] = InnerProduct(B1[i], B1[i], n);
   }

   // cerr << "\n";
   // cerr << "first LLL\n";

   m = ll_LLL_XD(B, U, delta, 0, check, B1, mu, b, c, m, 1, quit);

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
            ComputeBKZThresh(&c[jj], kk-jj+1);

         cbar = c[jj];
         utildavec[jj] = uvec[jj] = 1;
   
         yvec[jj] = vvec[jj] = 0;
         Deltavec[jj] = 0;
   
   
         s = t = jj;
         deltavec[jj] = 1;
   
         for (i = jj+1; i <= kk+1; i++) {
            ctilda[i] = uvec[i] = utildavec[i] = yvec[i] = 0;
            Deltavec[i] = 0;
            vvec[i] = 0;
            deltavec[i] = 1;
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


            ctilda[t] = ctilda[t+1] + 
               (yvec[t]+utildavec[t])*(yvec[t]+utildavec[t])*c[t];

            if (prune > 0 && t > jj) {
               eta = BKZThresh(t-jj);
            }
            else
               eta = 0;
   
            if (ctilda[t] < cbar - eta) {
               if (t > jj) {
                  t--;
                  t1 = 0;
                  for (i = t+1; i <= s; i++) {
                     t1 += utildavec[i]*mu[i][t];
                  }


                  yvec[t] = t1;
                  t1 = -t1;
                  if (t1 >= 0)
                     t1 = ceil(t1-0.5);
                  else
                     t1 = floor(t1+0.5);

                  utildavec[t] = vvec[t] = t1;
                  Deltavec[t] = 0;
                  if (utildavec[t] > -yvec[t]) 
                     deltavec[t] = -1;
                  else
                     deltavec[t] = 1;
               }
               else {
                  cbar = ctilda[jj];
                  for (i = jj; i <= kk; i++) {
                     uvec[i] = utildavec[i];
                  }
               }
            }
            else {
               t++;
               s = max(s, t);
               if (t < s) Deltavec[t] = -Deltavec[t];
               if (Deltavec[t]*deltavec[t] >= 0) Deltavec[t] += deltavec[t];
               utildavec[t] = vvec[t] + Deltavec[t];
            }
         }
         
         if (verb) {
            tt1 = GetTime() - tt1;
            enum_time += tt1;
         }

         NumIterations++;

         h = min(kk+1, m);
   
         if ((delta-8*red_fudge)*c[jj] > cbar) {

            clean = 0;

            // we treat the case that the new vector is b_s (jj < s <= kk)
            // as a special case that appears to occur most of the time.
   
            s = 0;
            for (i = jj+1; i <= kk; i++) {
               if (uvec[i] != 0) {
                  if (s == 0)
                     s = i;
                  else
                     s = -1;
               }
            }
   
            if (s == 0) LogicError("BKZ_XD: internal error");
   
            if (s > 0) {
               // special case

               NumTrivial++;
   
               for (i = s; i > jj; i--) {
                  // swap i, i-1
                  swap(B(i-1), B(i));
                  if (U) swap((*U)(i-1), (*U)(i));
                  tp = B1[i-1]; B1[i-1] = B1[i]; B1[i] = tp;
                  t1 = b[i-1]; b[i-1] = b[i]; b[i] = t1;
               }
   
               // cerr << "special case\n";
               new_m = ll_LLL_XD(B, U, delta, 0, check, 
                                B1, mu, b, c, h, jj, quit);
               if (new_m != h) LogicError("BKZ_XD: internal error");
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
                  if (uvec[i] == 0) continue;
                  conv(MU, uvec[i]);
                  RowTransform2(B(m+1), B(i), MU);
                  if (U) RowTransform2((*U)(m+1), (*U)(i), MU);
               }
      
               for (i = m+1; i >= jj+1; i--) {
                  // swap i, i-1
                  swap(B(i-1), B(i));
                  if (U) swap((*U)(i-1), (*U)(i));
                  tp = B1[i-1]; B1[i-1] = B1[i]; B1[i] = tp;
                  t1 = b[i-1]; b[i-1] = b[i]; b[i] = t1;
               }
      
               for (i = 1; i <= n; i++)
                  conv(B1[jj][i], B(jj, i));
      
               b[jj] = InnerProduct(B1[jj], B1[jj], n);
      
               if (b[jj] == 0) LogicError("BKZ_XD: internal error"); 
      
               // remove linear dependencies
   
               // cerr << "general case\n";
               new_m = ll_LLL_XD(B, U, delta, 0, 0, B1, mu, b, c, kk+1, jj, quit);
              
               if (new_m != kk) LogicError("BKZ_XD: internal error"); 

               // remove zero vector
      
               for (i = kk+2; i <= m+1; i++) {
                  // swap i, i-1
                  swap(B(i-1), B(i));
                  if (U) swap((*U)(i-1), (*U)(i));
                  tp = B1[i-1]; B1[i-1] = B1[i]; B1[i] = tp;
                  t1 = b[i-1]; b[i-1] = b[i]; b[i] = t1;
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
   
                  new_m = ll_LLL_XD(B, U, delta, 0, check, 
                                   B1, mu, b, c, h, h, quit);
   
                  if (new_m != h) LogicError("BKZ_XD: internal error");
                  if (quit) break;
               }
            }
   
            z = 0;
         }
         else {
            // LLL_XD
            // cerr << "progress\n";

            NumNoOps++;

            if (!clean) {
               new_m = 
                  ll_LLL_XD(B, U, delta, 0, check, B1, mu, b, c, h, h, quit);
               if (new_m != h) LogicError("BKZ_XD: internal error");
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

long BKZ_XD(mat_ZZ& BB, mat_ZZ& UU, double delta, 
         long beta, long prune, LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }


   if (delta < 0.50 || delta >= 1) LogicError("BKZ_XD: bad delta");
   if (beta < 2) LogicError("BKZ_XD: bad block size");

   return BKZ_XD(BB, &UU, to_xdouble(delta), beta, prune, check);
}

long BKZ_XD(mat_ZZ& BB, double delta, 
         long beta, long prune, LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }



   if (delta < 0.50 || delta >= 1) LogicError("BKZ_XD: bad delta");
   if (beta < 2) LogicError("BKZ_XD: bad block size");

   return BKZ_XD(BB, 0, to_xdouble(delta), beta, prune, check);
}

NTL_END_IMPL

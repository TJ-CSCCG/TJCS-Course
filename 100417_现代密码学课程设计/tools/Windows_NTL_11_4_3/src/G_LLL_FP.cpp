
#include <NTL/LLL.h>
#include <NTL/fileio.h>
#include <NTL/vec_double.h>



NTL_START_IMPL

static inline
void CheckFinite(double *p)
{
   if (!IsFinite(p)) ResourceError("G_LLL_FP: numbers too big...use G_LLL_XD");
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



#define TR_BND (NTL_FDOUBLE_PRECISION/2.0)
// Just to be safe!!

static double max_abs(double *v, long n)
{
   long i;
   double res, t;

   res = 0;

   for (i = 1; i <= n; i++) {
      t = fabs(v[i]);
      if (t > res) res = t;
   }

   return res;
}


static void RowTransformStart(double *a, long *in_a, long& in_float, long n)
{
   long i;
   long inf = 1;

   for (i = 1; i <= n; i++) {
      in_a[i] = (a[i] < TR_BND && a[i] > -TR_BND);
      inf = inf & in_a[i];
   }

   in_float = inf;
}


static void RowTransformFinish(vec_ZZ& A, double *a, long *in_a)
{
   long n = A.length();
   long i;

   for (i = 1; i <= n; i++) {
      if (in_a[i])  {
         conv(A(i), a[i]);
      }
      else {
         conv(a[i], A(i));
         CheckFinite(&a[i]);
      }
   }
}


static void RowTransform(vec_ZZ& A, vec_ZZ& B, const ZZ& MU1, 
                         double *a, double *b, long *in_a,
                         double& max_a, double max_b, long& in_float)
// x = x - y*MU
{
   NTL_ZZRegister(T);
   NTL_ZZRegister(MU);
   long k;
   double mu;

   conv(mu, MU1);
   CheckFinite(&mu);

   long n = A.length();
   long i;

   if (in_float) {
      double mu_abs = fabs(mu);
      if (mu_abs > 0 && max_b > 0 && (mu_abs >= TR_BND || max_b >= TR_BND)) {
         in_float = 0;
      }
      else {
         max_a += mu_abs*max_b;
         if (max_a >= TR_BND)
            in_float = 0;
      }
   }

   if (in_float) {
      if (mu == 1) {
         for (i = 1; i <= n; i++)
            a[i] -= b[i];

         return;
      }

      if (mu == -1) {
         for (i = 1; i <= n; i++)
            a[i] += b[i];

         return;
      }

      if (mu == 0) return;

      for (i = 1; i <= n; i++)
         a[i] -= mu*b[i];


      return;
   }


   MU = MU1;

   if (MU == 1) {
      for (i = 1; i <= n; i++) {
         if (in_a[i] && a[i] < TR_BND && a[i] > -TR_BND &&
             b[i] < TR_BND && b[i] > -TR_BND) {

            a[i] -= b[i];
         }
         else {
            if (in_a[i]) {
               conv(A(i), a[i]);
               in_a[i] = 0;
            }
         
            sub(A(i), A(i), B(i));
         }
      }
      return;
   }

   if (MU == -1) {
      for (i = 1; i <= n; i++) {
         if (in_a[i] && a[i] < TR_BND && a[i] > -TR_BND &&
             b[i] < TR_BND && b[i] > -TR_BND) {

            a[i] += b[i];
         }
         else {
            if (in_a[i]) {
               conv(A(i), a[i]);
               in_a[i] = 0;
            }
         
            add(A(i), A(i), B(i));
         }
      }
      return;
   }

   if (MU == 0) return;

   double b_bnd = fabs(TR_BND/mu) - 1;
   if (b_bnd < 0) b_bnd = 0; 

   if (NumTwos(MU) >= NTL_ZZ_NBITS) 
      k = MakeOdd(MU);
   else
      k = 0;


   if (MU.WideSinglePrecision()) {
      long mu1;
      conv(mu1, MU);

      if (k > 0) {
         for (i = 1; i <= n; i++) {
            if (in_a[i]) {
               conv(A(i), a[i]);
               in_a[i] = 0;
            }

            mul(T, B(i), mu1);
            LeftShift(T, T, k);
            sub(A(i), A(i), T);
         }
      }
      else {
         for (i = 1; i <= n; i++) {
            if (in_a[i] && a[i] < TR_BND && a[i] > -TR_BND &&
                b[i] < b_bnd && b[i] > -b_bnd) {
   
               a[i] -= b[i]*mu;
            }
            else {
               if (in_a[i]) {
                  conv(A(i), a[i]);
                  in_a[i] = 0;
               }
               MulSubFrom(A(i), B(i), mu1);
            }
         }
      }
   }
   else {
      for (i = 1; i <= n; i++) {
         if (in_a[i]) {
            conv(A(i), a[i]);
            in_a[i] = 0;
         }
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



class GivensCache_FP {
public:
   GivensCache_FP(long m, long n);

   void flush();
   void selective_flush(long l);
   void swap(long l);
   void swap();
   void touch();
   void incr();

   long sz;

   Unique2DArray<double> buf;
   UniqueArray<long> bl;
   UniqueArray<long> bv;

   long bp;
};


GivensCache_FP::GivensCache_FP(long m, long n)
{
   sz = min(m, n)/10; 
   if (sz < 2) 
      sz = 2;
   else if (sz > 20)
      sz = 20;


   buf.SetDims(sz, n+1);
   bl.SetLength(sz);
   bv.SetLength(sz);

   long i;
   for (i = 0; i < sz; i++) bl[i] = 0;
   for (i = 0; i < sz; i++) bv[i] = 0;

   bp = 0;
}

void GivensCache_FP::flush()
{
   long i;
   for (i = 0; i < sz; i++) bl[i] = 0;
}

void GivensCache_FP::selective_flush(long l)
{
   long i;

   for (i = 0; i < sz; i++)
      if (bl[i] && bv[i] >= l)
         bl[i] = 0;
}

void GivensCache_FP::swap(long l)
{
   long k = bl[bp];
   long i;

   i = 0;
   while (i < sz && bl[i] != l)
      i++;

   if (i < sz) {
      bl[bp] = l;
      bl[i] = k;
   }
   else
      bl[bp] = l;

   selective_flush(l);
}

void GivensCache_FP::swap()
{
   swap(bl[bp] - 1);
}

void GivensCache_FP::touch()
{
   long k = bl[bp];
   bl[bp] = 0;
   selective_flush(k);
}

void GivensCache_FP::incr()
{
   long k = bl[bp];
   long k1 = k+1;
   long i;

   i = 0;
   while (i < sz && bl[i] != k1)
      i++;

   if (i < sz) {
      bp = i;
      return;
   }

   i = 0; 
   while (i < sz && bl[i] != 0)
      i++;

   if (i < sz) {
      bp = i;
      return;
   }

   long max_val = 0;
   long max_index = 0;
   for (i = 0; i < sz; i++) {
      long t = labs(bl[i]-k1);
      if (t > max_val) {
         max_val = t;
         max_index = i;
      }
   }

   bp = max_index;
   bl[max_index] = 0;
}


static
void GivensComputeGS(double **B1, double **mu, double **aux, long k, long n,
                     GivensCache_FP& cache)
{
   long i, j;

   double c, s, a, b, t;

   double *p = mu[k];

   double *pp = cache.buf[cache.bp];

   if (!cache.bl[cache.bp]) {
      for (j = 1; j <= n; j++)
         pp[j] = B1[k][j];

      long backoff;
      backoff = k/4;
      if (backoff < 2)
         backoff = 2;
      else if (backoff > cache.sz + 2)
         backoff = cache.sz + 2; 

      long ub = k-(backoff-1);

      for (i = 1; i < ub; i++) {
         double *cptr = mu[i];
         double *sptr = aux[i];
   
         for (j = n; j > i; j--) {
            c = cptr[j];
            s = sptr[j];
   
            a = c*pp[j-1] - s*pp[j];
            b = s*pp[j-1] + c*pp[j];
   
            pp[j-1] = a;
            pp[j] = b;
         }
   
         pp[i] = pp[i]/mu[i][i]; 
      }

      cache.bl[cache.bp] = k;
      cache.bv[cache.bp] = k-backoff;
   }

   for (j = 1; j <= n; j++)
      p[j] = pp[j];

   for (i = max(cache.bv[cache.bp]+1, 1); i < k; i++) {
      double *cptr = mu[i];
      double *sptr = aux[i];
  
      for (j = n; j > i; j--) {
         c = cptr[j];
         s = sptr[j];
  
         a = c*p[j-1] - s*p[j];
         b = s*p[j-1] + c*p[j];
  
         p[j-1] = a;
         p[j] = b;
      }
  
      p[i] = p[i]/mu[i][i];
   }

   for (j = n; j > k; j--) {
      a = p[j-1];
      b = p[j];

      if (b == 0) {
         c = 1;
         s = 0;
      }
      else if (fabs(b) > fabs(a)) {
         t = -a/b;
         s = 1/sqrt(1 + t*t);
         c = s*t;
      }
      else {
         t = -b/a;
         c = 1/sqrt(1 + t*t);
         s = c*t;
      }
   
      p[j-1] = c*a - s*b;
      p[j] = c;
      aux[k][j] = s;
   }

   if (k > n+1) LogicError("G_LLL_FP: internal error");
   if (k > n) p[k] = 0;

   for (i = 1; i <= k; i++)
      CheckFinite(&p[i]);
}

static NTL_CHEAP_THREAD_LOCAL double red_fudge = 0;
static NTL_CHEAP_THREAD_LOCAL long log_red = 0;
static NTL_CHEAP_THREAD_LOCAL long verbose = 0;
static NTL_CHEAP_THREAD_LOCAL unsigned long NumSwaps = 0;
static NTL_CHEAP_THREAD_LOCAL double StartTime = 0;
static NTL_CHEAP_THREAD_LOCAL double LastTime = 0;



static void G_LLLStatus(long max_k, double t, long m, const mat_ZZ& B)
{
   cerr << "---- G_LLL_FP status ----\n";
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

static void init_red_fudge()
{
   long i;

   log_red = long(0.50*NTL_DOUBLE_PRECISION);
   red_fudge = 1;

   for (i = log_red; i > 0; i--)
      red_fudge = red_fudge*0.5;
}

static void inc_red_fudge()
{

   red_fudge = red_fudge * 2;
   log_red--;

   
   cerr << "G_LLL_FP: warning--relaxing reduction (" << log_red << ")\n";

   if (log_red < 4)
      ResourceError("G_LLL_FP: too much loss of precision...stop!");
}


#if 0

static void print_mus(double **mu, long k)
{
   long i;

   for (i = k-1; i >= 1; i--)
      cerr << mu[k][i] << " ";
   cerr << "\n";
}

#endif



static
long ll_G_LLL_FP(mat_ZZ& B, mat_ZZ* U, double delta, long deep, 
           LLLCheckFct check, double **B1, double **mu, 
           double **aux,
           long m, long init_k, long &quit, GivensCache_FP& cache)
{
   long n = B.NumCols();

   long i, j, k, Fc1;
   ZZ MU;
   double mu1;

   double t1;
   ZZ T1;
   double *tp;

   double half_plus_fudge = 0.5 + red_fudge;

   quit = 0;
   k = init_k;

   vec_long in_vec_mem;
   in_vec_mem.SetLength(n+1);
   long *in_vec = in_vec_mem.elts();

   UniqueArray<double> max_b_store;
   max_b_store.SetLength(m+1);
   double *max_b = max_b_store.get();

   for (i = 1; i <= m; i++)
      max_b[i] = max_abs(B1[i], n);

   long in_float;

   long counter;

   long trigger_index;
   long small_trigger;
   long cnt;


   long max_k = 0;

   double tt;

   long swap_cnt = 0;

   cache.flush();

   while (k <= m) {

      if (k > max_k) {
         max_k = k;
         swap_cnt = 0;
      }

      if (verbose) {
         tt = GetTime();

         if (tt > LastTime + LLLStatusInterval)
            G_LLLStatus(max_k, tt, m, B);
      }

      GivensComputeGS(B1, mu, aux, k, n, cache);

      if (swap_cnt > 200000) {
         cerr << "G_LLL_FP: swap loop?\n";
         swap_cnt = 0;
      }

      counter = 0;
      trigger_index = k;
      small_trigger = 0;
      cnt = 0;

      long sz=0, new_sz;

      do {
         // size reduction

         counter++;
         if ((counter & 127) == 0) {

            new_sz = 0;
            for (j = 1; j <= n; j++)
               new_sz += NumBits(B(k,j));

            if ((counter >> 7) == 1 || new_sz < sz) {
               sz = new_sz;
            }
            else {
               cerr << "G_LLL_FP: warning--infinite loop? (" << k << ")\n";
            }
         }

         Fc1 = 0;
   
         for (j = k-1; j >= 1; j--) {
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

                  Fc1 = 1;
                  RowTransformStart(B1[k], in_vec, in_float, n);
               }
                  

               mu1 = mu[k][j];
               if (mu1 >= 0)
                  mu1 = ceil(mu1-0.5);
               else
                  mu1 = floor(mu1+0.5);
   
               double *mu_k = mu[k];
               double *mu_j = mu[j];
   
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
                     mu_k[i] -= mu1*mu_j[i];
               }
   
               mu_k[j] -= mu1;
   
               conv(MU, mu1);

               RowTransform(B(k), B(j), MU, B1[k], B1[j], in_vec,
                            max_b[k], max_b[j], in_float);
               if (U) RowTransform((*U)(k), (*U)(j), MU);
            }
         }


         if (Fc1) {
            RowTransformFinish(B(k), B1[k], in_vec);
            max_b[k] = max_abs(B1[k], n);
            cache.touch();
            GivensComputeGS(B1, mu, aux, k, n, cache);
         }
      } while (Fc1);

      if (check && (*check)(B(k))) 
         quit = 1;

      if (IsZero(B(k))) {
         for (i = k; i < m; i++) {
            // swap i, i+1
            swap(B(i), B(i+1));
            tp = B1[i]; B1[i] = B1[i+1]; B1[i+1] = tp;
            t1 = max_b[i]; max_b[i] = max_b[i+1]; max_b[i+1] = t1;
            if (U) swap((*U)(i), (*U)(i+1));
         }

         cache.flush();

         m--;
         if (quit) break;
         continue;
      }

      if (quit) break;

      if (deep > 0) {
         // deep insertions

         LogicError("sorry...deep insertions not implemented");
      } // end deep insertions

      // test G_LLL reduction condition

      if (k > 1 && 
         sqrt(delta - mu[k][k-1]*mu[k][k-1])*fabs(mu[k-1][k-1]) > 
         fabs(mu[k][k])) {
         // swap rows k, k-1

         swap(B(k), B(k-1));
         tp = B1[k]; B1[k] = B1[k-1]; B1[k-1] = tp;
         t1 = max_b[k]; max_b[k] = max_b[k-1]; max_b[k-1] = t1;
         if (U) swap((*U)(k), (*U)(k-1));

         cache.swap();

         k--;
         NumSwaps++;
         swap_cnt++;
         // cout << "-\n";
      }
      else {

         cache.incr();

         k++;
         // cout << "+\n";
      }

   }

   if (verbose) {
      G_LLLStatus(m+1, GetTime(), m, B);
   }


   return m;
}





static
long G_LLL_FP(mat_ZZ& B, mat_ZZ* U, double delta, long deep, 
           LLLCheckFct check)
{
   long m = B.NumRows();
   long n = B.NumCols();

   long i, j;
   long new_m, dep, quit;
   ZZ MU;

   ZZ T1;

   init_red_fudge();

   if (U) ident(*U, m);

   Unique2DArray<double> B1_store;
   B1_store.SetDimsFrom1(m+1, n+1);
   double **B1 = B1_store.get();  // approximates B

   Unique2DArray<double> mu_store;
   mu_store.SetDimsFrom1(m+1, n+2);
   double **mu = mu_store.get();

   Unique2DArray<double> aux_store;
   aux_store.SetDimsFrom1(m+1, n+1);
   double **aux = aux_store.get();

   for (i = 1; i <=m; i++)
      for (j = 1; j <= n; j++) {
         conv(B1[i][j], B(i, j));
         CheckFinite(&B1[i][j]);
      }

         
   GivensCache_FP cache(m, n);

   new_m = ll_G_LLL_FP(B, U, delta, deep, check, B1, mu, aux, m, 1, quit, cache);
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

         

long G_LLL_FP(mat_ZZ& B, double delta, long deep, LLLCheckFct check, 
           long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("G_LLL_FP: bad delta");
   if (deep < 0) LogicError("G_LLL_FP: bad deep");
   return G_LLL_FP(B, 0, delta, deep, check);
}

long G_LLL_FP(mat_ZZ& B, mat_ZZ& U, double delta, long deep, 
           LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("G_LLL_FP: bad delta");
   if (deep < 0) LogicError("G_LLL_FP: bad deep");
   return G_LLL_FP(B, &U, delta, deep, check);
}



NTL_TLS_GLOBAL_DECL(vec_double, G_BKZConstant)

static
void ComputeG_BKZConstant(long beta, long p)
{
   NTL_TLS_GLOBAL_ACCESS(G_BKZConstant);

   const double c_PI = 3.14159265358979323846264338328;
   const double LogPI = 1.14472988584940017414342735135;

   G_BKZConstant.SetLength(beta-1);

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

      G_BKZConstant(i) = x*y/c_PI;
   }
}

NTL_TLS_GLOBAL_DECL(vec_double, G_BKZThresh)

static 
void ComputeG_BKZThresh(double *c, long beta)
{
   NTL_TLS_GLOBAL_ACCESS(G_BKZConstant);
   NTL_TLS_GLOBAL_ACCESS(G_BKZThresh);

   G_BKZThresh.SetLength(beta-1);

   long i;
   double x;

   x = 0;

   for (i = 1; i <= beta-1; i++) {
      x += log(c[i-1]);
      G_BKZThresh(i) = exp(x/double(i))*G_BKZConstant(i);
      if (!IsFinite(&G_BKZThresh(i))) G_BKZThresh(i) = 0;
   }
}

static 
void G_BKZStatus(double tt, double enum_time, unsigned long NumIterations, 
               unsigned long NumTrivial, unsigned long NumNonTrivial, 
               unsigned long NumNoOps, long m, 
               const mat_ZZ& B)
{
   cerr << "---- G_BKZ_FP status ----\n";
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
long G_BKZ_FP(mat_ZZ& BB, mat_ZZ* UU, double delta, 
         long beta, long prune, LLLCheckFct check)
{
   NTL_TLS_GLOBAL_ACCESS(G_BKZThresh);


   

   long m = BB.NumRows();
   long n = BB.NumCols();
   long m_orig = m;
   
   long i, j;
   ZZ MU;

   double t1;
   ZZ T1;
   double *tp;

   init_red_fudge();

   mat_ZZ B;
   B = BB;

   B.SetDims(m+1, n);

   Unique2DArray<double> B1_store;
   B1_store.SetDimsFrom1(m+2, n+1);
   double **B1 = B1_store.get();  // approximates B

   Unique2DArray<double> mu_store;
   mu_store.SetDimsFrom1(m+2, n+2);
   double **mu = mu_store.get();

   Unique2DArray<double> aux_store;
   aux_store.SetDimsFrom1(m+2, n+1);
   double **aux = aux_store.get();

   UniqueArray<double> c_store;
   c_store.SetLength(m+2);
   double *c = c_store.get(); // squared lengths of Gramm-Schmidt basis vectors


   double cbar;

   UniqueArray<double> ctilda_store;
   ctilda_store.SetLength(m+2);
   double *ctilda = ctilda_store.get();

   UniqueArray<double> vvec_store;
   vvec_store.SetLength(m+2);
   double *vvec = vvec_store.get();


   UniqueArray<double> yvec_store;
   yvec_store.SetLength(m+2);
   double *yvec = yvec_store.get();

   UniqueArray<double> uvec_store;
   uvec_store.SetLength(m+2);
   double *uvec = uvec_store.get();

   UniqueArray<double> utildavec_store;
   utildavec_store.SetLength(m+2);
   double *utildavec = utildavec_store.get();


   UniqueArray<long> Deltavec_store;
   Deltavec_store.SetLength(m+2);
   long *Deltavec = Deltavec_store.get();

   UniqueArray<long> deltavec_store;
   deltavec_store.SetLength(m+2);
   long *deltavec = deltavec_store.get();


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
   double eta;


   for (i = 1; i <=m; i++)
      for (j = 1; j <= n; j++) {
         conv(B1[i][j], B(i, j));
         CheckFinite(&B1[i][j]);
      }

         
   GivensCache_FP cache(m, n);

   m = ll_G_LLL_FP(B, U, delta, 0, check, B1, mu, aux, m, 1, quit, cache);

   double tt;

   double enum_time = 0;
   unsigned long NumIterations = 0;
   unsigned long NumTrivial = 0;
   unsigned long NumNonTrivial = 0;
   unsigned long NumNoOps = 0;

   long verb = verbose;

   verbose = 0;

   long clean = 1;

   if (m < m_orig) {
      for (i = m_orig+1; i >= m+2; i--) {
         // swap i, i-1

         swap(B(i), B(i-1));
         if (U) swap((*U)(i), (*U)(i-1));
      }
   }

   if (!quit && m > 1) {
      if (beta > m) beta = m;

      if (prune > 0) 
         ComputeG_BKZConstant(beta, prune);

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
               G_BKZStatus(tt, enum_time, NumIterations, NumTrivial,
                         NumNonTrivial, NumNoOps, m, B);
         }

   
         // ENUM

         double tt1;

         if (verb) {
            tt1 = GetTime();
         }

         for (i = jj; i <= kk; i++) {
            c[i] = mu[i][i]*mu[i][i];
            CheckFinite(&c[i]);
         }

         if (prune > 0)
            ComputeG_BKZThresh(&c[jj], kk-jj+1);
   
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
                     G_BKZStatus(tt, enum_time, NumIterations, NumTrivial,
                               NumNonTrivial, NumNoOps, m, B);
                  }
               }
            }

            ctilda[t] = ctilda[t+1] + 
               (yvec[t]+utildavec[t])*(yvec[t]+utildavec[t])*c[t];

            ForceToMem(&ctilda[t]); // prevents an infinite loop
   
            if (prune > 0 && t > jj) {
               eta = G_BKZThresh(t-jj);
            }
            else
               eta = 0;
   
            if (ctilda[t] < cbar - eta) {
               if (t > jj) {
                  t--;
                  t1 = 0;
                  for (i = t+1; i <= s; i++)
                     t1 += utildavec[i]*mu[i][t];
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
   
         if ((delta - 8*red_fudge)*c[jj] > cbar) {

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
   
            if (s == 0) LogicError("G_BKZ_FP: internal error");
   
            if (s > 0) {
               // special case

               NumTrivial++;
   
               for (i = s; i > jj; i--) {
                  // swap i, i-1
                  swap(B(i-1), B(i));
                  if (U) swap((*U)(i-1), (*U)(i));
                  tp = B1[i-1]; B1[i-1] = B1[i]; B1[i] = tp;
               }
   
               // cerr << "special case\n";
               new_m = ll_G_LLL_FP(B, U, delta, 0, check, 
                                B1, mu, aux, h, jj, quit, cache);
               if (new_m != h) LogicError("G_BKZ_FP: internal error");
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
               }
      
               for (i = 1; i <= n; i++) {
                  conv(B1[jj][i], B(jj, i));
                  CheckFinite(&B1[jj][i]);
               }

               if (IsZero(B(jj))) LogicError("G_BKZ_FP: internal error");
      
               // remove linear dependencies
   
               // cerr << "general case\n";
               new_m = ll_G_LLL_FP(B, U, delta, 0, 0, B1, mu, aux, 
                                  kk+1, jj, quit, cache);
              
               if (new_m != kk) LogicError("G_BKZ_FP: internal error"); 

               // remove zero vector
      
               for (i = kk+2; i <= m+1; i++) {
                  // swap i, i-1
                  swap(B(i-1), B(i));
                  if (U) swap((*U)(i-1), (*U)(i));
                  tp = B1[i-1]; B1[i-1] = B1[i]; B1[i] = tp;
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
   
                  new_m = ll_G_LLL_FP(B, U, delta, 0, check, 
                                   B1, mu, aux, h, h, quit, cache);
   
                  if (new_m != h) LogicError("G_BKZ_FP: internal error");
                  if (quit) break;
               }
            }
   
            z = 0;
         }
         else {
            // G_LLL_FP
            // cerr << "progress\n";

            NumNoOps++;

            if (!clean) {
               new_m = ll_G_LLL_FP(B, U, delta, 0, check, B1, mu, aux, 
                                   h, h, quit, cache);
               if (new_m != h) LogicError("G_BKZ_FP: internal error");
               if (quit) break;
            }
   
            z++;
         }
      }
   }


   if (verb) {
      G_BKZStatus(GetTime(), enum_time, NumIterations, NumTrivial, NumNonTrivial, 
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

long G_BKZ_FP(mat_ZZ& BB, mat_ZZ& UU, double delta, 
         long beta, long prune, LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("G_BKZ_FP: bad delta");
   if (beta < 2) LogicError("G_BKZ_FP: bad block size");

   return G_BKZ_FP(BB, &UU, delta, beta, prune, check);
}

long G_BKZ_FP(mat_ZZ& BB, double delta, 
         long beta, long prune, LLLCheckFct check, long verb)
{
   verbose = verb;
   NumSwaps = 0;
   if (verbose) {
      StartTime = GetTime();
      LastTime = StartTime;
   }

   if (delta < 0.50 || delta >= 1) LogicError("G_BKZ_FP: bad delta");
   if (beta < 2) LogicError("G_BKZ_FP: bad block size");

   return G_BKZ_FP(BB, 0, delta, beta, prune, check);
}

NTL_END_IMPL

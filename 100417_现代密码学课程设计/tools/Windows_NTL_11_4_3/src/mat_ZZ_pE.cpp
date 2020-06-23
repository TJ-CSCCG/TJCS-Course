#include <NTL/mat_ZZ_pE.h>
#include <NTL/BasicThreadPool.h>


NTL_START_IMPL


//===================================================

#define PAR_THRESH (40000.0)

static double
ZZ_pE_SizeInWords()
{
   return double(deg(ZZ_pE::modulus()))*double(ZZ_p::ModulusSize());
}


static
void mul_aux(Mat<ZZ_pE>& X, const Mat<ZZ_pE>& A, const Mat<ZZ_pE>& B)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
   long m = B.NumCols();  
  
   if (l != B.NumRows())  
      LogicError("matrix mul: dimension mismatch");  
  
   X.SetDims(n, m);  


   ZZ_pContext ZZ_p_context;
   ZZ_p_context.save();
   ZZ_pEContext ZZ_pE_context;
   ZZ_pE_context.save();
   double sz = ZZ_pE_SizeInWords();

   bool seq = (double(n)*double(l)*double(m)*sz*sz < PAR_THRESH);

   NTL_GEXEC_RANGE(seq, m, first, last)
   NTL_IMPORT(n)
   NTL_IMPORT(l)
   NTL_IMPORT(m)

   ZZ_p_context.restore();
   ZZ_pE_context.restore();

   long i, j, k;  
   ZZ_pX acc, tmp;  

   Vec<ZZ_pE> B_col;
   B_col.SetLength(l);

   for (j = first; j < last; j++) {
      for (k = 0; k < l; k++) B_col[k] = B[k][j];

      for (i = 0; i < n; i++) {
         clear(acc);
         for (k = 0; k < l; k++) {
            mul(tmp, rep(A[i][k]), rep(B_col[k]));
            add(acc, acc, tmp);
         }
         conv(X[i][j], acc);
      }
   }

   NTL_GEXEC_RANGE_END
}  


void mul(mat_ZZ_pE& X, const mat_ZZ_pE& A, const mat_ZZ_pE& B)  
{  
   if (&X == &A || &X == &B) {  
      mat_ZZ_pE tmp;  
      mul_aux(tmp, A, B);  
      X = tmp;  
   }  
   else  
      mul_aux(X, A, B);  
}  
  

void inv(ZZ_pE& d, Mat<ZZ_pE>& X, const Mat<ZZ_pE>& A)
{
   long n = A.NumRows();

   if (A.NumCols() != n)
      LogicError("inv: nonsquare matrix");

   if (n == 0) {
      set(d);
      X.SetDims(0, 0);
      return;
   }

   const ZZ_pXModulus& G = ZZ_pE::modulus();

   ZZ_pX t1, t2;
   ZZ_pX pivot;
   ZZ_pX pivot_inv;

   Vec< Vec<ZZ_pX> > M;
   // scratch space

   M.SetLength(n);
   for (long i = 0; i < n; i++) {
      M[i].SetLength(n);
      for (long j = 0; j < n; j++) {
         M[i][j].SetMaxLength(2*deg(G)-1);
         M[i][j] = rep(A[i][j]);
      }
   }

   ZZ_pX det;
   det = 1;


   Vec<long> P;
   P.SetLength(n);
   for (long k = 0; k < n; k++) P[k] = k;
   // records swap operations
   

   ZZ_pContext ZZ_p_context;
   ZZ_p_context.save();
   double sz = ZZ_pE_SizeInWords();

   bool seq = double(n)*double(n)*sz*sz < PAR_THRESH;

   bool pivoting = false;

   for (long k = 0; k < n; k++) {

      long pos = -1;

      for (long i = k; i < n; i++) {
         rem(pivot, M[i][k], G);
         if (pivot != 0) {
            InvMod(pivot_inv, pivot, G);
            pos = i;
            break;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            negate(det, det); 
            P[k] = pos;
            pivoting = true;
         }

         MulMod(det, det, pivot, G);

         {
            // multiply row k by pivot_inv
            ZZ_pX *y = &M[k][0];
            for (long j = 0; j < n; j++) {
               rem(t2, y[j], G);
               MulMod(y[j], t2, pivot_inv, G);
            }
            y[k] = pivot_inv;
         }


         NTL_GEXEC_RANGE(seq, n, first, last)  
         NTL_IMPORT(n)
         NTL_IMPORT(k)

         ZZ_p_context.restore();

         ZZ_pX *y = &M[k][0]; 
         ZZ_pX t1, t2;

         for (long i = first; i < last; i++) {
            if (i == k) continue; // skip row k

            ZZ_pX *x = &M[i][0]; 
            rem(t1, x[k], G);
            negate(t1, t1); 
            x[k] = 0;
            if (t1 == 0) continue;

            // add t1 * row k to row i
            for (long j = 0; j < n; j++) {
               mul(t2, y[j], t1);
               add(x[j], x[j], t2);
            }
         }
         NTL_GEXEC_RANGE_END
      }
      else {
         clear(d);
         return;
      }
   }

   if (pivoting) {
      // pivot colums, using reverse swap sequence

      for (long i = 0; i < n; i++) {
         ZZ_pX *x = &M[i][0]; 

         for (long k = n-1; k >= 0; k--) {
            long pos = P[k];
            if (pos != k) swap(x[pos], x[k]);
         }
      }
   }

   X.SetDims(n, n);
   for (long i = 0; i < n; i++)
      for (long j = 0; j < n; j++)
         conv(X[i][j], M[i][j]);

   conv(d, det);
}

static
void solve_impl(ZZ_pE& d, Vec<ZZ_pE>& X, 
                const Mat<ZZ_pE>& A, const Vec<ZZ_pE>& b, bool trans)

{
   long n = A.NumRows();
   if (A.NumCols() != n)
      LogicError("solve: nonsquare matrix");

   if (b.length() != n)
      LogicError("solve: dimension mismatch");

   if (n == 0) {
      set(d);
      X.SetLength(0);
      return;
   }

   ZZ_pX t1, t2;

   const ZZ_pXModulus& G = ZZ_pE::modulus();

   Vec< Vec<ZZ_pX> > M;

   M.SetLength(n);

   for (long i = 0; i < n; i++) {
      M[i].SetLength(n+1);
      for (long j = 0; j < n; j++) M[i][j].SetMaxLength(2*deg(G)-1);

      if (trans) 
         for (long j = 0; j < n; j++) M[i][j] = rep(A[j][i]);
      else
         for (long j = 0; j < n; j++) M[i][j] = rep(A[i][j]);

      M[i][n] = rep(b[i]);
   }

   ZZ_pX det;
   set(det);

   ZZ_pContext ZZ_p_context;
   ZZ_p_context.save();
   double sz = ZZ_pE_SizeInWords();

   for (long k = 0; k < n; k++) {
      long pos = -1;
      for (long i = k; i < n; i++) {
         rem(t1, M[i][k], G);
         M[i][k] = t1;
         if (pos == -1 && !IsZero(t1)) {
            pos = i;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            negate(det, det); 
         }

         MulMod(det, det, M[k][k], G);

         // make M[k, k] == -1 mod G, and make row k reduced

         InvMod(t1, M[k][k], G);
         negate(t1, t1); 
         for (long j = k+1; j <= n; j++) {
            rem(t2, M[k][j], G);
            MulMod(M[k][j], t2, t1, G);
         }

         bool seq =
            double(n-(k+1))*(n-(k+1))*sz*sz < PAR_THRESH;

         NTL_GEXEC_RANGE(seq, n-(k+1), first, last)
         NTL_IMPORT(n)
         NTL_IMPORT(k)

         ZZ_p_context.restore();

         ZZ_pX t1, t2;

         for (long ii = first; ii < last; ii++) {
            long i = ii + k+1;

            // M[i] = M[i] + M[k]*M[i,k]

            t1 = M[i][k];   // this is already reduced

            ZZ_pX *x = M[i].elts() + (k+1);
            ZZ_pX *y = M[k].elts() + (k+1);

            for (long j = k+1; j <= n; j++, x++, y++) {
               // *x = *x + (*y)*t1

               mul(t2, *y, t1);
               add(*x, *x, t2);
            }
         }

         NTL_GEXEC_RANGE_END

      }
      else {
         clear(d);
         return;
      }
   }

   X.SetLength(n);
   for (long i = n-1; i >= 0; i--) {
      clear(t1);
      for (long j = i+1; j < n; j++) {
         mul(t2, rep(X[j]), M[i][j]);
         add(t1, t1, t2);
      }
      sub(t1, t1, M[i][n]);
      conv(X[i], t1);
   }

   conv(d, det);
}


void solve(ZZ_pE& d, Vec<ZZ_pE>& x, 
               const Mat<ZZ_pE>& A, const Vec<ZZ_pE>& b)
{
   solve_impl(d, x, A, b, true);
}

void solve(ZZ_pE& d, const Mat<ZZ_pE>& A, 
               Vec<ZZ_pE>& x, const Vec<ZZ_pE>& b)
{
   solve_impl(d, x, A, b, false);
}



long gauss(Mat<ZZ_pE>& M_in, long w)
{
   ZZ_pX t1, t2;
   ZZ_pX piv;

   long n = M_in.NumRows();
   long m = M_in.NumCols();

   if (w < 0 || w > m)
      LogicError("gauss: bad args");

   const ZZ_pXModulus& G = ZZ_pE::modulus();

   Vec< Vec<ZZ_pX> > M;

   M.SetLength(n);
   for (long i = 0; i < n; i++) {
      M[i].SetLength(m);
      for (long j = 0; j < m; j++) {
         M[i][j].SetLength(2*deg(G)-1);
         M[i][j] = rep(M_in[i][j]);
      }
   }

   ZZ_pContext ZZ_p_context;
   ZZ_p_context.save();
   double sz = ZZ_pE_SizeInWords();

   long l = 0;
   for (long k = 0; k < w && l < n; k++) {

      long pos = -1;
      for (long i = l; i < n; i++) {
         rem(t1, M[i][k], G);
         M[i][k] = t1;
         if (pos == -1 && !IsZero(t1)) {
            pos = i;
         }
      }

      if (pos != -1) {
         swap(M[pos], M[l]);

         InvMod(piv, M[l][k], G);
         negate(piv, piv);

         for (long j = k+1; j < m; j++) {
            rem(M[l][j], M[l][j], G);
         }

         bool seq =
            double(n-(l+1))*double(m-(k+1))*sz*sz < PAR_THRESH;

         NTL_GEXEC_RANGE(seq, n-(l+1), first, last)
         NTL_IMPORT(m)
         NTL_IMPORT(k)
         NTL_IMPORT(l)

         ZZ_p_context.restore();

         ZZ_pX t1, t2;


         for (long ii = first; ii < last; ii++) {
            long i = ii + l+1;

            // M[i] = M[i] + M[l]*M[i,k]*piv

            MulMod(t1, M[i][k], piv, G);

            clear(M[i][k]);

            ZZ_pX *x = M[i].elts() + (k+1);
            ZZ_pX *y = M[l].elts() + (k+1);

            for (long j = k+1; j < m; j++, x++, y++) {
               // *x = *x + (*y)*t1

               mul(t2, *y, t1);
               add(t2, t2, *x);
               *x = t2;
            }
         }

         NTL_GEXEC_RANGE_END

         l++;
      }
   }
   
   for (long i = 0; i < n; i++)
      for (long j = 0; j < m; j++)
         conv(M_in[i][j], M[i][j]);

   return l;
}


long gauss(Mat<ZZ_pE>& M)
{
   return gauss(M, M.NumCols());
}

void image(Mat<ZZ_pE>& X, const Mat<ZZ_pE>& A)
{
   Mat<ZZ_pE> M;
   M = A;
   long r = gauss(M);
   M.SetDims(r, M.NumCols());
   X = M;
}



void kernel(Mat<ZZ_pE>& X, const Mat<ZZ_pE>& A)
{
   long m = A.NumRows();
   long n = A.NumCols();

   const ZZ_pXModulus& G = ZZ_pE::modulus();

   Mat<ZZ_pE> M;

   transpose(M, A);
   long r = gauss(M);

   if (r == 0) {
      ident(X, m);
      return;
   }

   X.SetDims(m-r, m);

   if (m-r == 0 || m == 0) return;


   Vec<long> D;
   D.SetLength(m);
   for (long j = 0; j < m; j++) D[j] = -1;

   Vec<ZZ_pE> inverses;
   inverses.SetLength(m);

   for (long i = 0, j = -1; i < r; i++) {
      do {
         j++;
      } while (IsZero(M[i][j]));

      D[j] = i;
      inv(inverses[j], M[i][j]); 
   }

   ZZ_pEContext ZZ_pE_context;
   ZZ_pE_context.save();
   ZZ_pContext ZZ_p_context;
   ZZ_p_context.save();
   double sz = ZZ_pE_SizeInWords();

   bool seq = 
      double(m-r)*double(r)*double(r)*sz*sz < PAR_THRESH;

   NTL_GEXEC_RANGE(seq, m-r, first, last)
   NTL_IMPORT(m)
   NTL_IMPORT(r)

   ZZ_p_context.restore();
   ZZ_pE_context.restore();

   ZZ_pX t1, t2;
   ZZ_pE T3;

   for (long k = first; k < last; k++) {
      Vec<ZZ_pE>& v = X[k];
      long pos = 0;
      for (long j = m-1; j >= 0; j--) {
         if (D[j] == -1) {
            if (pos == k)
               set(v[j]);
            else
               clear(v[j]);
            pos++;
         }
         else {
            long i = D[j];

            clear(t1);

            for (long s = j+1; s < m; s++) {
               mul(t2, rep(v[s]), rep(M[i][s]));
               add(t1, t1, t2);
            }

            conv(T3, t1);
            mul(T3, T3, inverses[j]);
            negate(v[j], T3); 
         }
      }
   }

   NTL_GEXEC_RANGE_END
}



void determinant(ZZ_pE& d, const Mat<ZZ_pE>& M_in)
{
   ZZ_pX t1, t2;

   const ZZ_pXModulus& G = ZZ_pE::modulus();

   long n = M_in.NumRows();

   if (M_in.NumCols() != n)
      LogicError("determinant: nonsquare matrix");

   if (n == 0) {
      set(d);
      return;
   }

   Vec< Vec<ZZ_pX> > M;

   M.SetLength(n);
   for (long i = 0; i < n; i++) {
      M[i].SetLength(n);
      for (long j = 0; j < n; j++) { 
         M[i][j].SetMaxLength(2*deg(G)-1);
         M[i][j] = rep(M_in[i][j]);
      }
   }

   ZZ_pX det;
   set(det);

   ZZ_pContext ZZ_p_context;
   ZZ_p_context.save();
   double sz = ZZ_pE_SizeInWords();

   for (long k = 0; k < n; k++) {
      long pos = -1;
      for (long i = k; i < n; i++) {
         rem(t1, M[i][k], G);
         M[i][k] = t1;
         if (pos == -1 && !IsZero(t1))
            pos = i;
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            negate(det, det);
         }

         MulMod(det, det, M[k][k], G);

         // make M[k, k] == -1 mod G, and make row k reduced

         InvMod(t1, M[k][k], G);
         negate(t1, t1);
         for (long j = k+1; j < n; j++) {
            rem(t2, M[k][j], G);
            MulMod(M[k][j], t2, t1, G);
         }


         bool seq =
            double(n-(k+1))*(n-(k+1))*sz*sz < PAR_THRESH;

         NTL_GEXEC_RANGE(seq, n-(k+1), first, last)
         NTL_IMPORT(n)
         NTL_IMPORT(k)

         ZZ_p_context.restore();

         ZZ_pX t1, t2;

         for (long ii = first; ii < last; ii++) {
            long i = ii + k+1;

            // M[i] = M[i] + M[k]*M[i,k]

            t1 = M[i][k];   // this is already reduced

            ZZ_pX *x = M[i].elts() + (k+1);
            ZZ_pX *y = M[k].elts() + (k+1);

            for (long j = k+1; j < n; j++, x++, y++) {
               // *x = *x + (*y)*t1

               mul(t2, *y, t1);
               add(*x, *x, t2);
            }
         }

         NTL_GEXEC_RANGE_END

      }
      else {
         clear(d);
         return;
      }
   }

   conv(d, det);
}



//===================================================

  
void add(mat_ZZ_pE& X, const mat_ZZ_pE& A, const mat_ZZ_pE& B)  
{  
   long n = A.NumRows();  
   long m = A.NumCols();  
  
   if (B.NumRows() != n || B.NumCols() != m)   
      LogicError("matrix add: dimension mismatch");  
  
   X.SetDims(n, m);  
  
   long i, j;  
   for (i = 1; i <= n; i++)   
      for (j = 1; j <= m; j++)  
         add(X(i,j), A(i,j), B(i,j));  
}  
  
void sub(mat_ZZ_pE& X, const mat_ZZ_pE& A, const mat_ZZ_pE& B)  
{  
   long n = A.NumRows();  
   long m = A.NumCols();  
  
   if (B.NumRows() != n || B.NumCols() != m)  
      LogicError("matrix sub: dimension mismatch");  
  
   X.SetDims(n, m);  
  
   long i, j;  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= m; j++)  
         sub(X(i,j), A(i,j), B(i,j));  
}  

void negate(mat_ZZ_pE& X, const mat_ZZ_pE& A)  
{  
   long n = A.NumRows();  
   long m = A.NumCols();  
  
  
   X.SetDims(n, m);  
  
   long i, j;  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= m; j++)  
         negate(X(i,j), A(i,j));  
}  
  
  
  
static
void mul_aux(vec_ZZ_pE& x, const mat_ZZ_pE& A, const vec_ZZ_pE& b)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
  
   if (l != b.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(n);  
  
   long i, k;  
   ZZ_pX acc, tmp;  
  
   for (i = 1; i <= n; i++) {  
      clear(acc);  
      for (k = 1; k <= l; k++) {  
         mul(tmp, rep(A(i,k)), rep(b(k)));  
         add(acc, acc, tmp);  
      }  
      conv(x(i), acc);  
   }  
}  
  
  
void mul(vec_ZZ_pE& x, const mat_ZZ_pE& A, const vec_ZZ_pE& b)  
{  
   if (&b == &x || A.alias(x)) {
      vec_ZZ_pE tmp;
      mul_aux(tmp, A, b);
      x = tmp;
   }
   else
      mul_aux(x, A, b);
}  

static
void mul_aux(vec_ZZ_pE& x, const vec_ZZ_pE& a, const mat_ZZ_pE& B)  
{  
   long n = B.NumRows();  
   long l = B.NumCols();  
  
   if (n != a.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(l);  
  
   long i, k;  
   ZZ_pX acc, tmp;  
  
   for (i = 1; i <= l; i++) {  
      clear(acc);  
      for (k = 1; k <= n; k++) {  
         mul(tmp, rep(a(k)), rep(B(k,i)));
         add(acc, acc, tmp);  
      }  
      conv(x(i), acc);  
   }  
}  

void mul(vec_ZZ_pE& x, const vec_ZZ_pE& a, const mat_ZZ_pE& B)
{
   if (&a == &x) {
      vec_ZZ_pE tmp;
      mul_aux(tmp, a, B);
      x = tmp;
   }
   else
      mul_aux(x, a, B);

}

     
  
void ident(mat_ZZ_pE& X, long n)  
{  
   X.SetDims(n, n);  
   long i, j;  
  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= n; j++)  
         if (i == j)  
            set(X(i, j));  
         else  
            clear(X(i, j));  
} 


long IsIdent(const mat_ZZ_pE& A, long n)
{
   if (A.NumRows() != n || A.NumCols() != n)
      return 0;

   long i, j;

   for (i = 1; i <= n; i++)
      for (j = 1; j <= n; j++)
         if (i != j) {
            if (!IsZero(A(i, j))) return 0;
         }
         else {
            if (!IsOne(A(i, j))) return 0;
         }

   return 1;
}
            

void transpose(mat_ZZ_pE& X, const mat_ZZ_pE& A)
{
   long n = A.NumRows();
   long m = A.NumCols();

   long i, j;

   if (&X == & A) {
      if (n == m)
         for (i = 1; i <= n; i++)
            for (j = i+1; j <= n; j++)
               swap(X(i, j), X(j, i));
      else {
         mat_ZZ_pE tmp;
         tmp.SetDims(m, n);
         for (i = 1; i <= n; i++)
            for (j = 1; j <= m; j++)
               tmp(j, i) = A(i, j);
         X.kill();
         X = tmp;
      }
   }
   else {
      X.SetDims(m, n);
      for (i = 1; i <= n; i++)
         for (j = 1; j <= m; j++)
            X(j, i) = A(i, j);
   }
}
   
void mul(mat_ZZ_pE& X, const mat_ZZ_pE& A, const ZZ_pE& b_in)
{
   ZZ_pE b = b_in;
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);

   long i, j;
   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
         mul(X[i][j], A[i][j], b);
}

void mul(mat_ZZ_pE& X, const mat_ZZ_pE& A, const ZZ_p& b_in)
{
   NTL_ZZ_pRegister(b);
   b = b_in;
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);

   long i, j;
   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
         mul(X[i][j], A[i][j], b);
}

void mul(mat_ZZ_pE& X, const mat_ZZ_pE& A, long b_in)
{
   NTL_ZZ_pRegister(b);
   b = b_in;
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);

   long i, j;
   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
         mul(X[i][j], A[i][j], b);
}

void diag(mat_ZZ_pE& X, long n, const ZZ_pE& d_in)  
{  
   ZZ_pE d = d_in;
   X.SetDims(n, n);  
   long i, j;  
  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= n; j++)  
         if (i == j)  
            X(i, j) = d;  
         else  
            clear(X(i, j));  
} 

long IsDiag(const mat_ZZ_pE& A, long n, const ZZ_pE& d)
{
   if (A.NumRows() != n || A.NumCols() != n)
      return 0;

   long i, j;

   for (i = 1; i <= n; i++)
      for (j = 1; j <= n; j++)
         if (i != j) {
            if (!IsZero(A(i, j))) return 0;
         }
         else {
            if (A(i, j) != d) return 0;
         }

   return 1;
}


long IsZero(const mat_ZZ_pE& a)
{
   long n = a.NumRows();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

void clear(mat_ZZ_pE& x)
{
   long n = x.NumRows();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}


mat_ZZ_pE operator+(const mat_ZZ_pE& a, const mat_ZZ_pE& b)
{
   mat_ZZ_pE res;
   add(res, a, b);
   NTL_OPT_RETURN(mat_ZZ_pE, res);
}

mat_ZZ_pE operator*(const mat_ZZ_pE& a, const mat_ZZ_pE& b)
{
   mat_ZZ_pE res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(mat_ZZ_pE, res);
}

mat_ZZ_pE operator-(const mat_ZZ_pE& a, const mat_ZZ_pE& b)
{
   mat_ZZ_pE res;
   sub(res, a, b);
   NTL_OPT_RETURN(mat_ZZ_pE, res);
}


mat_ZZ_pE operator-(const mat_ZZ_pE& a)
{
   mat_ZZ_pE res;
   negate(res, a);
   NTL_OPT_RETURN(mat_ZZ_pE, res);
}


vec_ZZ_pE operator*(const mat_ZZ_pE& a, const vec_ZZ_pE& b)
{
   vec_ZZ_pE res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_ZZ_pE, res);
}

vec_ZZ_pE operator*(const vec_ZZ_pE& a, const mat_ZZ_pE& b)
{
   vec_ZZ_pE res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_ZZ_pE, res);
}

void inv(mat_ZZ_pE& X, const mat_ZZ_pE& A)
{
   ZZ_pE d;
   inv(d, X, A);
   if (d == 0) ArithmeticError("inv: non-invertible matrix");
}

void power(mat_ZZ_pE& X, const mat_ZZ_pE& A, const ZZ& e)
{
   if (A.NumRows() != A.NumCols()) LogicError("power: non-square matrix");

   if (e == 0) {
      ident(X, A.NumRows());
      return;
   }

   mat_ZZ_pE T1, T2;
   long i, k;

   k = NumBits(e);
   T1 = A;

   for (i = k-2; i >= 0; i--) {
      sqr(T2, T1);
      if (bit(e, i))
         mul(T1, T2, A);
      else
         T1 = T2;
   }

   if (e < 0)
      inv(X, T1);
   else
      X = T1;
}

void random(mat_ZZ_pE& x, long n, long m)
{
   x.SetDims(n, m);
   for (long i = 0; i < n; i++) random(x[i], m);
}

NTL_END_IMPL

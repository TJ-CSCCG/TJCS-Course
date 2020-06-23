
#include <NTL/mat_RR.h>


NTL_START_IMPL

  
void add(mat_RR& X, const mat_RR& A, const mat_RR& B)  
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
  
void sub(mat_RR& X, const mat_RR& A, const mat_RR& B)  
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
  
void mul_aux(mat_RR& X, const mat_RR& A, const mat_RR& B)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
   long m = B.NumCols();  
  
   if (l != B.NumRows())  
      LogicError("matrix mul: dimension mismatch");  
  
   X.SetDims(n, m);  
  
   long i, j, k;  
   RR acc, tmp;  
  
   for (i = 1; i <= n; i++) {  
      for (j = 1; j <= m; j++) {  
         clear(acc);  
         for(k = 1; k <= l; k++) {  
            mul(tmp, A(i,k), B(k,j));  
            add(acc, acc, tmp);  
         }  
         X(i,j) = acc;  
      }  
   }  
}  
  
  
void mul(mat_RR& X, const mat_RR& A, const mat_RR& B)  
{  
   if (&X == &A || &X == &B) {  
      mat_RR tmp;  
      mul_aux(tmp, A, B);  
      X = tmp;  
   }  
   else  
      mul_aux(X, A, B);  
}  
  
  
static
void mul_aux(vec_RR& x, const mat_RR& A, const vec_RR& b)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
  
   if (l != b.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(n);  
  
   long i, k;  
   RR acc, tmp;  
  
   for (i = 1; i <= n; i++) {  
      clear(acc);  
      for (k = 1; k <= l; k++) {  
         mul(tmp, A(i,k), b(k));  
         add(acc, acc, tmp);  
      }  
      x(i) = acc;  
   }  
}  
  
  
void mul(vec_RR& x, const mat_RR& A, const vec_RR& b)  
{  
   if (&b == &x || A.alias(x)) {
      vec_RR tmp;
      mul_aux(tmp, A, b);
      x = tmp;
   }
   else
      mul_aux(x, A, b);
}  

static
void mul_aux(vec_RR& x, const vec_RR& a, const mat_RR& B)  
{  
   long n = B.NumRows();  
   long l = B.NumCols();  
  
   if (n != a.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(l);  
  
   long i, k;  
   RR acc, tmp;  
  
   for (i = 1; i <= l; i++) {  
      clear(acc);  
      for (k = 1; k <= n; k++) {  
         mul(tmp, a(k), B(k,i));
         add(acc, acc, tmp);  
      }  
      x(i) = acc;  
   }  
}  

void mul(vec_RR& x, const vec_RR& a, const mat_RR& B)
{
   if (&a == &x) {
      vec_RR tmp;
      mul_aux(tmp, a, B);
      x = tmp;
   }
   else
      mul_aux(x, a, B);
}

     
  
void ident(mat_RR& X, long n)  
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


void determinant(RR& d, const mat_RR& M_in)
{
   long k, n;
   long i, j;
   long pos;
   RR t1, t2;
   RR *x, *y;

   n = M_in.NumRows();

   if (M_in.NumCols() != n)
      LogicError("determinant: nonsquare matrix");

   if (n == 0) {
      set(d);
      return;
   }

   mat_RR M;

   M = M_in;


   RR det;
   set(det);

   RR maxval;


   for (k = 0; k < n; k++) {
      pos = -1;
      clear(maxval);
      for (i = k; i < n; i++) {
         abs(t1, M[i][k]);
         if (t1 > maxval) {
            pos = i;
            maxval = t1;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            negate(det, det);
         }

         mul(det, det, M[k][k]);

         // make M[k, k] == -1 

         inv(t1, M[k][k]);
         negate(t1, t1);
         for (j = k+1; j < n; j++) {
            mul(M[k][j], M[k][j], t1);
         }

         for (i = k+1; i < n; i++) {
            // M[i] = M[i] + M[k]*M[i,k]

            t1 = M[i][k];   

            x = M[i].elts() + (k+1);
            y = M[k].elts() + (k+1);

            for (j = k+1; j < n; j++, x++, y++) {
               // *x = *x + (*y)*t1

               mul(t2, *y, t1);
               add(*x, *x, t2);
            }
         }
      }
      else {
         clear(d);
         return;
      }
   }

   d = det;
}

RR determinant(const mat_RR& a)
   { RR x; determinant(x, a); NTL_OPT_RETURN(RR, x); }


long IsIdent(const mat_RR& A, long n)
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
            

void transpose(mat_RR& X, const mat_RR& A)
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
         mat_RR tmp;
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
   

void solve(RR& d, vec_RR& X, 
           const mat_RR& A, const vec_RR& b)

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

   long i, j, k, pos;
   RR t1, t2;
   RR *x, *y;

   mat_RR M;
   M.SetDims(n, n+1);

   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) 
         M[i][j] = A[j][i];
      M[i][n] = b[i];
   }

   RR det;
   set(det);

   RR maxval;

   for (k = 0; k < n; k++) {
      pos = -1;
      clear(maxval);
      for (i = k; i < n; i++) {
         abs(t1, M[i][k]);
         if (t1 > maxval) {
            pos = i;
            maxval = t1;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            negate(det, det);
         }

         mul(det, det, M[k][k]);

         // make M[k, k] == -1 

         inv(t1, M[k][k]);
         negate(t1, t1);
         for (j = k+1; j <= n; j++) {
            mul(M[k][j], M[k][j], t1);
         }

         for (i = k+1; i < n; i++) {
            // M[i] = M[i] + M[k]*M[i,k]

            t1 = M[i][k];   

            x = M[i].elts() + (k+1);
            y = M[k].elts() + (k+1);

            for (j = k+1; j <= n; j++, x++, y++) {
               // *x = *x + (*y)*t1

               mul(t2, *y, t1);
               add(*x, *x, t2);
            }
         }
      }
      else {
         clear(d);
         return;
      }
   }

   X.SetLength(n);
   for (i = n-1; i >= 0; i--) {
      clear(t1);
      for (j = i+1; j < n; j++) {
         mul(t2, X[j], M[i][j]);
         add(t1, t1, t2);
      }
      sub(t1, t1, M[i][n]);
      X[i] = t1;
   }

   d = det;
}

void inv(RR& d, mat_RR& X, const mat_RR& A)
{
   long n = A.NumRows();
   if (A.NumCols() != n)
      LogicError("inv: nonsquare matrix");

   if (n == 0) {
      set(d);
      X.SetDims(0, 0);
      return;
   }

   long i, j, k, pos;
   RR t1, t2;
   RR *x, *y;


   mat_RR M;
   M.SetDims(n, 2*n);

   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
         M[i][j] = A[i][j];
         clear(M[i][n+j]);
      }
      set(M[i][n+i]);
   }

   RR det;
   set(det);

   RR maxval;

   for (k = 0; k < n; k++) {
      pos = -1;
      clear(maxval);
      for (i = k; i < n; i++) {
         abs(t1, M[i][k]);
         if (t1 > maxval) {
            pos = i;
            maxval = t1;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            negate(det, det);
         }

         mul(det, det, M[k][k]);

         // make M[k, k] == -1 

         inv(t1, M[k][k]);
         negate(t1, t1);
         for (j = k+1; j < 2*n; j++) {
            mul(M[k][j], M[k][j], t1);
         }

         for (i = k+1; i < n; i++) {
            // M[i] = M[i] + M[k]*M[i,k]

            t1 = M[i][k];   

            x = M[i].elts() + (k+1);
            y = M[k].elts() + (k+1);

            for (j = k+1; j < 2*n; j++, x++, y++) {
               // *x = *x + (*y)*t1

               mul(t2, *y, t1);
               add(*x, *x, t2);
            }
         }
      }
      else {
         clear(d);
         return;
      }
   }

   X.SetDims(n, n);
   for (k = 0; k < n; k++) {
      for (i = n-1; i >= 0; i--) {
         clear(t1);
         for (j = i+1; j < n; j++) {
            mul(t2, X[j][k], M[i][j]);
            add(t1, t1, t2);
         }
         sub(t1, t1, M[i][n+k]);
         X[i][k] = t1;
      }
   }

   d = det;
}


   
void mul(mat_RR& X, const mat_RR& A, const RR& b_in)
{
   RR b = b_in;
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);

   long i, j;
   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
         mul(X[i][j], A[i][j], b);
}


void mul(mat_RR& X, const mat_RR& A, double b_in)
{
   RR b;
   b = b_in;
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);

   long i, j;
   for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
         mul(X[i][j], A[i][j], b);
}

void diag(mat_RR& X, long n, const RR& d_in)  
{  
   RR d = d_in;
   X.SetDims(n, n);  
   long i, j;  
  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= n; j++)  
         if (i == j)  
            X(i, j) = d;  
         else  
            clear(X(i, j));  
} 

long IsDiag(const mat_RR& A, long n, const RR& d)
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

void negate(mat_RR& X, const mat_RR& A)
{
   long n = A.NumRows();
   long m = A.NumCols();


   X.SetDims(n, m);

   long i, j;
   for (i = 1; i <= n; i++)
      for (j = 1; j <= m; j++)
         negate(X(i,j), A(i,j));
}

long IsZero(const mat_RR& a)
{
   long n = a.NumRows();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

void clear(mat_RR& x)
{
   long n = x.NumRows();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}


mat_RR operator+(const mat_RR& a, const mat_RR& b)
{
   mat_RR res;
   add(res, a, b);
   NTL_OPT_RETURN(mat_RR, res);
}

mat_RR operator*(const mat_RR& a, const mat_RR& b)
{
   mat_RR res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(mat_RR, res);
}

mat_RR operator-(const mat_RR& a, const mat_RR& b)
{
   mat_RR res;
   sub(res, a, b);
   NTL_OPT_RETURN(mat_RR, res);
}


mat_RR operator-(const mat_RR& a)
{
   mat_RR res;
   negate(res, a);
   NTL_OPT_RETURN(mat_RR, res);
}


vec_RR operator*(const mat_RR& a, const vec_RR& b)
{
   vec_RR res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_RR, res);
}

vec_RR operator*(const vec_RR& a, const mat_RR& b)
{
   vec_RR res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_RR, res);
}


void inv(mat_RR& X, const mat_RR& A)
{
   RR d;
   inv(d, X, A);
   if (d == 0) ArithmeticError("inv: non-invertible matrix");
}

void power(mat_RR& X, const mat_RR& A, const ZZ& e)
{
   if (A.NumRows() != A.NumCols()) LogicError("power: non-square matrix");

   if (e == 0) {
      ident(X, A.NumRows());
      return;
   }

   mat_RR T1, T2;
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

NTL_END_IMPL

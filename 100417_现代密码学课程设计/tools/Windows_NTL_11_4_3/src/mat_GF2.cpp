

#include <NTL/mat_GF2.h>
#include <NTL/vec_long.h>


NTL_START_IMPL


void add(mat_GF2& X, const mat_GF2& A, const mat_GF2& B)  
{  
   long n = A.NumRows();  
   long m = A.NumCols();  
  
   if (B.NumRows() != n || B.NumCols() != m)   
      LogicError("matrix add: dimension mismatch");  
  
   X.SetDims(n, m);  

   long mw = (m + NTL_BITS_PER_LONG - 1)/NTL_BITS_PER_LONG;
  
   long i;  
   for (i = 0; i < n; i++) {
      _ntl_ulong *xp = X[i].rep.elts();
      const _ntl_ulong *ap = A[i].rep.elts();
      const _ntl_ulong *bp = B[i].rep.elts();
      long j;
      for (j = 0; j < mw; j++)
         xp[j] = ap[j] ^ bp[j];
   }
}  
  
static
void mul_aux(vec_GF2& x, const mat_GF2& A, const vec_GF2& b)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
  
   if (l != b.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(n);  
  
   long i;  
  
   for (i = 0; i < n; i++) {  
      x.put(i, A[i] * b);
   }  
}  
  
  
void mul(vec_GF2& x, const mat_GF2& A, const vec_GF2& b)  
{  
   if (&b == &x || A.alias(x)) {
      vec_GF2 tmp;
      mul_aux(tmp, A, b);
      x = tmp;
   }
   else
      mul_aux(x, A, b);
}  

static
void mul_aux(vec_GF2& x, const vec_GF2& a, const mat_GF2& B)  
{  
   long n = B.NumRows();  
   long l = B.NumCols();  
  
   if (n != a.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(l);  
   clear(x);

   const _ntl_ulong *ap = a.rep.elts();
   _ntl_ulong a_mask = 1;

   _ntl_ulong *xp = x.rep.elts();

   long lw = (l + NTL_BITS_PER_LONG - 1)/NTL_BITS_PER_LONG;
  
   long i;  
   for (i = 0; i < n; i++) {  
      if (*ap & a_mask) {
         const _ntl_ulong *bp = B[i].rep.elts();
         long j;
         for (j = 0; j < lw; j++)
            xp[j] ^= bp[j];
      }

      a_mask <<= 1;
      if (!a_mask) {
         a_mask = 1;
         ap++;
      }
   }  
}  

void mul(vec_GF2& x, const vec_GF2& a, const mat_GF2& B)
{
   if (&a == &x || B.alias(x)) {
      vec_GF2 tmp;
      mul_aux(tmp, a, B);
      x = tmp;
   }
   else
      mul_aux(x, a, B);
}
  
void mul_aux(mat_GF2& X, const mat_GF2& A, const mat_GF2& B)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
   long m = B.NumCols();  
  
   if (l != B.NumRows())  
      LogicError("matrix mul: dimension mismatch");  
  
   X.SetDims(n, m);  
  
   long i;  
  
   for (i = 1; i <= n; i++) {  
      mul_aux(X(i), A(i), B);
   }  
}  
  
  
void mul(mat_GF2& X, const mat_GF2& A, const mat_GF2& B)  
{  
   if (&X == &A || &X == &B) {  
      mat_GF2 tmp;  
      mul_aux(tmp, A, B);  
      X = tmp;  
   }  
   else  
      mul_aux(X, A, B);  
}  
  

     
  
void ident(mat_GF2& X, long n)  
{  
   X.SetDims(n, n);  
   clear(X);
   long i;  
  
   for (i = 0; i < n; i++)  
      X.put(i, i, to_GF2(1));
} 


void determinant(ref_GF2 d, const mat_GF2& M_in)
{
   long k, n;
   long i, j;
   long pos;

   n = M_in.NumRows();

   if (M_in.NumCols() != n)
      LogicError("determinant: nonsquare matrix");

   if (n == 0) {
      set(d);
      return;
   }

   mat_GF2 M;

   M = M_in;

   long wn = (n + NTL_BITS_PER_LONG - 1)/NTL_BITS_PER_LONG;

   for (k = 0; k < n; k++) {
      long wk = k/NTL_BITS_PER_LONG;
      long bk = k - wk*NTL_BITS_PER_LONG;
      _ntl_ulong k_mask = 1UL << bk;

      pos = -1;
      for (i = k; i < n; i++) {
         if (M[i].rep.elts()[wk] & k_mask) {
            pos = i;
            break;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
         }


         _ntl_ulong *y = M[k].rep.elts();

         for (i = k+1; i < n; i++) {
            // M[i] = M[i] + M[k]*M[i,k]

            if (M[i].rep.elts()[wk] & k_mask) {
               _ntl_ulong *x = M[i].rep.elts();

               for (j = wk; j < wn; j++)
                  x[j] ^= y[j];
            }

         }
      }
      else {
         clear(d);
         return;
      }
   }

   set(d);
   return;
}

static
long IsUnitVector(const vec_GF2& a, long i)
{
   long wi = i/NTL_BITS_PER_LONG;
   long bi = i - wi*NTL_BITS_PER_LONG;

   const _ntl_ulong *p = a.rep.elts();
   long wdlen = a.rep.length();

   long j;

   for (j = 0; j < wi; j++)
      if (p[j] != 0) return 0;

   if (p[wi] != (1UL << bi))
      return 0;

   for (j = wi+1; j < wdlen; j++)
      if (p[j] != 0) return 0;

   return 1;
}


long IsIdent(const mat_GF2& A, long n)
{
   if (A.NumRows() != n || A.NumCols() != n)
      return 0;

   if (n == 0) return 1;

   long i;

   for (i = 0; i < n; i++)
      if (!IsUnitVector(A[i], i))
         return 0;

   return 1;
}

void AddToCol(mat_GF2& x, long j, const vec_GF2& a)
// add a to column j of x
// ALIAS RESTRICTION: a should not alias any row of x
{
   long n = x.NumRows();
   long m = x.NumCols();

   if (a.length() != n || j < 0 || j >= m)
      LogicError("AddToCol: bad args");

   long wj = j/NTL_BITS_PER_LONG;
   long bj = j - wj*NTL_BITS_PER_LONG;
   _ntl_ulong j_mask = 1UL << bj;

   const _ntl_ulong *ap = a.rep.elts();
   _ntl_ulong a_mask = 1;

   long i;
   for (i = 0; i < n; i++) {
      if (*ap & a_mask) 
         x[i].rep.elts()[wj] ^= j_mask;

      a_mask <<= 1;
      if (!a_mask) {
         a_mask = 1;
         ap++;
      }
   }
}


void transpose_aux(mat_GF2& X, const mat_GF2& A)
{
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(m, n);
   clear(X);

   long i;
   for (i = 0; i < n; i++)
      AddToCol(X, i, A[i]);
}
            

void transpose(mat_GF2& X, const mat_GF2& A)
{
   if (&X == &A) {
      mat_GF2 tmp;
      transpose_aux(tmp, A);
      X = tmp;
   }
   else
      transpose_aux(X, A);
}

   

static
void solve_impl(ref_GF2 d, vec_GF2& X, const mat_GF2& A, const vec_GF2& b, bool trans)

{
   long n = A.NumRows();
   if (A.NumCols() != n)
      LogicError("solve: nonsquare matrix");

   if (b.length() != n)
      LogicError("solve: dimension mismatch");

   if (n == 0) {
      X.SetLength(0);
      set(d);
      return;
   }

   long i, j, k, pos;

   mat_GF2 M;
   M.SetDims(n, n+1);

   if (trans) {
      for (i = 0; i < n; i++) {
	 AddToCol(M, i, A[i]);
      }
   }
   else {
      for (i = 0; i < n; i++) {
         VectorCopy(M[i], A[i], n+1);
      }
   }

   AddToCol(M, n, b);

   long wn = ((n+1) + NTL_BITS_PER_LONG - 1)/NTL_BITS_PER_LONG;

   for (k = 0; k < n; k++) {
      long wk = k/NTL_BITS_PER_LONG;
      long bk = k - wk*NTL_BITS_PER_LONG;
      _ntl_ulong k_mask = 1UL << bk;

      pos = -1;
      for (i = k; i < n; i++) {
         if (M[i].rep.elts()[wk] & k_mask) {
            pos = i;
            break;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
         }

         _ntl_ulong *y = M[k].rep.elts();

         for (i = k+1; i < n; i++) {
            // M[i] = M[i] + M[k]*M[i,k]

            if (M[i].rep.elts()[wk] & k_mask) {
               _ntl_ulong *x = M[i].rep.elts();

               for (j = wk; j < wn; j++)
                  x[j] ^= y[j];
            }


         }
      }
      else {
         clear(d);
         return;
      }
   }

   vec_GF2 XX;
   XX.SetLength(n+1);
   XX.put(n, 1);

   for (i = n-1; i >= 0; i--) {
      XX.put(i, XX*M[i]);
   }

   XX.SetLength(n);
   X = XX;

   set(d);
   return;
}

void solve(ref_GF2 d, vec_GF2& x, const mat_GF2& A, const vec_GF2& b)
{
   solve_impl(d, x, A, b, true);
}

void solve(ref_GF2 d, const mat_GF2& A, vec_GF2& x,  const vec_GF2& b)
{
   solve_impl(d, x, A, b, false);
}


void inv(ref_GF2 d, mat_GF2& X, const mat_GF2& A)
{
   long n = A.NumRows();
   if (A.NumCols() != n)
      LogicError("solve: nonsquare matrix");

   if (n == 0) {
      X.SetDims(0, 0);
      set(d);
   }

   long i, j, k, pos;

   mat_GF2 M;
   M.SetDims(n, 2*n);

   vec_GF2 aa;
   aa.SetLength(2*n);


   for (i = 0; i < n; i++) {
      aa = A[i];
      aa.SetLength(2*n);
      aa.put(n+i, 1);
      M[i] = aa;
   }

   long wn = ((2*n) + NTL_BITS_PER_LONG - 1)/NTL_BITS_PER_LONG;

   for (k = 0; k < n; k++) {
      long wk = k/NTL_BITS_PER_LONG;
      long bk = k - wk*NTL_BITS_PER_LONG;
      _ntl_ulong k_mask = 1UL << bk;

      pos = -1;
      for (i = k; i < n; i++) {
         if (M[i].rep.elts()[wk] & k_mask) {
            pos = i;
            break;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
         }

         _ntl_ulong *y = M[k].rep.elts();

         for (i = k+1; i < n; i++) {
            // M[i] = M[i] + M[k]*M[i,k]

            if (M[i].rep.elts()[wk] & k_mask) {
               _ntl_ulong *x = M[i].rep.elts();

               for (j = wk; j < wn; j++)
                  x[j] ^= y[j];
            }


         }
      }
      else {
         clear(d);
         return;
      }
   }

   vec_GF2 XX;
   XX.SetLength(2*n);

   X.SetDims(n, n);
   clear(X);

   for (j = 0; j < n; j++) {
      XX.SetLength(n+j+1);
      clear(XX);
      XX.put(n+j, to_GF2(1));
      
      for (i = n-1; i >= 0; i--) {
         XX.put(i, XX*M[i]);
      }
   
      XX.SetLength(n);
      AddToCol(X, j, XX);
   }

   set(d);
   return;
}





long gauss(mat_GF2& M, long w)
{
   long k, l;
   long i, j;
   long pos;

   long n = M.NumRows();
   long m = M.NumCols();

   if (w < 0 || w > m)
      LogicError("gauss: bad args");

   long wm = (m + NTL_BITS_PER_LONG - 1)/NTL_BITS_PER_LONG;

   l = 0;
   for (k = 0; k < w && l < n; k++) {
      long wk = k/NTL_BITS_PER_LONG;
      long bk = k - wk*NTL_BITS_PER_LONG;
      _ntl_ulong k_mask = 1UL << bk;


      pos = -1;
      for (i = l; i < n; i++) {
         if (M[i].rep.elts()[wk] & k_mask) {
            pos = i;
            break;
         }
      }

      if (pos != -1) {
         if (l != pos)
            swap(M[pos], M[l]);

         _ntl_ulong *y = M[l].rep.elts();

         for (i = l+1; i < n; i++) {
            // M[i] = M[i] + M[l]*M[i,k]

            if (M[i].rep.elts()[wk] & k_mask) {
               _ntl_ulong *x = M[i].rep.elts();

               for (j = wk; j < wm; j++)
                  x[j] ^= y[j];
            }
         }

         l++;
      }
   }
   
   return l;
}

long gauss(mat_GF2& M)
{
   return gauss(M, M.NumCols());
}


void image(mat_GF2& X, const mat_GF2& A)
{
   mat_GF2 M;
   M = A;
   long r = gauss(M);
   M.SetDims(r, M.NumCols());
   X = M;
}

void kernel(mat_GF2& X, const mat_GF2& A)
{
   long m = A.NumRows();
   long n = A.NumCols();

   mat_GF2 M;
   long r;

   transpose(M, A);
   r = gauss(M);

   X.SetDims(m-r, m);
   clear(X);

   long i, j, k;

   vec_long D;
   D.SetLength(m);
   for (j = 0; j < m; j++) D[j] = -1;

   j = -1;
   for (i = 0; i < r; i++) {
      do {
         j++;
      } while (M.get(i, j) == 0); 

      D[j] = i;
   }

   for (k = 0; k < m-r; k++) {
      vec_GF2& v = X[k];
      long pos = 0;
      for (j = m-1; j >= 0; j--) {
         if (D[j] == -1) {
            if (pos == k) {
               v[j] = 1;
               // v.put(j, to_GF2(1));
            }
            pos++;
         }
         else {
            v[j] = v*M[D[j]];
            // v.put(j, v*M[D[j]]);
         }
      }
   }
}

   
void mul(mat_GF2& X, const mat_GF2& A, GF2 b)
{
   X = A;
   if (b == 0)
      clear(X);
}

void diag(mat_GF2& X, long n, GF2 d)  
{  
   if (d == 1)
      ident(X, n);
   else {
      X.SetDims(n, n);
      clear(X);
   }
} 

long IsDiag(const mat_GF2& A, long n, GF2 d)
{
   if (A.NumRows() != n || A.NumCols() != n)
      return 0;

   if (d == 1)
      return IsIdent(A, n);
   else
      return IsZero(A);
}


long IsZero(const mat_GF2& a)
{
   long n = a.NumRows();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

void clear(mat_GF2& x)
{
   long n = x.NumRows();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}


mat_GF2 operator+(const mat_GF2& a, const mat_GF2& b)
{
   mat_GF2 res;
   add(res, a, b);
   NTL_OPT_RETURN(mat_GF2, res);
}

mat_GF2 operator*(const mat_GF2& a, const mat_GF2& b)
{
   mat_GF2 res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(mat_GF2, res);
}

mat_GF2 operator-(const mat_GF2& a, const mat_GF2& b)
{
   mat_GF2 res;
   add(res, a, b);
   NTL_OPT_RETURN(mat_GF2, res);
}


vec_GF2 operator*(const mat_GF2& a, const vec_GF2& b)
{
   vec_GF2 res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_GF2, res);
}

vec_GF2 operator*(const vec_GF2& a, const mat_GF2& b)
{
   vec_GF2 res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_GF2, res);
}


void inv(mat_GF2& X, const mat_GF2& A)
{
   GF2 d;
   inv(d, X, A);
   if (d == 0) ArithmeticError("inv: non-invertible matrix");
}

void power(mat_GF2& X, const mat_GF2& A, const ZZ& e)
{
   if (A.NumRows() != A.NumCols()) LogicError("power: non-square matrix");

   if (e == 0) {
      ident(X, A.NumRows());
      return;
   }

   mat_GF2 T1, T2;
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

void random(mat_GF2& x, long n, long m)
{
   x.SetDims(n, m);
   for (long i = 0; i < n; i++) random(x[i], m);
}

NTL_END_IMPL

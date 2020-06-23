
#include <NTL/mat_ZZ_p.h>
#include <NTL/vec_ZZVec.h>
#include <NTL/vec_long.h>
#include <NTL/BasicThreadPool.h>


// FIXME: only needed if we use multi-modular MM
#include <NTL/MatPrime.h>
#include <NTL/mat_lzz_p.h>



NTL_START_IMPL



// ******************** Matrix Multiplication ************************

#ifdef NTL_HAVE_LL_TYPE
#define NTL_USE_MM_MATMUL (1)
#else
#define NTL_USE_MM_MATMUL (0)
#endif

#define PAR_THRESH (40000.0)


// *********************** Plain Matrix Multiplication ***************



void plain_mul_aux(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
   long m = B.NumCols();  
  
   if (l != B.NumRows())  
      LogicError("matrix mul: dimension mismatch");  
  
   X.SetDims(n, m);  

   ZZ_pContext context;
   context.save();

   long sz = ZZ_p::ModulusSize();
   bool seq = (double(n)*double(l)*double(m)*double(sz)*double(sz) < PAR_THRESH);
  
   NTL_GEXEC_RANGE(seq, m, first, last)
   NTL_IMPORT(n)
   NTL_IMPORT(l)
   NTL_IMPORT(m)

   context.restore();

   long i, j, k;  
   ZZ acc, tmp;  

   vec_ZZ_p B_col;
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
  
  
void plain_mul(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B)  
{  
   if (&X == &A || &X == &B) {  
      mat_ZZ_p tmp;  
      plain_mul_aux(tmp, A, B);  
      X = tmp;  
   }  
   else  
      plain_mul_aux(X, A, B);  
}  

// X = A*transpose(B)

void plain_mul_transpose_aux(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
   long m = B.NumRows();  
  
   if (l != B.NumCols())  
      LogicError("matrix mul: dimension mismatch");  
  
   X.SetDims(n, m);  

   ZZ_pContext context;
   context.save();

   long sz = ZZ_p::ModulusSize();
   bool seq = (double(n)*double(l)*double(m)*double(sz)*double(sz) < PAR_THRESH);
  
   NTL_GEXEC_RANGE(seq, m, first, last)
   NTL_IMPORT(n)
   NTL_IMPORT(l)
   NTL_IMPORT(m)

   context.restore();

   long i, j, k;  
   ZZ acc, tmp;  

   for (j = first; j < last; j++) {
      const ZZ_p *B_col = B[j].elts();

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
  
  
void plain_mul_transpose(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B)  
{  
   if (&X == &A || &X == &B) {  
      mat_ZZ_p tmp;  
      plain_mul_transpose_aux(tmp, A, B);  
      X = tmp;  
   }  
   else  
      plain_mul_transpose_aux(X, A, B);  
}  



// ***************** Multi-modular Matrix Multiplication *************

struct mat_ZZ_p_crt_rep {

   Vec< Mat<MatPrime_residue_t> > rep;

};


static
const MatPrime_crt_helper& get_MatPrime_crt_helper_info()
{
   do {
      Lazy<MatPrime_crt_helper,ZZ_pInfoT::MatPrime_crt_helper_deleter_policy>::Builder
         builder(ZZ_pInfo->MatPrime_crt_helper_info);
      if (!builder()) break;

      UniquePtr<MatPrime_crt_helper,ZZ_pInfoT::MatPrime_crt_helper_deleter_policy> p;
      p.make();
      build(*p, ZZ_pInfo->p);
      builder.move(p);
   } while (0);

   return *ZZ_pInfo->MatPrime_crt_helper_info;
}

static
void RawConvert(Mat<zz_p>& X, const Mat<MatPrime_residue_t>& A)
{
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);
   for (long i = 0; i < n; i++) {
      const MatPrime_residue_t *Ai = A[i].elts();
      zz_p *Xi = X[i].elts();
      for (long j = 0; j < m; j++)
         Xi[j].LoopHole() = Ai[j];
   }
} 

static
void RawConvertTranspose(Mat<zz_p>& X, const Mat<MatPrime_residue_t>& A)
{
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(m, n);
   for (long i = 0; i < n; i++) {
      const MatPrime_residue_t *Ai = A[i].elts();
      for (long j = 0; j < m; j++)
         X[j][i] = Ai[j];
   }
} 

static
void RawConvert(Mat<MatPrime_residue_t>& X, const Mat<zz_p>& A)
{
   long n = A.NumRows();
   long m = A.NumCols();

   X.SetDims(n, m);
   for (long i = 0; i < n; i++) {
      const zz_p *Ai = A[i].elts();
      MatPrime_residue_t *Xi = X[i].elts();
      for (long j = 0; j < m; j++)
         Xi[j] = rep(Ai[j]);
   }
} 

#define CRT_BLK (8)

void to_mat_ZZ_p_crt_rep(mat_ZZ_p_crt_rep& X, const mat_ZZ_p& A)
{
   long n = A.NumRows();
   long m = A.NumCols();

   const MatPrime_crt_helper& H = get_MatPrime_crt_helper_info();
   long nprimes = H.GetNumPrimes();

   if (NTL_OVERFLOW(nprimes, CRT_BLK, 0))
      ResourceError("overflow"); // this is pretty academic

   X.rep.SetLength(nprimes);
   for (long k = 0; k < nprimes; k++) X.rep[k].SetDims(n, m);

   ZZ_pContext context;
   context.save();


   bool seq = (double(n)*double(m)*H.GetCost() < PAR_THRESH);

   // FIXME: right now, we just partition the rows, but if
   // #cols > #rows, we should perhaps partition the cols
   NTL_GEXEC_RANGE(seq, n, first, last)
   NTL_IMPORT(n)
   NTL_IMPORT(m)
   NTL_IMPORT(nprimes)

   context.restore();

   MatPrime_crt_helper_scratch scratch;
   Vec<MatPrime_residue_t> remainders_store;
   remainders_store.SetLength(nprimes*CRT_BLK);
   MatPrime_residue_t *remainders = remainders_store.elts();

   for (long i = first; i < last; i++) {
      const ZZ_p *a = A[i].elts();

      long jj = 0; 
      for (; jj <= m-CRT_BLK; jj += CRT_BLK) {
         for (long j = 0; j < CRT_BLK; j++)
            reduce(H, rep(a[jj+j]), remainders + j*nprimes, scratch);
         for (long k = 0; k < nprimes; k++) {
            MatPrime_residue_t *x = X.rep[k][i].elts();
            for (long j = 0; j < CRT_BLK; j++)
               x[jj+j] = remainders[j*nprimes+k];
         }
      }
      if (jj < m) {
         for (long j = 0; j < m-jj; j++)
            reduce(H, rep(a[jj+j]), remainders + j*nprimes, scratch);
         for (long k = 0; k < nprimes; k++) {
            MatPrime_residue_t *x = X.rep[k][i].elts();
            for (long j = 0; j < m-jj; j++)
               x[jj+j] = remainders[j*nprimes+k];
         }
      }
   }

   NTL_GEXEC_RANGE_END
}

void from_mat_ZZ_p_crt_rep(const mat_ZZ_p_crt_rep& X, mat_ZZ_p& A)
{
   long n = X.rep[0].NumRows();
   long m = X.rep[0].NumCols();

   const MatPrime_crt_helper& H = get_MatPrime_crt_helper_info();
   long nprimes = H.GetNumPrimes();

   if (NTL_OVERFLOW(nprimes, CRT_BLK, 0))
      ResourceError("overflow"); // this is pretty academic

   A.SetDims(n, m);

   ZZ_pContext context;
   context.save();

   bool seq = (double(n)*double(m)*H.GetCost() < PAR_THRESH);

   // FIXME: right now, we just partition the rows, but if
   // #cols > #rows, we should perhaps partition the cols
   NTL_GEXEC_RANGE(seq, n, first, last)
   NTL_IMPORT(n)
   NTL_IMPORT(m)
   NTL_IMPORT(nprimes)

   context.restore();

   MatPrime_crt_helper_scratch scratch;
   Vec<MatPrime_residue_t> remainders_store;
   remainders_store.SetLength(nprimes*CRT_BLK);
   MatPrime_residue_t *remainders = remainders_store.elts();

   for (long i = first; i < last; i++) {
      ZZ_p *a = A[i].elts();

      long jj = 0; 
      for (; jj <= m-CRT_BLK; jj += CRT_BLK) {
         for (long k = 0; k < nprimes; k++) {
            const MatPrime_residue_t *x = X.rep[k][i].elts();
            for (long j = 0; j < CRT_BLK; j++)
               remainders[j*nprimes+k] = x[jj+j];
         }
         for (long j = 0; j < CRT_BLK; j++)
            reconstruct(H, a[jj+j].LoopHole(), remainders + j*nprimes, scratch);
      }
      if (jj < m) {
         for (long k = 0; k < nprimes; k++) {
            const MatPrime_residue_t *x = X.rep[k][i].elts();
            for (long j = 0; j < m-jj; j++)
               remainders[j*nprimes+k] = x[jj+j];
         }
         for (long j = 0; j < m-jj; j++)
            reconstruct(H, a[jj+j].LoopHole(), remainders + j*nprimes, scratch);
      }
   }

   NTL_GEXEC_RANGE_END
}

void mul(mat_ZZ_p_crt_rep& X, const mat_ZZ_p_crt_rep& A, const mat_ZZ_p_crt_rep& B)
{
   long nprimes = A.rep.length();

   long n = A.rep[0].NumRows();
   long l = A.rep[0].NumCols();
   long m = B.rep[0].NumCols();

   X.rep.SetLength(nprimes);
   for (long k = 0; k < nprimes; k++) X.rep[k].SetDims(n, m);

   bool seq = (double(n)*double(l)*double(m)*double(nprimes) < PAR_THRESH);


   NTL_GEXEC_RANGE(seq, nprimes, first, last)
   NTL_IMPORT(n)
   NTL_IMPORT(l)
   NTL_IMPORT(m)

   zz_pPush push;

   Mat<zz_p> x, a, b;
   x.SetDims(n, m);
   a.SetDims(n, l);
   b.SetDims(l, m);

   for (long k = first; k < last; k++) {
      RestoreMatPrime(k);
      RawConvert(a, A.rep[k]);
      RawConvert(b, B.rep[k]);
      mul(x, a, b);
      RawConvert(X.rep[k], x);
   }

   NTL_GEXEC_RANGE_END
}


// X = A*transpose(B)
void mul_transpose(mat_ZZ_p_crt_rep& X, const mat_ZZ_p_crt_rep& A, const mat_ZZ_p_crt_rep& B)
{
   long nprimes = A.rep.length();

   long n = A.rep[0].NumRows();
   long l = A.rep[0].NumCols();
   long m = B.rep[0].NumRows();

   X.rep.SetLength(nprimes);
   for (long k = 0; k < nprimes; k++) X.rep[k].SetDims(n, m);

   bool seq = (double(n)*double(l)*double(m)*double(nprimes) < PAR_THRESH);

   NTL_GEXEC_RANGE(seq, nprimes, first, last)
   NTL_IMPORT(n)
   NTL_IMPORT(l)
   NTL_IMPORT(m)

   zz_pPush push;

   Mat<zz_p> x, a, b;
   x.SetDims(n, m);
   a.SetDims(n, l);
   b.SetDims(l, m);

   for (long k = first; k < last; k++) {
      RestoreMatPrime(k);
      RawConvert(a, A.rep[k]);
      RawConvertTranspose(b, B.rep[k]);
      mul(x, a, b);
      RawConvert(X.rep[k], x);
   }

   NTL_GEXEC_RANGE_END
}


void multi_modular_mul(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B)
{
   long l = A.NumCols();

   if (l != B.NumRows())
      LogicError("matrix mul: dimension mismatch");  

   if (l > NTL_MatPrimeLimit)
      ResourceError("matrix mul: dimension too large");

   mat_ZZ_p_crt_rep x, a, b;

   to_mat_ZZ_p_crt_rep(a, A);
   to_mat_ZZ_p_crt_rep(b, B);
   mul(x, a, b);
   from_mat_ZZ_p_crt_rep(x, X);
}

void multi_modular_mul(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p_crt_rep& B)
{
   long l = A.NumCols();

   if (l != B.rep[0].NumRows())
      LogicError("matrix mul: dimension mismatch");  

   if (l > NTL_MatPrimeLimit)
      ResourceError("matrix mul: dimension too large");

   mat_ZZ_p_crt_rep x, a;

   to_mat_ZZ_p_crt_rep(a, A);
   mul(x, a, B);
   from_mat_ZZ_p_crt_rep(x, X);
}

void multi_modular_mul_transpose(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p_crt_rep& B)
{
   long l = A.NumCols();

   if (l != B.rep[0].NumCols())
      LogicError("matrix mul: dimension mismatch");  

   if (l > NTL_MatPrimeLimit)
      ResourceError("matrix mul: dimension too large");

   mat_ZZ_p_crt_rep x, a;

   to_mat_ZZ_p_crt_rep(a, A);
   mul_transpose(x, a, B);
   from_mat_ZZ_p_crt_rep(x, X);
}


// ******************** mat_ZZ_p_opaque implementation ************

// This could (and maybe eventually will) be implemented using
// derived types, if we want run-time polymorphism...we'll see

struct mat_ZZ_p_opaque_body_crt : mat_ZZ_p_opaque_body {
   mat_ZZ_p_crt_rep body;

   mat_ZZ_p_opaque_body* clone() const 
   {
      return MakeRaw<mat_ZZ_p_opaque_body_crt>(*this);
   }

   long NumRows() const
   {
      return body.rep.length() == 0 ? 0 : body.rep[0].NumRows();
   }

   long NumCols() const
   {
      return body.rep.length() == 0 ? 0 : body.rep[0].NumCols();
   }

   void mul(mat_ZZ_p& X, const mat_ZZ_p& A) const
   { 
      multi_modular_mul(X, A, body);
   }

   void mul_transpose(mat_ZZ_p& X, const mat_ZZ_p& A) const
   { 
      multi_modular_mul_transpose(X, A, body);
   }
   
};

struct mat_ZZ_p_opaque_body_plain : mat_ZZ_p_opaque_body {
   mat_ZZ_p body;

   mat_ZZ_p_opaque_body* clone() const 
   {
      return MakeRaw<mat_ZZ_p_opaque_body_plain>(*this);
   }

   long NumRows() const
   {
      return body.NumRows();
   }

   long NumCols() const
   {
      return body.NumCols();
   }

   void mul(mat_ZZ_p& X, const mat_ZZ_p& A) const
   { 
      plain_mul(X, A, body);
   }

   void mul_transpose(mat_ZZ_p& X, const mat_ZZ_p& A) const
   { 
      plain_mul_transpose(X, A, body);
   }
   
};



// This is a "factory" method that makes a mat_ZZ_p_opaque_body
// from a matrix A.  The matrix A is destroyed in the process.

mat_ZZ_p_opaque_body *mat_ZZ_p_opaque_body_move(mat_ZZ_p& A)
{
   if (NTL_USE_MM_MATMUL && A.NumRows() >= 16 && A.NumCols() >= 16) {
      UniquePtr<mat_ZZ_p_opaque_body_crt> tmp;
      tmp.make();
      to_mat_ZZ_p_crt_rep(tmp->body, A);
      A.kill();
      return tmp.release();
   }
   else {
      UniquePtr<mat_ZZ_p_opaque_body_plain> tmp;
      tmp.make();
      tmp->body.move(A);
      return tmp.release();
   }
}



// *******************************************************************



void mul(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B)
{
   long n = A.NumRows();
   long l = A.NumCols();
   long m = B.NumCols();

   if (l != B.NumRows()) LogicError("matrix mul: dimension mismatch");

   if (NTL_USE_MM_MATMUL && n >= 24 && l >= 24 && m >= 24) 
      multi_modular_mul(X, A, B);
   else
      plain_mul(X, A, B);
}




// *******************************************************************



  
void add(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B)  
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
  
void sub(mat_ZZ_p& X, const mat_ZZ_p& A, const mat_ZZ_p& B)  
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

void negate(mat_ZZ_p& X, const mat_ZZ_p& A)  
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
void mul_aux(vec_ZZ_p& x, const mat_ZZ_p& A, const vec_ZZ_p& b)  
{  
   long n = A.NumRows();  
   long l = A.NumCols();  
  
   if (l != b.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(n);  
  
   long i, k;  
   ZZ acc, tmp;  
  
   for (i = 1; i <= n; i++) {  
      clear(acc);  
      for (k = 1; k <= l; k++) {  
         mul(tmp, rep(A(i,k)), rep(b(k)));  
         add(acc, acc, tmp);  
      }  
      conv(x(i), acc);  
   }  
}  
  
  
void mul(vec_ZZ_p& x, const mat_ZZ_p& A, const vec_ZZ_p& b)  
{  
   if (&b == &x || A.alias(x)) {
      vec_ZZ_p tmp;
      mul_aux(tmp, A, b);
      x = tmp;
   }
   else
      mul_aux(x, A, b);
}  

static
void mul_aux(vec_ZZ_p& x, const vec_ZZ_p& a, const mat_ZZ_p& B)  
{  
   long n = B.NumRows();  
   long l = B.NumCols();  
  
   if (n != a.length())  
      LogicError("matrix mul: dimension mismatch");  
  
   x.SetLength(l);  
  
   long i, k;  
   ZZ acc, tmp;  
  
   for (i = 1; i <= l; i++) {  
      clear(acc);  
      for (k = 1; k <= n; k++) {  
         mul(tmp, rep(a(k)), rep(B(k,i)));
         add(acc, acc, tmp);  
      }  
      conv(x(i), acc);  
   }  
}  

void mul(vec_ZZ_p& x, const vec_ZZ_p& a, const mat_ZZ_p& B)
{
   if (&a == &x) {
      vec_ZZ_p tmp;
      mul_aux(tmp, a, B);
      x = tmp;
   }
   else
      mul_aux(x, a, B);
}

     
  
void ident(mat_ZZ_p& X, long n)  
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



void determinant(ZZ_p& d, const mat_ZZ_p& M_in)
{
   ZZ t1, t2;

   const ZZ& p = ZZ_p::modulus();

   long n = M_in.NumRows();

   if (M_in.NumCols() != n)
      LogicError("determinant: nonsquare matrix");

   if (n == 0) {
      set(d);
      return;
   }

   Vec<ZZVec> M;
   sqr(t1, p);
   mul(t1, t1, n);

   M.SetLength(n);
   for (long i = 0; i < n; i++) {
      M[i].SetSize(n, t1.size());
      for (long j = 0; j < n; j++)
         M[i][j] = rep(M_in[i][j]);
   }

   ZZ det;
   set(det);

   for (long k = 0; k < n; k++) {
      long pos = -1;
      for (long i = k; i < n; i++) {
         rem(t1, M[i][k], p);
         M[i][k] = t1;
         if (pos == -1 && !IsZero(t1))
            pos = i;
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            NegateMod(det, det, p);
         }

         MulMod(det, det, M[k][k], p);

         // make M[k, k] == -1 mod p, and make row k reduced

         InvMod(t1, M[k][k], p);
         NegateMod(t1, t1, p);
         for (long j = k+1; j < n; j++) {
            rem(t2, M[k][j], p);
            MulMod(M[k][j], t2, t1, p);
         }


         bool seq =
            double(n-(k+1))*(n-(k+1))*double(p.size())*double(p.size()) < PAR_THRESH;
         NTL_GEXEC_RANGE(seq, n-(k+1), first, last)
         NTL_IMPORT(n)
         NTL_IMPORT(k)

         ZZ t1, t2;

         for (long ii = first; ii < last; ii++) {
            long i = ii + k+1;

            // M[i] = M[i] + M[k]*M[i,k]

            t1 = M[i][k];   // this is already reduced

            ZZ *x = M[i].elts() + (k+1);
            ZZ *y = M[k].elts() + (k+1);

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





long IsIdent(const mat_ZZ_p& A, long n)
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
            

void transpose(mat_ZZ_p& X, const mat_ZZ_p& A)
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
         mat_ZZ_p tmp;
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
   


static
void solve_impl(ZZ_p& d, vec_ZZ_p& X, const mat_ZZ_p& A, const vec_ZZ_p& b, bool trans)

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

   ZZ t1, t2;

   const ZZ& p = ZZ_p::modulus();

   Vec<ZZVec> M;
   sqr(t1, p);
   mul(t1, t1, n);

   M.SetLength(n);

   for (long i = 0; i < n; i++) {
      M[i].SetSize(n+1, t1.size());

      if (trans) 
         for (long j = 0; j < n; j++) M[i][j] = rep(A[j][i]);
      else
         for (long j = 0; j < n; j++) M[i][j] = rep(A[i][j]);

      M[i][n] = rep(b[i]);
   }

   ZZ det;
   set(det);

   for (long k = 0; k < n; k++) {
      long pos = -1;
      for (long i = k; i < n; i++) {
         rem(t1, M[i][k], p);
         M[i][k] = t1;
         if (pos == -1 && !IsZero(t1)) {
            pos = i;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            NegateMod(det, det, p);
         }

         MulMod(det, det, M[k][k], p);

         // make M[k, k] == -1 mod p, and make row k reduced

         InvMod(t1, M[k][k], p);
         NegateMod(t1, t1, p);
         for (long j = k+1; j <= n; j++) {
            rem(t2, M[k][j], p);
            MulMod(M[k][j], t2, t1, p);
         }

         bool seq =
            double(n-(k+1))*(n-(k+1))*double(p.size())*double(p.size()) < PAR_THRESH;
         NTL_GEXEC_RANGE(seq, n-(k+1), first, last)
         NTL_IMPORT(n)
         NTL_IMPORT(k)

         ZZ t1, t2;

         for (long ii = first; ii < last; ii++) {
            long i = ii + k+1;

            // M[i] = M[i] + M[k]*M[i,k]

            t1 = M[i][k];   // this is already reduced

            ZZ *x = M[i].elts() + (k+1);
            ZZ *y = M[k].elts() + (k+1);

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


void solve(ZZ_p& d, vec_ZZ_p& x, const mat_ZZ_p& A, const vec_ZZ_p& b)
{
   solve_impl(d, x, A, b, true);
}

void solve(ZZ_p& d, const mat_ZZ_p& A, vec_ZZ_p& x,  const vec_ZZ_p& b)
{
   solve_impl(d, x, A, b, false);
}

void inv(ZZ_p& d, mat_ZZ_p& X, const mat_ZZ_p& A)
{
   long n = A.NumRows();

   if (A.NumCols() != n)
      LogicError("inv: nonsquare matrix");

   if (n == 0) {
      set(d);
      X.SetDims(0, 0);
      return;
   }

   const ZZ& p = ZZ_p::modulus();

   ZZ t1, t2;
   ZZ pivot;
   ZZ pivot_inv;

   Vec<ZZVec> M;
   // scratch space

   sqr(t1, p);
   mul(t1, t1, n);

   M.SetLength(n);
   for (long i = 0; i < n; i++) {
      M[i].SetSize(n, t1.size());
      for (long j = 0; j < n; j++) 
         M[i][j] = rep(A[i][j]);
   }

   ZZ det;
   det = 1;


   Vec<long> P;
   P.SetLength(n);
   for (long k = 0; k < n; k++) P[k] = k;
   // records swap operations
   

   bool seq = 
      double(n)*double(n)*double(p.size())*double(p.size()) < PAR_THRESH;

   bool pivoting = false;

   for (long k = 0; k < n; k++) {

      long pos = -1;

      for (long i = k; i < n; i++) {
         rem(pivot, M[i][k], p);
         if (pivot != 0) {
            InvMod(pivot_inv, pivot, p);
            pos = i;
            break;
         }
      }

      if (pos != -1) {
         if (k != pos) {
            swap(M[pos], M[k]);
            NegateMod(det, det, p);
            P[k] = pos;
            pivoting = true;
         }

         MulMod(det, det, pivot, p);

         {
            // multiply row k by pivot_inv
            ZZ *y = &M[k][0];
            for (long j = 0; j < n; j++) {
               rem(t2, y[j], p);
               MulMod(y[j], t2, pivot_inv, p);
            }
            y[k] = pivot_inv;
         }


         NTL_GEXEC_RANGE(seq, n, first, last)  
         NTL_IMPORT(n)
         NTL_IMPORT(k)

         ZZ *y = &M[k][0]; 
         ZZ t1, t2;

         for (long i = first; i < last; i++) {
            if (i == k) continue; // skip row k

            ZZ *x = &M[i][0]; 
            rem(t1, x[k], p);
            NegateMod(t1, t1, p);
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
         ZZ *x = &M[i][0]; 

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




long gauss(mat_ZZ_p& M_in, long w)
{
   ZZ t1, t2;
   ZZ piv;

   long n = M_in.NumRows();
   long m = M_in.NumCols();

   if (w < 0 || w > m)
      LogicError("gauss: bad args");

   const ZZ& p = ZZ_p::modulus();

   Vec<ZZVec> M;
   sqr(t1, p);
   mul(t1, t1, n);

   M.SetLength(n);
   for (long i = 0; i < n; i++) {
      M[i].SetSize(m, t1.size());
      for (long j = 0; j < m; j++) {
         M[i][j] = rep(M_in[i][j]);
      }
   }

   long l = 0;
   for (long k = 0; k < w && l < n; k++) {

      long pos = -1;
      for (long i = l; i < n; i++) {
         rem(t1, M[i][k], p);
         M[i][k] = t1;
         if (pos == -1 && !IsZero(t1)) {
            pos = i;
         }
      }

      if (pos != -1) {
         swap(M[pos], M[l]);

         InvMod(piv, M[l][k], p);
         NegateMod(piv, piv, p);

         for (long j = k+1; j < m; j++) {
            rem(M[l][j], M[l][j], p);
         }

         bool seq =
            double(n-(l+1))*double(m-(k+1))*double(p.size())*double(p.size()) < PAR_THRESH;

         NTL_GEXEC_RANGE(seq, n-(l+1), first, last)
         NTL_IMPORT(m)
         NTL_IMPORT(k)
         NTL_IMPORT(l)

         ZZ t1, t2;


         for (long ii = first; ii < last; ii++) {
            long i = ii + l+1;

            // M[i] = M[i] + M[l]*M[i,k]*piv

            MulMod(t1, M[i][k], piv, p);

            clear(M[i][k]);

            ZZ *x = M[i].elts() + (k+1);
            ZZ *y = M[l].elts() + (k+1);

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







long gauss(mat_ZZ_p& M)
{
   return gauss(M, M.NumCols());
}

void image(mat_ZZ_p& X, const mat_ZZ_p& A)
{
   mat_ZZ_p M;
   M = A;
   long r = gauss(M);
   M.SetDims(r, M.NumCols());
   X = M;
}



void kernel(mat_ZZ_p& X, const mat_ZZ_p& A)
{
   long m = A.NumRows();
   long n = A.NumCols();

   const ZZ& p = ZZ_p::modulus();

   mat_ZZ_p M;

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

   Vec<ZZ_p> inverses;
   inverses.SetLength(m);

   for (long i = 0, j = -1; i < r; i++) {
      do {
         j++;
      } while (IsZero(M[i][j]));

      D[j] = i;
      inv(inverses[j], M[i][j]); 
   }

   bool seq = 
      double(m-r)*double(r)*double(r)*double(p.size())*double(p.size()) < PAR_THRESH;

   NTL_GEXEC_RANGE(seq, m-r, first, last)
   NTL_IMPORT(m)
   NTL_IMPORT(r)

   ZZ t1, t2;
   ZZ_p T3;

   for (long k = first; k < last; k++) {
      vec_ZZ_p& v = X[k];
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

   
void mul(mat_ZZ_p& X, const mat_ZZ_p& A, const ZZ_p& b_in)
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
   
void mul(mat_ZZ_p& X, const mat_ZZ_p& A, long b_in)
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

void diag(mat_ZZ_p& X, long n, const ZZ_p& d_in)  
{  
   ZZ_p d = d_in;
   X.SetDims(n, n);  
   long i, j;  
  
   for (i = 1; i <= n; i++)  
      for (j = 1; j <= n; j++)  
         if (i == j)  
            X(i, j) = d;  
         else  
            clear(X(i, j));  
} 

long IsDiag(const mat_ZZ_p& A, long n, const ZZ_p& d)
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


long IsZero(const mat_ZZ_p& a)
{
   long n = a.NumRows();
   long i;

   for (i = 0; i < n; i++)
      if (!IsZero(a[i]))
         return 0;

   return 1;
}

void clear(mat_ZZ_p& x)
{
   long n = x.NumRows();
   long i;
   for (i = 0; i < n; i++)
      clear(x[i]);
}


mat_ZZ_p operator+(const mat_ZZ_p& a, const mat_ZZ_p& b)
{
   mat_ZZ_p res;
   add(res, a, b);
   NTL_OPT_RETURN(mat_ZZ_p, res);
}

mat_ZZ_p operator*(const mat_ZZ_p& a, const mat_ZZ_p& b)
{
   mat_ZZ_p res;
   mul(res, a, b);
   NTL_OPT_RETURN(mat_ZZ_p, res);
}

mat_ZZ_p operator-(const mat_ZZ_p& a, const mat_ZZ_p& b)
{
   mat_ZZ_p res;
   sub(res, a, b);
   NTL_OPT_RETURN(mat_ZZ_p, res);
}


mat_ZZ_p operator-(const mat_ZZ_p& a)
{
   mat_ZZ_p res;
   negate(res, a);
   NTL_OPT_RETURN(mat_ZZ_p, res);
}


vec_ZZ_p operator*(const mat_ZZ_p& a, const vec_ZZ_p& b)
{
   vec_ZZ_p res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_ZZ_p, res);
}

vec_ZZ_p operator*(const vec_ZZ_p& a, const mat_ZZ_p& b)
{
   vec_ZZ_p res;
   mul_aux(res, a, b);
   NTL_OPT_RETURN(vec_ZZ_p, res);
}

void inv(mat_ZZ_p& X, const mat_ZZ_p& A)
{
   ZZ_p d;
   inv(d, X, A);
   if (d == 0) ArithmeticError("inv: non-invertible matrix");
}

void power(mat_ZZ_p& X, const mat_ZZ_p& A, const ZZ& e)
{
   if (A.NumRows() != A.NumCols()) LogicError("power: non-square matrix");

   if (e == 0) {
      ident(X, A.NumRows());
      return;
   }

   mat_ZZ_p T1, T2;
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

void random(mat_ZZ_p& x, long n, long m)
{
   x.SetDims(n, m);
   for (long i = 0; i < n; i++) random(x[i], m);
}

NTL_END_IMPL


#include <NTL/mat_lzz_p.h>

NTL_CLIENT



void FillRandom(Mat<zz_p>& A)
{
   long n = A.NumRows();
   long m = A.NumCols();
   for (long i = 0; i < n; i++)
      for (long j = 0; j < m; j++)
         random(A[i][j]);
}

void FillRandom1(Mat<zz_p>& A)
{
   long n = A.NumRows();
   long m = A.NumCols();

   long r;
   long choice = RandomBnd(5);
   
   if (choice == 0 || n == 0) {
      r = 0;
   }
   else if (choice == 1) {
      r = min(n, 1+RandomBnd(10));
   }
   else {
      r = 1+RandomBnd(n);
   }

   Mat<zz_p> B, C;

   B.SetDims(n, n);
   FillRandom(B);

   C.SetDims(n, m);
   for (long i = 0; i < r; i++)
      for (long j = 0; j < m; j++)
         random(C[i][j]);

   mul(A, B, C);
}

void FillRandom(Vec<zz_p>& A)
{
   long n = A.length();
   for (long i = 0; i < n; i++)
      random(A[i]);
}

long old_gauss(mat_zz_p& M, long w)
{
   using NTL_NAMESPACE::negate;
   long k, l;
   long i, j;
   long pos;
   zz_p t1, t2, t3;
   zz_p *x, *y;

   long n = M.NumRows();
   long m = M.NumCols();

   if (w < 0 || w > m)
      LogicError("gauss: bad args");

   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();
   long T1, T2;

   l = 0;
   for (k = 0; k < w && l < n; k++) {

      pos = -1;
      for (i = l; i < n; i++) {
         if (!IsZero(M[i][k])) {
            pos = i;
            break;
         }
      }

      if (pos != -1) {
         swap(M[pos], M[l]);

         inv(t3, M[l][k]);
         negate(t3, t3);

         for (i = l+1; i < n; i++) {
            // M[i] = M[i] + M[l]*M[i,k]*t3

            mul(t1, M[i][k], t3);

            T1 = rep(t1);
            mulmod_precon_t T1pinv = PrepMulModPrecon(T1, p, pinv); 

            clear(M[i][k]);

            x = M[i].elts() + (k+1);
            y = M[l].elts() + (k+1);

            for (j = k+1; j < m; j++, x++, y++) {
               // *x = *x + (*y)*t1

               T2 = MulModPrecon(rep(*y), T1, p, T1pinv);
               T2 = AddMod(T2, rep(*x), p);
               (*x).LoopHole() = T2;
            }
         }

         l++;
      }
   }

   return l;
}

long old_gauss(mat_zz_p& M)
{
   return old_gauss(M, M.NumCols());
}

void old_image(mat_zz_p& X, const mat_zz_p& A)
{
   mat_zz_p M;
   M = A;
   long r = old_gauss(M);
   M.SetDims(r, M.NumCols());
   X = M;
}

int main(int argc, char **argv)
{
   ZZ seed;
   RandomLen(seed, 30);
   SetSeed(seed);
   cerr << "\nseed=" << seed << "\n";

   long iters = 100; 


#if 1
   cerr << "testing multiplication";
   for (long cnt = 0; cnt < iters; cnt++) {
      cerr << ".";

      long bnd = (cnt%2) ? 25 : 2000;
 
      long len = RandomBnd(NTL_SP_NBITS-3)+4;
      long n = RandomBnd(bnd);
      long l = RandomBnd(bnd);
      long m = RandomBnd(bnd);

      long p = RandomPrime_long(len);
      zz_p::init(p);

      Mat<zz_p> A, B, X;

      A.SetDims(n, l);
      B.SetDims(l, m);

      FillRandom(A);
      FillRandom(B);

      X.SetDims(n, m);

      vec_zz_p R;

      R.SetLength(m);
      for (long i = 0; i < m; i++) random(R[i]);

      mul(X, A, B);

      if (X*R != A*(B*R)) 
         cerr << "*\n*\n*\n*\n*\n*********** oops " << len << " " << n << " " << l << " " 
              << m << "\n";
   }
#endif

#if 1
   cerr << "\ntesting inversion";
   for (long cnt = 0; cnt < iters; cnt++) {
      cerr << ".";
      long bnd = (cnt%2) ? 25 : 1500;
 
      long len = RandomBnd(NTL_SP_NBITS-3)+4;
      long n = RandomBnd(bnd);

      long p = RandomPrime_long(len);
      zz_p::init(p);

      Mat<zz_p> A, X;

      A.SetDims(n, n);

      FillRandom(A);


      vec_zz_p R;

      R.SetLength(n);
      for (long i = 0; i < n; i++) random(R[i]);

      zz_p d;

      inv(d, X, A);

      if (d != 0) {
	 if (R != A*(X*R)) 
	    cerr << "\n*\n*\n*\n*\n*********** oops " << len << " " << n << "\n";
      }
      else {
         cerr << "[singular]";
      }
   }
#endif

#if 1
   cerr << "\ntesting solve";
   for (long cnt = 0; cnt < iters; cnt++) {
      cerr << ".";
      long bnd = (cnt%2) ? 25 : 2000;
 
      long len = RandomBnd(NTL_SP_NBITS-3)+4;
      long n = RandomBnd(bnd);

      long p = RandomPrime_long(len);
      zz_p::init(p);

      Mat<zz_p> A;

      A.SetDims(n, n);
      FillRandom(A);

      Vec<zz_p> x, b;
      b.SetLength(n);
      FillRandom(b);

      zz_p d;

      solve(d, A, x, b);

      if (d != 0) {
	 if (A*x != b)
	    cerr << "\n*\n*\n*\n*\n*********** oops " << len << " " << n << "\n";
      }
      else {
         cerr << "[singular]";
      }
   }
#endif

#if 1
   cerr << "\ntesting image and kernel";
   for (long cnt = 0; cnt < 4*iters; cnt++) {
      cerr << ".";
      long bnd = (cnt%2) ? 25 : 1500;
 
      long len = RandomBnd(NTL_SP_NBITS-3)+4;
      long n = RandomBnd(bnd);
      long m = RandomBnd(bnd);

      long p = RandomPrime_long(len);
      zz_p::init(p);

      Mat<zz_p> A;

      A.SetDims(n, m);
      FillRandom1(A);

      Mat<zz_p> im, im1, ker1;

      old_image(im, A);
      image(im1, A);
      kernel(ker1, A);

      //cerr << n << ":" << ":" << m << ":" << im.NumRows() << "||";


      if (im != im1 || !IsZero(ker1*A) || im1.NumRows() + ker1.NumRows() != n) {
         cerr << "\n*\n*\n*\n*\n*********** oops " << len << " " << n << m << "\n";
      }
   }
#endif

   cerr << "\n";

}


#include <NTL/ZZ_pX.h>
#include <NTL/ZZX.h>
#include <NTL/BasicThreadPool.h>

NTL_CLIENT


#define ITER (500)


void multest()
{
   cerr << "mul";
   for (long iter = 0; iter < ITER; iter++) {
      if (iter % 100 == 0) cerr << ".";

      long da, db;

      if (RandomBnd(2)) {
         da = RandomBnd(5000) + 100;
         db = RandomBnd(5000) + 100;
      }
      else {
         da = RandomBnd(200) + 1;
         db = RandomBnd(200) + 1;
      }

      ZZ_pX a, b, c1, c2;

      random(a, da);
      random(b, db);

      FFTMul(c1, a, b);

      ZZX A, B, C;
      conv(A, a);
      conv(B, b);
      mul(C, A, B);
      conv(c2, C);

      if (c1 != c2) {
         cerr << "******* oops\n";
         break;
      }
   }

   cerr << "\n";
}


void sqrtest()
{
   cerr << "sqr";
   for (long iter = 0; iter < ITER; iter++) {
      if (iter % 100 == 0) cerr << ".";

      long da = RandomBnd(5000) + 100;
      long db = RandomBnd(5000) + 100;

      ZZ_pX a, b, c1, c2;

      random(a, da);

      if (deg(a) < 80) {
         cerr << "*";
         continue;
      }

      FFTSqr(c1, a);

      ZZX A, B, C;
      conv(A, a);
      sqr(C, A);
      conv(c2, C);

      if (c1 != c2) {
         cerr << "******* oops\n";
         break;
      }
   }

   cerr << "\n";
}




void mulmodtest()
{
   cerr << "mulmod";
   for (long iter = 0; iter < ITER; iter++) {
      if (iter % 100 == 0) cerr << ".";

      long n = RandomBnd(5000) + 300;
      long da = RandomBnd(n)+1;
      long db = RandomBnd(n)+1;

      if (RandomBnd(2)) { da = n; db = n; }

      ZZ_pX f;
      random(f, n);
      SetCoeff(f, n);
      ZZ_pXModulus F(f);

      ZZ_pX a, b, c1, c2;
      random(a, da);
      random(b, db);

      MulMod(c1, a, b, F);

      ZZX A, B, C;
      conv(A, a);
      conv(B, b);
      mul(C, A, B);
      conv(c2, C);
      rem(c2, c2, F);

      if (c1 != c2) {
         cerr << "******** oops\n";
         break;
      }
   }

   cerr << "\n";
}


void sqrmodtest()
{
   cerr << "sqrmod";
   for (long iter = 0; iter < ITER; iter++) {
      if (iter % 100 == 0) cerr << ".";

      long n = RandomBnd(5000) + 300;
      long da = RandomBnd(n)+1;
      long db = RandomBnd(n)+1;

      if (RandomBnd(2)) { da = n; db = n; }

      ZZ_pX f;
      random(f, n);
      SetCoeff(f, n);
      ZZ_pXModulus F(f);

      ZZ_pX a, b, c1, c2;
      random(a, da);
      random(b, db);

      SqrMod(c1, a, F);

      ZZX A, B, C;
      conv(A, a);
      conv(B, b);
      sqr(C, A);
      conv(c2, C);
      rem(c2, c2, F);

      if (c1 != c2) {
         cerr << "******** oops\n";
         break;
      }
   }

   cerr << "\n";
}



void mulmod1test()
{
   cerr << "mulmod1";
   for (long iter = 0; iter < ITER; iter++) {
      if (iter % 100 == 0) cerr << ".";

      long n = RandomBnd(5000) + 300;
      long da = RandomBnd(n)+1;
      long db = RandomBnd(n)+1;

      if (RandomBnd(2)) { da = n; db = n; }

      ZZ_pX f;
      random(f, n);
      SetCoeff(f, n);
      ZZ_pXModulus F(f);

      ZZ_pX a, b, c1, c2;
      random(a, da);
      random(b, db);

      ZZ_pXMultiplier bb;
      build(bb, b, F);

      MulMod(c1, a, bb, F);

      ZZX A, B, C;
      conv(A, a);
      conv(B, b);
      mul(C, A, B);
      conv(c2, C);
      rem(c2, c2, F);

      if (c1 != c2) {
         cerr << "******** oops\n";
         break;
      }
   }

   cerr << "\n";
}


namespace NTL {

void CopyReverse(ZZ_pX& x, const ZZ_pX& a, long lo, long hi);

}



struct ZZ_pXTransMultiplier {
   ZZ_pX f0, fbi, b;
   long shamt, shamt_fbi, shamt_b;
};




void build(ZZ_pXTransMultiplier& B, const ZZ_pX& b, const ZZ_pXModulus& F)
{
   long db = deg(b);

   if (db >= F.n) LogicError("build TransMultiplier: bad args");

   ZZ_pX t;

   LeftShift(t, b, F.n-1);
   div(t, t, F);

   // we optimize for low degree b

   long d;

   d = deg(t);
   if (d < 0)
      B.shamt_fbi = 0;
   else
      B.shamt_fbi = F.n-2 - d;

   CopyReverse(B.fbi, t, 0, d);

   // The following code optimizes the case when
   // f = X^n + low degree poly

   trunc(t, F.f, F.n);
   d = deg(t);
   if (d < 0)
      B.shamt = 0;
   else
      B.shamt = d;

   CopyReverse(B.f0, t, 0, d);

   if (db < 0)
      B.shamt_b = 0;
   else
      B.shamt_b = db;

   CopyReverse(B.b, b, 0, db);
}



void TransMulMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXTransMultiplier& B,
               const ZZ_pXModulus& F)
{
   if (deg(a) >= F.n) LogicError("TransMulMod: bad args");

   ZZ_pX t1, t2;

   mul(t1, a, B.b);
   RightShift(t1, t1, B.shamt_b);

   mul(t2, a, B.f0);
   RightShift(t2, t2, B.shamt);
   trunc(t2, t2, F.n-1);

   mul(t2, t2, B.fbi);
   if (B.shamt_fbi > 0) LeftShift(t2, t2, B.shamt_fbi);
   trunc(t2, t2, F.n-1);
   LeftShift(t2, t2, 1);

   sub(x, t1, t2);
}



void UpdateMap(vec_ZZ_p& x, const vec_ZZ_p& a,
         const ZZ_pXTransMultiplier& B, const ZZ_pXModulus& F)
{
   ZZ_pX xx;
   TransMulMod(xx, to_ZZ_pX(a), B, F);
   x = xx.rep;
}



void updatetest()
{
   cerr << "update";
   for (long iter = 0; iter < ITER; iter++) {
      if (iter % 100 == 0) cerr << ".";

      long n = RandomBnd(5000) + 300;
      long da = RandomBnd(n)+1;
      long db = RandomBnd(n)+1;

      if (RandomBnd(2)) { da = n; db = n; }

      ZZ_pX f;
      random(f, n);
      SetCoeff(f, n);
      ZZ_pXModulus F(f);

      ZZ_pX a, b;
      random(a, da);
      random(b, db);

      ZZ_pXMultiplier bb1;
      build(bb1, b, F);

      ZZ_pXTransMultiplier bb2;
      build(bb2, b, F);

      Vec<ZZ_p> x1, x2;

      UpdateMap(x1, a.rep, bb1, F);
      UpdateMap(x2, a.rep, bb2, F);


      if (x1 != x2) {
         cerr << "******** oops\n";
         break;
      }
   }

   cerr << "\n";
}

void divremtest()
{
   cerr << "divrem";
   for (long iter = 0; iter < ITER; iter++) {
      if (iter % 100 == 0) cerr << ".";

      long n = RandomBnd(5000) + 300;
      long dq = RandomBnd(n);


      ZZ_pX f;
      random(f, n);
      SetCoeff(f, n);
      ZZ_pXModulus F(f);

      ZZ_pX a, q, r, q1, r1;

      random(a, 2*n-1);

      DivRem(q, r, a, F);
      rem(r1, a, F);
      div(q1, a, F);

      if (deg(r) >= n || a != q*f + r || q != q1 || r != r1) {
         cerr << "******** oops\n";
         break;
      }
   }

   cerr << "\n";
}


int main()
{
   ZZ p;
   GenPrime(p, 100);

   ZZ_p::init(p);

   multest();
   sqrtest();
   mulmodtest();
   sqrmodtest();
   mulmod1test();
   divremtest();
   updatetest();

#ifdef NTL_THREAD_BOOST

   GenPrime(p, 500);
   ZZ_p::init(p);

   SetNumThreads(4);
   cerr << "numthreads=4\n";

   multest();
   sqrtest();
   mulmodtest();
   sqrmodtest();
   mulmod1test();
   divremtest();
   updatetest();

#endif

}


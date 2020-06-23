#include <NTL/lzz_pX.h>

NTL_CLIENT

#define ITER (500)

void multest()
{
   cerr << "mul";
   for (long iter = 0; iter < ITER; iter++) {
      if (iter % 100 == 0) cerr << ".";

      long da = RandomBnd(5000) + 100;
      long db = RandomBnd(5000) + 100;

      zz_pX a, b, c1, c2;

      random(a, da);
      random(b, db);

      if (deg(a) < 80 || deg(b) < 80) {
         cerr << "*";
         continue;
      }

      FFTMul(c1, a, b);
      PlainMul(c2, a, b);

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

      zz_pX a, b, c1, c2;

      random(a, da);

      if (deg(a) < 80) {
         cerr << "*";
         continue;
      }

      FFTSqr(c1, a);
      PlainMul(c2, a, a);

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

      zz_pX f;
      random(f, n);
      SetCoeff(f, n);
      zz_pXModulus F(f);

      zz_pX a, b, c1, c2;
      random(a, da);
      random(b, db);

      MulMod(c1, a, b, F);
      PlainMul(c2, a, b);
      rem(c2, c2, f);

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

      zz_pX f;
      random(f, n);
      SetCoeff(f, n);
      zz_pXModulus F(f);

      zz_pX a, b, c1, c2;
      random(a, da);
      random(b, db);

      SqrMod(c1, a, F);

      PlainMul(c2, a, a);
      rem(c2, c2, f);

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

      zz_pX f;
      random(f, n);
      SetCoeff(f, n);
      zz_pXModulus F(f);

      zz_pX a, b, c1, c2;
      random(a, da);
      random(b, db);

      zz_pXMultiplier bb;
      build(bb, b, F);

      MulMod(c1, a, bb, F);

      PlainMul(c2, a, b);
      rem(c2, c2, f);

      if (c1 != c2) {
         cerr << "******** oops\n";
         break;
      }
   }

   cerr << "\n";
}


namespace NTL {

void CopyReverse(zz_pX& x, const zz_pX& a, long lo, long hi);

}



struct zz_pXTransMultiplier {
   zz_pX f0, fbi, b;
   long shamt, shamt_fbi, shamt_b;
};




void build(zz_pXTransMultiplier& B, const zz_pX& b, const zz_pXModulus& F)
{
   long db = deg(b);

   if (db >= F.n) LogicError("build TransMultiplier: bad args");

   zz_pX t;

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



void TransMulMod(zz_pX& x, const zz_pX& a, const zz_pXTransMultiplier& B,
               const zz_pXModulus& F)
{
   if (deg(a) >= F.n) LogicError("TransMulMod: bad args");

   zz_pX t1, t2;

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



void UpdateMap(vec_zz_p& x, const vec_zz_p& a,
         const zz_pXTransMultiplier& B, const zz_pXModulus& F)
{
   zz_pX xx;
   TransMulMod(xx, to_zz_pX(a), B, F);
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

      zz_pX f;
      random(f, n);
      SetCoeff(f, n);
      zz_pXModulus F(f);

      zz_pX a, b;
      random(a, da);
      random(b, db);

      zz_pXMultiplier bb1;
      build(bb1, b, F);

      zz_pXTransMultiplier bb2;
      build(bb2, b, F);

      Vec<zz_p> x1, x2;

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


      zz_pX f;
      random(f, n);
      SetCoeff(f, n);
      zz_pXModulus F(f);

      zz_pX a, q, r, q1, r1;

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
   long p;
   p = GenPrime_long(NTL_SP_NBITS);

   zz_p::init(p);

   multest();
   sqrtest();
   mulmodtest();
   sqrmodtest();
   mulmod1test();
   divremtest();
   updatetest();

   zz_p::FFTInit(0);

   cerr << "FFT Prime\n";

   multest();
   sqrtest();
   mulmodtest();
   sqrmodtest();
   mulmod1test();
   divremtest();
   updatetest();

}


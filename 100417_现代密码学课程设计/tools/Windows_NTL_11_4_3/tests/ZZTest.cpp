
#include <NTL/ZZ.h>

NTL_CLIENT

#define CHECK(x) do { if (!(x)) { cerr << "FAIL\n"; return -1; } } while(0)

int main()
{
   ZZ seed;
   RandomLen(seed, 30);
   SetSeed(seed);
   cerr << "\nseed=" << seed << "\n";

   cerr << "\nvalidating RandomLen...";
   for (long i = 1; i < 10000; i++) {
      ZZ x;
      RandomLen(x, i);
      CHECK(x.validate() && NumBits(x) == i);
   }

   cerr << "\nvalidating basic arithmetic...";
   for (long i = 0; i < 200000; i++) {
      long a_len = RandomBnd(8000)+5;
      long b_len = RandomBnd(8000)+5;
      long c_len = RandomBnd(8000)+5;
      long d_len = RandomBnd(8000)+5;

      ZZ a, b, c, d;
      RandomLen(a, a_len);
      RandomLen(b, b_len);
      RandomLen(c, c_len);
      RandomLen(d, d_len);

      ZZ t1, t2;
      t1 = (a-b)*(c-d);
      t2 = a*c - a*d - b*c + b*d; 
      CHECK(t1.validate() && t2.validate() && t1 == t2);

      long p = 7919;

      long d1 = rem(t1, p);
      long d2 = MulMod(rem(a-b, p), rem(c-d, p), p);
      CHECK(d1 == d2);
   }

   cerr << "\nvalidating DivRem...";
   for (long i = 0; i < 200000; i++) {
      long b_len = RandomBnd(8000)+5;
      long q_len = RandomBnd(8000)+5;

      ZZ a, b, q, r, q1, r1;
      RandomLen(b, b_len);
      RandomLen(q, q_len);
      RandomBnd(r, b);
      a = b*q + r;
     
      DivRem(q1, r1, a, b);

      CHECK(q1.validate() && r1.validate() && q == q1 && r == r1);
   }

   cerr << "\nvalidating mul...";
   for (long i = 0; i < 1000000; i++) {
      long a_len = RandomBnd(1000)+1;
      long b_len = RandomBnd(1000)+1;

      ZZ a, b, c;

      RandomLen(a, a_len);
      RandomLen(b, b_len);

      if (RandomBnd(2)) a = -a;
      if (RandomBnd(2)) b = -b;

      long p = 7919;
      long r = MulMod(rem(a, p), rem(b, p), p);
      long s = MulMod(rem(a, p), rem(a, p), p);

      switch (RandomBnd(5)) {
      case 0:
         mul(c, a, b);
         CHECK(c.validate() && rem(c, p) == r);
         break;

      case 1:
         mul(a, a, b);
         CHECK(a.validate() && rem(a, p) == r);
         break;

      case 2:
         mul(b, a, b);
         CHECK(b.validate() && rem(b, p) == r);
         break;

      case 3:
         mul(c, a, a);
         CHECK(c.validate() && rem(c, p) == s);
         break;

      case 4:
         mul(a, a, a);
         CHECK(a.validate() && rem(a, p) == s);
         break;
      }
   }

   cerr << "\nvalidating squaring...";
   for (long i = 0; i < 1000000; i++) {
      long a_len = RandomBnd(1000)+1;

      ZZ a, b, a1, a2, c;
      RandomLen(a, a_len);

      if (RandomBnd(2)) a = -a;

      a1 = a;
      a2 = a;

      if (RandomBnd(2)) {
         sqr(b, a);
         mul(c, a1, a2);
         CHECK(b.validate() && c.validate() && b == c);
      }
      else {
         sqr(a, a);
         mul(c, a1, a2);
         CHECK(a.validate() && c.validate() && a == c);
      }
   }

   cerr << "\nvalidating SqrRoot...";
   for (long i = 0; i < 200000; i++) {
      long a_len = RandomBnd(8000)+5;

      ZZ a, b;
      RandomLen(a, a_len);

      SqrRoot(b, a);
      CHECK(b.validate() && b*b <= a && (b+1)*(b+1) > a);
   }


   cerr << "\nvalidating shifts...";
   for (long i = 0; i < 200000; i++) {
      long a_len = RandomBnd(5000)+5;
      long shamt = RandomBnd(a_len+100);

      ZZ a;
      RandomLen(a, a_len);

      ZZ t = ZZ(1);
      for (long k = 0; k < shamt; k++) t += t;

      ZZ xL, xR; 
      LeftShift(xL, a, shamt);
      RightShift(xR, a, shamt);

      CHECK(xL.validate() && xR.validate());
      CHECK(xL == a*t && xR == a/t);
   }


   cerr << "\nvalidating Preconditioned Remainder...";
   for (long i = 0; i < 1000000; i++) {
      sp_ZZ_reduce_struct red_struct;

      long p_len = RandomBnd(NTL_SP_NBITS-1)+2;
      long p = RandomLen_long(p_len);

      long a_len = RandomBnd(30000)+5;
      ZZ a;
      RandomLen(a, a_len);


      red_struct.build(p);
      long r1 = red_struct.rem(a);
      long r2 = rem(a, p);

      CHECK(r1 == r2);
   }

   cerr << "\nvalidating MulAddTo...";
   for (long i = 0; i < 1000000; i++) {
      long a_len = RandomBnd(4000)+5;
      long b_len = RandomBnd(4000)+5;
      long c_len = RandomBnd(4000)+5;
      long d_len = RandomBnd(4000)+5;

      ZZ a, b, c, d;
      RandomLen(a, a_len);
      RandomLen(b, b_len);
      RandomLen(c, c_len);
      RandomLen(d, d_len);

      ZZ t1, t2;
      t1 = a-b;
      t2 = c-d;

      long s_len, s;
      s_len = RandomBnd(NTL_NSP_NBITS)+1;
      s = RandomLen_long(s_len);
      if (RandomBnd(2)) s = -s;

      ZZ r1, r2;
      r1 = t1;
      MulAddTo(r1, t2, s);

      r2 = t1 + t2*s;
      CHECK(r1.validate() && r2.validate() && r1 == r2);
   }

   cerr << "\nvalidating GCD...";
   for (long i = 0; i < 1000000; i++) {
      long a_len = RandomBnd(1000)+1;
      long b_len = RandomBnd(1000)+1;
      long c_len = RandomBnd(200)+1;

      ZZ a, b, c;
      RandomLen(a, a_len);
      RandomLen(b, b_len);
      RandomLen(c, c_len);

      a *= c;
      b *= c;

      if (RandomBnd(2)) a = -a;
      if (RandomBnd(2)) b = -b;

      ZZ d, s, t, d1;

      XGCD(d, s, t, a, b);
      GCD(d1, a, b);

      CHECK(d.validate() && s.validate() && t.validate() && d1.validate());
      CHECK(d == d1 && d == a*s + b*t);
      CHECK(divide(a, d) && divide(b, d)); 

      CHECK(abs(s) <= 1 || 2*d*abs(s) < abs(b));
      CHECK(abs(t) <= 1 || 2*d*abs(t) < abs(a));

      if (a < 0) { a = -a; s = -s; }
      if (b < 0) { b = -b; t = -t; }
      if (a < b) { swap(a, b); swap(s, t); }
      
      // so now we have a >= b >= 0
      // check that s in (-b/2*d, b/2*d]
      CHECK(2*d*s > -b && 2*d*s <= b);
   }

   cerr << "\nvalidating InvMod...";
   for (long i = 0; i < 100000; i++) {
      long n_len = RandomBnd(4000)+4;
      
      ZZ a, n, x;
      RandomLen(n, n_len);
      RandomBnd(a, n);

      long r = InvModStatus(x, a, n);
      CHECK((r == 0 && (x * a) % n == 1 && 0 <= x && x < n) || 
            (r == 1 && x != 1 && x == GCD(a, n)) );
   }

   cerr << "\nvalidating RatRecon...";

   // This exercises RatRecon using the example from Section 4.6.1
   // in A Computational Introduction to Number Theory

   for (long i = 0; i < 100000; i++) {
      long m_len = RandomBnd(4000)+5;

      ZZ m;
      RandomLen(m, m_len);

      ZZ t;
      RandomBnd(t, m);
      t += 1;

      ZZ s;
      RandomBnd(s, t);

      ZZ bnd = 2*m*m;

      long k = 0;
      ZZ ten_k = ZZ(1);
      while (ten_k <= bnd) { ten_k *= 10; k++; } 

      ZZ z = (s*ten_k)/t;

      ZZ a, r, b;
      long res = ReconstructRational(r, b, z, ten_k, m, m); 

      CHECK(res == 1);

      a = (b*z - r)/ten_k;
      CHECK(a*t == b*s);
   }

   cerr << "\n";

   return 0;
}

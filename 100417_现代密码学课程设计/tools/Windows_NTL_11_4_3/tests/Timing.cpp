
#include <NTL/ZZ_pXFactoring.h>
#include <NTL/ZZXFactoring.h>
#include <NTL/GF2XFactoring.h>
#include <NTL/GF2EXFactoring.h>

NTL_CLIENT


#define TIME_IT(t, action) \
do { \
   double _t0, _t1; \
   long _iter = 1; \
   long _cnt = 0; \
   do { \
      _t0 = GetTime(); \
      for (long _i = 0; _i < _iter; _i++) { action; _cnt++; } \
      _t1 = GetTime(); \
   } while ( _t1 - _t0 < 4 && (_iter *= 2)); \
   t = (_t1 - _t0)/_iter; \
} while(0) 




int main()
{
   double t;


   long k = 1000;
   long n = 1000;

   {
      SetSeed(conv<ZZ>(1));
      ZZ p = RandomPrime_ZZ(k);

      ZZ_p::init(p);

      ZZ x, y, z, w, s1, s2;

      SetSeed(conv<ZZ>(2));
      RandomBnd(x, p);


      SetSeed(conv<ZZ>(3));
      RandomBnd(y, p);

      TIME_IT(t, mul(z, x, y));
      cout << "multiply 1000-bit ints: " << t << "\n";

      TIME_IT(t, sqr(w, x));
      cout << "square 1000-bit ints: " << t << "\n";

      TIME_IT(t, rem(w, z, p));
      cout << "remainder 2000/1000-bit ints: " << t << "\n";

      TIME_IT(t, GCD(w, x, y));
      cout << "gcd 1000-bit ints: " << t << "\n";

      TIME_IT(t, XGCD(w, s1, s2, x, y));
      cout << "xgcd 1000-bit ints: " << t << "\n";

      TIME_IT(t, PowerMod(w, x, y, p));
      cout << "power mod 1000-bit ints: " << t << "\n";
      

      ZZ_pX a, b, c;

      SetSeed(conv<ZZ>(4));
      random(a, n);

      SetSeed(conv<ZZ>(5));
      random(b, n);

      mul(c, a, b);

      TIME_IT(t, mul(c, a, b));
      cout << "multiply degree-1000 poly mod 1000-bit prime: " << t << "\n";


      ZZ_pX f;
      SetSeed(conv<ZZ>(6));
      random(f, n);
      SetCoeff(f, n);

      ZZ_pX A, B;

      SetSeed(conv<ZZ>(7));
      random(A, 2*(deg(f)-1)); 

      TIME_IT(t, rem(B, A, f));
      cout << "remainder degree-2000/1000 poly mod 1000-bit prime: " << t << "\n";

      ZZ_pXModulus F(f);

      TIME_IT(t, rem(B, A, F));
      cout << "preconditioned remainder degree-2000/1000 poly mod 1000-bit prime: " << t << "\n";


      TIME_IT(t, GCD(a, b));
      cout << "gcd degree-1000 poly mod 1000-bit prime: " << t << "\n";


      ZZX AA = conv<ZZX>(a);
      ZZX BB = conv<ZZX>(b);
      ZZX CC;


      TIME_IT(t, mul(CC, AA, BB));
      cout << "multiply degree-1000 int poly with 1000-bit coeffs: " << t << "\n";

      cout << "\n";
      cout << "factoring degree-1000 poly mod 1000-bit prime...\n";
      TIME_IT(t, CanZass(f, _cnt == 0));
      cout << "...total time = " << t << "\n\n";
   }
   {
      n = 500;
      k = 500;

      SetSeed(conv<ZZ>(8));
      GF2X p = BuildRandomIrred(BuildIrred_GF2X(k));

      GF2E::init(p);

      GF2X x, y, z, w;

      SetSeed(conv<ZZ>(9));
      random(x, deg(p));


      SetSeed(conv<ZZ>(10));
      random(y, deg(p));

      TIME_IT(t, mul(z, x, y));
      cout << "multiply 500-bit GF2Xs: " << t << "\n";


      TIME_IT(t, rem(w, z, p));
      cout << "remainder 1000/500-bit GF2Xs: " << t << "\n";

      TIME_IT(t, GCD(w, x, y));
      cout << "gcd 500-bit GF2Xs: " << t << "\n";

      SetSeed(conv<ZZ>(11));
      GF2X fff;
      random(fff, k);
      SetCoeff(fff, k);

      cout << "\n";
      TIME_IT(t, CanZass(fff, 0));
      cout << "factoring degree-500 GF2X: " << t << "\n";


      TIME_IT(t, GCD(w, x, y));
      cout << "gcd 500-bit GF2X: " << t << "\n";

      GF2EX a, b, c;

      SetSeed(conv<ZZ>(12));
      random(a, n);

      SetSeed(conv<ZZ>(13));
      random(b, n);

      mul(c, a, b);

      TIME_IT(t, mul(c, a, b));
      cout << "multiply degree-500 poly mod 500-bit GF2X: " << t << "\n";



      GF2EX f;
      SetSeed(conv<ZZ>(14));
      random(f, n);
      SetCoeff(f, n);

      GF2EX A, B;

      SetSeed(conv<ZZ>(15));
      random(A, 2*(deg(f)-1)); 

      TIME_IT(t, rem(B, A, f));
      cout << "remainder degree-1000/500 poly mod 500-bit GF2X: " << t << "\n";

      GF2EXModulus F(f);

      TIME_IT(t, rem(B, A, F));
      cout << "preconditioned remainder degree-1000/500 poly mod 500-bit GF2X: " << t << "\n";


      TIME_IT(t, GCD(a, b));
      cout << "gcd degree-500 poly mod 500-bit GF2X: " << t << "\n";


      f = f >> n/2;
      cout << "\n";
      cout << "factoring degree-500 poly mod 500-bit GF2X...\n";
      TIME_IT(t, CanZass(f, _cnt == 0));
      cout << "\n...total time = " << t << "\n";
   }
}

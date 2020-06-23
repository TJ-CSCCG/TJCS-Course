
#include <NTL/GF2XFactoring.h>
#include <NTL/GF2EXFactoring.h>


NTL_OPEN_NNS

void PlainMul(GF2EX&, const GF2EX&, const GF2EX&);

NTL_CLOSE_NNS

NTL_CLIENT

int main()
{
   GF2X p;

   BuildIrred(p, 200);

   GF2E::init(p);

   GF2EX f;

   SetCoeff(f, 41);
   SetCoeff(f, 1);
   SetCoeff(f, 0);

   GF2X a;
   SetCoeff(a, 117);
   SetCoeff(a, 10);
   SetCoeff(a, 0);

   GF2EX g, h;
   SetX(g);
   SetCoeff(g, 0, to_GF2E(a));

   MinPolyMod(h, g, f);

   f = h;

   vec_pair_GF2EX_long u;

   CanZass(u, f, 1);

   cerr << "factorization pattern:";
   long i;

   for (i = 0; i < u.length(); i++) {
      cerr << " ";
      long k = u[i].b;
      if (k > 1)
         cerr << k << "*";
      cerr << deg(u[i].a);
   }

   cerr << "\n\n\n";

   GF2EX ff;
   mul(ff, u);

   if (f != ff || u.length() != 11) {
      cerr << "GF2EXTest NOT OK\n";
      return 1;
   }

   {

   cerr << "multiplication test...\n";

   BuildIrred(p, 512);
   GF2E::init(p);

   GF2EX A, B, C, C1;


   random(A, 512);
   random(B, 512);

   double t;
   long i;

   t = GetTime();
   for (i = 0; i < 10; i++) PlainMul(C, A, B);
   t = GetTime() - t;
   cerr << "time for plain mul of degree 511 over GF(2^512): " << (t/10) << "s\n";

   t = GetTime();
   for (i = 0; i < 10; i++) mul(C1, A, B);
   t = GetTime() - t;
   cerr << "time for karatsuba mul of degree 511 over GF(2^512): " << (t/10) << "s\n";

   if (C != C1) {
      cerr << "GF2EXTest NOT OK\n";
      return 1;
   }

   }

   {

   cerr << "multiplication test...\n";

   BuildIrred(p, 16);
   GF2E::init(p);

   GF2EX A, B, C, C1;


   random(A, 512);
   random(B, 512);

   double t;

   t = GetTime();
   for (i = 0; i < 10; i++) PlainMul(C, A, B);
   t = GetTime() - t;
   cerr << "time for plain mul of degree 511 over GF(2^16): " << (t/10) << "s\n";

   t = GetTime();
   for (i = 0; i < 10; i++) mul(C1, A, B);
   t = GetTime() - t;
   cerr << "time for karatsuba mul of degree 511 over GF(2^16): " << (t/10) << "s\n";

   if (C != C1) {
      cerr << "GF2EXTest NOT OK\n";
      return 1;
   }

   }

   cerr << "GF2EXTest OK\n";
   return 0;
}


#include <NTL/ZZ_pXFactoring.h>
#include <NTL/ZZ_pEXFactoring.h>

unsigned long exception_counter = 0;

NTL_CLIENT

int main()
{
   ZZ_p::init(to_ZZ(17));

   ZZ_pX P;
   BuildIrred(P, 10);

   ZZ_pE::init(P);

   ZZ_pEX f, g, h;

   random(f, 20);
   SetCoeff(f, 20);

   random(h, 20);

   g = MinPolyMod(h, f);

   if (deg(g) < 0) TerminalError("bad ZZ_pEXTest (1)");
   if (CompMod(g, h, f) != 0)
      TerminalError("bad ZZ_pEXTest (2)");


   
   vec_pair_ZZ_pEX_long v;

   long n = 100;

   random(f, n);
   SetCoeff(f, n);

   double running_counter = 100;

   bool done = false;

   while (!done) {
      done = true;
      running_counter *= 1.521;
      exception_counter = running_counter;
      cerr << "counter = " << exception_counter << "\n";
      try {
         CanZass(v, f, 1);
      }
      catch(...) {
         cerr << "\n**** caught exception -- retry...\n";
         done = false;
      }
   }

   exception_counter = 0;
   

   g = mul(v);
   if (f != g) cerr << "oops1\n";

   long i;
   for (i = 0; i < v.length(); i++)
      if (!DetIrredTest(v[i].a))
         TerminalError("bad ZZ_pEXTest (3)");


}

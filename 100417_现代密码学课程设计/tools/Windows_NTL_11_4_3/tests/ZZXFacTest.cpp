#include <NTL/ZZXFactoring.h>

NTL_CLIENT


long compare(const ZZX& a, const ZZX& b)
{
   if (deg(a) < deg(b))
      return 0;

   if (deg(a) > deg(b))
      return 1;

   long n = a.rep.length();
   long i;

   for (i = 0; i < n; i++) {
      if (a.rep[i] < b.rep[i]) return 0;
      if (a.rep[i] > b.rep[i]) return 1;
   }

   return 0;
}
      

void sort(vec_pair_ZZX_long& v)
{
   long n = v.length();
   long i, j;

   for (i = 0; i < n-1; i++)
      for (j = 0; j < n-1-i; j++)
         if (compare(v[j].a, v[j+1].a)) {
            swap(v[j].a, v[j+1].a);
            swap(v[j].b, v[j+1].b);
         }
}
            
 

int main(int argc, char **argv)
{
   ZZX f1, f;

   if (argc > 1) 
      ZZXFac_MaxPrune = atoi(argv[1]);

   cin >> f;

   vec_pair_ZZX_long factors;
   ZZ c;

   double t;

   t = GetTime();
   factor(c, factors, f, 0);
   t = GetTime()-t;

   cerr << "total time: " << t << "\n";


   mul(f1, factors);
   mul(f1, f1, c);

   if (f != f1)
      TerminalError("FACTORIZATION INCORRECT!!!");



   sort(factors);

   cout << c << "\n";
   cout << factors << "\n";

   return 0;
}



#include <NTL/ZZ_pXFactoring.h>

NTL_CLIENT


long compare(const ZZ_pX& a, const ZZ_pX& b)
{
   if (deg(a) < deg(b))
      return 0;

   if (deg(a) > deg(b))
      return 1;

   long n = a.rep.length();
   long i;

   for (i = 0; i < n; i++) {
      if (rep(a.rep[i]) < rep(b.rep[i])) return 0;
      if (rep(a.rep[i]) > rep(b.rep[i])) return 1;
   }

   return 0;
}

void sort(vec_pair_ZZ_pX_long& v)
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


int main()
{
   ZZ p;
   cin >> p;
   ZZ_p::init(p);
   ZZ_pX f;
   cin >> f;

   vec_pair_ZZ_pX_long factors;

   double t = GetTime();
   berlekamp(factors, f, 1);
   t = GetTime()-t;
   cerr << "total time: " << t << "\n";

   ZZ_pX ff;

   mul(ff, factors);
   if (f != ff)
      TerminalError("Incorrect factorization!!");

   sort(factors);

   cerr << "factorization pattern:";
   long i;

   for (i = 0; i < factors.length(); i++) {
      cerr << " ";
      long k = factors[i].b;
      if (k > 1)
         cerr << k << "*";
      cerr << deg(factors[i].a);
   }

   cerr << "\n";



   cout << factors << "\n";

   return 0;
}

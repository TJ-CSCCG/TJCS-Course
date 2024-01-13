#include <NTL/ZZXFactoring.h>

NTL_CLIENT

long NumFacs(const vec_pair_ZZX_long& v)
{
   long i;
   long res;

   res = 0;
   
   for (i = 0; i < v.length(); i++)
      res += v[i].b;

   return res;
}


int main()
{
   long cnt = 0;
   while (SkipWhiteSpace(cin)) {
      cnt++;
      cerr << ".";

      vec_ZZ w;
      ZZX f1, f;
      long nfacs;

      cin >> w;
      cin >> nfacs;

      long i, n;
      n = w.length();
      f.rep.SetLength(n);
      for (i = 0; i < n; i++)
         f.rep[i] = w[n-1-i];
      f.normalize();

      vec_pair_ZZX_long factors;
      ZZ c;

      factor(c, factors, f, 0);


      mul(f1, factors);
      mul(f1, f1, c);

      if (f != f1) {
         cerr << f << "\n";
         cerr << c << " " << factors << "\n";
         TerminalError("FACTORIZATION INCORRECT (1) !!!");
      }

      long nfacs1 = NumFacs(factors);

      if (nfacs1 != nfacs)
         TerminalError("FACTORIZATION INCORRECT (2) !!!");
   }


   cerr << "\n";
   cerr << "MoreFacTest OK\n";

   return 0;
}

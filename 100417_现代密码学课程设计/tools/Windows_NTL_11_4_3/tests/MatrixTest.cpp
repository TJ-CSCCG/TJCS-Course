
#include <NTL/mat_ZZ.h>
#include <NTL/mat_poly_ZZ.h>
#include <NTL/HNF.h>

NTL_CLIENT

int main()
{
   mat_ZZ B, X;
   vec_ZZ v, w;

   cin >> B;
   cin >> v;

   ZZ d;

   double t;
   cerr << "matrix inverse...";
   t = GetTime();
   inv(d, X, B);
   cerr << (GetTime()-t) << "\n";

   cout << d << "\n";
   cout << X << "\n";

   cout << "\n\n\n";

   cerr << "hensel solve...";
   t = GetTime();
   HenselSolve1(d, w, B, v);
   cerr << (GetTime()-t) << "\n";

   cout << d << "\n";
   cout << w << "\n";

   cout << "\n\n\n";

   ZZX f;

   cerr << "char poly...";
   t = GetTime();
   CharPoly(f, B);
   cerr << (GetTime()-t) << "\n";

   cout << f << "\n";

   cout << "\n\n\n";

   cerr << "HNF...";
   t = GetTime();
   HNF(X, B, d);
   cerr << (GetTime()-t) << "\n";

   cout << X;

   return 0;
}

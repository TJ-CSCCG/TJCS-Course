
#include <NTL/ZZX.h>

NTL_CLIENT

int main()
{
   ZZX a, f, g;

   cin >> a;
   cin >> f;

   double t = GetTime();;
   CharPolyMod(g, a, f);
   cerr << GetTime()-t << "\n";

   cout << g << "\n";
   return 0;
}

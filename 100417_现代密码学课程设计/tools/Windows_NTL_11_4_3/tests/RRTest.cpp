
#include <NTL/mat_RR.h>

NTL_CLIENT

int main()
{
   mat_RR A;
   vec_RR x, y, z;
   RR d;

   RR::SetPrecision(200);

   cin >> A;
   cin >> y;

   solve(d, x, A, y);

   // mul(z, x, A);
   // sub(z, z, y);

   z = x*A - y;

   cout << d << "\n";
   cout << x << "\n";
   cout << z << "\n";
}


#include <NTL/quad_float.h>

NTL_CLIENT

int main()
{
   quad_float a, b, c, d;

   quad_float::SetOutputPrecision(25);

   long pok;
   double one = 1.0;
   quad_float_PrecisionOK(pok, one);
   if (pok)
      cout << "Precision OK\n";
   else
      cout << "Precision not OK\n";


   cin >> a;
   cout << a << "\n";

   cin >> b;
   cout << b << "\n";

   c = a + b;
   d = a;
   d += b;
   cout << c << "\n";
   cout << d << "\n";

   c = a - b;
   d = a;
   d -= b;
   cout << c << "\n";
   cout << d << "\n";

   c = a * b;
   d = a;
   d *= b;
   cout << c << "\n";
   cout << d << "\n";

   c = a / b;
   d = a;
   d /= b;
   cout << c << "\n";
   cout << d << "\n";

   c = -a;
   cout << c << "\n";

   c = sqrt(a);
   cout << c << "\n";

   power(c, to_quad_float(10), 20);
   cout << c << "\n";

   {

   long n, n1;
   int shamt = min(NTL_BITS_PER_LONG,2*NTL_DOUBLE_PRECISION);

   n = to_long((1UL << (shamt-1)) - 1UL);
   c = to_quad_float(n);
   n1 = to_long(c);

   if (n1 == n)
      cout << "long conversion OK\n";
   else
      cout << "long conversion not OK\n";

   n = to_long(1UL << (shamt-1));
   c = to_quad_float(n);
   n1 = to_long(c);

   if (n1 == n)
      cout << "long conversion OK\n";
   else
      cout << "long conversion not OK\n";

   }

   {

   unsigned long n;
   ZZ n1;
   int shamt = min(NTL_BITS_PER_LONG,2*NTL_DOUBLE_PRECISION);

   n = (1UL << (shamt-1)) - 1UL;
   c = to_quad_float(n);
   n1 = to_ZZ(c);

   if (n1 == to_ZZ(n))
      cout << "ulong conversion OK\n";
   else
      cout << "ulong conversion not OK\n";

   n = 1UL << (shamt-1);
   c = to_quad_float(n);
   n1 = to_ZZ(c);

   if (n1 == to_ZZ(n))
      cout << "ulong conversion OK\n";
   else
      cout << "ulong conversion not OK\n";

   }

}

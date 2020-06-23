
#include <NTL/GF2.h>


NTL_START_IMPL


GF2 power(GF2 a, long e)
{
   if (e == 0) {
      return to_GF2(1); 
   }

   if (e < 0 && IsZero(a)) 
      ArithmeticError("GF2: division by zero");

   return a;
}

ostream& operator<<(ostream& s, GF2 a)
{
   if (a == 0)
      s << "0";
   else
      s << "1";

   return s;
}

istream& operator>>(istream& s, ref_GF2 x)
{
   NTL_ZZRegister(a);

   NTL_INPUT_CHECK_RET(s, s >> a);

   conv(x, a);
   return s;
}
 
NTL_END_IMPL

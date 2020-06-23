
/************************************************************************

This program can be compiled under either C or C++.
It copies its input to its output, substituting all old
NTL macro names by new NTL macro names.
This is intended to automate the transition from NTL 3.1 to 3.5.

Each maximal length alphanumeric substring in the input
is looked up in a table, and if there is a match, the substring
is replaced.

*************************************************************************/


#include <stdio.h>
#include <string.h>

#define NumNames (79)

const char *names[NumNames][2] = {
{ "BB_HALF_MUL_CODE", "NTL_BB_HALF_MUL_CODE" },
{ "BB_MUL_CODE", "NTL_BB_MUL_CODE" },
{ "BB_REV_CODE", "NTL_BB_REV_CODE" },
{ "BB_SQR_CODE", "NTL_BB_SQR_CODE" },
{ "FFTFudge", "NTL_FFTFudge" },
{ "FFTMaxRoot", "NTL_FFTMaxRoot" },
{ "FFTMaxRootBnd", "NTL_FFTMaxRootBnd" },
{ "QUAD_FLOAT_SPLIT", "NTL_QUAD_FLOAT_SPLIT" },
{ "WV_NTL_RANGE_CHECK_CODE", "NTL_WV_RANGE_CHECK_CODE" },
{ "WordVectorExpansionRatio", "NTL_WordVectorExpansionRatio" },
{ "WordVectorInputBlock", "NTL_WordVectorInputBlock" },
{ "WordVectorMinAlloc", "NTL_WordVectorMinAlloc" },
{ "XD_BOUND", "NTL_XD_BOUND" },
{ "XD_BOUND_INV", "NTL_XD_BOUND_INV" },
{ "XD_HBOUND", "NTL_XD_HBOUND" },
{ "XD_HBOUND_INV", "NTL_XD_HBOUND_INV" },
{ "ZZ_ARITH_RIGHT_SHIFT", "NTL_ARITH_RIGHT_SHIFT" },
{ "ZZ_BITS_PER_INT", "NTL_BITS_PER_INT" },
{ "ZZ_BITS_PER_LONG", "NTL_BITS_PER_LONG" },
{ "ZZ_DOUBLES_LOW_HIGH", "NTL_DOUBLES_LOW_HIGH" },
{ "ZZ_DOUBLE_PRECISION", "NTL_DOUBLE_PRECISION" },
{ "ZZ_EXT_DOUBLE", "NTL_EXT_DOUBLE" },
{ "ZZ_FDOUBLE_PRECISION", "NTL_FDOUBLE_PRECISION" },
{ "ZZ_FRADIX", "NTL_FRADIX" },
{ "ZZ_FRADIX_INV", "NTL_FRADIX_INV" },
{ "ZZ_FetchHiLo", "NTL_FetchHiLo" },
{ "ZZ_FetchLo", "NTL_FetchLo" },
{ "ZZ_HI_WD", "NTL_HI_WD" },
{ "ZZ_LO_WD", "NTL_LO_WD" },
{ "ZZ_MAX_INT", "NTL_MAX_INT" },
{ "ZZ_MAX_LONG", "NTL_MAX_LONG" },
{ "ZZ_MIN_INT", "NTL_MIN_INT" },
{ "ZZ_MIN_LONG", "NTL_MIN_LONG" },
{ "ZZ_NBITS", "NTL_NBITS" },
{ "ZZ_NBITSH", "NTL_NBITSH" },
{ "ZZ_NBITS_MAX", "NTL_NBITS_MAX" },
{ "ZZ_NTL_SINGLE_MUL_OK", "NTL_SINGLE_MUL_OK" },
{ "ZZ_PRIME_BND", "NTL_PRIME_BND" },
{ "ZZ_RADIX", "NTL_RADIX" },
{ "ZZ_RADIXM", "NTL_RADIXM" },
{ "ZZ_RADIXROOT", "NTL_RADIXROOT" },
{ "ZZ_RADIXROOTM", "NTL_RADIXROOTM" },
{ "ZZ_pRegister", "NTL_ZZ_pRegister" },
{ "ZZ_pX_BERMASS_CROSSOVER", "NTL_ZZ_pX_BERMASS_CROSSOVER" },
{ "ZZ_pX_DIV_CROSSOVER", "NTL_ZZ_pX_DIV_CROSSOVER" },
{ "ZZ_pX_FFT_CROSSOVER", "NTL_ZZ_pX_FFT_CROSSOVER" },
{ "ZZ_pX_GCD_CROSSOVER", "NTL_ZZ_pX_GCD_CROSSOVER" },
{ "ZZ_pX_HalfGCD_CROSSOVER", "NTL_ZZ_pX_HalfGCD_CROSSOVER" },
{ "ZZ_pX_NEWTON_CROSSOVER", "NTL_ZZ_pX_NEWTON_CROSSOVER" },
{ "ZZ_pX_TRACE_CROSSOVER", "NTL_ZZ_pX_TRACE_CROSSOVER" },
{ "ntl_eq_matrix_decl", "NTL_eq_matrix_decl" },
{ "ntl_eq_matrix_impl", "NTL_eq_matrix_impl" },
{ "ntl_eq_vector_decl", "NTL_eq_vector_decl" },
{ "ntl_eq_vector_impl", "NTL_eq_vector_impl" },
{ "ntl_io_matrix_decl", "NTL_io_matrix_decl" },
{ "ntl_io_matrix_impl", "NTL_io_matrix_impl" },
{ "ntl_io_vector_decl", "NTL_io_vector_decl" },
{ "ntl_io_vector_impl", "NTL_io_vector_impl" },
{ "ntl_matrix_decl", "NTL_matrix_decl" },
{ "ntl_matrix_impl", "NTL_matrix_impl" },
{ "ntl_pair_decl", "NTL_pair_decl" },
{ "ntl_pair_eq_decl", "NTL_pair_eq_decl" },
{ "ntl_pair_eq_impl", "NTL_pair_eq_impl" },
{ "ntl_pair_impl", "NTL_pair_impl" },
{ "ntl_pair_io_decl", "NTL_pair_io_decl" },
{ "ntl_pair_io_impl", "NTL_pair_io_impl" },
{ "ntl_vector_decl", "NTL_vector_decl" },
{ "ntl_vector_default", "NTL_vector_default" },
{ "ntl_vector_impl", "NTL_vector_impl" },
{ "ntl_vector_impl_plain", "NTL_vector_impl_plain" },
{ "zz_pRegister", "NTL_zz_pRegister" },
{ "zz_pX_BERMASS_CROSSOVER", "NTL_zz_pX_BERMASS_CROSSOVER" },
{ "zz_pX_DIV_CROSSOVER", "NTL_zz_pX_DIV_CROSSOVER" },
{ "zz_pX_GCD_CROSSOVER", "NTL_zz_pX_GCD_CROSSOVER" },
{ "zz_pX_HalfGCD_CROSSOVER", "NTL_zz_pX_HalfGCD_CROSSOVER" },
{ "zz_pX_MOD_CROSSOVER", "NTL_zz_pX_MOD_CROSSOVER" },
{ "zz_pX_MUL_CROSSOVER", "NTL_zz_pX_MUL_CROSSOVER" },
{ "zz_pX_NEWTON_CROSSOVER", "NTL_zz_pX_NEWTON_CROSSOVER" },
{ "zz_pX_TRACE_CROSSOVER", "NTL_zz_pX_TRACE_CROSSOVER" },
};


void PrintName(const char *name)
{
   int i;

   i = 0;
   while (i < NumNames && strcmp(name, names[i][0]))
      i++;

   if (i >= NumNames)
      printf("%s", name);
   else
      printf("%s", names[i][1]);
}


int IsAlphaNum(int c)
{
   return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c == '_') || (c >= '0' && c <= '9'));
}

char buf[10000];


int main()
{
   int c;
   int state;
   int len;

   state = 0;
   len = 0;


   do {
      c = getchar();

      switch (state) {
      case 0:
         if (IsAlphaNum(c)) {
            buf[len] = c;
            len++;
            state = 1;
         }
         else {
            if (c != EOF) putchar(c);
         }

         break;

      case 1:
         if (IsAlphaNum(c)) {
            buf[len] = c;
            len++;
         }
         else {
            buf[len] = '\0';
            PrintName(buf);
            len = 0;

            if (c != EOF) putchar(c);
            state = 0;
         }

         break;
      }
   } while (c != EOF);
   
   return 0;
}

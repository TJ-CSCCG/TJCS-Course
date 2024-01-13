
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>



#include <NTL/config.h>

using namespace std;


#ifndef NTL_GMP_LIP


int main()
{
   fprintf(stderr, "NTL_GMP_LIP flag not set\n");

   return 0;
}



#else


#include <gmp.h>
#include <NTL/mach_desc.h>

void print2k(FILE *f, long k, long bpl)
{
   long m, l;
   long first;

   if (k <= 0) {
      fprintf(f, "((double) 1.0)");
      return;
   }

   m = bpl - 2;
   first = 1;

   fprintf(f, "(");

   while (k > 0) {
      if (k > m)
         l = m;
      else
         l = k;

      k = k - l;


      if (first)
         first = 0;
      else
         fprintf(f, "*");

      fprintf(f, "((double)(1L<<%ld))", l);
   }

   fprintf(f, ")");
}



void Error(const char *s)
{
   fprintf(stderr, "*** %s\n", s);
   abort();
}


char buffer[1024];


int main()
{
   long bpl;
   long ntl_zz_nbits;
   long nail_bits;

   fprintf(stderr, "NTL_GMP_LIP flag set\n");

   bpl = NTL_BITS_PER_LONG;


   /*
    * We require that the number of bits per limb quantity correspond to the
    * number of bits of a long, or possibly a "long long" that is twice as wide
    * as a long.  These restrictions will almost certainly be satisfied.
    */

   ntl_zz_nbits = 0;

   nail_bits = 0;
#ifdef GMP_NAIL_BITS
   nail_bits = GMP_NAIL_BITS;
#endif

   if (nail_bits > 0)
      fprintf(stderr, "WARNING: GMP_NAIL_BITS > 0: this has not been well tested\n");

   if (__GNU_MP_VERSION < 5) 
      Error("GMP version 5.0.0 or later required");

   // check that GMP_LIMB_BITS == mp_bits_per_limb as a consistency check
   if (GMP_LIMB_BITS != mp_bits_per_limb) 
      Error("GMP_LIMB_BITS != mp_bits_per_limb: inconsistency between gmp.h and libgmp");

   // check that vesrion numbers match as a consistency check
   // This is adapted from MPFR's configure script
   bool bad_version = false;
   sprintf(buffer, "%d.%d.%d", __GNU_MP_VERSION, __GNU_MP_VERSION_MINOR,
           __GNU_MP_VERSION_PATCHLEVEL);
   fprintf(stderr, "GMP version check (%s/%s)\n", buffer, gmp_version);
   if (strcmp(buffer, gmp_version)) {
      if (__GNU_MP_VERSION_PATCHLEVEL != 0)
         bad_version = true;
      else {
         sprintf(buffer, "%d.%d", __GNU_MP_VERSION, __GNU_MP_VERSION_MINOR);
         if (strcmp(buffer, gmp_version)) bad_version = true;
      }
   }

   if (bad_version)
      Error("version number mismatch: inconsistency between gmp.h and libgmp");

   if (sizeof(mp_limb_t) == sizeof(long) && mp_bits_per_limb == bpl)
      ntl_zz_nbits = bpl-nail_bits;
   else if (sizeof(mp_limb_t) == 2*sizeof(long) && mp_bits_per_limb == 2*bpl)
      ntl_zz_nbits = 2*bpl-nail_bits;
   else
      Error("sorry...this is a funny gmp");

   if (ntl_zz_nbits % 2 != 0 || ntl_zz_nbits < 30)
      Error("sorry...this is a funny gmp");

   if (sizeof(mp_size_t) != sizeof(long) && sizeof(mp_size_t) != sizeof(int))
      Error("sorry...this is a funny gmp");


   if (sizeof(mp_size_t) < sizeof(long)) {
      printf("#define NTL_SMALL_MP_SIZE_T\n");
      fprintf(stderr, "setting NTL_SMALL_MP_SIZE_T\n");
   }


   fprintf(stderr, "NTL_ZZ_NBITS = %ld\n", ntl_zz_nbits);
   printf("#define NTL_ZZ_NBITS (%ld)\n",  ntl_zz_nbits);

   fprintf(stderr, "NTL_BITS_PER_LIMB_T = %ld\n", ntl_zz_nbits+nail_bits);
   printf("#define NTL_BITS_PER_LIMB_T (%ld)\n", ntl_zz_nbits+nail_bits);

   printf("#define NTL_ZZ_FRADIX ");
   print2k(stdout, ntl_zz_nbits, bpl);
   printf("\n");

   return 0;
}

#endif

#ifndef NTL_mach_desc__H
#define NTL_mach_desc__H


#define NTL_BITS_PER_LONG (32)
#define NTL_NUMBITS_BPL (6)
#define NTL_MAX_LONG (2147483647L)
#define NTL_MAX_INT (2147483647)
#define NTL_BITS_PER_INT (32)
#define NTL_BITS_PER_SIZE_T (32)
#define NTL_ARITH_RIGHT_SHIFT (1)
#define NTL_NBITS_MAX (30)
#define NTL_WNBITS_MAX (30)
#define NTL_DOUBLE_PRECISION (53)
#define NTL_FDOUBLE_PRECISION (((double)(1L<<30))*((double)(1L<<22)))
#define NTL_LONGDOUBLE_OK (0)
#define NTL_WIDE_DOUBLE_DP ((wide_double(1L<<52)))
#define NTL_QUAD_FLOAT_SPLIT ((((double)(1L<<27)))+1.0)
#define NTL_EXT_DOUBLE (0)
#define NTL_FMA_DETECTED (1)
#define NTL_BIG_POINTERS (1)
#define NTL_MIN_LONG (-NTL_MAX_LONG - 1L)
#define NTL_MIN_INT  (-NTL_MAX_INT - 1)



#define NTL_BB_MUL_CODE0 \
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[8];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   lo = A[b & 7]; t = A[(b >> 3) & 7]; hi = t >> 29; lo ^= t << 3;\
   t = A[(b >> 6) & 7]; hi ^= t >> 26; lo ^= t << 6;\
   t = A[(b >> 9) & 7]; hi ^= t >> 23; lo ^= t << 9;\
   t = A[(b >> 12) & 7]; hi ^= t >> 20; lo ^= t << 12;\
   t = A[(b >> 15) & 7]; hi ^= t >> 17; lo ^= t << 15;\
   t = A[(b >> 18) & 7]; hi ^= t >> 14; lo ^= t << 18;\
   t = A[(b >> 21) & 7]; hi ^= t >> 11; lo ^= t << 21;\
   t = A[(b >> 24) & 7]; hi ^= t >> 8; lo ^= t << 24;\
   t = A[(b >> 27) & 7]; hi ^= t >> 5; lo ^= t << 27;\
   t = A[b >> 30]; hi ^= t >> 2; lo ^= t << 30;\
   if (a >> 31) hi ^= ((b & 0xb6db6db6UL) >> 1);\
   if ((a >> 30) & 1) hi ^= ((b & 0x24924924UL) >> 2);\
   c[0] = lo;    c[1] = hi;\





#define NTL_BB_MUL_CODE1 \
   long i;\
   _ntl_ulong carry = 0, b;\
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      b = bp[i];\
      lo = A[b & 15]; t = A[(b >> 4) & 15]; hi = t >> 28; lo ^= t << 4;\
      t = A[(b >> 8) & 15]; hi ^= t >> 24; lo ^= t << 8;\
      t = A[(b >> 12) & 15]; hi ^= t >> 20; lo ^= t << 12;\
      t = A[(b >> 16) & 15]; hi ^= t >> 16; lo ^= t << 16;\
      t = A[(b >> 20) & 15]; hi ^= t >> 12; lo ^= t << 20;\
      t = A[(b >> 24) & 15]; hi ^= t >> 8; lo ^= t << 24;\
      t = A[b >> 28]; hi ^= t >> 4; lo ^= t << 28;\
      if (a >> 31) hi ^= ((b & 0xeeeeeeeeUL) >> 1);\
      if ((a >> 30) & 1) hi ^= ((b & 0xccccccccUL) >> 2);\
      if ((a >> 29) & 1) hi ^= ((b & 0x88888888UL) >> 3);\
      cp[i] = carry ^ lo;    carry = hi;\
   }\
   cp[sb] = carry;\





#define NTL_BB_MUL_CODE2 \
   long i;\
   _ntl_ulong carry = 0, b;\
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      b = bp[i];\
      lo = A[b & 15]; t = A[(b >> 4) & 15]; hi = t >> 28; lo ^= t << 4;\
      t = A[(b >> 8) & 15]; hi ^= t >> 24; lo ^= t << 8;\
      t = A[(b >> 12) & 15]; hi ^= t >> 20; lo ^= t << 12;\
      t = A[(b >> 16) & 15]; hi ^= t >> 16; lo ^= t << 16;\
      t = A[(b >> 20) & 15]; hi ^= t >> 12; lo ^= t << 20;\
      t = A[(b >> 24) & 15]; hi ^= t >> 8; lo ^= t << 24;\
      t = A[b >> 28]; hi ^= t >> 4; lo ^= t << 28;\
      if (a >> 31) hi ^= ((b & 0xeeeeeeeeUL) >> 1);\
      if ((a >> 30) & 1) hi ^= ((b & 0xccccccccUL) >> 2);\
      if ((a >> 29) & 1) hi ^= ((b & 0x88888888UL) >> 3);\
      cp[i] ^= (carry ^ lo);    carry = hi;\
   }\
   cp[sb] ^= carry;\





#define NTL_SHORT_BB_MUL_CODE1 \
   long i;\
   _ntl_ulong carry = 0, b;\
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      b = bp[i];\
      lo = A[b & 15]; t = A[(b >> 4) & 15]; hi = t >> 28; lo ^= t << 4;\
      t = A[(b >> 8) & 15]; hi ^= t >> 24; lo ^= t << 8;\
      t = A[(b >> 12) & 15]; hi ^= t >> 20; lo ^= t << 12;\
      t = A[(b >> 16) & 15]; hi ^= t >> 16; lo ^= t << 16;\
      t = A[(b >> 20) & 15]; hi ^= t >> 12; lo ^= t << 20;\
      t = A[(b >> 24) & 15]; hi ^= t >> 8; lo ^= t << 24;\
      t = A[b >> 28]; hi ^= t >> 4; lo ^= t << 28;\
      cp[i] = carry ^ lo;    carry = hi;\
   }\
   cp[sb] = carry;\





#define NTL_HALF_BB_MUL_CODE0 \
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[4];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   lo = A[b & 3]; t = A[(b >> 2) & 3]; hi = t >> 30; lo ^= t << 2;\
   t = A[(b >> 4) & 3]; hi ^= t >> 28; lo ^= t << 4;\
   t = A[(b >> 6) & 3]; hi ^= t >> 26; lo ^= t << 6;\
   t = A[(b >> 8) & 3]; hi ^= t >> 24; lo ^= t << 8;\
   t = A[(b >> 10) & 3]; hi ^= t >> 22; lo ^= t << 10;\
   t = A[(b >> 12) & 3]; hi ^= t >> 20; lo ^= t << 12;\
   t = A[b >> 14]; hi ^= t >> 18; lo ^= t << 14;\
   if (a >> 31) hi ^= ((b & 0xaaaaUL) >> 1);\
   c[0] = lo;    c[1] = hi;\





#define NTL_ALT_BB_MUL_CODE0 \
   _ntl_ulong A[8];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   const _ntl_ulong t3 = A[(b >> 3) & 7]; \
   const _ntl_ulong t6 = A[(b >> 6) & 7]; \
   const _ntl_ulong t9 = A[(b >> 9) & 7]; \
   const _ntl_ulong t12 = A[(b >> 12) & 7]; \
   const _ntl_ulong t15 = A[(b >> 15) & 7]; \
   const _ntl_ulong t18 = A[(b >> 18) & 7]; \
   const _ntl_ulong t21 = A[(b >> 21) & 7]; \
   const _ntl_ulong t24 = A[(b >> 24) & 7]; \
   const _ntl_ulong t27 = A[(b >> 27) & 7]; \
   const _ntl_ulong t30 = A[b >> 30]; \
   const _ntl_ulong lo = A[b & 7] \
      ^ (t3 << 3)\
      ^ (t6 << 6)\
      ^ (t9 << 9)\
      ^ (t12 << 12)\
      ^ (t15 << 15)\
      ^ (t18 << 18)\
      ^ (t21 << 21)\
      ^ (t24 << 24)\
      ^ (t27 << 27)\
      ^ (t30 << 30);\
   const _ntl_ulong hi = (t3 >> 29)\
      ^ (t6 >> 26)\
      ^ (t9 >> 23)\
      ^ (t12 >> 20)\
      ^ (t15 >> 17)\
      ^ (t18 >> 14)\
      ^ (t21 >> 11)\
      ^ (t24 >> 8)\
      ^ (t27 >> 5)\
      ^ (t30 >> 2)\
      ^ (((b & 0xb6db6db6UL) >> 1) & (-(a >> 31)))\
      ^ (((b & 0x24924924UL) >> 2) & (-((a >> 30) & 1UL)));\
   c[0] = lo;    c[1] = hi;\





#define NTL_ALT_BB_MUL_CODE1 \
   long i;\
   _ntl_ulong carry = 0;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      const _ntl_ulong b = bp[i];\
      const _ntl_ulong t4 = A[(b >> 4) & 15]; \
      const _ntl_ulong t8 = A[(b >> 8) & 15]; \
      const _ntl_ulong t12 = A[(b >> 12) & 15]; \
      const _ntl_ulong t16 = A[(b >> 16) & 15]; \
      const _ntl_ulong t20 = A[(b >> 20) & 15]; \
      const _ntl_ulong t24 = A[(b >> 24) & 15]; \
      const _ntl_ulong t28 = A[b >> 28]; \
      const _ntl_ulong lo = A[b & 15] \
         ^ (t4 << 4)\
         ^ (t8 << 8)\
         ^ (t12 << 12)\
         ^ (t16 << 16)\
         ^ (t20 << 20)\
         ^ (t24 << 24)\
         ^ (t28 << 28);\
      const _ntl_ulong hi = (t4 >> 28)\
         ^ (t8 >> 24)\
         ^ (t12 >> 20)\
         ^ (t16 >> 16)\
         ^ (t20 >> 12)\
         ^ (t24 >> 8)\
         ^ (t28 >> 4)\
         ^ (((b & 0xeeeeeeeeUL) >> 1) & (-(a >> 31)))\
         ^ (((b & 0xccccccccUL) >> 2) & (-((a >> 30) & 1UL)))\
         ^ (((b & 0x88888888UL) >> 3) & (-((a >> 29) & 1UL)));\
      cp[i] = carry ^ lo;    carry = hi;\
   }\
   cp[sb] = carry;\





#define NTL_ALT_BB_MUL_CODE2 \
   long i;\
   _ntl_ulong carry = 0;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      const _ntl_ulong b = bp[i];\
      const _ntl_ulong t4 = A[(b >> 4) & 15]; \
      const _ntl_ulong t8 = A[(b >> 8) & 15]; \
      const _ntl_ulong t12 = A[(b >> 12) & 15]; \
      const _ntl_ulong t16 = A[(b >> 16) & 15]; \
      const _ntl_ulong t20 = A[(b >> 20) & 15]; \
      const _ntl_ulong t24 = A[(b >> 24) & 15]; \
      const _ntl_ulong t28 = A[b >> 28]; \
      const _ntl_ulong lo = A[b & 15] \
         ^ (t4 << 4)\
         ^ (t8 << 8)\
         ^ (t12 << 12)\
         ^ (t16 << 16)\
         ^ (t20 << 20)\
         ^ (t24 << 24)\
         ^ (t28 << 28);\
      const _ntl_ulong hi = (t4 >> 28)\
         ^ (t8 >> 24)\
         ^ (t12 >> 20)\
         ^ (t16 >> 16)\
         ^ (t20 >> 12)\
         ^ (t24 >> 8)\
         ^ (t28 >> 4)\
         ^ (((b & 0xeeeeeeeeUL) >> 1) & (-(a >> 31)))\
         ^ (((b & 0xccccccccUL) >> 2) & (-((a >> 30) & 1UL)))\
         ^ (((b & 0x88888888UL) >> 3) & (-((a >> 29) & 1UL)));\
      cp[i] ^= (carry ^ lo);    carry = hi;\
   }\
   cp[sb] ^= carry;\





#define NTL_ALT_SHORT_BB_MUL_CODE1 \
   long i;\
   _ntl_ulong carry = 0;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      const _ntl_ulong b = bp[i];\
      const _ntl_ulong t4 = A[(b >> 4) & 15]; \
      const _ntl_ulong t8 = A[(b >> 8) & 15]; \
      const _ntl_ulong t12 = A[(b >> 12) & 15]; \
      const _ntl_ulong t16 = A[(b >> 16) & 15]; \
      const _ntl_ulong t20 = A[(b >> 20) & 15]; \
      const _ntl_ulong t24 = A[(b >> 24) & 15]; \
      const _ntl_ulong t28 = A[b >> 28]; \
      const _ntl_ulong lo = A[b & 15] \
         ^ (t4 << 4)\
         ^ (t8 << 8)\
         ^ (t12 << 12)\
         ^ (t16 << 16)\
         ^ (t20 << 20)\
         ^ (t24 << 24)\
         ^ (t28 << 28);\
      const _ntl_ulong hi = (t4 >> 28)\
         ^ (t8 >> 24)\
         ^ (t12 >> 20)\
         ^ (t16 >> 16)\
         ^ (t20 >> 12)\
         ^ (t24 >> 8)\
         ^ (t28 >> 4);\
      cp[i] = carry ^ lo;    carry = hi;\
   }\
   cp[sb] = carry;\





#define NTL_ALT_HALF_BB_MUL_CODE0 \
   _ntl_ulong A[4];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   const _ntl_ulong t2 = A[(b >> 2) & 3]; \
   const _ntl_ulong t4 = A[(b >> 4) & 3]; \
   const _ntl_ulong t6 = A[(b >> 6) & 3]; \
   const _ntl_ulong t8 = A[(b >> 8) & 3]; \
   const _ntl_ulong t10 = A[(b >> 10) & 3]; \
   const _ntl_ulong t12 = A[(b >> 12) & 3]; \
   const _ntl_ulong t14 = A[b >> 14]; \
   const _ntl_ulong lo = A[b & 3] \
      ^ (t2 << 2)\
      ^ (t4 << 4)\
      ^ (t6 << 6)\
      ^ (t8 << 8)\
      ^ (t10 << 10)\
      ^ (t12 << 12)\
      ^ (t14 << 14);\
   const _ntl_ulong hi = (t2 >> 30)\
      ^ (t4 >> 28)\
      ^ (t6 >> 26)\
      ^ (t8 >> 24)\
      ^ (t10 >> 22)\
      ^ (t12 >> 20)\
      ^ (t14 >> 18)\
      ^ (((b & 0xaaaaUL) >> 1) & (-(a >> 31)));\
   c[0] = lo;    c[1] = hi;\





#define NTL_ALT1_BB_MUL_CODE0 \
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[8];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   lo = A[b & 7]; t = A[(b >> 3) & 7]; hi = t >> 29; lo ^= t << 3;\
   t = A[(b >> 6) & 7]; hi ^= t >> 26; lo ^= t << 6;\
   t = A[(b >> 9) & 7]; hi ^= t >> 23; lo ^= t << 9;\
   t = A[(b >> 12) & 7]; hi ^= t >> 20; lo ^= t << 12;\
   t = A[(b >> 15) & 7]; hi ^= t >> 17; lo ^= t << 15;\
   t = A[(b >> 18) & 7]; hi ^= t >> 14; lo ^= t << 18;\
   t = A[(b >> 21) & 7]; hi ^= t >> 11; lo ^= t << 21;\
   t = A[(b >> 24) & 7]; hi ^= t >> 8; lo ^= t << 24;\
   t = A[(b >> 27) & 7]; hi ^= t >> 5; lo ^= t << 27;\
   t = A[b >> 30]; hi ^= t >> 2; lo ^= t << 30;\
   hi ^= (((b & 0xb6db6db6UL) >> 1) & (-(a >> 31)))\
      ^ (((b & 0x24924924UL) >> 2) & (-((a >> 30) & 1UL)));\
   c[0] = lo;    c[1] = hi;\





#define NTL_ALT1_BB_MUL_CODE1 \
   long i;\
   _ntl_ulong carry = 0, b;\
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      b = bp[i];\
      lo = A[b & 15]; t = A[(b >> 4) & 15]; hi = t >> 28; lo ^= t << 4;\
      t = A[(b >> 8) & 15]; hi ^= t >> 24; lo ^= t << 8;\
      t = A[(b >> 12) & 15]; hi ^= t >> 20; lo ^= t << 12;\
      t = A[(b >> 16) & 15]; hi ^= t >> 16; lo ^= t << 16;\
      t = A[(b >> 20) & 15]; hi ^= t >> 12; lo ^= t << 20;\
      t = A[(b >> 24) & 15]; hi ^= t >> 8; lo ^= t << 24;\
      t = A[b >> 28]; hi ^= t >> 4; lo ^= t << 28;\
      hi ^= (((b & 0xeeeeeeeeUL) >> 1) & (-(a >> 31)))\
         ^ (((b & 0xccccccccUL) >> 2) & (-((a >> 30) & 1UL)))\
         ^ (((b & 0x88888888UL) >> 3) & (-((a >> 29) & 1UL)));\
      cp[i] = carry ^ lo;    carry = hi;\
   }\
   cp[sb] = carry;\





#define NTL_ALT1_BB_MUL_CODE2 \
   long i;\
   _ntl_ulong carry = 0, b;\
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      b = bp[i];\
      lo = A[b & 15]; t = A[(b >> 4) & 15]; hi = t >> 28; lo ^= t << 4;\
      t = A[(b >> 8) & 15]; hi ^= t >> 24; lo ^= t << 8;\
      t = A[(b >> 12) & 15]; hi ^= t >> 20; lo ^= t << 12;\
      t = A[(b >> 16) & 15]; hi ^= t >> 16; lo ^= t << 16;\
      t = A[(b >> 20) & 15]; hi ^= t >> 12; lo ^= t << 20;\
      t = A[(b >> 24) & 15]; hi ^= t >> 8; lo ^= t << 24;\
      t = A[b >> 28]; hi ^= t >> 4; lo ^= t << 28;\
      hi ^= (((b & 0xeeeeeeeeUL) >> 1) & (-(a >> 31)))\
         ^ (((b & 0xccccccccUL) >> 2) & (-((a >> 30) & 1UL)))\
         ^ (((b & 0x88888888UL) >> 3) & (-((a >> 29) & 1UL)));\
      cp[i] ^= (carry ^ lo);    carry = hi;\
   }\
   cp[sb] ^= carry;\





#define NTL_ALT1_SHORT_BB_MUL_CODE1 \
   long i;\
   _ntl_ulong carry = 0, b;\
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[16];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   A[4] = A[2] << 1;\
   A[5] = A[4] ^ A[1];\
   A[6] = A[3] << 1;\
   A[7] = A[6] ^ A[1];\
   A[8] = A[4] << 1;\
   A[9] = A[8] ^ A[1];\
   A[10] = A[5] << 1;\
   A[11] = A[10] ^ A[1];\
   A[12] = A[6] << 1;\
   A[13] = A[12] ^ A[1];\
   A[14] = A[7] << 1;\
   A[15] = A[14] ^ A[1];\
   for (i = 0; i < sb; i++) {\
      b = bp[i];\
      lo = A[b & 15]; t = A[(b >> 4) & 15]; hi = t >> 28; lo ^= t << 4;\
      t = A[(b >> 8) & 15]; hi ^= t >> 24; lo ^= t << 8;\
      t = A[(b >> 12) & 15]; hi ^= t >> 20; lo ^= t << 12;\
      t = A[(b >> 16) & 15]; hi ^= t >> 16; lo ^= t << 16;\
      t = A[(b >> 20) & 15]; hi ^= t >> 12; lo ^= t << 20;\
      t = A[(b >> 24) & 15]; hi ^= t >> 8; lo ^= t << 24;\
      t = A[b >> 28]; hi ^= t >> 4; lo ^= t << 28;\
      cp[i] = carry ^ lo;    carry = hi;\
   }\
   cp[sb] = carry;\





#define NTL_ALT1_HALF_BB_MUL_CODE0 \
   _ntl_ulong hi, lo, t;\
   _ntl_ulong A[4];\
   A[0] = 0;\
   A[1] = a;\
   A[2] = A[1] << 1;\
   A[3] = A[2] ^ A[1];\
   lo = A[b & 3]; t = A[(b >> 2) & 3]; hi = t >> 30; lo ^= t << 2;\
   t = A[(b >> 4) & 3]; hi ^= t >> 28; lo ^= t << 4;\
   t = A[(b >> 6) & 3]; hi ^= t >> 26; lo ^= t << 6;\
   t = A[(b >> 8) & 3]; hi ^= t >> 24; lo ^= t << 8;\
   t = A[(b >> 10) & 3]; hi ^= t >> 22; lo ^= t << 10;\
   t = A[(b >> 12) & 3]; hi ^= t >> 20; lo ^= t << 12;\
   t = A[b >> 14]; hi ^= t >> 18; lo ^= t << 14;\
   hi ^= (((b & 0xaaaaUL) >> 1) & (-(a >> 31)));\
   c[0] = lo;    c[1] = hi;\



#define NTL_BB_MUL1_BITS (4)





#define NTL_BB_SQR_CODE \
lo=sqrtab[a&255];\
lo=lo|(sqrtab[(a>>8)&255]<<16);\
hi=sqrtab[(a>>16)&255];\
hi=hi|(sqrtab[(a>>24)&255]<<16);\




#define NTL_BB_REV_CODE (revtab[(a>>0)&255]<<24)\
|(revtab[(a>>8)&255]<<16)\
|(revtab[(a>>16)&255]<<8)\
|(revtab[(a>>24)&255]<<0)

#endif


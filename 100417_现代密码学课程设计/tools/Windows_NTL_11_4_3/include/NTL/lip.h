
#ifndef NTL_g_lip__H
#define NTL_g_lip__H

#include <NTL/ctools.h>

#ifdef NTL_GMP_LIP
#include <NTL/gmp_aux.h>
#endif


/*
 * This way of defining the bigint handle type is a bit non-standard,
 * but better for debugging.
 */

struct _ntl_gbigint_body {
   long alloc_;
   long size_;
};

typedef _ntl_gbigint_body *_ntl_gbigint;




#ifdef NTL_GMP_LIP


#if (defined(NTL_HAVE_LL_TYPE) && !defined(NTL_LEGACY_SP_MULMOD))

#define NTL_LONGLONG_SP_MULMOD

// on 64 bit machines, hold NTL_SP_NBITS to 60 bits,
// as certain operations (in particular, TBL_REM in g_lip_impl.h)
// are a bit faster


#if (!defined(NTL_MAXIMIZE_SP_NBITS) && NTL_BITS_PER_LONG >= 64)
#define NTL_SP_NBITS (NTL_BITS_PER_LONG-4)
#else
#define NTL_SP_NBITS (NTL_BITS_PER_LONG-2)
#endif


#if (defined(NTL_ENABLE_AVX_FFT) && (NTL_SP_NBITS > 50))
#undef NTL_SP_NBITS
#define NTL_SP_NBITS (50)
#endif


#elif (NTL_LONGDOUBLE_OK && !defined(NTL_LEGACY_SP_MULMOD) && !defined(NTL_DISABLE_LONGDOUBLE) && !defined(NTL_ENABLE_AVX_FFT))

#define NTL_LONGDOUBLE_SP_MULMOD

#define NTL_SP_NBITS NTL_WNBITS_MAX

// on 64 bit machines, hold NTL_SP_NBITS to 60 bits (see above)

#if (!defined(NTL_MAXIMIZE_SP_NBITS) && NTL_BITS_PER_LONG >= 64 && NTL_SP_NBITS > NTL_BITS_PER_LONG-4)
#undef NTL_SP_NBITS
#define NTL_SP_NBITS (NTL_BITS_PER_LONG-4)
#endif


#else


#define NTL_SP_NBITS NTL_NBITS_MAX


#endif

#if (NTL_SP_NBITS > NTL_ZZ_NBITS)
// if nails, we need to ensure NTL_SP_NBITS does not exceed
// NTL_ZZ_NBITS

#undef NTL_SP_NBITS
#define NTL_SP_NBITS NTL_ZZ_NBITS
#endif

#define NTL_NSP_NBITS NTL_NBITS_MAX
#if (NTL_NSP_NBITS > NTL_SP_NBITS)
#undef NTL_NSP_NBITS
#define NTL_NSP_NBITS NTL_SP_NBITS
#endif

#define NTL_WSP_NBITS (NTL_BITS_PER_LONG-2)
#if (NTL_WSP_NBITS > NTL_ZZ_NBITS)
// if nails, we need to ensure NTL_WSP_NBITS does not exceed
// NTL_ZZ_NBITS

#undef NTL_WSP_NBITS
#define NTL_WSP_NBITS NTL_ZZ_NBITS
#endif

#define NTL_SP_BOUND (1L << NTL_SP_NBITS)
#define NTL_NSP_BOUND (1L << NTL_NSP_NBITS)
#define NTL_WSP_BOUND (1L << NTL_WSP_NBITS)

/* define the following so an error is raised */

#define NTL_RADIX ......
#define NTL_NBITSH ......
#define NTL_RADIXM ......
#define NTL_RADIXROOT ......
#define NTL_RADIXROOTM ......
#define NTL_FRADIX_INV ......




#else

#define NTL_NBITS NTL_NBITS_MAX


#define NTL_RADIX           (1L<<NTL_NBITS)
#define NTL_NBITSH          (NTL_NBITS>>1)
#define NTL_RADIXM          (NTL_RADIX-1)
#define NTL_RADIXROOT       (1L<<NTL_NBITSH)
#define NTL_RADIXROOTM      (NTL_RADIXROOT-1)

#define NTL_FRADIX ((double) NTL_RADIX)
#define NTL_FRADIX_INV  (((double) 1.0)/((double) NTL_RADIX))


#define NTL_BITS_PER_LIMB_T NTL_BITS_PER_LONG
#define NTL_ZZ_NBITS NTL_NBITS
#define NTL_ZZ_FRADIX ((double) (1L << NTL_NBITS))
#define NTL_ZZ_WIDE_FRADIX ((double) (1L << NTL_NBITS))

#define NTL_SP_NBITS NTL_NBITS
#define NTL_SP_BOUND (1L << NTL_SP_NBITS)

#define NTL_NSP_NBITS NTL_NBITS
#define NTL_NSP_BOUND (1L << NTL_NSP_NBITS)

#define NTL_WSP_NBITS NTL_ZZ_NBITS
#define NTL_WSP_BOUND (1L << NTL_WSP_NBITS)



// Legacy function
long _ntl_gdigit(_ntl_gbigint a, long i);

#endif


// Some sanity checks on NTL_SP_NBITS...

// First check that NTL_SP_NBITS >= 30, as the documentation
// guarantees this.  This should only be a problem if GMP
// uses some really funny nail bits.

#if (NTL_SP_NBITS < 30)
#error "NTL_SP_NBITS too small"
#endif

// Second, check that NTL_BITS_PER_LONG-NTL_SP_NBITS == 2 or 
// NTL_BITS_PER_LONG-NTL_SP_NBITS >= 4.
// Some code in sp_arith.h seems to rely on this assumption.
// Again, this should only be a problem if GMP
// uses some really funny nail bits.

#if (NTL_BITS_PER_LONG-NTL_SP_NBITS != 2 && NTL_BITS_PER_LONG-NTL_SP_NBITS < 4)
#error "NTL_SP_NBITS is invalid"
#endif






// DIRT: These are copied from lip.cpp file

inline long& _ntl_ALLOC(_ntl_gbigint p)
   { return p->alloc_; }

inline long& _ntl_SIZE(_ntl_gbigint p)
   { return p->size_; }

inline long _ntl_ZEROP(_ntl_gbigint p)
{
   return !p || !_ntl_SIZE(p);
}

inline long _ntl_PINNED(_ntl_gbigint p)
  { return p && (_ntl_ALLOC(p) & 1); }


/***********************************************************************

   Basic Functions

***********************************************************************/
    


    void _ntl_gsadd(_ntl_gbigint a, long d, _ntl_gbigint *b);
       /* *b = a + d */

    void _ntl_gssub(_ntl_gbigint a, long d, _ntl_gbigint *b);
       /* *b = a - d */

    void _ntl_gadd(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *c);
       /*  *c = a + b */

    void _ntl_gsub(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *c);
       /* *c = a - b */

    void _ntl_gsubpos(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *c);
       /* *c = a - b; assumes a >= b >= 0 */

    void _ntl_gsmul(_ntl_gbigint a, long d, _ntl_gbigint *b);
       /* *b = d * a */

    void _ntl_gmul(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *c);
       /* *c = a * b */

    void _ntl_gsq(_ntl_gbigint a, _ntl_gbigint *c);
       /* *c = a * a */

    long _ntl_gsdiv(_ntl_gbigint a, long b, _ntl_gbigint *q);
       /* (*q) = floor(a/b) and a - floor(a/b)*(*q) is returned;
          error is raised if b == 0;
          if b does not divide a, then sign(*q) == sign(b) */

    void _ntl_gdiv(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *q, _ntl_gbigint *r);
       /* (*q) = floor(a/b) and (*r) = a - floor(a/b)*(*q);
          error is raised if b == 0;
          if b does not divide a, then sign(*q) == sign(b) */

    void _ntl_gmod(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *r);
       /* same as _ntl_gdiv, but only remainder is computed */

    long _ntl_gsmod(_ntl_gbigint a, long d);
       /* same as _ntl_gsdiv, but only remainder is computed */

    void _ntl_gquickmod(_ntl_gbigint *r, _ntl_gbigint b);
       /* *r = *r % b; 
	  The division is performed in place (but may sometimes
	  assumes b > 0 and *r >= 0;
          cause *r to grow by one digit) */

    void _ntl_gsaddmul(_ntl_gbigint x, long y,  _ntl_gbigint *ww);
      /* *ww += x*y */

    void _ntl_gaddmul(_ntl_gbigint x, _ntl_gbigint y,  _ntl_gbigint *ww);
      /* *ww += x*y */

    void _ntl_gssubmul(_ntl_gbigint x, long y,  _ntl_gbigint *ww);
      /* *ww -= x*y */

    void _ntl_gsubmul(_ntl_gbigint x, _ntl_gbigint y,  _ntl_gbigint *ww);
      /* *ww -= x*y */





/********************************************************************

   Shifting and bit manipulation

*********************************************************************/


    void _ntl_glshift(_ntl_gbigint n, long k, _ntl_gbigint *a);
       /* *a = sign(n) * (|n| << k);
          shift is in reverse direction for negative k */

    void _ntl_grshift(_ntl_gbigint n, long k, _ntl_gbigint *a);
       /* *a = sign(n) * (|n| >> k);
          shift is in reverse direction for negative k */
    
    long _ntl_gmakeodd(_ntl_gbigint *n);
       /*
          if (n != 0)
              *n = m;
              return (k such that n == 2 ^ k * m with m odd);
          else
              return (0); 
        */

    long _ntl_gnumtwos(_ntl_gbigint n);
        /* return largest e such that 2^e divides n, or zero if n is zero */

    long _ntl_godd(_ntl_gbigint a);
       /* returns 1 if n is odd and 0 if it is even */

    long _ntl_gbit(_ntl_gbigint a, long p);
       /* returns p-th bit of a, where the low order bit is indexed by 0;
          p out of range returns 0 */

    long _ntl_gsetbit(_ntl_gbigint *a, long p);
       /* returns original value of p-th bit of |a|, and replaces
          p-th bit of a by 1 if it was zero;
          error if p < 0 */

    long _ntl_gswitchbit(_ntl_gbigint *a, long p);
       /* returns original value of p-th bit of |a|, and switches
          the value of p-th bit of a;
          p starts counting at 0;
          error if p < 0 */


     void _ntl_glowbits(_ntl_gbigint a, long k, _ntl_gbigint *b);
        /* places k low order bits of |a| in b */ 

     long _ntl_gslowbits(_ntl_gbigint a, long k);
        /* returns k low order bits of |a| */

    long _ntl_gweights(long a);
        /* returns Hamming weight of |a| */

    long _ntl_gweight(_ntl_gbigint a);
        /* returns Hamming weight of |a| */

    void _ntl_gand(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *c);
        /* c gets bit pattern `bits of |a|` and `bits of |b|` */

    void _ntl_gor(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *c);
        /* c gets bit pattern `bits of |a|` inclusive or `bits of |b|` */

    void _ntl_gxor(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *c);
        /* c gets bit pattern `bits of |a|` exclusive or `bits of |b|` */




/************************************************************************

   Comparison

*************************************************************************/

    long _ntl_gcompare(_ntl_gbigint a, _ntl_gbigint b);
       /*
          if (a > b)
              return (1);
          if (a == b)
              return (0);
          if (a < b)
              return (-1);
         */

    long _ntl_gscompare(_ntl_gbigint a, long b);
       /* single-precision version of the above */

    inline
    long _ntl_giszero (_ntl_gbigint a)
    {
      return _ntl_ZEROP(a);
    }
       /* test for 0 */


    inline
    long _ntl_gsign(_ntl_gbigint a)
    {
       long sa;

       if (!a) return 0;

       sa = _ntl_SIZE(a);
       if (sa > 0) return 1;
       if (sa == 0) return 0;
       return -1;
    }
       /* 
          if (a > 0)
              return (1);
          if (a == 0)
              return (0);
          if (a < 0)
              return (-1);
        */

    void _ntl_gabs(_ntl_gbigint *a);
       /* *a = |a| */

    void _ntl_gnegate(_ntl_gbigint *a);
       /* *a = -a */

    void _ntl_gcopy(_ntl_gbigint a, _ntl_gbigint *b);
       /* *b = a;  */

    void _ntl_gswap(_ntl_gbigint *a, _ntl_gbigint *b);
       /* swap a and b (by swaping pointers) */

    long _ntl_g2log(_ntl_gbigint a);
       /* number of bits in |a|; returns 0 if a = 0 */

    inline
    long _ntl_g2logs(long a)
        /* single-precision version of the above */
    {
       unsigned long aa = a >= 0 ? a : - ((unsigned long) a);
       return _ntl_count_bits(aa);
    }


/********************************************************************

   Conversion

*********************************************************************/
        
    void _ntl_gzero(_ntl_gbigint *a);
       /* *a = 0;  */

    void _ntl_gone(_ntl_gbigint *a);
       /* *a = 1 */

    void _ntl_gintoz(long d, _ntl_gbigint *a);
       /* *a = d;  */


    void _ntl_guintoz(unsigned long d, _ntl_gbigint *a);
       /* *a = d;  space is allocated  */

    long _ntl_gtoint(_ntl_gbigint a);
       /* converts a to a long;  overflow results in value
          mod 2^{NTL_BITS_PER_LONG}. */

    unsigned long _ntl_gtouint(_ntl_gbigint a);
       /* converts a to a long;  overflow results in value
          mod 2^{NTL_BITS_PER_LONG}. */

   


    double _ntl_gdoub(_ntl_gbigint n);
       /* converts a to a double;  no overflow check */

    long _ntl_ground_correction(_ntl_gbigint a, long k, long residual);
       /* k >= 1, |a| >= 2^k, and residual is 0, 1, or -1.
          The result is what we should add to (a >> k) to round
          x = a/2^k to the nearest integer using IEEE-like rounding rules
          (i.e., round to nearest, and round to even to break ties).
          The result is either 0 or sign(a).
          If residual is not zero, it is as if x were replaced by
          x' = x + residual*2^{-(k+1)}.
          This can be used to break ties when x is exactly
          half way between two integers. */

    double _ntl_glog(_ntl_gbigint a);
       /* computes log(a), protecting against overflow */

    void _ntl_gdoubtoz(double a, _ntl_gbigint *x);
       /* x = floor(a);  */
    



/************************************************************************

   Square roots

*************************************************************************/


    long _ntl_gsqrts(long n);
       /* return floor(sqrt(n));  error raised in n < 0 */

    void _ntl_gsqrt(_ntl_gbigint n, _ntl_gbigint *r);
       /* *r =  floor(sqrt(n));  error raised in n < 0 */

/*********************************************************************
 
    Exponentiation
 
**********************************************************************/

   void _ntl_gexp(_ntl_gbigint a, long e, _ntl_gbigint *b);
       /* *b = a^e;  error raised if e < 0 */

   void _ntl_gexps(long a, long e, _ntl_gbigint *b);
       /* *b = a^e;  error raised if e < 0 */
       

/*********************************************************************

   Modular Arithmetic

   Addition, subtraction, multiplication, squaring division, inversion,
   and exponentiation modulo a positive modulus n, where all operands
   (except for the exponent in exponentiation) and results are in the
   range [0, n-1].   

   ALIAS RESTRICTION:  output parameters should not alias n

***********************************************************************/

    void _ntl_gaddmod(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint n, _ntl_gbigint *c);
       /* *c = (a + b) % n */

    void _ntl_gsubmod(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint n, _ntl_gbigint *c);
       /* *c = (a - b) % n */

    void _ntl_gsmulmod(_ntl_gbigint a, long b, _ntl_gbigint n, _ntl_gbigint *c);
       /* *c = (a * b) % n */

    void _ntl_gmulmod(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint n, _ntl_gbigint *c);
       /* *c = (a * b) % n */

    void _ntl_gsqmod(_ntl_gbigint a, _ntl_gbigint n, _ntl_gbigint *c);
       /* *c = (a ^ 2) % n */

    void _ntl_ginvmod(_ntl_gbigint a, _ntl_gbigint n, _ntl_gbigint *c);
       /* *c = (1 / a) % n; error raised if gcd(b, n) != 1 */

    void _ntl_gpowermod(_ntl_gbigint g, _ntl_gbigint e, _ntl_gbigint F,
                        _ntl_gbigint *h);

       /* *b = (a ^ e) % n; */




/**************************************************************************

   Euclidean Algorithms

***************************************************************************/
    void _ntl_ggcd(_ntl_gbigint m1, _ntl_gbigint m2, _ntl_gbigint *r);
       /* *r = greatest common divisor of m1 and m2;  */

    void _ntl_ggcd_alt(_ntl_gbigint m1, _ntl_gbigint m2, _ntl_gbigint *r);
       /* *r = greatest common divisor of m1 and m2;  
          a simpler algorithm used for validation
        */


    void _ntl_gexteucl(_ntl_gbigint a, _ntl_gbigint *xa,
                 _ntl_gbigint b, _ntl_gbigint *xb,
                 _ntl_gbigint *d);
       /*
          *d = a * *xa + b * *xb = gcd(a, b);
          sets *d, *xa and *xb given a and b;
        */


    long _ntl_ginv(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *c);
       /*
          if (a and b coprime)
          {
              *c = inv; 
              return(0);
          }
          else
          {
              *c = gcd(a, b);
              return(1);
          }
          
          where inv is such that (inv * a)  == 1 mod b;
          error raised if a < 0 or b <= 0
        */

     long _ntl_gxxratrecon(_ntl_gbigint x, _ntl_gbigint m,  
                      _ntl_gbigint a_bound, _ntl_gbigint b_bound,
                      _ntl_gbigint *a, _ntl_gbigint *b);

        /* rational reconstruction: see doc in ZZ.txt */


        
/**********************************************************************

    Storage Allocation

    These routines use malloc and free.

***********************************************************************/

    inline
    long _ntl_gmaxalloc(_ntl_gbigint x)
    {
      if (!x)
         return 0;
      else
         return _ntl_ALLOC(x) >> 2;
    }

    // DIRT: see lip.c for more info on ALLOC 

    void _ntl_gsetlength(_ntl_gbigint *v, long len);
       /* Allocates enough space to hold a len-digit number,
          where each digit has NTL_NBITS bits.
          If space must be allocated, space for one extra digit
          is always allocated. if (exact) then no rounding
          occurs. */

    void _ntl_gfree(_ntl_gbigint x);
       /* Free's space held by x. */


/*******************************************************************

    Special routines

********************************************************************/

inline
long _ntl_gsize(_ntl_gbigint rep)
{
  if (!rep)
      return 0;
   else if (_ntl_SIZE(rep) < 0)
      return -_ntl_SIZE(rep);
   else
      return _ntl_SIZE(rep);
}

long _ntl_gisone(_ntl_gbigint n);

long _ntl_gsptest(_ntl_gbigint a);
long _ntl_gwsptest(_ntl_gbigint a);
long _ntl_gcrtinrange(_ntl_gbigint g, _ntl_gbigint a);

void _ntl_gfrombytes(_ntl_gbigint *x, const unsigned char *p, long n);
void _ntl_gbytesfromz(unsigned char *p, _ntl_gbigint a, long nn);


long _ntl_gblock_construct_alloc(_ntl_gbigint *x, long d, long n);
void _ntl_gblock_construct_set(_ntl_gbigint x, _ntl_gbigint *y, long i);
long _ntl_gblock_destroy(_ntl_gbigint x);
long _ntl_gblock_storage(long d);



// These are common to both implementations

class _ntl_tmp_vec {
public:
   virtual ~_ntl_tmp_vec() { }
};

class _ntl_crt_struct {
public:
   virtual ~_ntl_crt_struct() { }
   virtual bool special() = 0;
   virtual void insert(long i, _ntl_gbigint m) = 0;
   virtual _ntl_tmp_vec *extract() = 0;
   virtual _ntl_tmp_vec *fetch() = 0;
   virtual void eval(_ntl_gbigint *x, const long *b, 
                     _ntl_tmp_vec *tmp_vec) = 0;
};

_ntl_crt_struct * 
_ntl_crt_struct_build(long n, _ntl_gbigint p, long (*primes)(long));

class _ntl_rem_struct {
public:
   virtual ~_ntl_rem_struct() { }
   virtual void eval(long *x, _ntl_gbigint a, _ntl_tmp_vec *tmp_vec) = 0;
   virtual _ntl_tmp_vec *fetch() = 0;
};

_ntl_rem_struct *
_ntl_rem_struct_build(long n, _ntl_gbigint modulus, long (*p)(long));


// montgomery
class _ntl_reduce_struct {
public:
   virtual ~_ntl_reduce_struct() { }
   virtual void eval(_ntl_gbigint *x, _ntl_gbigint *a) = 0;
   virtual void adjust(_ntl_gbigint *x) = 0;
};

_ntl_reduce_struct *
_ntl_reduce_struct_build(_ntl_gbigint modulus, _ntl_gbigint excess);


// faster reduction with preconditioning -- general usage, single modulus

struct _ntl_general_rem_one_struct;

_ntl_general_rem_one_struct *
_ntl_general_rem_one_struct_build(long p);

long 
_ntl_general_rem_one_struct_apply(_ntl_gbigint a, long p, _ntl_general_rem_one_struct *pinfo);

void
_ntl_general_rem_one_struct_delete(_ntl_general_rem_one_struct *pinfo);

long _ntl_gvalidate(_ntl_gbigint a);


// special-purpose routines for accumulating CRT-like summations
void
_ntl_quick_accum_begin(_ntl_gbigint *xp, long sz);

void
_ntl_quick_accum_muladd(_ntl_gbigint x, _ntl_gbigint y, long b);

void
_ntl_quick_accum_end(_ntl_gbigint x);

// special-purpose routines for SSMul in ZZX

#if (defined(NTL_GMP_LIP) && (NTL_ZZ_NBITS & (NTL_ZZ_NBITS-1)) == 0)
// NOTE: the test (NTL_ZZ_NBITS & (NTL_ZZ_NBITS-1)) == 0
// effectively checks that NTL_ZZ_NBITS is a power of two

#define NTL_PROVIDES_SS_LIP_IMPL

void
_ntl_leftrotate(_ntl_gbigint *a, const _ntl_gbigint *b, long e,
                _ntl_gbigint p, long n, _ntl_gbigint *scratch);

void 
_ntl_ss_addmod(_ntl_gbigint *x, const _ntl_gbigint *a,
               const _ntl_gbigint *b, _ntl_gbigint p, long n);
void 
_ntl_ss_submod(_ntl_gbigint *x, const _ntl_gbigint *a,
               const _ntl_gbigint *b, _ntl_gbigint p, long n);
#endif


#endif

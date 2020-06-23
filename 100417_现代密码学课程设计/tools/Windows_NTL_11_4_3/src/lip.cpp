
/*
 * This is a "wrapper" layer that builds on top of the "mpn" layer of gmp.
 * This layer provides much of the same functionality of the "mpz"
 * layer of gmp, but the interface it provides is much more like
 * the interface provided by lip.
 *
 * This layer was written under the following assumptions about gmp:
 *  1) mp_limb_t is an unsigned integral type
 *  2) sizeof(mp_limb_t) == sizeof(long) or sizeof(mp_limb_t) == 2*sizeof(long)
 *  3) the number of bits of an mp_limb_t is equal to that of a long,
 *     or twice that of a long
 *  4) the number of bits of a gmp radix is equal to the number of bits
 *     of an mp_limb_t
 *
 * Except for assumption (1), these assumptions are verified in the
 * installation script, and they should be universally satisfied in practice,
 * except when gmp is built using the proposed, new "nail" fetaure
 * (in which some bits of an mp_limb_t are unused).
 * The code here will not work properly with the "nail" feature;
 * however, I have (attempted to) identify all such problem spots,
 * and any other places where assumptions (2-4) are made,
 * with a comment labeled "DIRT".
 */



#include <NTL/lip.h>

#include <NTL/tools.h>
#include <NTL/vector.h>
#include <NTL/SmartPtr.h>

#include <NTL/sp_arith.h>


#ifdef NTL_GMP_LIP
#include <gmp.h>

#if (__GNU_MP_VERSION < 5)
#error "GMP version 5.0.0 or later required"
#endif 

#endif

NTL_IMPORT_FROM_STD
NTL_USE_NNS


#if (defined(NTL_HAVE_LL_TYPE) && NTL_BITS_PER_LIMB_T == NTL_BITS_PER_LONG)
#define NTL_VIABLE_LL
#endif


#ifdef NTL_GMP_LIP

typedef mp_limb_t _ntl_limb_t;

#define NTL_MPN(fun) mpn_ ## fun

#else

typedef unsigned long _ntl_limb_t;
typedef long _ntl_signed_limb_t;

#define NTL_MPN(fun) _ntl_mpn_ ## fun

#endif



typedef _ntl_limb_t *_ntl_limb_t_ptr;

#define NTL_NAIL_BITS (NTL_BITS_PER_LIMB_T-NTL_ZZ_NBITS)

#define NTL_LIMB_MASK (_ntl_limb_t(-1) >> NTL_NAIL_BITS)

#define NTL_ZZ_RADIX (NTL_LIMB_MASK+_ntl_limb_t(1))
// this will be zero if no nails

#define NTL_ZZ_FRADIX_INV  (1.0/NTL_ZZ_FRADIX)




#if (NTL_ZZ_NBITS > NTL_BITS_PER_LONG-2)

static inline double 
DBL(_ntl_limb_t x)
{
   return double(x);
}

#else

// this might be a bit faster
static inline double 
DBL(_ntl_limb_t x)
{
   return double(long(x));
}

#endif


// DIRT: we assume that NTL_BITS_PER_LIMB_T >= BITS_PER_LONG
static inline _ntl_limb_t
ABS(long x)
{
   if (x < 0)
      return -_ntl_limb_t(x); // careful !
   else
      return _ntl_limb_t(x);
}

static inline long
XOR(long a, long b)
{
   return a ^ b;
}


static 
inline _ntl_limb_t CLIP(_ntl_limb_t a)
{
   return a & NTL_LIMB_MASK;
}

static 
inline _ntl_limb_t XCLIP(_ntl_limb_t a)
{
   return a & ~NTL_LIMB_MASK;
}


#if (NTL_BITS_PER_LIMB_T == NTL_BITS_PER_LONG)
static
inline long COUNT_BITS(_ntl_limb_t x)
{
   return _ntl_count_bits(x);
}
#else
static
inline long COUNT_BITS(_ntl_limb_t x)
{
   if (!x) { return 0; } 

   long res = NTL_BITS_PER_LIMB_T;
   while (x < (_ntl_limb_t(1) << (NTL_BITS_PER_LIMB_T-1))) {
      x <<= 1;
      res--;
   }

   return res;
}
#endif



#ifndef NTL_GMP_LIP

/*
 * Kind of a "mini LIP" --- this implements some of GMP's mpn interface using
 * a floating point strategy similar to Arjen Lenstra's LIP library.  However,
 * most of the low-level details have evolved over time, so very little of Lenstra's
 * original code is actually left here.
 *
 */

_ntl_signed_limb_t 
_ntl_mpn_cmp(const _ntl_limb_t *s1p, const _ntl_limb_t *s2p, long n)
{
   for (long i = n-1; i >= 0; i--) {
      _ntl_signed_limb_t diff = _ntl_signed_limb_t(s1p[i]) - _ntl_signed_limb_t(s2p[i]);
      if (diff) return diff;
   }

   return 0;
}


_ntl_limb_t
_ntl_mpn_lshift (_ntl_limb_t *rp, const _ntl_limb_t *up, long n, long cnt)
{
  _ntl_limb_t high_limb, low_limb;
  long tnc;
  _ntl_limb_t retval;

  up += n;
  rp += n;

  tnc = NTL_ZZ_NBITS - cnt;
  low_limb = *--up;
  retval = low_limb >> tnc;
  high_limb = CLIP(low_limb << cnt);

  while (--n != 0)
    {
      low_limb = *--up;
      *--rp = high_limb | (low_limb >> tnc);
      high_limb = CLIP(low_limb << cnt);
    }
  *--rp = high_limb;

  return retval;
}


_ntl_limb_t
_ntl_mpn_rshift(_ntl_limb_t *rp, const _ntl_limb_t *up, long n, long cnt)
{
  _ntl_limb_t high_limb, low_limb;
  long tnc;
  _ntl_limb_t retval;

  tnc = NTL_ZZ_NBITS - cnt;
  high_limb = *up++;
  retval = CLIP(high_limb << tnc);
  low_limb = high_limb >> cnt;

  while (--n != 0)
    {
      high_limb = *up++;
      *rp++ = low_limb | CLIP(high_limb << tnc);
      low_limb = high_limb >> cnt;
    }
  *rp = low_limb;

  return retval;
}

_ntl_limb_t
_ntl_mpn_add_1 (_ntl_limb_t *rp, const _ntl_limb_t *ap, long  n, _ntl_limb_t b)
{
  long i;

  if (rp != ap) {
    i = 0;
    do
      {
	_ntl_limb_t r = ap[i] + b;
	rp[i] = CLIP(r);
	b = r >> NTL_ZZ_NBITS;
      }
    while (++i < n);

    return b;
  }
  else {
    i = 0;
    do
      {
        if (!b) return 0;
	_ntl_limb_t r = ap[i] + b;
	rp[i] = CLIP(r);
	b = r >> NTL_ZZ_NBITS;
      }
    while (++i < n);

    return b;
  }
}



_ntl_limb_t
_ntl_mpn_add_n (_ntl_limb_t *rp, const  _ntl_limb_t *ap, const _ntl_limb_t *bp, long n)
{
  long i;
  _ntl_limb_t cy;

  for (i = 0, cy = 0; i < n; i++)
    {
      _ntl_limb_t sum = ap[i] + bp[i] + cy;
      rp[i] = CLIP(sum);
      cy = sum >> NTL_ZZ_NBITS;
    }
  return cy;
}


_ntl_limb_t
_ntl_mpn_add (_ntl_limb_t *rp, const _ntl_limb_t *ap, long an, const  _ntl_limb_t *bp, long bn)
{
  _ntl_limb_t cy;

  cy = _ntl_mpn_add_n (rp, ap, bp, bn);
  if (an > bn)
    cy = _ntl_mpn_add_1 (rp + bn, ap + bn, an - bn, cy);
  return cy;
}


_ntl_limb_t
_ntl_mpn_sub_1 (_ntl_limb_t *rp, const _ntl_limb_t *ap, long  n, _ntl_limb_t b)
{
  long i;

  if (rp != ap) {
     i = 0;
     do
       {
	 _ntl_limb_t r = ap[i] - b;
	 rp[i] = CLIP(r);
	 b = (r >> NTL_ZZ_NBITS) & 1;
       }
     while (++i < n);

     return b;
  }
  else {
     i = 0;
     do
       {
         if (!b) return 0;
	 _ntl_limb_t r = ap[i] - b;
	 rp[i] = CLIP(r);
	 b = (r >> NTL_ZZ_NBITS) & 1;
       }
     while (++i < n);

     return b;
  }

}



_ntl_limb_t
_ntl_mpn_sub_n (_ntl_limb_t *rp, const  _ntl_limb_t *ap, const _ntl_limb_t *bp, long n)
{
  long i;
  _ntl_limb_t cy;

  for (i = 0, cy = 0; i < n; i++)
    {
      _ntl_limb_t sum = ap[i] - bp[i] - cy;
      rp[i] = CLIP(sum);
      cy = (sum >> NTL_ZZ_NBITS) & 1;
    }
  return cy;
}


_ntl_limb_t
_ntl_mpn_sub (_ntl_limb_t *rp, const _ntl_limb_t *ap, long an, const  _ntl_limb_t *bp, long bn)
{
  _ntl_limb_t cy;

  cy = _ntl_mpn_sub_n (rp, ap, bp, bn);
  if (an > bn)
    cy = _ntl_mpn_sub_1 (rp + bn, ap + bn, an - bn, cy);
  return cy;
}



#ifndef NTL_HAVE_LL_TYPE


// (t, a) = b*d + a + t
#if 0
// This is true to the original LIP spirit, and should
// still work assuming we have something close to the correct
// relative precision.  However, I find that on both haswell and skylake,
// it makes multiplication about twice as slow, which is a bit surprising
// I think the main issue is the extra int to double conversion.
static inline void
_ntl_addmulp(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t d, _ntl_limb_t& t) 
{
   _ntl_limb_t t1 = b * d; 
   _ntl_limb_t t2 = a + t;
   _ntl_limb_t t3 = CLIP(t1+t2);

   double d1 = DBL(b) * DBL(d);
   double d2 = d1 + double( _ntl_signed_limb_t(t2) - _ntl_signed_limb_t(t3) 
                         + _ntl_signed_limb_t(NTL_ZZ_RADIX/2) );
   double d3 = d2 * NTL_ZZ_FRADIX_INV;

   t = _ntl_signed_limb_t(d3);
   a = t3;
}

#else
#if (NTL_NAIL_BITS == 2)
static inline void
_ntl_addmulp(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t d, _ntl_limb_t& t) 
{ 
   _ntl_limb_t t1 = b * d; 
   _ntl_limb_t t2 = _ntl_signed_limb_t( DBL(b)*(DBL(d)*NTL_ZZ_FRADIX_INV) ) - 1; 
   t2 = t2 + ( (t1 - (t2 << NTL_ZZ_NBITS)) >> NTL_ZZ_NBITS ); 
   t1 = CLIP(t1) + a + t;
   t = t2 + (t1 >> NTL_ZZ_NBITS); 
   a = CLIP(t1);
}
#else
static inline void
_ntl_addmulp(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t d, _ntl_limb_t& t) 
{ 
   _ntl_limb_t t1 = b * d + a + t; 
   _ntl_limb_t t2 = _ntl_signed_limb_t( DBL(b)*(DBL(d)*NTL_ZZ_FRADIX_INV) ) - 1; 
   t = t2 + ( (t1 - (t2 << NTL_ZZ_NBITS)) >> NTL_ZZ_NBITS ); 
   a = CLIP(t1);
}
#endif
#endif

// (t, a) = b*b + a
static inline void
_ntl_addmulpsq(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t& t)
{ 
   _ntl_limb_t t1 =  b*b + a; 
   _ntl_limb_t t2 = _ntl_signed_limb_t( DBL(b)*(DBL(b)*NTL_ZZ_FRADIX_INV) ) - 1; 
   t = t2 + ((t1 - (t2 << NTL_ZZ_NBITS)) >> NTL_ZZ_NBITS); 
   a = CLIP(t1);
}

// (t, a) = b*d + t
static inline void
_ntl_mulp(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t d, _ntl_limb_t& t) 
{ 
   _ntl_limb_t t1 =  b*d + t; 
   _ntl_limb_t t2 = _ntl_signed_limb_t( DBL(b)*(DBL(d)*NTL_ZZ_FRADIX_INV) ) - 1; 
   t = t2 + ((t1 - (t2 << NTL_ZZ_NBITS)) >> NTL_ZZ_NBITS); 
   a = CLIP(t1);
}

// (t, a) = b*(-d) + a + t, where t is "signed"
static inline void
_ntl_submulp(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t d, _ntl_limb_t& t) 
{ 
   _ntl_limb_t t1 =  b*(NTL_ZZ_RADIX-d) + a;
   _ntl_limb_t t2 = _ntl_signed_limb_t( DBL(b)*(DBL(NTL_ZZ_RADIX-d)*NTL_ZZ_FRADIX_INV) ) - 1; 
   _ntl_limb_t lo = CLIP(t1);
   _ntl_limb_t hi = t2 + ((t1 - (t2 << NTL_ZZ_NBITS)) >> NTL_ZZ_NBITS); 
   lo += t;
   a = CLIP(lo);
   t = hi - b - (lo >> (NTL_BITS_PER_LIMB_T-1));
}


#else

// (t, a) = b*d + a + t
static inline void
_ntl_addmulp(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t d, _ntl_limb_t& t) 
{
   ll_type x;
   ll_imul(x, b, d);
   ll_add(x, a+t);
   a = CLIP(ll_get_lo(x));
   t = ll_rshift_get_lo<NTL_ZZ_NBITS>(x);
}

// (t, a) = b*b + a
static inline void
_ntl_addmulpsq(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t& t)
{
   ll_type x;
   ll_imul(x, b, b);
   ll_add(x, a);
   a = CLIP(ll_get_lo(x));
   t = ll_rshift_get_lo<NTL_ZZ_NBITS>(x);
}

// (t, a) = b*d + t
static inline void
_ntl_mulp(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t d, _ntl_limb_t& t) 
{
   ll_type x;
   ll_imul(x, b, d);
   ll_add(x, t);
   a = CLIP(ll_get_lo(x));
   t = ll_rshift_get_lo<NTL_ZZ_NBITS>(x);
}

// (t, a) = b*(-d) + a + t, where t is "signed"
static inline void
_ntl_submulp(_ntl_limb_t& a, _ntl_limb_t b, _ntl_limb_t d, _ntl_limb_t& t) 
{
   ll_type x;
   ll_imul(x, b, NTL_ZZ_RADIX-d);
   _ntl_limb_t lo = CLIP(ll_get_lo(x));
   _ntl_limb_t hi = ll_rshift_get_lo<NTL_ZZ_NBITS>(x);
   lo += a+t;
   a = CLIP(lo);

   // NOTE: the high-order bits of lo encode 0, 1, or -1
#if (!defined(NTL_CLEAN_INT) && NTL_ARITH_RIGHT_SHIFT)
   t = hi - b + (cast_signed(lo) >> NTL_ZZ_NBITS);
#else
   t = hi - b + ((lo + NTL_ZZ_RADIX) >> NTL_ZZ_NBITS) - 1;
#endif
}

#endif

void 
_ntl_addmulsq(long n, _ntl_limb_t *a, const _ntl_limb_t *b)
{
   _ntl_limb_t s = b[0];
   _ntl_limb_t carry = 0;
   for (long i = 0; i < n; i++) {
      _ntl_addmulp(a[i], b[i+1], s, carry);
   }
   a[n] += carry;
}


_ntl_limb_t
_ntl_mpn_mul_1 (_ntl_limb_t* rp, const _ntl_limb_t* up, long n, _ntl_limb_t vl) 
{
   _ntl_limb_t carry = 0;
   for (long i = 0; i < n; i++) 
      _ntl_mulp(rp[i], up[i], vl, carry);
   return carry;
}


_ntl_limb_t
_ntl_mpn_addmul_1 (_ntl_limb_t* rp, const _ntl_limb_t* up, long n, _ntl_limb_t vl)
{
   _ntl_limb_t carry = 0;
   for (long i = 0; i < n; i++) 
      _ntl_addmulp(rp[i], up[i], vl, carry);
   return carry;
}



_ntl_limb_t
_ntl_mpn_submul_1 (_ntl_limb_t* rp, const _ntl_limb_t* up, long n, _ntl_limb_t vl)
{
   _ntl_limb_t carry = 0;
   for (long i = 0; i < n; i++) {
      _ntl_submulp(rp[i], up[i], vl, carry);
   }
   return -carry;
}

// compute (carry, rp[n-1], ..., rp[0]) = (rp[n-1], ..., rp[0], shift_in) 
//                                          - (up[n-1], ..., up[0]) * vl,
// and return carry (which may be negative, but stored as an unsigned).  No
// aliasing is allowed.  This is a special-purpose used by the tdiv_qr routine,
// to avoid allocating extra buffer space and extra shifting.  It is not a part
// of GMP interface.

_ntl_limb_t
_ntl_mpn_shift_submul_1(_ntl_limb_t* NTL_RESTRICT rp, _ntl_limb_t shift_in, const _ntl_limb_t* NTL_RESTRICT up, long n, _ntl_limb_t vl)
{
#if 0
   _ntl_limb_t carry = 0;
   for (long i = 0; i < n; i++) {
      _ntl_submulp(shift_in, up[i], vl, carry);
      _ntl_swap(shift_in, rp[i]);
   }

   return carry + shift_in;
#else
   // NOTE: loop unrolling seems to help a little bit
   _ntl_limb_t carry = 0;
   long i = 0;
   for (; i <= n-4; i += 4) {
      _ntl_submulp(shift_in, up[i], vl, carry);
      _ntl_limb_t tmp1 = rp[i];
      rp[i] = shift_in;

      _ntl_submulp(tmp1, up[i+1], vl, carry);
      _ntl_limb_t tmp2 = rp[i+1];
      rp[i+1] = tmp1;

      _ntl_submulp(tmp2, up[i+2], vl, carry);
      _ntl_limb_t tmp3 = rp[i+2];
      rp[i+2] = tmp2;

      _ntl_submulp(tmp3, up[i+3], vl, carry);
      shift_in = rp[i+3];
      rp[i+3] = tmp3;
   }

   for (; i < n; i++) {
      _ntl_submulp(shift_in, up[i], vl, carry);
      _ntl_swap(shift_in, rp[i]);
   }

   return carry + shift_in;
#endif
}

static inline void
_ntl_mpn_base_sqr(_ntl_limb_t *c, const _ntl_limb_t *a, long sa)
{
   long sc = 2*sa;

   for (long i = 0; i < sc; i++) c[i] = 0;

   _ntl_limb_t carry = 0;
   for (long i = 0, j = 0; j < sa; i += 2, j++) {
      _ntl_limb_t uc, t; 
       uc = carry + (c[i] << 1);
       t = CLIP(uc);
       _ntl_addmulpsq(t, a[j], carry);
       c[i] = t;
       _ntl_addmulsq(sa-j-1, c+i+1, a+j);
       uc =  (uc >> NTL_ZZ_NBITS) + (c[i+1] << 1);
       uc += carry;
       carry = uc >> NTL_ZZ_NBITS;
       c[i+1] = CLIP(uc);
   }
}

static inline _ntl_limb_t
_ntl_mpn_base_mul (_ntl_limb_t* rp, const _ntl_limb_t* up, long un, const _ntl_limb_t* vp, long vn)
{
  rp[un] = _ntl_mpn_mul_1 (rp, up, un, vp[0]);

  while (--vn >= 1)
    {
      rp += 1, vp += 1;
      rp[un] = _ntl_mpn_addmul_1 (rp, up, un, vp[0]);
    }
  return rp[un];
}



// Karatsuba implementation: I didn't really want to port this
// implementation, but some higher-level code assumes that
// multiplication is subquadratic 

static 
long kar_fold(_ntl_limb_t *T, const _ntl_limb_t *b, long sb, long hsa)
{
   _ntl_limb_t carry = 0;

   for (long i = 0; i < sb-hsa; i++) {
      _ntl_limb_t t = b[i] + b[i+hsa] + carry;
      carry = t >> NTL_ZZ_NBITS;
      T[i] = CLIP(t);
   }

   for (long i = sb-hsa; i < hsa; i++) {
      _ntl_limb_t t = b[i] + carry;
      carry = t >> NTL_ZZ_NBITS;
      T[i] = CLIP(t);
   }

   if (carry) {
      T[hsa] = carry;
      return hsa+1;
   }
   else {
      return hsa;
   }

}

static
void kar_sub(_ntl_limb_t *T, const _ntl_limb_t *c, long sc)
{
   _ntl_limb_t carry = 0;

   for (long i = 0; i < sc; i++) {
      _ntl_limb_t t = T[i] - c[i] - carry;
      carry = (t >> NTL_ZZ_NBITS) & 1;
      T[i] = CLIP(t);
   }

   for (long i = sc; carry; i++) {
      _ntl_limb_t t = T[i] - 1;
      carry = (t >> NTL_ZZ_NBITS) & 1;
      T[i] = CLIP(t);
   }
}

static
void kar_add(_ntl_limb_t *c, const _ntl_limb_t *T, long sT, long hsa)
{
   c += hsa;
   _ntl_limb_t carry = 0;

   while (sT > 0 && T[sT-1] == 0) sT--;

   for (long i = 0; i < sT; i++) {
      _ntl_limb_t t = c[i] + T[i] + carry;
      carry = t >> NTL_NBITS;
      c[i] = CLIP(t);
   }

   for (long i = sT; carry; i++) {
      _ntl_limb_t t = c[i] + 1;
      carry = t >> NTL_NBITS;
      c[i] = CLIP(t);
   }
}

static
void kar_fix(_ntl_limb_t *c, const _ntl_limb_t *T, long sT, long hsa)
{
   for (long i = 0; i < hsa; i++) {
      c[i] = T[i];
   }

   _ntl_limb_t carry = 0;

   for (long i = hsa; i < sT; i++) {
      _ntl_limb_t t = c[i] + T[i] + carry;
      carry = t >> NTL_NBITS;
      c[i] = CLIP(t);
   }

   for (long i = sT; carry; i++) {
      _ntl_limb_t t = c[i] + 1;
      carry = t >> NTL_NBITS;
      c[i] = CLIP(t);
   }
}


#define KARX (16)

static
void kar_mul(_ntl_limb_t *c, const _ntl_limb_t *a, long sa, 
             const _ntl_limb_t *b, long sb, _ntl_limb_t *stk, long sp)
{

   if (sa < sb) {
      _ntl_swap(a, b);
      _ntl_swap(sa, sb);
   }

   if (sb < KARX) {
      /* classic algorithm */
 
      _ntl_mpn_base_mul(c, a, sa, b, sb);

   }
   else {
      long hsa = (sa + 1) >> 1;

      if (hsa < sb) {
         /* normal case */

         _ntl_limb_t *T1, *T2, *T3;

         /* allocate space */

         sp -= (hsa + 1) + (hsa + 1) + ((hsa << 1) + 2);
         if (sp < 0) TerminalError("internal error: kmem overflow");

         T1 = stk;  stk += hsa + 1;  
         T2 = stk;  stk += hsa + 1;  
         T3 = stk;  stk += (hsa << 1) + 2; 

         /* compute T1 = a_lo + a_hi */
         long sT1 = kar_fold(T1, a, sa, hsa);

         /* compute T2 = b_lo + b_hi */
         long sT2 = kar_fold(T2, b, sb, hsa);

         
         /* recursively compute T3 = T1 * T2 */
         kar_mul(T3, T1, sT1, T2, sT2, stk, sp);

         /* recursively compute a_hi * b_hi into high part of c */
         /* and subtract from T3 */
         kar_mul(c + (hsa << 1), a+hsa, sa-hsa, b+hsa, sb-hsa, stk, sp);
         kar_sub(T3, c + (hsa << 1), sa+sb-2*hsa);

         /* recursively compute a_lo*b_lo into low part of c */
         /* and subtract from T3 */
         kar_mul(c, a, hsa, b, hsa, stk, sp);
         kar_sub(T3, c, 2*hsa);

         /* finally, add T3 * NTL_RADIX^{hsa} to c */
         kar_add(c, T3, sT1+sT2, hsa);
      }
      else {
         /* degenerate case */

         _ntl_limb_t *T;
         
    
         sp -= (sb + hsa);
         if (sp < 0) TerminalError("internal error: kmem overflow");
 
         T = stk;  stk += sb + hsa;

         /* recursively compute b*a_hi into high part of c */
         kar_mul(c + hsa, a+hsa, sa-hsa, b, sb, stk, sp);

         /* recursively compute b*a_lo into T */
         kar_mul(T, a, hsa, b, sb, stk, sp);

         /* fix-up result */
         kar_fix(c, T, hsa+sb, hsa);
      }
   }
}

NTL_TLS_GLOBAL_DECL(Vec<_ntl_limb_t>, kmem)

static
void kar_mul(_ntl_limb_t *c, const _ntl_limb_t *a, long sa, 
             const _ntl_limb_t *b, long sb)
{
   long n = sa;
   long sp = 0;
   do {
      long hn = (n+1) >> 1;
      sp += (hn << 2) + 7;
      n = hn+1;
   } while (n >= KARX);

   NTL_TLS_GLOBAL_ACCESS(kmem);
   Vec<_ntl_limb_t>::Watcher kmem_watcher(kmem);

   kmem.SetLength(sp);
   kar_mul(c, a, sa, b, sb, kmem.elts(), sp);
}


#define KARSX (32) 

static
void kar_sq(_ntl_limb_t *c, const _ntl_limb_t *a, long sa, 
            _ntl_limb_t *stk, long sp)
{
   if (sa < KARSX) {
      /* classic algorithm */

      _ntl_mpn_base_sqr(c, a, sa);
   }
   else {
      long hsa = (sa + 1) >> 1;
      _ntl_limb_t *T1, *T2;

      sp -= (hsa + 1) + ((hsa << 1) + 2);
      if (sp < 0) TerminalError("internal error: kmem overflow");

      T1 = stk;  stk += hsa + 1;
      T2 = stk;  stk += (hsa << 1) + 2;

      long sT1 = kar_fold(T1, a, sa, hsa);
      kar_sq(T2, T1, sT1, stk, sp);

      kar_sq(c + (hsa << 1), a+hsa, sa-hsa, stk, sp);
      kar_sub(T2, c + (hsa << 1), 2*(sa-hsa));

      kar_sq(c, a, hsa, stk, sp);
      kar_sub(T2, c, 2*hsa);

      kar_add(c, T2, 2*sT1, hsa);
   }
}


static
void kar_sq(_ntl_limb_t *c, const _ntl_limb_t *a, long sa) 
{
   long n = sa;
   long sp = 0;
   do {
      long hn = (n+1) >> 1;
      sp += 3*hn + 5;
      n = hn+1;
   } while (n >= KARSX);

   NTL_TLS_GLOBAL_ACCESS(kmem);
   Vec<_ntl_limb_t>::Watcher kmem_watcher(kmem);

   kmem.SetLength(sp);
   kar_sq(c, a, sa, kmem.elts(), sp);
}






void
_ntl_mpn_sqr(_ntl_limb_t *c, const _ntl_limb_t *a, long sa)
{
  if (sa >= KARSX) {
    kar_sq(c, a, sa);
    return;
  }

  _ntl_mpn_base_sqr(c, a, sa);
}


// Like the corresponding GMP routine, this assumes un >= vn >= 1
_ntl_limb_t
_ntl_mpn_mul (_ntl_limb_t* rp, const _ntl_limb_t* up, long un, const _ntl_limb_t* vp, long vn)
{
  if (up == vp && un == vn) {
    _ntl_mpn_sqr(rp, up, un);
    return rp[2*un-1];
  }

  if (vn >= KARX) {
    kar_mul(rp, up, un, vp, vn);
    return rp[un+vn-1];
  }

  return _ntl_mpn_base_mul(rp, up, un, vp, vn);
}


// (quot, numhigh) = (num / denom, num % denom), where num = numhigh*RADIX + numlow
// deninv should be pre-computed as 1/double(denom).
// Normally, this should be invoked with numhigh < denom, which guarantees
// that quot < RADIX.  However, this implementation work with quot as large as RADIX+1,
// which could exploited in the general division routine (but it currently is not).

// NOTE: these are the only routines left that completely rely on the assumption
// that NTL_ZZ_NBITS is less than floating point precision.  It would be possible
// to increase NTL_ZZ_NBITS if defined(NTL_HAVE_LL_TYPE), but this is not something
// I want to spend time on.  Use GMP instead!
static inline void
_ntl_div21p(_ntl_limb_t& numhigh, _ntl_limb_t numlow, _ntl_limb_t denom, double deninv, _ntl_limb_t& quot) 
{ 
   _ntl_limb_t q21 = _ntl_signed_limb_t(((NTL_ZZ_FRADIX * DBL(numhigh)) + 
                                          DBL(numlow))* deninv); 

   _ntl_limb_t r21 = (numhigh << NTL_ZZ_NBITS) + numlow - denom*q21;

   r21 = sp_CorrectDeficitQuo(q21, r21, _ntl_signed_limb_t(denom));
   r21 = sp_CorrectExcessQuo (q21, r21, _ntl_signed_limb_t(denom));
 
   quot = q21; 
   numhigh = r21; 
}

// same as above, but quot not computed
static inline void
_ntl_rem21p(_ntl_limb_t& numhigh, _ntl_limb_t numlow, _ntl_limb_t denom, double deninv) 
{ 
   _ntl_limb_t q21 = _ntl_signed_limb_t(((NTL_ZZ_FRADIX * DBL(numhigh)) + 
                                          DBL(numlow))* deninv); 

   _ntl_limb_t r21 = (numhigh << NTL_ZZ_NBITS) + numlow - denom*q21;

   r21 = sp_CorrectDeficit(r21, _ntl_signed_limb_t(denom));
   r21 = sp_CorrectExcess (r21, _ntl_signed_limb_t(denom));
 
   numhigh = r21; 
}

// same as above, but remainder not computed, returns quot
static inline _ntl_limb_t
_ntl_quo21p(_ntl_limb_t numhigh, _ntl_limb_t numlow, _ntl_limb_t denom, double deninv)
{ 
   _ntl_limb_t q21 = _ntl_signed_limb_t(((NTL_ZZ_FRADIX * DBL(numhigh)) + 
                                          DBL(numlow))* deninv); 

   _ntl_limb_t r21 = (numhigh << NTL_ZZ_NBITS) + numlow - denom*q21;

   r21 = sp_CorrectDeficitQuo(q21, r21, _ntl_signed_limb_t(denom));
   r21 = sp_CorrectExcessQuo (q21, r21, _ntl_signed_limb_t(denom));
 
   return q21;
}


_ntl_limb_t
_ntl_mpn_divmod_1 (_ntl_limb_t *q, const _ntl_limb_t *a, long sa, _ntl_limb_t d)
{
   double dinv = 1.0/DBL(d);
   _ntl_limb_t carry = 0;

   if (a[sa-1] < d) {
      carry = a[sa-1];
      q[sa-1] = 0;
      sa--;
   }

   for (long i = sa-1; i >= 0; i--) {
      _ntl_div21p(carry, a[i], d, dinv, q[i]);
   }

   return carry;
}

_ntl_limb_t 
_ntl_mpn_mod_1 (const _ntl_limb_t *a, long sa, _ntl_limb_t d)
{
   double dinv = 1.0/DBL(d);
   _ntl_limb_t carry = 0;

   if (a[sa-1] < d) {
      carry = a[sa-1];
      sa--;
   }

   for (long i = sa-1; i >= 0; i--) {
      _ntl_rem21p(carry, a[i], d, dinv);
   }

   return carry;
}

// NOTE: no aliasing allowed (more recent versions of GMP allow a==r)
void 
_ntl_mpn_tdiv_qr (_ntl_limb_t *q, _ntl_limb_t *r,long  /* qxn */, 
                  const _ntl_limb_t *a, long sa, const _ntl_limb_t *d, long sd)
{
   if (sd == 1) {
      r[0] = _ntl_mpn_divmod_1(q, a, sa, d[0]);
      return;
   }

   // compute dhi = high order NTL_ZZ_NBITS of (d[sd-1], ..., d[0])
   _ntl_limb_t d1 = d[sd-1];
   _ntl_limb_t d0 = d[sd-2];
   long dbits = COUNT_BITS(d1); 
   _ntl_limb_t dhi = (d1 << (NTL_ZZ_NBITS-dbits)) | (d0 >> dbits);


   double dhi_inv = 1.0/DBL(dhi);

   // initialize buffer to (0, a[sa-1], ..., a[sa-sd+1])
   for (long j = 0; j < sd-1; j++)
      r[j] = a[sa-sd+1+j];
   r[sd-1] = 0;

   for (long i = sa-sd; i >= 0; i--) {
      // compute (rhi1, rhi0) = high order 2*NTL_ZZ_NBITS of buffer
      _ntl_limb_t r2, r1, r0;
      r2 = r[sd-1];
      r1 = r[sd-2];
      r0 = sd > 2 ? r[sd-3] : a[i];

      _ntl_limb_t rhi1 = (r2 << (NTL_ZZ_NBITS-dbits)) | (r1 >> dbits);
      _ntl_limb_t rhi0 = CLIP(r1 << (NTL_ZZ_NBITS-dbits)) | (r0 >> dbits);

      // compute estimate for quotient digit: it may be too large by at most  2
      _ntl_limb_t qdigit;
      if (rhi1 >= dhi) 
         qdigit = NTL_ZZ_RADIX-1;
      else
         qdigit = _ntl_quo21p(rhi1, rhi0, dhi, dhi_inv);

      _ntl_limb_t carry = _ntl_mpn_shift_submul_1(r, a[i], d, sd, qdigit);
      while (carry) {
         // loop body executes at most twice
         carry += _ntl_mpn_add_n(r, r, d, sd);
         qdigit--;
      }

      q[i] = qdigit;
   }
}


#endif


/* A bigint is represented as two long's, ALLOC and SIZE, followed by a 
 * vector DATA of _ntl_limb_t's.  
 * 
 * ALLOC is of the form
 *    (alloc << 2) | continue_flag | frozen_flag
 * where 
 *    - alloc is the number of allocated _ntl_limb_t's,
 *    - continue flag is either 2 or 0,
 *    - frozen_flag is either 1 or 0.
 * If frozen_flag is set, then the space for this bigint is *not*
 * managed by the _ntl_gsetlength and _ntl_gfree routines,
 * but are instead managed by the vec_ZZ_p and ZZVec routines.
 * The continue_flag is only set when the frozen_flag is set.
 * 
 * SIZE is the number of _ntl_limb_t's actually
 * used by the bigint, with the sign of SIZE having
 * the sign of the bigint.
 * Note that the zero bigint is represented as SIZE=0.
 * 
 * Bigint's are accessed through a handle, which is pointer to void.
 * A null handle logically represents the bigint zero.
 * This is done so that the interface presented to higher level
 * routines is essentially the same as that of NTL's traditional
 * long integer package.
 * 
 * The components ALLOC, SIZE, and DATA are all accessed through
 * macros using pointer casts.  While all of may seem a bit dirty, 
 * it should be quite portable: objects are never referenced
 * through pointers of different types, and no alignmement
 * problems should arise.
 * 
 * DIRT: This rule is broken in the file g_lip.h: the inline definition
 * of _ntl_gmaxalloc in that file has the definition of ALLOC pasted in.
 * 
 * Actually, _ntl_limb_t is usually the type unsigned long.
 * However, on some 64-bit platforms, the type long is only 32 bits,
 * and gmp makes _ntl_limb_t unsigned long long in this case.
 * This is fairly rare, as the industry standard for Unix is to
 * have 64-bit longs on 64-bit machines.
 */ 

/* DIRT: STORAGE computes the number of bytes to allocate for a bigint
 * of maximal SIZE len.  This should be computed so that one
 * can store several such bigints in a contiguous array
 * of memory without breaking any alignment requirements.
 * Currently, it is assumed (and explicitly checked in the NTL installation
 * script) that sizeof(_ntl_limb_t) is either sizeof(long) or
 * 2*sizeof(long), and therfore, nothing special needs to
 * be done to enfoce alignment requirements.  If this assumption
 * should change, then the storage layout for bigints must be
 * re-designed.   
 */


static
inline long& ALLOC(_ntl_gbigint p) 
   { return p->alloc_; }

static
inline long& SIZE(_ntl_gbigint p) 
   { return p->size_; }

static
inline _ntl_limb_t * DATA(_ntl_gbigint p) 
   { return (_ntl_limb_t *) (p+1); }

static
inline long STORAGE(long len)
   { return ((long)(sizeof(_ntl_gbigint_body) + (len)*sizeof(_ntl_limb_t))); }

static
inline long MustAlloc(_ntl_gbigint c, long len)  
   { return (!(c) || (ALLOC(c) >> 2) < (len)); }


static
inline void GET_SIZE_NEG(long& sz, long& neg, _ntl_gbigint p)
{ 
   long s; 
   s = SIZE(p); 
   if (s < 0) {
      sz = -s;
      neg = 1;
   }
   else {
      sz = s;
      neg = 0;
   }
}

static
inline void STRIP(long& sz, const _ntl_limb_t *p)
{
   long n = sz;
   while (n > 0 && p[n-1] == 0) n--;
   sz = n;
}

static
inline long ZEROP(_ntl_gbigint p)
{
   return !p || !SIZE(p);
}

static
inline long ONEP(_ntl_gbigint p)
{
   return p && SIZE(p) == 1 && DATA(p)[0] == 1;
}

static
inline void SWAP_BIGINT(_ntl_gbigint& a, _ntl_gbigint& b)
{
   _ntl_gbigint t;
   t = a;
   a = b;
   b = t;
}

static
inline void SWAP_LONG(long& a, long& b)
{
   long t;
   t = a;
   a = b;
   b = t;
}

static
inline void SWAP_LIMB_PTR(_ntl_limb_t_ptr& a, _ntl_limb_t_ptr& b)
{
   _ntl_limb_t_ptr t;
   t = a;
   a = b;
   b = t;
}


static void DUMP(_ntl_gbigint a)
{
   if (ZEROP(a)) 
      cerr << "[]";
   else {
      long sa = SIZE(a);
      if (sa < 0) { cerr << "-"; sa = -sa; }
      cerr << "[ ";
      for (long i = 0; i < sa; i++)
         cerr << DATA(a)[i] << " ";
      cerr << "]";
   }
   cerr << "\n";
}




#if (defined(NTL_CRT_ALTCODE) || defined(NTL_CRT_ALTCODE_SMALL))

#if (defined(NTL_VIABLE_LL) && NTL_NAIL_BITS == 0)

// alternative CRT code is requested and viable
// we do not attempt to implement this with non-empty nails,
// as it is not a huge win
#define NTL_TBL_CRT

#else


// raise an error if running the wizard
#ifdef NTL_WIZARD_HACK
#error "NTL_CRT_ALTCODE/NTL_CRT_ALTCODE_SMALL not viable"
#endif


#endif

#endif


#if (defined(NTL_TBL_REM) && !defined(NTL_VIABLE_LL))
#undef NTL_TBL_REM
// raise an error if running the wizard
#ifdef NTL_WIZARD_HACK
#error "NTL_TBL_REM not viable"
#endif
#endif




class _ntl_gbigint_watcher {
public:
   _ntl_gbigint *watched;

   explicit
   _ntl_gbigint_watcher(_ntl_gbigint *_watched) : watched(_watched) {}

   ~_ntl_gbigint_watcher() 
   {
      if (*watched && (ALLOC(*watched) >> 2) > NTL_RELEASE_THRESH) {
         _ntl_gfree(*watched);
         *watched = 0;
      }
   }
};



class _ntl_gbigint_deleter {
public:
   static void apply(_ntl_gbigint p) { _ntl_gfree(p); }
};

typedef WrappedPtr<_ntl_gbigint_body, _ntl_gbigint_deleter> _ntl_gbigint_wrapped;

static inline void
_ntl_swap(_ntl_gbigint_wrapped& p, _ntl_gbigint_wrapped& q)
{
   p.swap(q);
}



// GRegisters are used for local "scratch" variables.

// NOTE: the first implementation of GRegister below wraps a bigint in a class
// whose destructor ensures that its space is reclaimed at program/thread termination.
// It really only is necesary in a multi-threading environment, but it doesn't
// seem to incurr significant cost.

// The second implementation does not do this wrapping, and so should not be
// used in a multi-threading environment.

// Both versions use a local "watcher" variable, which does the following:
// when the local scope closes (e.g., the function returns), the space
// for the bigint is freed *unless* it is fairly small.  This balanced
// approach leads significantly faster performance, while not holding
// to too many resouces.

// The third version releases local memory every time.  It can be significantly
// slower.

// The fourth version --- which was the original strategy --- never releases
// memory.  It can be faster, but can become a memory hog.

// All of this code is overly complicated, due to the fact that I'm "retrofitting"
// this logic onto what was originally pure-C code.


#define GRegister(x) NTL_TLS_LOCAL(_ntl_gbigint_wrapped, x); _ntl_gbigint_watcher _WATCHER__ ## x(&x)

//#define GRegister(x) NTL_THREAD_LOCAL static _ntl_gbigint x(0); _ntl_gbigint_watcher _WATCHER__ ## x(&x)

// #define GRegister(x) _ntl_gbigint_wrapped x(0);

// #define GRegister(x) static _ntl_gbigint x = 0 





#define STORAGE_OVF(len) NTL_OVERFLOW(len, sizeof(_ntl_limb_t), 2*sizeof(long))



#ifndef NTL_GMP_LIP
// legacy function

long _ntl_gdigit(_ntl_gbigint a, long i)
{
   if (ZEROP(a) || i < 0) return 0;
   long sa = SIZE(a);
   if (sa < 0) sa = -sa;
   if (i >= sa) return 0;
   return DATA(a)[i];
}


#endif


long _ntl_gvalidate(_ntl_gbigint a)
{
   if (ZEROP(a)) return 1;
   long sa = SIZE(a);
   if (sa < 0) sa = -sa;

   _ntl_limb_t *adata = DATA(a);
   for (long i = 0; i < sa; i++)
      if (XCLIP(adata[i])) return 0;

   if (adata[sa-1] == 0) return 0;
   return 1;
}


/* ForceNormal ensures a normalized bigint */

static 
void ForceNormal(_ntl_gbigint x)
{
   long sx, xneg;
   _ntl_limb_t *xdata;

   if (!x) return;
   GET_SIZE_NEG(sx, xneg, x);
   xdata = DATA(x);
   STRIP(sx, xdata);
   if (xneg) sx = -sx;
   SIZE(x) = sx;
}


#define MIN_SETL	(4)
   /* _ntl_gsetlength allocates a multiple of MIN_SETL digits */



void _ntl_gsetlength(_ntl_gbigint *v, long len)
{
   _ntl_gbigint x = *v;

   if (len < 0)
      LogicError("negative size allocation in _ntl_zgetlength");

   if (NTL_OVERFLOW(len, NTL_ZZ_NBITS, 0))
      ResourceError("size too big in _ntl_gsetlength");

#ifdef NTL_SMALL_MP_SIZE_T
   /* this makes sure that numbers don't get too big for GMP */
   if (len >= (1L << (NTL_BITS_PER_INT-4)))
      ResourceError("size too big for GMP");
#endif


   if (x) {
      long oldlen = ALLOC(x);
      long fixed = oldlen & 1;
      oldlen = oldlen >> 2;

      if (fixed) {
         if (len > oldlen) 
            LogicError("internal error: can't grow this _ntl_gbigint");
         else
            return;
      }

      if (len <= oldlen) return;

      len++;  /* always allocate at least one more than requested */

      oldlen = _ntl_vec_grow(oldlen);
      if (len < oldlen)
         len = oldlen;

      /* round up to multiple of MIN_SETL */
      len = ((len+(MIN_SETL-1))/MIN_SETL)*MIN_SETL; 

      /* test len again */
      if (NTL_OVERFLOW(len, NTL_ZZ_NBITS, 0))
         ResourceError("size too big in _ntl_gsetlength");

      if (STORAGE_OVF(len))
         ResourceError("reallocation failed in _ntl_gsetlength");

      if (!(x = (_ntl_gbigint)NTL_SNS_REALLOC((void *) x, 1, STORAGE(len), 0))) {
         MemoryError();
      }
      ALLOC(x) = len << 2;
   }
   else {
      len++;  /* as above, always allocate one more than explicitly reqested */
      len = ((len+(MIN_SETL-1))/MIN_SETL)*MIN_SETL; 

      /* test len again */
      if (NTL_OVERFLOW(len, NTL_ZZ_NBITS, 0))
         ResourceError("size too big in _ntl_gsetlength");

      if (STORAGE_OVF(len))
         ResourceError("reallocation failed in _ntl_gsetlength");

      if (!(x = (_ntl_gbigint)NTL_SNS_MALLOC(1, STORAGE(len), 0))) {
         MemoryError();
      }
      ALLOC(x) = len << 2;
      SIZE(x) = 0;
   }

   *v = x;
}

void _ntl_gfree(_ntl_gbigint x)
{


   if (!x)
      return;

   if (ALLOC(x) & 1)
      TerminalError("Internal error: can't free this _ntl_gbigint");

   free((void*) x);
   return;
}

void
_ntl_gswap(_ntl_gbigint *a, _ntl_gbigint *b)
{
   if ((*a && (ALLOC(*a) & 1)) || (*b && (ALLOC(*b) & 1))) {
      // one of the inputs points to an bigint that is 
      // "pinned down" in memory, so we have to swap the data,
      // not just the pointers

      GRegister(t);
      long sz_a, sz_b, sz;

      sz_a = _ntl_gsize(*a); 
      sz_b = _ntl_gsize(*b); 
      sz = (sz_a > sz_b) ? sz_a : sz_b;

      _ntl_gsetlength(a, sz);
      _ntl_gsetlength(b, sz);

      // EXCEPTIONS: all of the above ensures that swap provides strong ES

      _ntl_gcopy(*a, &t);
      _ntl_gcopy(*b, a);
      _ntl_gcopy(t, b);
      return;
   }

   SWAP_BIGINT(*a, *b);
}


void _ntl_gcopy(_ntl_gbigint a, _ntl_gbigint *bb)
{
   _ntl_gbigint b;
   long sa, abs_sa, i;
   _ntl_limb_t *adata, *bdata;

   b = *bb;

   if (!a || (sa = SIZE(a)) == 0) {
      if (b) SIZE(b) = 0;
   }
   else {
      if (a != b) {
         if (sa >= 0)
            abs_sa = sa;
         else
            abs_sa = -sa;

         if (MustAlloc(b, abs_sa)) {
            _ntl_gsetlength(&b, abs_sa);
            *bb = b;
         }

         adata = DATA(a);
         bdata = DATA(b);

         for (i = 0; i < abs_sa; i++)
            bdata[i] = adata[i];

         SIZE(b) = sa;
      }
   }
}

void _ntl_glimbs_set(const _ntl_limb_t *p, long n, _ntl_gbigint *x)
{
   if (n < 0) LogicError("_ntl_glimbs_set: negative size");
   if (n > 0 && !p) LogicError("_ntl_glimbs_set: unexpected NULL pointer");

   STRIP(n, p);
   if (n == 0) {
      _ntl_gzero(x);
      return;
   }

   if (MustAlloc(*x, n)) _ntl_gsetlength(x, n);
   _ntl_limb_t *xdata = DATA(*x);
   for (long i = 0; i < n; i++) xdata[i] = p[i];
   SIZE(*x) = n;

}




void _ntl_gzero(_ntl_gbigint *aa) 
{
   _ntl_gbigint a = *aa;

   if (a) SIZE(a) = 0;
}

void _ntl_gone(_ntl_gbigint *aa)
{
   _ntl_gbigint a = *aa;
   if (!a) {
      _ntl_gsetlength(&a, 1);
      *aa = a;
   }

   SIZE(a) = 1;
   DATA(a)[0] = 1;
}

long _ntl_godd(_ntl_gbigint a)
{
   if (ZEROP(a)) 
      return 0;
   else
      return DATA(a)[0]&1;
}

long _ntl_gbit(_ntl_gbigint a, long p)
{
   long bl;
   long sa;
   _ntl_limb_t wh;

   if (p < 0 || !a) return 0;

   bl = p/NTL_ZZ_NBITS;
   wh = ((_ntl_limb_t) 1) << (p - NTL_ZZ_NBITS*bl);

   sa = SIZE(a);
   if (sa < 0) sa = -sa;

   if (sa <= bl) return 0;
   if (DATA(a)[bl] & wh) return 1;
   return 0;
}

void _ntl_glowbits(_ntl_gbigint a, long b, _ntl_gbigint *cc)
{
   _ntl_gbigint c;

   long bl;
   long wh;
   long sa;
   long i;
   _ntl_limb_t *adata, *cdata;

   if (ZEROP(a) || (b<=0)) {
      _ntl_gzero(cc);
      return;
   }

   bl = b/NTL_ZZ_NBITS;
   wh = b - NTL_ZZ_NBITS*bl;
   if (wh != 0) 
      bl++;
   else
      wh = NTL_ZZ_NBITS;

   sa = SIZE(a);
   if (sa < 0) sa = -sa;

   if (sa < bl) {
      _ntl_gcopy(a,cc);
      _ntl_gabs(cc);
      return;
   }

   c = *cc;

   /* a won't move if c aliases a */
   _ntl_gsetlength(&c, bl);
   *cc = c;

   adata = DATA(a);
   cdata = DATA(c);

   for (i = 0; i < bl-1; i++)
      cdata[i] = adata[i];

   if (wh == NTL_ZZ_NBITS)
      cdata[bl-1] = adata[bl-1];
   else
      cdata[bl-1] = adata[bl-1] & ((((_ntl_limb_t) 1) << wh) - ((_ntl_limb_t) 1));

   STRIP(bl, cdata);
   SIZE(c) = bl; 
}

long _ntl_gslowbits(_ntl_gbigint a, long p)
{
   GRegister(x);

   if (p > NTL_BITS_PER_LONG)
      p = NTL_BITS_PER_LONG;

   _ntl_glowbits(a, p, &x);

   return _ntl_gtoint(x);
}

long _ntl_gsetbit(_ntl_gbigint *a, long b)
{
   long bl;
   long sa, aneg;
   long i;
   _ntl_limb_t wh, *adata, tmp;

   if (b<0) LogicError("_ntl_gsetbit: negative index");

   bl = (b/NTL_ZZ_NBITS);
   wh = ((_ntl_limb_t) 1) << (b - NTL_ZZ_NBITS*bl);

   if (!*a) 
      sa = aneg = 0;
   else
      GET_SIZE_NEG(sa, aneg, *a);

   if (sa > bl) {
      adata = DATA(*a);
      tmp = adata[bl] & wh;
      adata[bl] |= wh;
      if (tmp) return 1;
      return 0;
   } 
   else {
      _ntl_gsetlength(a, bl+1);
      adata = DATA(*a);
      for (i = sa; i < bl; i++)
         adata[i] = 0;
      adata[bl] = wh;

      sa = bl+1;
      if (aneg) sa = -sa;
      SIZE(*a) = sa;
      return 0;
   }
}

long _ntl_gswitchbit(_ntl_gbigint *a, long b)
{
   long bl;
   long sa, aneg;
   long i;
   _ntl_limb_t wh, *adata, tmp;

   if (b<0) LogicError("_ntl_gswitchbit: negative index");

   bl = (b/NTL_ZZ_NBITS);
   wh = ((_ntl_limb_t) 1) << (b - NTL_ZZ_NBITS*bl);

   if (!*a) 
      sa = aneg = 0;
   else
      GET_SIZE_NEG(sa, aneg, *a);

   if (sa > bl) {
      adata = DATA(*a);
      tmp = adata[bl] & wh;
      adata[bl] ^= wh;

      if (bl == sa-1) {
         STRIP(sa, adata);
         if (aneg) sa = -sa;
         SIZE(*a) = sa;
      }

      if (tmp) return 1;
      return 0;
   } 
   else {
      _ntl_gsetlength(a, bl+1);
      adata = DATA(*a);
      for (i = sa; i < bl; i++)
         adata[i] = 0;
      adata[bl] = wh;

      sa = bl+1;
      if (aneg) sa = -sa;
      SIZE(*a) = sa;
      return 0;
   }
}

long
_ntl_gweights(
	long aa
	)
{
	unsigned long a;
	long res = 0;
	if (aa < 0) 
		a = -((unsigned long) aa);
	else
		a = aa;
   
	while (a) {
		if (a & 1) res ++;
		a >>= 1;
	}
	return (res);
}

static long
gweights_mp_limb(
	_ntl_limb_t a
	)
{
	long res = 0;
   
	while (a) {
		if (a & 1) res ++;
		a >>= 1;
	}
	return (res);
}

long
_ntl_gweight(
        _ntl_gbigint a
        )
{
	long i;
	long sa;
	_ntl_limb_t *adata;
	long res;

	if (!a) return (0);

	sa = SIZE(a);
	if (sa < 0) sa = -sa;
	adata = DATA(a);

	res = 0;
	for (i = 0; i < sa; i++)
		res += gweights_mp_limb(adata[i]);

	return (res);
}


long _ntl_g2log(_ntl_gbigint a)
{
   long la;
   long t;

   if (!a) return 0;
   la = SIZE(a);
   if (la == 0) return 0;
   if (la < 0) la = -la;
   t = COUNT_BITS(DATA(a)[la-1]);
   return NTL_ZZ_NBITS*(la - 1) + t;
}



long _ntl_gmakeodd(_ntl_gbigint *nn)
{
   _ntl_gbigint n = *nn;
   long shift;
   _ntl_limb_t *ndata;
   _ntl_limb_t i;

   if (ZEROP(n))
      return (0);

   shift = 0;
   ndata = DATA(n);

   while (ndata[shift] == 0)
      shift++;

   i = ndata[shift];

   shift = NTL_ZZ_NBITS * shift;

   while ((i & 1) == 0) {
      shift++;
      i >>= 1;
   }
   _ntl_grshift(n, shift, &n);
   return shift;
}


long _ntl_gnumtwos(_ntl_gbigint n)
{
   long shift;
   _ntl_limb_t *ndata;
   _ntl_limb_t i;

   if (ZEROP(n))
      return (0);

   shift = 0;
   ndata = DATA(n);

   while (ndata[shift] == 0)
      shift++;

   i = ndata[shift];

   shift = NTL_ZZ_NBITS * shift;

   while ((i & 1) == 0) {
      shift++;
      i >>= 1;
   }

   return shift;
}


void _ntl_gand(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *cc)
{
   _ntl_gbigint c;
   long sa;
   long sb;
   long sm;
   long i;
   long a_alias, b_alias;
   _ntl_limb_t *adata, *bdata, *cdata;

   if (ZEROP(a) || ZEROP(b)) {
      _ntl_gzero(cc);
      return;
   }

   c = *cc;
   a_alias = (a == c);
   b_alias = (b == c);

   sa = SIZE(a);
   if (sa < 0) sa = -sa;

   sb = SIZE(b);
   if (sb < 0) sb = -sb;

   sm = (sa > sb ? sb : sa);

   _ntl_gsetlength(&c, sm);
   if (a_alias) a = c;
   if (b_alias) b = c;
   *cc = c;

   adata = DATA(a);
   bdata = DATA(b);
   cdata = DATA(c);

   for (i = 0; i < sm; i++)
      cdata[i] = adata[i] & bdata[i];

   STRIP(sm, cdata);
   SIZE(c) = sm;
}


void _ntl_gxor(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *cc)
{
   _ntl_gbigint c;
   long sa;
   long sb;
   long sm;
   long la;
   long i;
   long a_alias, b_alias;
   _ntl_limb_t *adata, *bdata, *cdata;

   if (ZEROP(a)) {
      _ntl_gcopy(b,cc);
      _ntl_gabs(cc);
      return;
   }

   if (ZEROP(b)) {
      _ntl_gcopy(a,cc);
      _ntl_gabs(cc);
      return;
   }

   c = *cc;
   a_alias = (a == c);
   b_alias = (b == c);

   sa = SIZE(a);
   if (sa < 0) sa = -sa;

   sb = SIZE(b);
   if (sb < 0) sb = -sb;

   if (sa > sb) {
      la = sa;
      sm = sb;
   } 
   else {
      la = sb;
      sm = sa;
   }

   _ntl_gsetlength(&c, la);
   if (a_alias) a = c;
   if (b_alias) b = c;
   *cc = c;

   adata = DATA(a);
   bdata = DATA(b);
   cdata = DATA(c);

   for (i = 0; i < sm; i ++)
      cdata[i] = adata[i] ^ bdata[i];

   if (sa > sb)
      for (;i < la; i++) cdata[i] = adata[i];
   else
      for (;i < la; i++) cdata[i] = bdata[i];

   STRIP(la, cdata);
   SIZE(c) = la;
}


void _ntl_gor(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *cc)
{
   _ntl_gbigint c;
   long sa;
   long sb;
   long sm;
   long la;
   long i;
   long a_alias, b_alias;
   _ntl_limb_t *adata, *bdata, *cdata;

   if (ZEROP(a)) {
      _ntl_gcopy(b,cc);
      _ntl_gabs(cc);
      return;
   }

   if (ZEROP(b)) {
      _ntl_gcopy(a,cc);
      _ntl_gabs(cc);
      return;
   }

   c = *cc;
   a_alias = (a == c);
   b_alias = (b == c);

   sa = SIZE(a);
   if (sa < 0) sa = -sa;

   sb = SIZE(b);
   if (sb < 0) sb = -sb;

   if (sa > sb) {
      la = sa;
      sm = sb;
   } 
   else {
      la = sb;
      sm = sa;
   }

   _ntl_gsetlength(&c, la);
   if (a_alias) a = c;
   if (b_alias) b = c;
   *cc = c;

   adata = DATA(a);
   bdata = DATA(b);
   cdata = DATA(c);

   for (i = 0; i < sm; i ++)
      cdata[i] = adata[i] | bdata[i];

   if (sa > sb)
      for (;i < la; i++) cdata[i] = adata[i];
   else
      for (;i < la; i++) cdata[i] = bdata[i];

   STRIP(la, cdata);
   SIZE(c) = la;
}


void _ntl_gnegate(_ntl_gbigint *aa)
{
   _ntl_gbigint a = *aa;
   if (a) SIZE(a) = -SIZE(a);
}



#if (NTL_ZZ_NBITS >= NTL_BITS_PER_LONG)

void _ntl_gintoz(long d, _ntl_gbigint *aa)
{
   _ntl_gbigint a = *aa;

   if (d == 0) {
      if (a) SIZE(a) = 0;
   }
   else {
      if (!a) {
         _ntl_gsetlength(&a, 1);
         *aa = a;
      }
   
      SIZE(a) = d < 0 ? -1 : 1;
      DATA(a)[0] = ABS(d);
   }
}

#else


void _ntl_gintoz(long d, _ntl_gbigint *aa)
{
   long sa, i;
   _ntl_limb_t d1, d2;

   _ntl_gbigint a = *aa;

   if (d == 0) {
      if (a) SIZE(a) = 0;
      return;
   }

   d1 = ABS(d);

   sa = 0;
   d2 = d1;
   do {
      d2 >>= NTL_ZZ_NBITS;
      sa++;
   }
   while (d2);

   if (MustAlloc(a, sa)) {
      _ntl_gsetlength(&a, sa);
      *aa = a;
   }
 
   _ntl_limb_t *adata = DATA(a);

   for (i = 0; i < sa; i++) {
      adata[i] = CLIP(d1);
      d1 >>= NTL_ZZ_NBITS;
   }

   if (d < 0) sa = -sa;
   SIZE(a) = sa;
}

#endif




#if (NTL_ZZ_NBITS >= NTL_BITS_PER_LONG)

void _ntl_guintoz(unsigned long d, _ntl_gbigint *aa)
{
   _ntl_gbigint a = *aa;

   if (d == 0) {
      if (a) SIZE(a) = 0;
   }
   else {
      if (!a) {
         _ntl_gsetlength(&a, 1);
         *aa = a;
      }
   
      SIZE(a) = 1;
      DATA(a)[0] = d;
   }
}

#else


void _ntl_guintoz(unsigned long d, _ntl_gbigint *aa)
{
   long sa, i;
   _ntl_limb_t d1, d2;

   _ntl_gbigint a = *aa;

   if (d == 0) {
      if (a) SIZE(a) = 0;
      return;
   }

   d1 = d;

   sa = 0;
   d2 = d1;
   do {
      d2 >>= NTL_ZZ_NBITS;
      sa++;
   }
   while (d2);

   if (MustAlloc(a, sa)) {
      _ntl_gsetlength(&a, sa);
      *aa = a;
   }
 
   _ntl_limb_t *adata = DATA(a);

   for (i = 0; i < sa; i++) {
      adata[i] = CLIP(d1);
      d1 >>= NTL_ZZ_NBITS;
   }

   SIZE(a) = sa;
}


#endif



long _ntl_gtoint(_ntl_gbigint a)
{
   unsigned long res = _ntl_gtouint(a);
   return cast_signed(res);
}





#if (NTL_ZZ_NBITS >= NTL_BITS_PER_LONG)

unsigned long _ntl_gtouint(_ntl_gbigint a)
{
   if (ZEROP(a)) 
      return 0;

   if (SIZE(a) > 0) 
      return DATA(a)[0];

   return -DATA(a)[0];
}

#else

unsigned long _ntl_gtouint(_ntl_gbigint a)
{
   if (ZEROP(a))
      return 0;

   long sa, aneg;
   _ntl_limb_t *adata;
   GET_SIZE_NEG(sa, aneg, a);
   adata = DATA(a);

   unsigned long d = adata[0];
   long bits = NTL_ZZ_NBITS;
   long i = 1;
   while (bits < NTL_BITS_PER_LONG && i < sa) {
      d |= adata[i] << bits; 
      bits += NTL_ZZ_NBITS; 
      i++;
   }

   if (aneg) d = -d;
   return d;
}

#endif







long _ntl_gcompare(_ntl_gbigint a, _ntl_gbigint b)
{
   long sa, sb, cmp;
   _ntl_limb_t *adata, *bdata;

   if (!a) 
      sa = 0;
   else
      sa = SIZE(a);

   if (!b) 
      sb = 0;
   else
      sb = SIZE(b);

   if (sa != sb) {
      if (sa > sb)
         return 1;
      else
         return -1;
   }

   if (sa == 0)
      return 0;

   adata = DATA(a);
   bdata = DATA(b);

   if (sa > 0) {
      cmp = NTL_MPN(cmp)(adata, bdata, sa);

      if (cmp > 0)
         return 1;
      else if (cmp < 0) 
         return -1;
      else
         return 0;
   }
   else {
      cmp = NTL_MPN(cmp)(adata, bdata, -sa);

      if (cmp > 0)
         return -1;
      else if (cmp < 0) 
         return 1;
      else
         return 0;
   }
}


void _ntl_gabs(_ntl_gbigint *pa)
{
   _ntl_gbigint a = *pa;

   if (!a) return;
   if (SIZE(a) < 0) SIZE(a) = -SIZE(a);
}

long _ntl_gscompare(_ntl_gbigint a, long b)
{
   if (b == 0) {
      long sa;
      if (!a) return 0;
      sa = SIZE(a);
      if (sa > 0) return 1;
      if (sa == 0) return 0;
      return -1;
   }
   else {
      GRegister(B);
      _ntl_gintoz(b, &B);
      return _ntl_gcompare(a, B);
   }
}


void _ntl_glshift(_ntl_gbigint n, long k, _ntl_gbigint *rres)
{
   _ntl_gbigint res;
   _ntl_limb_t *ndata, *resdata, *resdata1;
   long limb_cnt, i, sn, nneg, sres;
   long n_alias;

   if (ZEROP(n)) {
      _ntl_gzero(rres);
      return;
   }

   res = *rres;
   n_alias = (n == res);

   if (!k) {
      if (!n_alias)
         _ntl_gcopy(n, rres);
      return;
   }

   if (k < 0) {
      if (k < -NTL_MAX_LONG) 
         _ntl_gzero(rres);
      else
         _ntl_grshift(n, -k, rres);
      return;
   }

   GET_SIZE_NEG(sn, nneg, n);

   limb_cnt = ((unsigned long) k) / NTL_ZZ_NBITS;
   k = ((unsigned long) k) % NTL_ZZ_NBITS;
   sres = sn + limb_cnt;
   if (k != 0) sres++;

   if (MustAlloc(res, sres)) {
      _ntl_gsetlength(&res, sres);
      if (n_alias) n = res;
      *rres = res;
   }

   ndata = DATA(n);
   resdata = DATA(res);
   resdata1 = resdata + limb_cnt;

   if (k != 0) {
      _ntl_limb_t t = NTL_MPN(lshift)(resdata1, ndata, sn, k);
      if (t != 0) 
         resdata[sres-1] = t;
      else
         sres--;
   }
   else {
      for (i = sn-1; i >= 0; i--)
         resdata1[i] = ndata[i];
   }

   for (i = 0; i < limb_cnt; i++)
      resdata[i] = 0;

   if (nneg) sres = -sres;
   SIZE(res) = sres;
}

void _ntl_grshift(_ntl_gbigint n, long k, _ntl_gbigint *rres)
{
   _ntl_gbigint res;
   _ntl_limb_t *ndata, *resdata, *ndata1;
   long limb_cnt, i, sn, nneg, sres;

   if (ZEROP(n)) {
      _ntl_gzero(rres);
      return;
   }

   if (!k) {
      if (n != *rres)
         _ntl_gcopy(n, rres);
      return;
   }

   if (k < 0) {
      if (k < -NTL_MAX_LONG) ResourceError("overflow in _ntl_glshift");
      _ntl_glshift(n, -k, rres);
      return;
   }

   GET_SIZE_NEG(sn, nneg, n);

   limb_cnt = ((unsigned long) k) / NTL_ZZ_NBITS;

   sres = sn - limb_cnt;

   if (sres <= 0) {
      _ntl_gzero(rres);
      return;
   }

   res = *rres;
   if (MustAlloc(res, sres)) {
      /* n won't move if res aliases n */
      _ntl_gsetlength(&res, sres);
      *rres = res;
   }

   ndata = DATA(n);
   resdata = DATA(res);
   ndata1 = ndata + limb_cnt;
   k = ((unsigned long) k) % NTL_ZZ_NBITS;

   if (k != 0) {
      NTL_MPN(rshift)(resdata, ndata1, sres, k);
      if (resdata[sres-1] == 0)
         sres--;
   }
   else {
      for (i = 0; i < sres; i++)
         resdata[i] = ndata1[i];
   }

   if (nneg) sres = -sres;
   SIZE(res) = sres;
}
   




void
_ntl_gadd(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *cc)
{
   long sa, aneg, sb, bneg, sc, cmp;
   _ntl_limb_t *adata, *bdata, *cdata, carry;
   _ntl_gbigint c;
   long a_alias, b_alias;

   if (ZEROP(a)) {
      _ntl_gcopy(b, cc);
      return;
   }

   if (ZEROP(b)) {
      _ntl_gcopy(a, cc);
      return;
   }

   GET_SIZE_NEG(sa, aneg, a);
   GET_SIZE_NEG(sb, bneg, b);

   if (sa < sb) {
      SWAP_BIGINT(a, b);
      SWAP_LONG(sa, sb);
      SWAP_LONG(aneg, bneg);
   }

   /* sa >= sb */

   c = *cc;
   a_alias = (a == c);
   b_alias = (b == c);

   if (aneg == bneg) {
      /* same sign => addition */

      sc = sa + 1;
      if (MustAlloc(c, sc)) {
         _ntl_gsetlength(&c, sc);
         if (a_alias) a = c; 
         if (b_alias) b = c;
         *cc = c;
      }

      adata = DATA(a);
      bdata = DATA(b);
      cdata = DATA(c);

      carry = NTL_MPN(add)(cdata, adata, sa, bdata, sb);
      if (carry) 
         cdata[sc-1] = carry;
      else
         sc--;

      if (aneg) sc = -sc;
      SIZE(c) = sc;
   }
   else {
      /* opposite sign => subtraction */

      sc = sa;
      if (MustAlloc(c, sc)) {
         _ntl_gsetlength(&c, sc);
         if (a_alias) a = c; 
         if (b_alias) b = c;
         *cc = c;
      }

      adata = DATA(a);
      bdata = DATA(b);
      cdata = DATA(c);

      if (sa > sb) 
         cmp = 1;
      else
         cmp = NTL_MPN(cmp)(adata, bdata, sa);

      if (cmp == 0) {
         SIZE(c) = 0;
      }
      else {
         if (cmp < 0) cmp = 0;
         if (cmp > 0) cmp = 1;
         /* abs(a) != abs(b) && (abs(a) > abs(b) <=> cmp) */

         if (cmp)
            NTL_MPN(sub)(cdata, adata, sa, bdata, sb);
         else
            NTL_MPN(sub)(cdata, bdata, sb, adata, sa); /* sa == sb */

         STRIP(sc, cdata);
         if (aneg == cmp) sc = -sc;
         SIZE(c) = sc;
      }
   }
}


void
_ntl_gsadd(_ntl_gbigint a, long b, _ntl_gbigint *cc)
{
   if (b == 0) {
      _ntl_gcopy(a, cc);
      return;
   }

   _ntl_limb_t abs_b = ABS(b);

   if (XCLIP(abs_b)) {
      GRegister(xb);
      _ntl_gintoz(b,&xb);
      _ntl_gadd(a, xb, cc);
      return;
   }

   long bneg = b < 0;


   if (ZEROP(a)) {
      if (!*cc) _ntl_gsetlength(cc, 1);
      SIZE(*cc) = 1 - 2*bneg;
      DATA(*cc)[0] = abs_b;
      return;
   }

   long sa, aneg;

   GET_SIZE_NEG(sa, aneg, a);

   if (aneg == bneg) {
      // signs equal: addition

      if (a == *cc) {
         // a aliases c

         _ntl_limb_t *adata = DATA(a);
         _ntl_limb_t carry = NTL_MPN(add_1)(adata, adata, sa, abs_b);

         if (carry) {
            if (MustAlloc(a, sa+1)) {
               _ntl_gsetlength(cc, sa+1);
               a = *cc;
               adata = DATA(a);
            } 
            adata[sa] = 1;
            sa++;
            if (aneg) sa = -sa;
            SIZE(a) = sa;
         }
      }
      else {
         // a and c do not alias
         if (MustAlloc(*cc, sa+1)) _ntl_gsetlength(cc, sa+1);
         _ntl_limb_t *adata = DATA(a);
         _ntl_limb_t *cdata = DATA(*cc);
         _ntl_limb_t carry = NTL_MPN(add_1)(cdata, adata, sa, abs_b);
         if (carry) {
            cdata[sa] = 1;
            sa++;
         }
         if (aneg) sa = -sa;
         SIZE(*cc) = sa;
      }
   }
   else {
      // opposite sign: subtraction

      if (sa == 1) {
         _ntl_limb_t abs_a = DATA(a)[0];
         if (abs_a == abs_b) 
            _ntl_gzero(cc);
         else if (abs_a > abs_b) {
            if (MustAlloc(*cc, 1)) _ntl_gsetlength(cc, 1);
            DATA(*cc)[0] = abs_a - abs_b;
            SIZE(*cc) = 1-2*aneg;
         }
         else {
            if (MustAlloc(*cc, 1)) _ntl_gsetlength(cc, 1);
            DATA(*cc)[0] = abs_b - abs_a;
            SIZE(*cc) = -1+2*aneg;
         }
      }
      else {
         if (MustAlloc(*cc, sa)) _ntl_gsetlength(cc, sa);
         _ntl_limb_t *adata = DATA(a);
         _ntl_limb_t *cdata = DATA(*cc);
         NTL_MPN(sub_1)(cdata, adata, sa, abs_b);
         if (cdata[sa-1] == 0) sa--;
         if (aneg) sa = -sa;
         SIZE(*cc) = sa;
      }
   }

}

void
_ntl_gssub(_ntl_gbigint a, long b, _ntl_gbigint *cc)
{
   if (b == 0) {
      _ntl_gcopy(a, cc);
      return;
   }

   _ntl_limb_t abs_b = ABS(b);

   if (XCLIP(abs_b)) {
      GRegister(xb);
      _ntl_gintoz(b,&xb);
      _ntl_gsub(a, xb, cc);
      return;
   }

   // the rest of this routine is precisely the same
   // as gsadd, except for the following line,
   // which has the sense of the test reversed
   long bneg = b >= 0;


   if (ZEROP(a)) {
      if (!*cc) _ntl_gsetlength(cc, 1);
      SIZE(*cc) = 1 - 2*bneg;
      DATA(*cc)[0] = abs_b;
      return;
   }

   long sa, aneg;

   GET_SIZE_NEG(sa, aneg, a);

   if (aneg == bneg) {
      // signs equal: addition

      if (a == *cc) {
         // a aliases c

         _ntl_limb_t *adata = DATA(a);
         _ntl_limb_t carry = NTL_MPN(add_1)(adata, adata, sa, abs_b);

         if (carry) {
            if (MustAlloc(a, sa+1)) {
               _ntl_gsetlength(cc, sa+1);
               a = *cc;
               adata = DATA(a);
            } 
            adata[sa] = 1;
            sa++;
            if (aneg) sa = -sa;
            SIZE(a) = sa;
         }
      }
      else {
         // a and c do not alias
         if (MustAlloc(*cc, sa+1)) _ntl_gsetlength(cc, sa+1);
         _ntl_limb_t *adata = DATA(a);
         _ntl_limb_t *cdata = DATA(*cc);
         _ntl_limb_t carry = NTL_MPN(add_1)(cdata, adata, sa, abs_b);
         if (carry) {
            cdata[sa] = 1;
            sa++;
         }
         if (aneg) sa = -sa;
         SIZE(*cc) = sa;
      }
   }
   else {
      // opposite sign: subtraction

      if (sa == 1) {
         _ntl_limb_t abs_a = DATA(a)[0];
         if (abs_a == abs_b) 
            _ntl_gzero(cc);
         else if (abs_a > abs_b) {
            if (MustAlloc(*cc, 1)) _ntl_gsetlength(cc, 1);
            DATA(*cc)[0] = abs_a - abs_b;
            SIZE(*cc) = 1-2*aneg;
         }
         else {
            if (MustAlloc(*cc, 1)) _ntl_gsetlength(cc, 1);
            DATA(*cc)[0] = abs_b - abs_a;
            SIZE(*cc) = -1+2*aneg;
         }
      }
      else {
         if (MustAlloc(*cc, sa)) _ntl_gsetlength(cc, sa);
         _ntl_limb_t *adata = DATA(a);
         _ntl_limb_t *cdata = DATA(*cc);
         NTL_MPN(sub_1)(cdata, adata, sa, abs_b);
         if (cdata[sa-1] == 0) sa--;
         if (aneg) sa = -sa;
         SIZE(*cc) = sa;
      }
   }

}



void
_ntl_gsub(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *cc)
{
   long sa, aneg, sb, bneg, sc, cmp, rev;
   _ntl_limb_t *adata, *bdata, *cdata, carry;
   _ntl_gbigint c;
   long a_alias, b_alias;

   if (ZEROP(a)) {
      _ntl_gcopy(b, cc);
      c = *cc;
      if (c) SIZE(c) = -SIZE(c); 
      return;
   }

   if (ZEROP(b)) {
      _ntl_gcopy(a, cc);
      return;
   }

   GET_SIZE_NEG(sa, aneg, a);
   GET_SIZE_NEG(sb, bneg, b);

   if (sa < sb) {
      SWAP_BIGINT(a, b);
      SWAP_LONG(sa, sb);
      SWAP_LONG(aneg, bneg);
      rev = 1;
   }
   else 
      rev = 0;

   /* sa >= sb */

   c = *cc;
   a_alias = (a == c);
   b_alias = (b == c);

   if (aneg != bneg) {
      /* opposite sign => addition */

      sc = sa + 1;
      if (MustAlloc(c, sc)) {
         _ntl_gsetlength(&c, sc);
         if (a_alias) a = c; 
         if (b_alias) b = c;
         *cc = c;
      }

      adata = DATA(a);
      bdata = DATA(b);
      cdata = DATA(c);

      carry = NTL_MPN(add)(cdata, adata, sa, bdata, sb);
      if (carry) 
         cdata[sc-1] = carry;
      else
         sc--;

      if (aneg ^ rev) sc = -sc;
      SIZE(c) = sc;
   }
   else {
      /* same sign => subtraction */

      sc = sa;
      if (MustAlloc(c, sc)) {
         _ntl_gsetlength(&c, sc);
         if (a_alias) a = c; 
         if (b_alias) b = c;
         *cc = c;
      }

      adata = DATA(a);
      bdata = DATA(b);
      cdata = DATA(c);

      if (sa > sb) 
         cmp = 1;
      else
         cmp = NTL_MPN(cmp)(adata, bdata, sa);

      if (cmp == 0) {
         SIZE(c) = 0;
      }
      else {
         if (cmp < 0) cmp = 0;
         if (cmp > 0) cmp = 1;
         /* abs(a) != abs(b) && (abs(a) > abs(b) <=> cmp) */

         if (cmp)
            NTL_MPN(sub)(cdata, adata, sa, bdata, sb);
         else
            NTL_MPN(sub)(cdata, bdata, sb, adata, sa); /* sa == sb */

         STRIP(sc, cdata);
         if ((aneg == cmp) ^ rev) sc = -sc;
         SIZE(c) = sc;
      }
   }
}

void
_ntl_gsubpos(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *cc)
{
   long sa, sb, sc;
   _ntl_limb_t *adata, *bdata, *cdata;
   _ntl_gbigint c;
   long a_alias, b_alias;

   if (ZEROP(a)) {
      _ntl_gzero(cc);
      return;
   }

   if (ZEROP(b)) {
      _ntl_gcopy(a, cc);
      return;
   }

   sa = SIZE(a);
   sb = SIZE(b);

   c = *cc;
   a_alias = (a == c);
   b_alias = (b == c);

   sc = sa;
   if (MustAlloc(c, sc)) {
      _ntl_gsetlength(&c, sc);
      if (a_alias) a = c; 
      if (b_alias) b = c;
      *cc = c;
   }

   adata = DATA(a);
   bdata = DATA(b);
   cdata = DATA(c);

   NTL_MPN(sub)(cdata, adata, sa, bdata, sb);

   STRIP(sc, cdata);
   SIZE(c) = sc;
}

#if 1

// This version is faster for small inputs.
// It avoids some overheads incurred only when dealing with
// aliased outputs.
// It also makes direct calls to lower-level mpn functions
// for smaller inputs (and for one limb inputs, it avoids
// function calls altogether (usually)).

// Speedup: 2.5x 1 limb
//          1.4x 2 limb
//          1.3x 3 limb

static inline _ntl_limb_t
base_mul (_ntl_limb_t* rp, const _ntl_limb_t* up, long un, const _ntl_limb_t* vp, long vn)
{
  rp[un] = NTL_MPN(mul_1) (rp, up, un, vp[0]);

  while (--vn >= 1)
    {
      rp += 1, vp += 1;
      rp[un] = NTL_MPN(addmul_1) (rp, up, un, vp[0]);
    }
  return rp[un];
}

void _ntl_gmul(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *cc)
{
   long sa, aneg, sb, bneg, alias, sc;
   _ntl_limb_t *adata, *bdata, *cdata, msl;
   _ntl_gbigint c;

   if (ZEROP(a) || ZEROP(b)) {
      _ntl_gzero(cc);
      return;
   }

   GET_SIZE_NEG(sa, aneg, a);
   GET_SIZE_NEG(sb, bneg, b);

   if (a != *cc && b != *cc) {
      // no aliasing

      c = *cc;

      sc = sa + sb;
      if (MustAlloc(c, sc)) {
	 _ntl_gsetlength(&c, sc);
         *cc = c;
      }

      adata = DATA(a);
      bdata = DATA(b);
      cdata = DATA(c);

      if (adata == bdata) {
#if (1 && defined(NTL_VIABLE_LL) && NTL_NAIL_BITS==0)
         if (sa == 1) {
            ll_type prod;
            ll_mul(prod, adata[0], adata[0]);
            cdata[0] = ll_get_lo(prod);
            msl = cdata[1] = ll_get_hi(prod);
         } else
#endif
         {
            NTL_MPN(sqr)(cdata, adata, sa);
            msl = cdata[2*sa-1];
         }
      }
      else {
#if 1
	 if (sa >= sb) {
#if (1 && defined(NTL_VIABLE_LL) && NTL_NAIL_BITS==0)
	    if (sa == 1) {
	       ll_type prod;
	       ll_mul(prod, adata[0], bdata[0]);
	       cdata[0] = ll_get_lo(prod);
	       msl = cdata[1] = ll_get_hi(prod);
	    } else
#endif
	    if (sa <= 4)
	       msl = base_mul(cdata, adata, sa, bdata, sb);
	    else
	       msl = NTL_MPN(mul)(cdata, adata, sa, bdata, sb);
	 }
	 else {
	    if (sb <= 4)
	       msl = base_mul(cdata, bdata, sb, adata, sa);
	    else
	       msl = NTL_MPN(mul)(cdata, bdata, sb, adata, sa);
	 }
#else
	 if (sa >= sb) {
	    msl = NTL_MPN(mul)(cdata, adata, sa, bdata, sb);
	 }
	 else {
	    msl = NTL_MPN(mul)(cdata, bdata, sb, adata, sa);
	 }
#endif
      }

      if (!msl) sc--;
      if (aneg != bneg) sc = -sc;
      SIZE(c) = sc;
   }
   else {
      // aliasing
      GRegister(mem);

      c = mem;

      sc = sa + sb;
      if (MustAlloc(c, sc)) {
	 _ntl_gsetlength(&c, sc);
         mem = c;
      }

      adata = DATA(a);
      bdata = DATA(b);
      cdata = DATA(c);

      if (adata == bdata) {
#if (1 && defined(NTL_VIABLE_LL) && NTL_NAIL_BITS==0)
         if (sa == 1) {
            ll_type prod;
            ll_mul(prod, adata[0], adata[0]);
            cdata[0] = ll_get_lo(prod);
            msl = cdata[1] = ll_get_hi(prod);
         } else
#endif
         {
            NTL_MPN(sqr)(cdata, adata, sa);
            msl = cdata[2*sa-1];
         }
      }
      else {
#if 1
	 if (sa >= sb) {
#if (1 && defined(NTL_VIABLE_LL) && NTL_NAIL_BITS==0)
	    if (sa == 1) {
	       ll_type prod;
	       ll_mul(prod, adata[0], bdata[0]);
	       cdata[0] = ll_get_lo(prod);
	       msl = cdata[1] = ll_get_hi(prod);
	    } else
#endif
	    if (sa <= 4)
	       msl = base_mul(cdata, adata, sa, bdata, sb);
	    else
	       msl = NTL_MPN(mul)(cdata, adata, sa, bdata, sb);
	 }
	 else {
	    if (sb <= 4)
	       msl = base_mul(cdata, bdata, sb, adata, sa);
	    else
	       msl = NTL_MPN(mul)(cdata, bdata, sb, adata, sa);
	 }
#else
	 if (sa >= sb) {
	    msl = NTL_MPN(mul)(cdata, adata, sa, bdata, sb);
	 }
	 else {
	    msl = NTL_MPN(mul)(cdata, bdata, sb, adata, sa);
	 }
#endif
      }

      if (!msl) sc--;
      if (aneg != bneg) sc = -sc;
      SIZE(c) = sc;

      _ntl_gcopy(mem, cc);
   }

}

#else
void _ntl_gmul(_ntl_gbigint a, _ntl_gbigint b, _ntl_gbigint *cc)
{
   GRegister(mem);

   long sa, aneg, sb, bneg, alias, sc;
   _ntl_limb_t *adata, *bdata, *cdata, msl;
   _ntl_gbigint c;

   if (ZEROP(a) || ZEROP(b)) {
      _ntl_gzero(cc);
      return;
   }

   GET_SIZE_NEG(sa, aneg, a);
   GET_SIZE_NEG(sb, bneg, b);

   if (a == *cc || b == *cc) {
      c = mem;
      alias = 1;
   }
   else {
      c = *cc;
      alias = 0;
   }

   sc = sa + sb;
   if (MustAlloc(c, sc))
      _ntl_gsetlength(&c, sc);

   if (alias)
      mem = c;
   else
      *cc = c;

   adata = DATA(a);
   bdata = DATA(b);
   cdata = DATA(c);

   if (sa >= sb)
      msl = NTL_MPN(mul)(cdata, adata, sa, bdata, sb);
   else
      msl = NTL_MPN(mul)(cdata, bdata, sb, adata, sa);

   if (!msl) sc--;
   if (aneg != bneg) sc = -sc;
   SIZE(c) = sc;

   if (alias) _ntl_gcopy(mem, cc);
}
#endif

void _ntl_gsq(_ntl_gbigint a, _ntl_gbigint *cc)
{
   long sa, aneg, alias, sc;
   _ntl_limb_t *adata, *cdata, msl;
   _ntl_gbigint c;

   if (ZEROP(a)) {
      _ntl_gzero(cc);
      return;
   }

   GET_SIZE_NEG(sa, aneg, a);

   if (a != *cc) {
      // no aliasing

      c = *cc;

      sc = sa + sa;
      if (MustAlloc(c, sc)) {
	 _ntl_gsetlength(&c, sc);
         *cc = c;
      }

      adata = DATA(a);
      cdata = DATA(c);

#if (1 && defined(NTL_VIABLE_LL) && NTL_NAIL_BITS==0)
      if (sa == 1) {
	 ll_type prod;
	 ll_mul(prod, adata[0], adata[0]);
	 cdata[0] = ll_get_lo(prod);
	 msl = cdata[1] = ll_get_hi(prod);
      } else
#endif
      {
	 NTL_MPN(sqr)(cdata, adata, sa);
	 msl = cdata[2*sa-1];
      }

      if (!msl) sc--;
      SIZE(c) = sc;
   }
   else {
      // aliasing
      GRegister(mem);

      c = mem;

      sc = sa + sa;
      if (MustAlloc(c, sc)) {
	 _ntl_gsetlength(&c, sc);
         mem = c;
      }

      adata = DATA(a);
      cdata = DATA(c);

#if (1 && defined(NTL_VIABLE_LL) && NTL_NAIL_BITS==0)
      if (sa == 1) {
	 ll_type prod;
	 ll_mul(prod, adata[0], adata[0]);
	 cdata[0] = ll_get_lo(prod);
	 msl = cdata[1] = ll_get_hi(prod);
      } else
#endif
      {
	 NTL_MPN(sqr)(cdata, adata, sa);
	 msl = cdata[2*sa-1];
      }

      if (!msl) sc--;
      SIZE(c) = sc;

      _ntl_gcopy(mem, cc);
   }

}



void
_ntl_gsmul(_ntl_gbigint a, long d, _ntl_gbigint *bb)
{
   long sa, sb;
   long anegative, bnegative;
   _ntl_gbigint b;
   _ntl_limb_t *adata, *bdata;
   _ntl_limb_t dd, carry;
   long a_alias;

   if (ZEROP(a) || !d) {
      _ntl_gzero(bb);
      return;
   }

   dd = ABS(d);

   if (XCLIP(dd)) {
      GRegister(xd);
      _ntl_gintoz(d,&xd);
      _ntl_gmul(a, xd, bb);
      return;
   }

   // we may now assume that |d| fits in one limb

   GET_SIZE_NEG(sa, anegative, a);

   bnegative = XOR(anegative, d < 0);

   sb = sa + 1;

   b = *bb;
   a_alias = (a == b);

   if (MustAlloc(b, sb)) {
      _ntl_gsetlength(&b, sb);
      if (a_alias) a = b;
      *bb = b;
   }

   adata = DATA(a);
   bdata = DATA(b);

   if (dd == 2) 
      carry = NTL_MPN(lshift)(bdata, adata, sa, 1);
   else
      carry = NTL_MPN(mul_1)(bdata, adata, sa, dd);

   if (carry) 
      bdata[sa] = carry;
   else
      sb--;

   if (bnegative) sb = -sb;
   SIZE(b) = sb;
}




long _ntl_gsdiv(_ntl_gbigint a, long d, _ntl_gbigint *bb)
{
   long sa, aneg, sb, dneg;
   _ntl_gbigint b;
   _ntl_limb_t dd, *adata, *bdata;
   long r;

   if (!d) {
      ArithmeticError("division by zero in _ntl_gsdiv");
   }

   if (ZEROP(a)) {
      _ntl_gzero(bb);
      return (0);
   }

   dd = ABS(d);

   if (XCLIP(dd)) {
      GRegister(xd);
      GRegister(xr);
      _ntl_gintoz(d,&xd);
      _ntl_gdiv(a, xd, bb, &xr);
      return _ntl_gtoint(xr);
   }

   // we may now assume that |d| fits in one limb

   GET_SIZE_NEG(sa, aneg, a);

   dneg = d < 0;

   sb = sa;
   b = *bb;
   if (MustAlloc(b, sb)) {
      /* if b aliases a, then b won't move */
      _ntl_gsetlength(&b, sb);
      *bb = b;
   }

   adata = DATA(a);
   bdata = DATA(b);

   if (dd == 2) 
      r = NTL_MPN(rshift)(bdata, adata, sa, 1) >> (NTL_ZZ_NBITS - 1);
   else
      r = NTL_MPN(divmod_1)(bdata, adata, sa, dd);

   if (bdata[sb-1] == 0)
      sb--;

   SIZE(b) = sb;

   if (aneg || dneg) {
      if (aneg != dneg) {
         if (!r) {
            SIZE(b) = -SIZE(b);
         }
         else {
            _ntl_gsadd(b, 1, &b);
            SIZE(b) = -SIZE(b);
            if (dneg)
               r = r + d;
            else
               r = d - r;
            *bb = b;
         }
      }
      else
         r = -r;
   }

   return r;
}

long _ntl_gsmod(_ntl_gbigint a, long d)
{
   long sa, aneg, dneg;
   _ntl_limb_t dd, *adata;
   long r;

   if (!d) {
      ArithmeticError("division by zero in _ntl_gsmod");
   }

   if (ZEROP(a)) {
      return (0);
   }

   dd = ABS(d);

   if (XCLIP(dd)) {
      GRegister(xd);
      GRegister(xr);
      _ntl_gintoz(d,&xd);
      _ntl_gmod(a, xd, &xr);
      return _ntl_gtoint(xr);
   }

   // we may now assume that |d| fits in one limb

   GET_SIZE_NEG(sa, aneg, a);

   dneg = d < 0;

   adata = DATA(a);

   if (dd == 2) 
      r = adata[0] & 1;
   else
      r = NTL_MPN(mod_1)(adata, sa, dd);

   if (aneg || dneg) {
      if (aneg != dneg) {
         if (r) {
            if (dneg)
               r = r + d;
            else
               r = d - r;
         }
      }
      else
         r = -r;
   }

   return r;
}




void _ntl_gdiv(_ntl_gbigint a, _ntl_gbigint d, 
               _ntl_gbigint *bb, _ntl_gbigint *rr)
{
   GRegister(b);
   GRegister(rmem);

   _ntl_gbigint *rp;

   long sa, aneg, sb, sd, dneg, sr, in_place;
   _ntl_limb_t *adata, *ddata, *bdata, *rdata;

   if (ZEROP(d)) {
      ArithmeticError("division by zero in _ntl_gdiv");
   }

   if (ZEROP(a)) {
      if (bb) _ntl_gzero(bb);
      if (rr) _ntl_gzero(rr);
      return;
   }

   GET_SIZE_NEG(sa, aneg, a);
   GET_SIZE_NEG(sd, dneg, d);

   if (!aneg && !dneg && rr && *rr != a && *rr != d) {
      in_place = 1;
      rp = rr;
   }
   else {
      in_place = 0;
      rp = &rmem;
   }


   if (sa < sd) {
      _ntl_gzero(&b);
      _ntl_gcopy(a, rp);
      if (aneg) SIZE(*rp) = -SIZE(*rp);
      goto done;
   }

   sb = sa-sd+1;
   if (MustAlloc(b, sb))
      _ntl_gsetlength(&b, sb);

   sr = sd;
   if (MustAlloc(*rp, sr))
      _ntl_gsetlength(rp, sr);


   adata = DATA(a);
   ddata = DATA(d);
   bdata = DATA(b);
   rdata = DATA(*rp);

   NTL_MPN(tdiv_qr)(bdata, rdata, 0, adata, sa, ddata, sd);

   if (bdata[sb-1] == 0)
      sb--;
   SIZE(b) = sb;

   STRIP(sr, rdata);
   SIZE(*rp) = sr;

done:

   if (aneg || dneg) {
      if (aneg != dneg) {
         if (ZEROP(*rp)) {
            SIZE(b) = -SIZE(b);
         }
         else {
            if (bb) {
               _ntl_gsadd(b, 1, &b);
               SIZE(b) = -SIZE(b);
            }
            if (rr) {
               if (dneg)
                  _ntl_gadd(*rp, d, rp);
               else
                  _ntl_gsub(d, *rp, rp);
            }
         }
      }
      else
         SIZE(*rp) = -SIZE(*rp);
   }

   if (bb) _ntl_gcopy(b, bb);

   if (rr && !in_place)
      _ntl_gcopy(*rp, rr);
}


/* a simplified mod operation:  assumes a >= 0, d > 0 are non-negative,
 * that space for the result has already been allocated,
 * and that inputs do not alias output. */

static
void gmod_simple(_ntl_gbigint a, _ntl_gbigint d, _ntl_gbigint *rr)
{
   GRegister(b);

   long sa, sb, sd, sr;
   _ntl_limb_t *adata, *ddata, *bdata, *rdata;
   _ntl_gbigint r;

   if (ZEROP(a)) {
      _ntl_gzero(rr);
      return;
   }

   sa = SIZE(a);
   sd = SIZE(d);

   if (sa < sd) {
      _ntl_gcopy(a, rr);
      return;
   }

   sb = sa-sd+1;
   if (MustAlloc(b, sb))
      _ntl_gsetlength(&b, sb);

   sr = sd;
   r = *rr;

   adata = DATA(a);
   ddata = DATA(d);
   bdata = DATA(b);
   rdata = DATA(r);

   NTL_MPN(tdiv_qr)(bdata, rdata, 0, adata, sa, ddata, sd);

   STRIP(sr, rdata);
   SIZE(r) = sr;
}


void _ntl_gmod(_ntl_gbigint a, _ntl_gbigint d, _ntl_gbigint *rr)
{
   _ntl_gdiv(a, d, 0, rr);
}

void _ntl_gquickmod(_ntl_gbigint *rr, _ntl_gbigint d)
{
   _ntl_gdiv(*rr, d, 0, rr);
}




#if (defined(NTL_GMP_LIP) && (NTL_ZZ_NBITS >= NTL_BITS_PER_LONG))

long _ntl_gsqrts(long n)
{
   _ntl_limb_t ndata, rdata;

   if (n == 0) {
      return 0;
   }

   if (n < 0) ArithmeticError("negative argument to _ntl_sqrts");

   ndata = n;

   NTL_MPN(sqrtrem)(&rdata, 0, &ndata, 1);

   return rdata;
}


#else

long 
_ntl_gsqrts(long n)
{

   if (n < 0) 
      ArithmeticError("_ntl_gsqrts: negative argument");

   if (n <= 0) return (0);
   if (n <= 3) return (1);
   if (n <= 8) return (2);

   if (n >= NTL_WSP_BOUND)
   {
      GRegister(xn);
      GRegister(xr);
      _ntl_gintoz(n,&xn);
      _ntl_gsqrt(xn,&xr);
      return _ntl_gtoint(xr);
   }

   long a;
   long ndiva;
   long newa;


   newa = 3L << (2 * ((NTL_WSP_NBITS/2) - 1)); 
   // DIRT: here we use the assumption that NTL_WSP_NBITS is
   // even --- this logic comes from Lenstra's LIP, and I don't know
   // what happens if it is odd

   a = 1L << (NTL_WSP_NBITS/2);
   while (!(n & newa)) {
      newa >>= 2;
      a >>= 1;
   }

   while (1) {
      newa = ((ndiva = n / a) + a) / 2;
      if (newa - ndiva <= 1) {
         if (newa * newa <= n)
            return newa;
         else
            return ndiva;
      }
      a = newa;
   }
}



#endif





#ifdef NTL_GMP_LIP 


void _ntl_gsqrt(_ntl_gbigint n, _ntl_gbigint *rr)
{
   GRegister(r);

   long sn, sr;
   _ntl_limb_t *ndata, *rdata;

   if (ZEROP(n)) {
      _ntl_gzero(rr);
      return;
   }

   sn = SIZE(n);
   if (sn < 0) ArithmeticError("negative argument to _ntl_gsqrt");

   sr = (sn+1)/2;
   _ntl_gsetlength(&r, sr);

   ndata = DATA(n);
   rdata = DATA(r);

   mpn_sqrtrem(rdata, 0, ndata, sn);

   STRIP(sr, rdata);
   SIZE(r) = sr;

   _ntl_gcopy(r, rr);
}

#else 


void _ntl_gsqrt(_ntl_gbigint n, _ntl_gbigint *rr)
{
   GRegister(a);
   GRegister(ndiva);
   GRegister(diff);
   GRegister(r);

   if (ZEROP(n)) {
      _ntl_gzero(rr);
      return;
   }

   long sn = SIZE(n);
   if (sn < 0) ArithmeticError("negative argument to _ntl_gsqrt");

   _ntl_limb_t *ndata = DATA(n);

   if (sn == 1) {
      _ntl_gintoz(_ntl_gsqrts(ndata[0]), rr);
      return;
   }

   _ntl_gsetlength(&a, sn);
   _ntl_gsetlength(&ndiva, sn);
   _ntl_gsetlength(&diff, sn);

   long sa = (sn+1)/2;
   _ntl_limb_t *adata = DATA(a);
   
   adata[sa-1] = _ntl_gsqrts(ndata[sn-1]) + 1;
   if (!(sn & 1))
      adata[sa-1] <<= (NTL_ZZ_NBITS/2);
      // DIRT: here we use the assumption that NTL_ZZ_NBITS is
      // even --- this logic comes from Lenstra's LIP, and I don't know
      // what happens if it is odd

   if (adata[sa-1] & NTL_ZZ_RADIX) {
      sa++;
      adata[sa-1] = 1;
   }

   for (long i = 0; i < sa-1; i++) adata[i] = 0;
   SIZE(a) = sa;

   while (1) {
      _ntl_gdiv(n, a, &ndiva, &r);
      _ntl_gadd(a, ndiva, &r);
      _ntl_grshift(r, 1, &r);
      if (_ntl_gcompare(r, ndiva) <= 0) 
         goto done;

      _ntl_gsubpos(r, ndiva, &diff);
      if (ZEROP(diff) || ONEP(diff)) {
         _ntl_gsq(r, &diff);
         if (_ntl_gcompare(diff, n) > 0)
            _ntl_gcopy(ndiva, &r);

         goto done;
      }
      _ntl_gcopy(r, &a);
   }
done:
   _ntl_gcopy(r, rr);
}



#endif










#ifdef NTL_GMP_LIP

void _ntl_ggcd(_ntl_gbigint m1, _ntl_gbigint m2, _ntl_gbigint *r)
{
   GRegister(s1);
   GRegister(s2);
   GRegister(res);

   long k1, k2, k_min, l1, l2, ss1, ss2, sres;

   _ntl_gcopy(m1, &s1);
   _ntl_gabs(&s1);

   _ntl_gcopy(m2, &s2);
   _ntl_gabs(&s2);

   if (ZEROP(s1)) {
      _ntl_gcopy(s2, r);
      return;
   }

   if (ZEROP(s2)) {
      _ntl_gcopy(s1, r);
      return;
   }

   k1 = _ntl_gmakeodd(&s1);
   k2 = _ntl_gmakeodd(&s2);

   if (k1 <= k2)
      k_min = k1;
   else
      k_min = k2;

   l1 = _ntl_g2log(s1);
   l2 = _ntl_g2log(s2);

   ss1 = SIZE(s1);
   ss2 = SIZE(s2);

   if (ss1 >= ss2)
      sres = ss1;
   else
      sres = ss2;

   /* set to max: gmp documentation is unclear on this point */

   _ntl_gsetlength(&res, sres);
   
   // NOTE: older versions of GMP require first operand has
   // at least as many bits as the second.
   // It seems this requirement has been relaxed in more
   // recent versions.

   if (l1 >= l2)
      SIZE(res) = mpn_gcd(DATA(res), DATA(s1), ss1, DATA(s2), ss2);
   else
      SIZE(res) = mpn_gcd(DATA(res), DATA(s2), ss2, DATA(s1), ss1);

   _ntl_glshift(res, k_min, &res);

   _ntl_gcopy(res, r);
}

void
_ntl_ggcd_alt(_ntl_gbigint mm1, _ntl_gbigint mm2, _ntl_gbigint *rres)
{
   _ntl_ggcd(mm1, mm2, rres);
}


#else



// Interestingly, the Lehmer code even for basic GCD
// about twice as fast as the binary gcd

static void
gxxeucl_basic(
   _ntl_gbigint ain,
   _ntl_gbigint nin,
   _ntl_gbigint *uu
   )
{
   GRegister(a);
   GRegister(n);
   GRegister(q);
   GRegister(x);
   GRegister(y);
   GRegister(z);


   long diff;
   long ilo;
   long sa;
   long sn;
   long temp;
   long e;
   long fast;
   long parity;
   long gotthem;
   _ntl_limb_t *p;
   long try11;
   long try12;
   long try21;
   long try22;
   long got11;
   long got12;
   long got21;
   long got22;
   double hi;
   double lo;
   double dt;
   double fhi, fhi1;
   double flo, flo1;
   double num;
   double den;
   double dirt;

   if (SIZE(ain) < SIZE(nin)) {
      _ntl_swap(ain, nin);
   }
   e = SIZE(ain)+2;

   _ntl_gsetlength(&a, e);
   _ntl_gsetlength(&n, e);
   _ntl_gsetlength(&q, e);
   _ntl_gsetlength(&x, e);
   _ntl_gsetlength(&y, e);
   _ntl_gsetlength(&z, e);

   fhi1 = double(1L) + double(32L)/NTL_FDOUBLE_PRECISION;
   flo1 = double(1L) - double(32L)/NTL_FDOUBLE_PRECISION;

   fhi = double(1L) + double(8L)/NTL_FDOUBLE_PRECISION;
   flo = double(1L) - double(8L)/NTL_FDOUBLE_PRECISION;

   _ntl_gcopy(ain, &a);
   _ntl_gcopy(nin, &n);


   while (SIZE(n) > 0)
   {
      gotthem = 0;
      sa = SIZE(a);
      sn = SIZE(n);
      diff = sa - sn;
      if (!diff || diff == 1)
      {
         sa = SIZE(a);
         p = DATA(a) + (sa-1);
         num = DBL(*p) * NTL_ZZ_FRADIX;
         if (sa > 1)
            num += DBL(*(--p));
         num *= NTL_ZZ_FRADIX;
         if (sa > 2)
            num += DBL(*(p - 1));

         sn = SIZE(n);
         p = DATA(n) + (sn-1);
         den = DBL(*p) * NTL_ZZ_FRADIX;
         if (sn > 1)
            den += DBL(*(--p));
         den *= NTL_ZZ_FRADIX;
         if (sn > 2)
            den += DBL(*(p - 1));

         hi = fhi1 * (num + double(1L)) / den;
         lo = flo1 * num / (den + double(1L));
         if (diff > 0)
         {
            hi *= NTL_ZZ_FRADIX;
            lo *= NTL_ZZ_FRADIX;
         }
         try11 = 1;
         try12 = 0;
         try21 = 0;
         try22 = 1;
         parity = 1;
         fast = 1; 
         while (fast > 0)
         {
            parity = 1 - parity;
            if (hi >= NTL_NSP_BOUND)
               fast = 0;
            else
            {
               ilo = (long)lo;
               dirt = hi - double(ilo);
               if (dirt < 1.0/NTL_FDOUBLE_PRECISION || !ilo || ilo < (long)hi)
                  fast = 0;
               else
               {
                  dt = lo-double(ilo);
                  lo = flo / dirt;
                  if (dt > 1.0/NTL_FDOUBLE_PRECISION)
                     hi = fhi / dt;
                  else
                     hi = double(NTL_NSP_BOUND);
                  temp = try11;
                  try11 = try21;
                  if ((NTL_WSP_BOUND - temp) / ilo < try21)
                     fast = 0;
                  else
                     try21 = temp + ilo * try21;
                  temp = try12;
                  try12 = try22;
                  if ((NTL_WSP_BOUND - temp) / ilo < try22)
                     fast = 0;
                  else
                     try22 = temp + ilo * try22;
                  if ((fast > 0) && (parity > 0))
                  {
                     gotthem = 1;
                     got11 = try11;
                     got12 = try12;
                     got21 = try21;
                     got22 = try22;
                  }
               }
            }
         }
      }
      if (gotthem)
      {
         _ntl_gsmul(a, got11, &x);
         _ntl_gsmul(n, got12, &y);
         _ntl_gsmul(a, got21, &z);
         _ntl_gsmul(n, got22, &n);
         _ntl_gsub(x, y, &a);
         _ntl_gsub(n, z, &n);
      }
      else
      {
         _ntl_gdiv(a, n, &q, &a);
         if (!ZEROP(a))
         {
            _ntl_gdiv(n, a, &q, &n);
         }
         else
         {
            _ntl_gcopy(n, &a);
            _ntl_gzero(&n);
         }
      }
   }

   _ntl_gcopy(a, uu);

   return;
}

void
_ntl_ggcd(_ntl_gbigint mm1, _ntl_gbigint mm2, _ntl_gbigint *rres)
{
   GRegister(a);
   GRegister(b);
   GRegister(inv);

   if (ZEROP(mm1))
   {
      _ntl_gcopy(mm2, rres);
      _ntl_gabs(rres);
      return;
   }

   if (ZEROP(mm2))
   {
      _ntl_gcopy(mm1, rres);
      _ntl_gabs(rres);
      return;
   }

   _ntl_gcopy(mm1, &a);
   _ntl_gabs(&a);
   _ntl_gcopy(mm2, &b);
   _ntl_gabs(&b);
   gxxeucl_basic(a, b, rres);
}


// This is the binary gcd algorithm

void
_ntl_ggcd_alt(_ntl_gbigint mm1, _ntl_gbigint mm2, _ntl_gbigint *rres)
{
   GRegister(a);
   GRegister(b);
   GRegister(c);

   long agrb;
   long shibl;

   if (ZEROP(mm1))
   {
      _ntl_gcopy(mm2, rres);
      _ntl_gabs(rres);
      return;
   }

   if (ZEROP(mm2))
   {
      _ntl_gcopy(mm1, rres);
      _ntl_gabs(rres);
      return;
   }

   if (mm1 == mm2)
   {
      _ntl_gcopy(mm1, rres);
      _ntl_gabs(rres);
      return;
   }

   long s1 = SIZE(mm1);
   if (s1 < 0) s1 = -s1;

   long s2 = SIZE(mm2);
   if (s2 < 0) s2 = -s2;

   long maxs1s2 = max(s1, s2);

   _ntl_gsetlength(&a, maxs1s2); 
   _ntl_gsetlength(&b, maxs1s2);
   _ntl_gsetlength(&c, maxs1s2);

   if (s1 != s2)
   {
      if (s1 > s2)
      {
         _ntl_gcopy(mm2, &a);
         _ntl_gabs(&a);

         _ntl_gcopy(mm1, &c);
         _ntl_gabs(&c);

         _ntl_gmod(c, a, &b);
      }
      else
      {
         _ntl_gcopy(mm1, &a);
         _ntl_gabs(&a);

         _ntl_gcopy(mm2, &c);
         _ntl_gabs(&c);
         _ntl_gmod(c, a, &b);
      }
      if (ZEROP(b)) goto done;
   }
   else
   {
      _ntl_gcopy(mm1, &a);
      _ntl_gabs(&a);
      _ntl_gcopy(mm2, &b);
      _ntl_gabs(&b);
   }

   if ((agrb = _ntl_gmakeodd(&a)) < (shibl = _ntl_gmakeodd(&b))) shibl = agrb;
   if (!(agrb = _ntl_gcompare(a, b))) goto endshift;

   if (agrb < 0)
   {
      _ntl_swap(a, b);
   }

   _ntl_gsubpos(a, b, &c);
   do
   {
      _ntl_gmakeodd(&c);
      if (!(agrb = _ntl_gcompare(b, c)))
      {
         _ntl_swap(a, b);
         goto endshift;
      }

      if (agrb > 0)
      {
         // (a, b, c) = (b, c, a)
         _ntl_swap(a, b);
         _ntl_swap(b, c);
      }
      else
      {
         // (a, b, c) = (c, b, a)
         _ntl_swap(a, c);
      }
      _ntl_gsubpos(a, b, &c);
   } while (!ZEROP(c));

endshift:
   _ntl_glshift(a, shibl, &a);

done:
   _ntl_gcopy(a, rres);
}

#endif





#ifdef NTL_GMP_LIP



void
_ntl_gexteucl(
	_ntl_gbigint ain,
	_ntl_gbigint *xap,
	_ntl_gbigint bin,
	_ntl_gbigint *xbp,
	_ntl_gbigint *dp
	)
{
   if (ZEROP(bin)) {
      long asign = _ntl_gsign(ain);

      _ntl_gcopy(ain, dp);
      _ntl_gabs(dp);
      _ntl_gintoz( (asign >= 0 ? 1 : -1), xap);
      _ntl_gzero(xbp);
   }
   else if (ZEROP(ain)) {
      long bsign = _ntl_gsign(bin);

      _ntl_gcopy(bin, dp);
      _ntl_gabs(dp);
      _ntl_gzero(xap);
      _ntl_gintoz(bsign, xbp); 
   }
   else {
      GRegister(a);
      GRegister(b);
      GRegister(xa);
      GRegister(xb);
      GRegister(d);
      GRegister(tmp);

      long sa, aneg, sb, bneg, rev;
      _ntl_limb_t *adata, *bdata, *ddata, *xadata;
      mp_size_t sxa, sd;

      GET_SIZE_NEG(sa, aneg, ain);
      GET_SIZE_NEG(sb, bneg, bin);

      _ntl_gsetlength(&a, sa+1); /* +1 because mpn_gcdext may need it */
      _ntl_gcopy(ain, &a);

      _ntl_gsetlength(&b, sb+1); /* +1 because mpn_gcdext may need it */
      _ntl_gcopy(bin, &b);


      adata = DATA(a);
      bdata = DATA(b);

      if (sa < sb || (sa == sb && NTL_MPN(cmp)(adata, bdata, sa) < 0)) {
         SWAP_BIGINT(ain, bin);
         SWAP_LONG(sa, sb);
         SWAP_LONG(aneg, bneg);
         SWAP_LIMB_PTR(adata, bdata);
         rev = 1;
      }
      else 
         rev = 0;

      _ntl_gsetlength(&d, sa+1); /* +1 because mpn_gcdext may need it...
                                    documentation is unclear, but this is
                                    what is done in mpz_gcdext */
      _ntl_gsetlength(&xa, sa+1); /* ditto */

      ddata = DATA(d);
      xadata = DATA(xa);

      sd = mpn_gcdext(ddata, xadata, &sxa, adata, sa, bdata, sb);

      SIZE(d) = sd;
      SIZE(xa) = sxa;

#if 0
      // since we're now requiring GMP version 5.0.0 or later,
      // these workarounds are no longer required

      /* These two ForceNormal's are work-arounds for GMP bugs 
         in GMP 4.3.0 */
      ForceNormal(d);
      ForceNormal(xa);

      /* now we normalize xa, so that so that xa in ( -b/2d, b/2d ],
         which makes the output agree with Euclid's algorithm,
         regardless of what mpn_gcdext does */

      if (!ZEROP(xa)) {
         _ntl_gcopy(bin, &b);
         SIZE(b) = sb;
         if (!ONEP(d)) {
            _ntl_gdiv(b, d, &b, &tmp);
            if (!ZEROP(tmp)) TerminalError("internal bug in _ntl_gexteucl");
         }

         if (SIZE(xa) > 0) { /* xa positive */
            if (_ntl_gcompare(xa, b) > 0) { 
               _ntl_gmod(xa, b, &xa);
            }
            _ntl_glshift(xa, 1, &tmp);
            if (_ntl_gcompare(tmp, b) > 0) {
               _ntl_gsub(xa, b, &xa);
            }
         }
         else { /* xa negative */
            SIZE(xa) = -SIZE(xa);
            if (_ntl_gcompare(xa, b) > 0) {
               SIZE(xa) = -SIZE(xa);
               _ntl_gmod(xa, b, &xa);
               _ntl_gsub(xa, b, &xa);
            }
            else {
               SIZE(xa) = -SIZE(xa);
            }
            _ntl_glshift(xa, 1, &tmp);
            SIZE(tmp) = -SIZE(tmp);
            if (_ntl_gcompare(tmp, b) >= 0) {
               _ntl_gadd(xa, b, &xa);
            }
         }
      }

      /* end normalize */
#endif
    

      if (aneg) _ntl_gnegate(&xa);

      _ntl_gmul(ain, xa, &tmp);
      _ntl_gsub(d, tmp, &tmp);
      _ntl_gdiv(tmp, bin, &xb, &tmp);

      if (!ZEROP(tmp)) TerminalError("internal bug in _ntl_gexteucl");

      if (rev) SWAP_BIGINT(xa, xb);

      _ntl_gcopy(xa, xap);
      _ntl_gcopy(xb, xbp);
      _ntl_gcopy(d, dp); 
   }
}


long _ntl_ginv(_ntl_gbigint ain, _ntl_gbigint nin, _ntl_gbigint *invv)
{
   GRegister(u);
   GRegister(d);
   GRegister(a);
   GRegister(n);

   long sz; 
   long sd;
   mp_size_t su;

   if (_ntl_gscompare(nin, 1) <= 0) {
      LogicError("InvMod: second input <= 1");
   }

   if (_ntl_gsign(ain) < 0) {
      LogicError("InvMod: first input negative");
   }

   if (_ntl_gcompare(ain, nin) >= 0) {
      LogicError("InvMod: first input too big");
   }

   sz = SIZE(nin) + 2;

   if (MustAlloc(a, sz))
      _ntl_gsetlength(&a, sz);


   if (MustAlloc(n, sz))
       _ntl_gsetlength(&n, sz);


   if (MustAlloc(d, sz))
       _ntl_gsetlength(&d, sz);

   if (MustAlloc(u, sz))
       _ntl_gsetlength(&u, sz);

   _ntl_gadd(ain, nin, &a);
   _ntl_gcopy(nin, &n);

   /* We apply mpn_gcdext to (a, n) = (ain+nin, nin), because that function
    * only computes the co-factor of the larger input. This way, we avoid
    * a multiplication and a division.
    */

   sd = mpn_gcdext(DATA(d), DATA(u), &su, DATA(a), SIZE(a), DATA(n), SIZE(n));

   SIZE(d) = sd;
   SIZE(u) = su;

#if 0
   // since we're now requiring GMP version 5.0.0 or later,
   // these workarounds are no longer required

   /* Thes two ForceNormal's are work-arounds for GMP bugs 
      in GMP 4.3.0 */
   ForceNormal(d);
   ForceNormal(u);
#endif

   if (ONEP(d)) {

      /*
       * We make sure that u is in range 0..n-1, just in case
       * GMP is sloppy.
       */

#if 0
      // since we're now requiring GMP version 5.0.0 or later,
      // these workarounds are no longer required

      if (_ntl_gsign(u) < 0) {
         _ntl_gadd(u, nin, &u);
         if (_ntl_gsign(u) < 0) {
            _ntl_gmod(u, nin, &u);
         }
      }
      else if (_ntl_gcompare(u, nin) >= 0) {
         _ntl_gsub(u, nin, &u);
         if (_ntl_gcompare(u, nin) >= 0) {
             _ntl_gmod(u, nin, &u);
         }
      }
#else
      if (_ntl_gsign(u) < 0) {
         _ntl_gadd(u, nin, &u);
      }

#endif

      _ntl_gcopy(u, invv);
      return 0;
   }
   else {
      _ntl_gcopy(d, invv);
      return 1;
   }
}



#else

static long 
gxxeucl(
   _ntl_gbigint ain,
   _ntl_gbigint nin,
   _ntl_gbigint *invv,
   _ntl_gbigint *uu
   )
{
   GRegister(a);
   GRegister(n);
   GRegister(q);
   GRegister(w);
   GRegister(x);
   GRegister(y);
   GRegister(z);

   GRegister(inv);

   long diff;
   long ilo;
   long sa;
   long sn;
   long temp;
   long e;
   long fast;
   long parity;
   long gotthem;
   _ntl_limb_t *p;
   long try11;
   long try12;
   long try21;
   long try22;
   long got11;
   long got12;
   long got21;
   long got22;
   double hi;
   double lo;
   double dt;
   double fhi, fhi1;
   double flo, flo1;
   double num;
   double den;
   double dirt;

   _ntl_gsetlength(&a, (e = 2 + (SIZE(ain) > SIZE(nin) ? SIZE(ain) : SIZE(nin))));
   _ntl_gsetlength(&n, e);
   _ntl_gsetlength(&q, e);
   _ntl_gsetlength(&w, e);
   _ntl_gsetlength(&x, e);
   _ntl_gsetlength(&y, e);
   _ntl_gsetlength(&z, e);
   _ntl_gsetlength(&inv, e);

   fhi1 = double(1L) + double(32L)/NTL_FDOUBLE_PRECISION;
   flo1 = double(1L) - double(32L)/NTL_FDOUBLE_PRECISION;

   fhi = double(1L) + double(8L)/NTL_FDOUBLE_PRECISION;
   flo = double(1L) - double(8L)/NTL_FDOUBLE_PRECISION;

   _ntl_gcopy(ain, &a);
   _ntl_gcopy(nin, &n);

   _ntl_gone(&inv);
   _ntl_gzero(&w);

   while (SIZE(n) > 0)
   {
      gotthem = 0;
      sa = SIZE(a);
      sn = SIZE(n);
      diff = sa - sn;
      if (!diff || diff == 1)
      {
         sa = SIZE(a);
         p = DATA(a) + (sa-1);
         num = DBL(*p) * NTL_ZZ_FRADIX;
         if (sa > 1)
            num += DBL(*(--p));
         num *= NTL_ZZ_FRADIX;
         if (sa > 2)
            num += DBL(*(p - 1));

         sn = SIZE(n);
         p = DATA(n) + (sn-1);
         den = DBL(*p) * NTL_ZZ_FRADIX;
         if (sn > 1)
            den += DBL(*(--p));
         den *= NTL_ZZ_FRADIX;
         if (sn > 2)
            den += DBL(*(p - 1));

         hi = fhi1 * (num + double(1L)) / den;
         lo = flo1 * num / (den + double(1L));
         if (diff > 0)
         {
            hi *= NTL_ZZ_FRADIX;
            lo *= NTL_ZZ_FRADIX;
         }
         try11 = 1;
         try12 = 0;
         try21 = 0;
         try22 = 1;
         parity = 1;
         fast = 1; 
         while (fast > 0)
         {
            parity = 1 - parity;
            if (hi >= NTL_NSP_BOUND)
               fast = 0;
            else
            {
               ilo = (long)lo;
               dirt = hi - double(ilo);
               if (dirt < 1.0/NTL_FDOUBLE_PRECISION || !ilo || ilo < (long)hi)
                  fast = 0;
               else
               {
                  dt = lo-double(ilo);
                  lo = flo / dirt;
                  if (dt > 1.0/NTL_FDOUBLE_PRECISION)
                     hi = fhi / dt;
                  else
                     hi = double(NTL_NSP_BOUND);
                  temp = try11;
                  try11 = try21;
                  if ((NTL_WSP_BOUND - temp) / ilo < try21)
                     fast = 0;
                  else
                     try21 = temp + ilo * try21;
                  temp = try12;
                  try12 = try22;
                  if ((NTL_WSP_BOUND - temp) / ilo < try22)
                     fast = 0;
                  else
                     try22 = temp + ilo * try22;
                  if ((fast > 0) && (parity > 0))
                  {
                     gotthem = 1;
                     got11 = try11;
                     got12 = try12;
                     got21 = try21;
                     got22 = try22;
                  }
               }
            }
         }
      }
      if (gotthem)
      {
         _ntl_gsmul(inv, got11, &x);
         _ntl_gsmul(w, got12, &y);
         _ntl_gsmul(inv, got21, &z);
         _ntl_gsmul(w, got22, &w);
         _ntl_gadd(x, y, &inv);
         _ntl_gadd(z, w, &w);
         _ntl_gsmul(a, got11, &x);
         _ntl_gsmul(n, got12, &y);
         _ntl_gsmul(a, got21, &z);
         _ntl_gsmul(n, got22, &n);
         _ntl_gsub(x, y, &a);
         _ntl_gsub(n, z, &n);
      }
      else
      {
         _ntl_gdiv(a, n, &q, &a);
         _ntl_gmul(q, w, &x);
         _ntl_gadd(inv, x, &inv);
         if (!ZEROP(a))
         {
            _ntl_gdiv(n, a, &q, &n);
            _ntl_gmul(q, inv, &x);
            _ntl_gadd(w, x, &w);
         }
         else
         {
            _ntl_gcopy(n, &a);
            _ntl_gzero(&n);
            _ntl_gcopy(w, &inv);
            _ntl_gnegate(&inv);
         }
      }
   }

   if (_ntl_gscompare(a, 1) == 0)
      e = 0;
   else 
      e = 1;

   _ntl_gcopy(a, uu);
   _ntl_gcopy(inv, invv);

   return (e);
}

void
_ntl_gexteucl(
	_ntl_gbigint aa,
	_ntl_gbigint *xa,
	_ntl_gbigint bb,
	_ntl_gbigint *xb,
	_ntl_gbigint *d
	)
{
   GRegister(modcon);
   GRegister(a);
   GRegister(b);

   long anegative = 0;
   long bnegative = 0;

   _ntl_gcopy(aa, &a);
   _ntl_gcopy(bb, &b);

   if (a && SIZE(a) < 0) {
      anegative = 1;
      SIZE(a) = -SIZE(a);
   }
   else
      anegative = 0;

   if (b && SIZE(b) < 0) {
      bnegative = 1;
      SIZE(b) = -SIZE(b);
   }
   else
      bnegative = 0;


   if (ZEROP(b))
   {
      _ntl_gone(xa);
      _ntl_gzero(xb);
      _ntl_gcopy(a, d);
      goto done;
   }

   if (ZEROP(a))
   {
      _ntl_gzero(xa);
      _ntl_gone(xb);
      _ntl_gcopy(b, d);
      goto done;
   }

   gxxeucl(a, b, xa, d);
   _ntl_gmul(a, *xa, xb);
   _ntl_gsub(*d, *xb, xb);
   _ntl_gdiv(*xb, b, xb, &modcon);

   if (!ZEROP(modcon))
   {
      TerminalError("non-zero remainder in _ntl_gexteucl   BUG");
   }


done:
   if (anegative)
   {
      _ntl_gnegate(xa);
   }
   if (bnegative)
   {
      _ntl_gnegate(xb);
   }
}

long 
_ntl_ginv(
        _ntl_gbigint ain,
        _ntl_gbigint nin,
        _ntl_gbigint *invv
        )
{
        GRegister(u);
        GRegister(v);
        long sgn;


        if (_ntl_gscompare(nin, 1) <= 0) {
                LogicError("InvMod: second input <= 1");
        }

        sgn = _ntl_gsign(ain);
        if (sgn < 0) {
                LogicError("InvMod: first input negative");
        }

        if (_ntl_gcompare(ain, nin) >= 0) {
                LogicError("InvMod: first input too big");
        }


        if (sgn == 0) {
                _ntl_gcopy(nin, invv);
                return 1;
        }

        if (!(gxxeucl(ain, nin, &v, &u))) {
                if (_ntl_gsign(v) < 0) _ntl_gadd(v, nin, &v);
                _ntl_gcopy(v, invv);
                return 0;
        }

        _ntl_gcopy(u, invv);
        return 1;
}



#endif



void
_ntl_ginvmod(
	_ntl_gbigint a,
	_ntl_gbigint n,
	_ntl_gbigint *c
	)
{
	if (_ntl_ginv(a, n, c))
		ArithmeticError("undefined inverse in _ntl_ginvmod");
}

void
_ntl_gaddmod(
	_ntl_gbigint a,
	_ntl_gbigint b,
	_ntl_gbigint n,
	_ntl_gbigint *c
	)
{
	if (*c != n) {
		_ntl_gadd(a, b, c);
		if (_ntl_gcompare(*c, n) >= 0)
			_ntl_gsubpos(*c, n, c);
	}
	else {
                GRegister(mem);

		_ntl_gadd(a, b, &mem);
		if (_ntl_gcompare(mem, n) >= 0)
			_ntl_gsubpos(mem, n, c);
		else
			_ntl_gcopy(mem, c);
	}
}


void
_ntl_gsubmod(
	_ntl_gbigint a,
	_ntl_gbigint b,
	_ntl_gbigint n,
	_ntl_gbigint *c
	)
{
        GRegister(mem);
	long cmp;

	if ((cmp=_ntl_gcompare(a, b)) < 0) {
		_ntl_gadd(n, a, &mem);
		_ntl_gsubpos(mem, b, c);
	} else if (!cmp) 
		_ntl_gzero(c);
	else 
		_ntl_gsubpos(a, b, c);
}

void
_ntl_gsmulmod(
	_ntl_gbigint a,
	long d,
	_ntl_gbigint n,
	_ntl_gbigint *c
	)
{
        GRegister(mem);

	_ntl_gsmul(a, d, &mem);
	_ntl_gmod(mem, n, c);
}



void
_ntl_gmulmod(
	_ntl_gbigint a,
	_ntl_gbigint b,
	_ntl_gbigint n,
	_ntl_gbigint *c
	)
{
        GRegister(mem);

	_ntl_gmul(a, b, &mem);
	_ntl_gmod(mem, n, c);
}

void
_ntl_gsqmod(
	_ntl_gbigint a,
	_ntl_gbigint n,
	_ntl_gbigint *c
	)
{
	_ntl_gmulmod(a, a, n, c);
}


double _ntl_gdoub_aux(_ntl_gbigint n)
{
   double res;
   _ntl_limb_t *ndata;
   long i, sn, nneg;

   if (!n)
      return ((double) 0);

   GET_SIZE_NEG(sn, nneg, n);

   ndata = DATA(n);

   res = 0;
   for (i = sn-1; i >= 0; i--)
      res = res * NTL_ZZ_FRADIX + DBL(ndata[i]);

   if (nneg) res = -res;

   return res;
}

long _ntl_ground_correction(_ntl_gbigint a, long k, long residual)
{
   long direction;
   long p;
   long sgn;
   long bl;
   _ntl_limb_t wh;
   long i;
   _ntl_limb_t *adata;

   if (SIZE(a) > 0)
      sgn = 1;
   else
      sgn = -1;

   adata = DATA(a);

   p = k - 1;
   bl = (p/NTL_ZZ_NBITS);
   wh = ((_ntl_limb_t) 1) << (p - NTL_ZZ_NBITS*bl);

   if (adata[bl] & wh) {
      /* bit is 1...we have to see if lower bits are all 0
         in order to implement "round to even" */

      if (adata[bl] & (wh - ((_ntl_limb_t) 1))) 
         direction = 1;
      else {
         i = bl - 1;
         while (i >= 0 && adata[i] == 0) i--;
         if (i >= 0)
            direction = 1;
         else
            direction = 0;
      }

      /* use residual to break ties */

      if (direction == 0 && residual != 0) {
         if (residual == sgn)
            direction = 1;
         else 
            direction = -1;
      }

      if (direction == 0) {
         /* round to even */

         wh = CLIP(wh << 1); 

         if (wh == 0) {
            wh = 1;
            bl++;
         }

         if (adata[bl] & wh)
            direction = 1;
         else
            direction = -1;
      }
   }
   else
      direction = -1;

   if (direction == 1)
      return sgn;

   return 0;
}




double _ntl_gdoub(_ntl_gbigint n)
{
   GRegister(tmp);

   long s;
   long shamt;
   long correction;
   double x;

   s = _ntl_g2log(n);
   shamt = s - NTL_DOUBLE_PRECISION;

   if (shamt <= 0)
      return _ntl_gdoub_aux(n);

   _ntl_grshift(n, shamt, &tmp);

   correction = _ntl_ground_correction(n, shamt, 0);

   if (correction) _ntl_gsadd(tmp, correction, &tmp);

   x = _ntl_gdoub_aux(tmp);

   x = _ntl_ldexp(x, shamt);

   return x;
}


double _ntl_glog(_ntl_gbigint n)
{
   GRegister(tmp);

   static const double log_2 = log(2.0); // GLOBAL (assumes C++11 thread-safe init)

   long s;
   long shamt;
   long correction;
   double x;

   if (_ntl_gsign(n) <= 0)
      ArithmeticError("log argument <= 0");

   s = _ntl_g2log(n);
   shamt = s - NTL_DOUBLE_PRECISION;

   if (shamt <= 0)
      return log(_ntl_gdoub_aux(n));

   _ntl_grshift(n, shamt, &tmp);

   correction = _ntl_ground_correction(n, shamt, 0);

   if (correction) _ntl_gsadd(tmp, correction, &tmp);

   x = _ntl_gdoub_aux(tmp);

   return log(x) + shamt*log_2;
}



void _ntl_gdoubtoz(double a, _ntl_gbigint *xx)
{
   GRegister(x);

   long neg, i, t, sz;

   a = floor(a);

   if (!IsFinite(&a))
      ArithmeticError("_ntl_gdoubtoz: attempt to convert non-finite value");

   if (a < 0) {
      a = -a;
      neg = 1;
   }
   else
      neg = 0;

   if (a == 0) {
      _ntl_gzero(xx);
      return;
   }

   sz = 0;
   while (a >= 1) {
      a = a*(1.0/double(NTL_NSP_BOUND));
      sz++;
   }

   i = 0;
   _ntl_gzero(&x);

   while (a != 0) {
      i++;
      a = a*double(NTL_NSP_BOUND);
      t = (long) a;
      a = a - t; // NOTE: this subtraction should be exact

      if (i == 1) {
         _ntl_gintoz(t, &x);
      }
      else {
         _ntl_glshift(x, NTL_NSP_NBITS, &x);
         _ntl_gsadd(x, t, &x);
      }
   }

   if (i > sz) TerminalError("bug in _ntl_gdoubtoz");

   _ntl_glshift(x, (sz-i)*NTL_NSP_NBITS, xx);
   if (neg) _ntl_gnegate(xx);
}



/* I've adapted LIP's extended euclidean algorithm to
 * do rational reconstruction.  -- VJS.
 */


long 
_ntl_gxxratrecon(
   _ntl_gbigint ain,
   _ntl_gbigint nin,
   _ntl_gbigint num_bound,
   _ntl_gbigint den_bound,
   _ntl_gbigint *num_out,
   _ntl_gbigint *den_out
   )
{
   GRegister(a);
   GRegister(n);
   GRegister(q);
   GRegister(w);
   GRegister(x);
   GRegister(y);
   GRegister(z);
   GRegister(inv);
   GRegister(u);
   GRegister(a_bak);
   GRegister(n_bak);
   GRegister(inv_bak);
   GRegister(w_bak);

   _ntl_limb_t *p;

   long diff;
   long ilo;
   long sa;
   long sn;
   long snum;
   long sden;
   long e;
   long fast;
   long temp;
   long parity;
   long gotthem;
   long try11;
   long try12;
   long try21;
   long try22;
   long got11;
   long got12;
   long got21;
   long got22;

   double hi;
   double lo;
   double dt;
   double fhi, fhi1;
   double flo, flo1;
   double num;
   double den;
   double dirt;

   if (_ntl_gsign(num_bound) < 0)
      LogicError("rational reconstruction: bad numerator bound");

   if (!num_bound)
      snum = 0;
   else
      snum = SIZE(num_bound);

   if (_ntl_gsign(den_bound) <= 0)
      LogicError("rational reconstruction: bad denominator bound");

   sden = SIZE(den_bound);

   if (_ntl_gsign(nin) <= 0)
      LogicError("rational reconstruction: bad modulus");

   if (_ntl_gsign(ain) < 0 || _ntl_gcompare(ain, nin) >= 0)
      LogicError("rational reconstruction: bad residue");

      
   e = 2+SIZE(nin);

   _ntl_gsetlength(&a, e);
   _ntl_gsetlength(&n, e);
   _ntl_gsetlength(&q, e);
   _ntl_gsetlength(&w, e);
   _ntl_gsetlength(&x, e);
   _ntl_gsetlength(&y, e);
   _ntl_gsetlength(&z, e);
   _ntl_gsetlength(&inv, e);
   _ntl_gsetlength(&u, e);
   _ntl_gsetlength(&a_bak, e);
   _ntl_gsetlength(&n_bak, e);
   _ntl_gsetlength(&inv_bak, e);
   _ntl_gsetlength(&w_bak, e);

   fhi1 = double(1L) + double(32L)/NTL_FDOUBLE_PRECISION;
   flo1 = double(1L) - double(32L)/NTL_FDOUBLE_PRECISION;

   fhi = double(1L) + double(8L)/NTL_FDOUBLE_PRECISION;
   flo = double(1L) - double(8L)/NTL_FDOUBLE_PRECISION;

   _ntl_gcopy(ain, &a);
   _ntl_gcopy(nin, &n);

   _ntl_gone(&inv);
   _ntl_gzero(&w);

   while (1)
   {
      if (SIZE(w) >= sden && _ntl_gcompare(w, den_bound) > 0) break;
      if (SIZE(n) <= snum && _ntl_gcompare(n, num_bound) <= 0) break;

      _ntl_gcopy(a, &a_bak);
      _ntl_gcopy(n, &n_bak);
      _ntl_gcopy(w, &w_bak);
      _ntl_gcopy(inv, &inv_bak);

      gotthem = 0;
      sa = SIZE(a);
      sn = SIZE(n);
      diff = sa - sn;
      if (!diff || diff == 1)
      {
         sa = SIZE(a);
         p = DATA(a) + (sa-1);
         num = DBL(*p) * NTL_ZZ_FRADIX;
         if (sa > 1)
            num += DBL(*(--p));
         num *= NTL_ZZ_FRADIX;
         if (sa > 2)
            num += DBL(*(p - 1));

         sn = SIZE(n);
         p = DATA(n) + (sn-1);
         den = DBL(*p) * NTL_ZZ_FRADIX;
         if (sn > 1)
            den += DBL(*(--p));
         den *= NTL_ZZ_FRADIX;
         if (sn > 2)
            den += DBL(*(p - 1));

         hi = fhi1 * (num + double(1L)) / den;
         lo = flo1 * num / (den + double(1L));
         if (diff > 0)
         {
            hi *= NTL_ZZ_FRADIX;
            lo *= NTL_ZZ_FRADIX;
         }

         try11 = 1;
         try12 = 0;
         try21 = 0;
         try22 = 1;
         parity = 1;
         fast = 1; 
         while (fast > 0)
         {
            parity = 1 - parity;
            if (hi >= NTL_NSP_BOUND)
               fast = 0;
            else
            {
               ilo = (long)lo;
               dirt = hi - double(ilo);
               if (dirt < 1.0/NTL_FDOUBLE_PRECISION || !ilo || ilo < (long)hi)
                  fast = 0;
               else
               {
                  dt = lo-double(ilo);
                  lo = flo / dirt;
                  if (dt > 1.0/NTL_FDOUBLE_PRECISION)
                     hi = fhi / dt;
                  else
                     hi = double(NTL_NSP_BOUND);
                  temp = try11;
                  try11 = try21;
                  if ((NTL_WSP_BOUND - temp) / ilo < try21)
                     fast = 0;
                  else
                     try21 = temp + ilo * try21;
                  temp = try12;
                  try12 = try22;
                  if ((NTL_WSP_BOUND - temp) / ilo < try22)
                     fast = 0;
                  else
                     try22 = temp + ilo * try22;
                  if ((fast > 0) && (parity > 0))
                  {
                     gotthem = 1;
                     got11 = try11;
                     got12 = try12;
                     got21 = try21;
                     got22 = try22;
                  }
               }
            }
         }
      }
      if (gotthem)
      {
         _ntl_gsmul(inv, got11, &x);
         _ntl_gsmul(w, got12, &y);
         _ntl_gsmul(inv, got21, &z);
         _ntl_gsmul(w, got22, &w);
         _ntl_gadd(x, y, &inv);
         _ntl_gadd(z, w, &w);
         _ntl_gsmul(a, got11, &x);
         _ntl_gsmul(n, got12, &y);
         _ntl_gsmul(a, got21, &z);
         _ntl_gsmul(n, got22, &n);
         _ntl_gsub(x, y, &a);
         _ntl_gsub(n, z, &n);
      }
      else
      {
         _ntl_gdiv(a, n, &q, &a);
         _ntl_gmul(q, w, &x);
         _ntl_gadd(inv, x, &inv);
         if (!ZEROP(a))
         {
            _ntl_gdiv(n, a, &q, &n);
            _ntl_gmul(q, inv, &x);
            _ntl_gadd(w, x, &w);
         }
         else
         {
            break;
         }
      }
   }

   _ntl_gcopy(a_bak, &a);
   _ntl_gcopy(n_bak, &n);
   _ntl_gcopy(w_bak, &w);
   _ntl_gcopy(inv_bak, &inv);

   _ntl_gnegate(&w);

   while (1)
   {
      sa = SIZE(w);
      if (sa < 0) SIZE(w) = -sa;
      if (SIZE(w) >= sden && _ntl_gcompare(w, den_bound) > 0) return 0;
      SIZE(w) = sa;

      if (SIZE(n) <= snum && _ntl_gcompare(n, num_bound) <= 0) break;
      
      fast = 0;
      sa = SIZE(a);
      sn = SIZE(n);
      diff = sa - sn;
      if (!diff || diff == 1)
      {
         sa = SIZE(a);
         p = DATA(a) + (sa-1);
         num = DBL(*p) * NTL_ZZ_FRADIX;
         if (sa > 1)
            num += DBL(*(--p));
         num *= NTL_ZZ_FRADIX;
         if (sa > 2)
            num += DBL(*(p - 1));

         sn = SIZE(n);
         p = DATA(n) + (sn-1);
         den = DBL(*p) * NTL_ZZ_FRADIX;
         if (sn > 1)
            den += DBL(*(--p));
         den *= NTL_ZZ_FRADIX;
         if (sn > 2)
            den += DBL(*(p - 1));

         hi = fhi1 * (num + double(1L)) / den;
         lo = flo1 * num / (den + double(1L));
         if (diff > 0)
         {
            hi *= NTL_ZZ_FRADIX;
            lo *= NTL_ZZ_FRADIX;
         }

         if (hi < NTL_NSP_BOUND)
         {
            ilo = (long)lo;
            if (ilo == (long)hi)
               fast = 1;
         }
      }

      if (fast) 
      {
         if (ilo != 0) {
            if (ilo == 1) {
               _ntl_gsub(inv, w, &inv);
               _ntl_gsubpos(a, n, &a);
            }
            else {
               _ntl_gsmul(w, ilo, &x);
               _ntl_gsub(inv, x, &inv);
               _ntl_gsmul(n, ilo, &x);
               _ntl_gsubpos(a, x, &a);
            }
         }
      }
      else {
         _ntl_gdiv(a, n, &q, &a);
         _ntl_gmul(q, w, &x);
         _ntl_gsub(inv, x, &inv);
      }

      _ntl_gswap(&a, &n);
      _ntl_gswap(&inv, &w);
   }

   if (_ntl_gsign(w) < 0) {
      _ntl_gnegate(&w);
      _ntl_gnegate(&n);
   }

   _ntl_gcopy(n, num_out);
   _ntl_gcopy(w, den_out);

   return 1;
}


void
_ntl_gexp(
	_ntl_gbigint a,
	long e,
	_ntl_gbigint *bb
	)
{
	long k;
	long len_a;
        GRegister(res);

	if (!e)
	{
		_ntl_gone(bb);
		return;
	}

	if (e < 0)
		ArithmeticError("negative exponent in _ntl_gexp");

	if (ZEROP(a))
	{
		_ntl_gzero(bb);
		return;
	}

	len_a = _ntl_g2log(a);
	if (len_a > (NTL_MAX_LONG-(NTL_ZZ_NBITS-1))/e)
		ResourceError("overflow in _ntl_gexp");

	_ntl_gsetlength(&res, (len_a*e+NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS);

	_ntl_gcopy(a, &res);
	k = 1;
	while ((k << 1) <= e)
		k <<= 1;
	while (k >>= 1) {
		_ntl_gsq(res, &res);
		if (e & k)
			_ntl_gmul(a, res, &res);
	}

	_ntl_gcopy(res, bb);
}

void
_ntl_gexps(
	long a,
	long e,
	_ntl_gbigint *bb
	)
{
	long k;
	long len_a;
        GRegister(res);

	if (!e)
	{
		_ntl_gone(bb);
		return;
	}

	if (e < 0)
		ArithmeticError("negative exponent in _ntl_zexps");

	if (!a)
	{
		_ntl_gzero(bb);
		return;
	}

	len_a = _ntl_g2logs(a);
	if (len_a > (NTL_MAX_LONG-(NTL_ZZ_NBITS-1))/e)
		ResourceError("overflow in _ntl_gexps");

	_ntl_gsetlength(&res, (len_a*e+NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS);

	_ntl_gintoz(a, &res);
	k = 1;
	while ((k << 1) <= e)
		k <<= 1;
	while (k >>= 1) {
		_ntl_gsq(res, &res);
		if (e & k)
			_ntl_gsmul(res, a, &res);
	}

	_ntl_gcopy(res, bb);
}


static
long OptWinSize(long n)
/* finds k that minimizes n/(k+1) + 2^{k-1} */

{
   long k;
   double v, v_new;


   v = n/2.0 + 1.0;
   k = 1;

   for (;;) {
      v_new = n/((double)(k+2)) + ((double)(1L << k));
      if (v_new >= v) break;
      v = v_new;
      k++;
   }

   return k;
}



static
_ntl_limb_t neg_inv_mod_limb(_ntl_limb_t m0)
{
   _ntl_limb_t x; 
   long k;

   x = 1; 
   k = 1; 
   while (k < NTL_ZZ_NBITS) {
      x += x * (1UL - x * m0); 
      k <<= 1;
   }

   return CLIP(-x); 
}


/* Montgomery reduction:
 * This computes res = T/b^m mod N, where b = 2^{NTL_ZZ_NBITS}.
 * It is assumed that N has n limbs, and that T has at most n + m limbs.
 * Also, inv should be set to -N^{-1} mod b.
 * Finally, it is assumed that T has space allocated for n + m limbs,
 * and that res has space allocated for n limbs.  
 * Note: res should not overlap any inputs, and T is destroyed.
 * Note: res will have at most n limbs, but may not be fully reduced
 * mod N.  In general, we will have res < T/b^m + N.
 */


static
void redc(_ntl_gbigint T, _ntl_gbigint N, long m, _ntl_limb_t inv, 
          _ntl_gbigint res) 
{
   long n, sT, i;
   _ntl_limb_t *Ndata, *Tdata, *resdata, q, d, t, c;

   n = SIZE(N);
   Ndata = DATA(N);
   sT = SIZE(T);
   Tdata = DATA(T);
   resdata = DATA(res);

   for (i = sT; i < m+n; i++)
      Tdata[i] = 0;

   c = 0;
   for (i = 0; i < m; i++) {
      q = CLIP(Tdata[i]*inv);
      d = NTL_MPN(addmul_1)(Tdata+i, Ndata, n, q);

      // (c, Tdata[i+n]) = c + d + Tdata[i+n]
      t = CLIP(Tdata[i+n] + d);
      Tdata[i+n] = CLIP(t + c);
      if (t < d || (c == 1 && CLIP(t + c)  == 0)) 
         c = 1;
      else
         c = 0;
   }

   if (c) {
      NTL_MPN(sub_n)(resdata, Tdata + m, Ndata, n);
   }
   else {
      for (i = 0; i < n; i++)
         resdata[i] = Tdata[m + i];
   }

   i = n;
   STRIP(i, resdata);

   SIZE(res) = i;
   SIZE(T) = 0;
}


// This montgomery code is for external consumption...
// This is currently used in the CRT reconstruction step
// for ZZ_pX arithmetic.  It gives a nontrivial speedup
// for smallish p (up to a few hundred bits)

class _ntl_reduce_struct_montgomery : public _ntl_reduce_struct {
public:
   long m;
   _ntl_limb_t inv;
   _ntl_gbigint_wrapped N;

   void eval(_ntl_gbigint *rres, _ntl_gbigint *TT);
   void adjust(_ntl_gbigint *x);
};




void _ntl_reduce_struct_montgomery::eval(_ntl_gbigint *rres, _ntl_gbigint *TT)
{
   long n, sT, i;
   _ntl_limb_t *Ndata, *Tdata, *resdata, q, d, t, c;
   _ntl_gbigint res, T;


   T = *TT;

   // quick zero test, in case of sparse polynomials
   if (ZEROP(T)) {
      _ntl_gzero(rres);
      return;
   }

   n = SIZE(N);
   Ndata = DATA(N);

   if (MustAlloc(T, m+n)) {
      _ntl_gsetlength(&T, m+n);
      *TT = T;
   }

   res = *rres;
   if (MustAlloc(res, n)) {
      _ntl_gsetlength(&res, n);
      *rres = res;
   }

   sT = SIZE(T);
   Tdata = DATA(T);
   resdata = DATA(res);

   for (i = sT; i < m+n; i++)
      Tdata[i] = 0;

   c = 0;
   for (i = 0; i < m; i++) {
      q = CLIP(Tdata[i]*inv);
      d = NTL_MPN(addmul_1)(Tdata+i, Ndata, n, q);

      // (c, Tdata[i+n]) = c + d + Tdata[i+n]
      t = CLIP(Tdata[i+n] + d);
      Tdata[i+n] = CLIP(t + c);
      if (t < d || (c == 1 && CLIP(t + c)  == 0)) 
         c = 1;
      else
         c = 0;
   }

   if (c || NTL_MPN(cmp)(Tdata + m, Ndata, n) >= 0) {
      NTL_MPN(sub_n)(resdata, Tdata + m, Ndata, n);
   }
   else {
      for (i = 0; i < n; i++)
         resdata[i] = Tdata[m + i];
   }

   i = n;
   STRIP(i, resdata);

   SIZE(res) = i;
   SIZE(T) = 0;
}

// this will adjust the given number by multiplying by the
// montgomery scaling factor

void _ntl_reduce_struct_montgomery::adjust(_ntl_gbigint *x)
{
   GRegister(tmp);
   _ntl_glshift(*x, m*NTL_ZZ_NBITS, &tmp); 
   _ntl_gmod(tmp, N, x);
}




class _ntl_reduce_struct_plain : public _ntl_reduce_struct {
public:
   _ntl_gbigint_wrapped N;

   void eval(_ntl_gbigint *rres, _ntl_gbigint *TT)
   {
      _ntl_gmod(*TT, N, rres);
   }

   void adjust(_ntl_gbigint *x) { }
};

// assumption: all values passed to eval for montgomery reduction
// are in [0, modulus*excess]

_ntl_reduce_struct *
_ntl_reduce_struct_build(_ntl_gbigint modulus, _ntl_gbigint excess)
{
   if (_ntl_godd(modulus)) {
      UniquePtr<_ntl_reduce_struct_montgomery> C;
      C.make();

      C->m = _ntl_gsize(excess);
      C->inv = neg_inv_mod_limb(DATA(modulus)[0]);
      _ntl_gcopy(modulus, &C->N);

      return C.release();
   }
   else {
      UniquePtr<_ntl_reduce_struct_plain> C;
      C.make();

      _ntl_gcopy(modulus, &C->N);

      return C.release();
   }
}


#if (defined(NTL_GMP_LIP) && NTL_NAIL_BITS == 0)
// DIRT: only works with empty nails
// Assumes: F > 1,   0 < g < F,   e > 0

struct wrapped_mpz {
   mpz_t body;

   wrapped_mpz() { mpz_init(body); }
   ~wrapped_mpz() { mpz_clear(body); }
};

static
void _ntl_gmp_powermod(_ntl_gbigint g, _ntl_gbigint e, _ntl_gbigint F,
                       _ntl_gbigint *h)
{
   wrapped_mpz gg;
   wrapped_mpz ee;
   wrapped_mpz FF;
   wrapped_mpz res;

   mpz_import(gg.body, SIZE(g), -1, sizeof(mp_limb_t), 0, 0, DATA(g));
   mpz_import(ee.body, SIZE(e), -1, sizeof(mp_limb_t), 0, 0, DATA(e));
   mpz_import(FF.body, SIZE(F), -1, sizeof(mp_limb_t), 0, 0, DATA(F));

   mpz_powm(res.body, gg.body, ee.body, FF.body);

   if (mpz_sgn(res.body) == 0) {
      _ntl_gzero(h);
      return;
   }

   long sz = mpz_size(res.body);

   _ntl_gsetlength(h, sz);
   _ntl_limb_t *hdata = DATA(*h);
   SIZE(*h) = sz;

   mpz_export(hdata, 0, -1, sizeof(mp_limb_t), 0, 0, res.body);
}


#if 1
// This version avoids memory allocations.
// On 2-limb numbers, it is about 10% faster.

static
void _ntl_gmp_powermod_alt(_ntl_gbigint g, _ntl_gbigint e, _ntl_gbigint F,
                           _ntl_gbigint *h)
{
   NTL_TLS_LOCAL(wrapped_mpz, gg);
   NTL_TLS_LOCAL(wrapped_mpz, ee);
   NTL_TLS_LOCAL(wrapped_mpz, FF);
   NTL_TLS_LOCAL(wrapped_mpz, res);

   mpz_import(gg.body, SIZE(g), -1, sizeof(mp_limb_t), 0, 0, DATA(g));
   mpz_import(ee.body, SIZE(e), -1, sizeof(mp_limb_t), 0, 0, DATA(e));
   mpz_import(FF.body, SIZE(F), -1, sizeof(mp_limb_t), 0, 0, DATA(F));

   mpz_powm(res.body, gg.body, ee.body, FF.body);

   if (mpz_sgn(res.body) == 0) {
      _ntl_gzero(h);
      return;
   }

   long sz = mpz_size(res.body);

   _ntl_gsetlength(h, sz);
   _ntl_limb_t *hdata = DATA(*h);
   SIZE(*h) = sz;

   mpz_export(hdata, 0, -1, sizeof(mp_limb_t), 0, 0, res.body);
}
#endif


#endif

#define REDC_CROSS (32)

void _ntl_gpowermod(_ntl_gbigint g, _ntl_gbigint e, _ntl_gbigint F,
                    _ntl_gbigint *h)

/* h = g^e mod f using "sliding window" algorithm

   remark: the notation (h, g, e, F) is strange, because I
   copied the code from BB.c.
*/

{
 
   
   if (_ntl_gsign(e) < 0 || _ntl_gsign(g) < 0 || _ntl_gcompare(g, F) >= 0 || 
       _ntl_gscompare(F, 1) <= 0) {
      LogicError("PowerMod: bad args");
   }

   if (ZEROP(e)) {
      _ntl_gone(h);
      return;
   }

   if (ONEP(e)) {
      _ntl_gcopy(g, h);
      return;
   }

   if (_ntl_gscompare(e, 2) == 0) {
      _ntl_gsqmod(g, F, h);
      return;
   }

   if (ZEROP(g)) {
      _ntl_gzero(h);
      return;
   }

   long n = _ntl_g2log(e);

#if (1 && defined(NTL_GMP_LIP) && NTL_NAIL_BITS == 0)
   if (n > 10) {
      if (SIZE(F) < 6 && SIZE(e) < 10) 
         _ntl_gmp_powermod_alt(g, e, F, h); 
      else
         _ntl_gmp_powermod(g, e, F, h);
      return;
   }
#endif

   _ntl_gbigint_wrapped res, gg, t;
   UniqueArray<_ntl_gbigint_wrapped> v;

   long i, k, val, cnt, m;
   long use_redc, sF;
   _ntl_limb_t inv;

   sF = SIZE(F);

   res = 0;
   _ntl_gsetlength(&res, sF*2);

   t = 0;
   _ntl_gsetlength(&t, sF*2);

#ifdef NTL_GMP_LIP
   // NOTE: GMP has a fast division routine for larger 
   // numbers, so we only use Montgomery for smallish moduli
   use_redc = (DATA(F)[0] & 1) && sF < REDC_CROSS;
#else
   use_redc = (DATA(F)[0] & 1);
#endif

   gg = 0;

   if (use_redc) {
      _ntl_glshift(g, sF*NTL_ZZ_NBITS, &res);
      _ntl_gmod(res, F, &gg);

      inv = neg_inv_mod_limb(DATA(F)[0]);
   }
   else
      _ntl_gcopy(g, &gg);


   if (_ntl_gscompare(g, 2) == 0) {
      /* plain square-and-multiply algorithm, optimized for g == 2 */

      _ntl_gbigint_wrapped F1;

      if (use_redc) {
         long shamt;

         shamt = COUNT_BITS(DATA(F)[sF-1]);
         shamt = NTL_ZZ_NBITS - shamt;
         _ntl_glshift(F, shamt, &F1);
      }

      _ntl_gcopy(gg, &res);

      for (i = n - 2; i >= 0; i--) {
         _ntl_gsq(res, &t);
         if (use_redc) redc(t, F, sF, inv, res); else _ntl_gmod(t, F, &res);

         if (_ntl_gbit(e, i)) {
            _ntl_gadd(res, res, &res);

            if (use_redc) {
               while (SIZE(res) > sF) {
                  _ntl_gsubpos(res, F1, &res);
               }
            }
            else {
               if (_ntl_gcompare(res, F) >= 0)
                  _ntl_gsubpos(res, F, &res);
            }
         }
      }


      if (use_redc) {
         _ntl_gcopy(res, &t);
         redc(t, F, sF, inv, res);
         if (_ntl_gcompare(res, F) >= 0) {
            _ntl_gsub(res, F, &res);
         }
      }

      _ntl_gcopy(res, h);
      return;
   }


   if (n < 16) { 
      /* plain square-and-multiply algorithm */

      _ntl_gcopy(gg, &res);

      for (i = n - 2; i >= 0; i--) {
         _ntl_gsq(res, &t);
         if (use_redc) redc(t, F, sF, inv, res); else _ntl_gmod(t, F, &res);

         if (_ntl_gbit(e, i)) {
            _ntl_gmul(res, gg, &t);
            if (use_redc) redc(t, F, sF, inv, res); else _ntl_gmod(t, F, &res);
         }
      }


      if (use_redc) {
         _ntl_gcopy(res, &t);
         redc(t, F, sF, inv, res);
         if (_ntl_gcompare(res, F) >= 0) {
            _ntl_gsub(res, F, &res);
         }
      }

      _ntl_gcopy(res, h);
      return;
   }

   k = OptWinSize(n);

   if (k > 5) k = 5;

   v.SetLength(1L << (k-1));
   for (i = 0; i < (1L << (k-1)); i++) {
      v[i] = 0; 
      _ntl_gsetlength(&v[i], sF);
   }

   _ntl_gcopy(gg, &v[0]);
 
   if (k > 1) {
      _ntl_gsq(gg, &t);
      if (use_redc) redc(t, F, sF, inv, res); else _ntl_gmod(t, F, &res);

      for (i = 1; i < (1L << (k-1)); i++) {
         _ntl_gmul(v[i-1], res, &t);
         if (use_redc) redc(t, F, sF, inv, v[i]); else _ntl_gmod(t, F, &v[i]);
      }
   }

   _ntl_gcopy(gg, &res);

   val = 0;
   for (i = n-2; i >= 0; i--) {
      val = (val << 1) | _ntl_gbit(e, i); 
      if (val == 0) {
         _ntl_gsq(res, &t);
         if (use_redc) redc(t, F, sF, inv, res); else _ntl_gmod(t, F, &res);
      }
      else if (val >= (1L << (k-1)) || i == 0) {
         cnt = 0;
         while ((val & 1) == 0) {
            val = val >> 1;
            cnt++;
         }

         m = val;
         while (m > 0) {
            _ntl_gsq(res, &t);
            if (use_redc) redc(t, F, sF, inv, res); else _ntl_gmod(t, F, &res);
            m = m >> 1;
         }

         _ntl_gmul(res, v[val >> 1], &t);
         if (use_redc) redc(t, F, sF, inv, res); else _ntl_gmod(t, F, &res);

         while (cnt > 0) {
            _ntl_gsq(res, &t);
            if (use_redc) redc(t, F, sF, inv, res); else _ntl_gmod(t, F, &res);
            cnt--;
         }

         val = 0;
      }
   }

   if (use_redc) {
      _ntl_gcopy(res, &t);
      redc(t, F, sF, inv, res);
      if (_ntl_gcompare(res, F) >= 0) {
         _ntl_gsub(res, F, &res);
      }
   }

   _ntl_gcopy(res, h);
}


long _ntl_gisone(_ntl_gbigint rep)
{
   return ONEP(rep); 
}

long _ntl_gsptest(_ntl_gbigint rep)
{
   return !rep || SIZE(rep) == 0 ||
          ((SIZE(rep) == 1 || SIZE(rep) == -1) && 
           DATA(rep)[0] < ((_ntl_limb_t) NTL_SP_BOUND));
}

long _ntl_gwsptest(_ntl_gbigint rep)
{
   return !rep || SIZE(rep) == 0 ||
          ((SIZE(rep) == 1 || SIZE(rep) == -1) && 
           DATA(rep)[0] < ((_ntl_limb_t) NTL_WSP_BOUND));
}



long _ntl_gcrtinrange(_ntl_gbigint g, _ntl_gbigint a)
{
   long sa, sg, i; 
   _ntl_limb_t carry, u, v;
   _ntl_limb_t *adata, *gdata;

   if (!a || SIZE(a) <= 0) return 0;

   sa = SIZE(a);

   if (!g) return 1;

   sg = SIZE(g);

   if (sg == 0) return 1;

   if (sg < 0) sg = -sg;

   if (sa-sg > 1) return 1;

   if (sa-sg < 0) return 0;

   adata = DATA(a);
   gdata = DATA(g);

   carry=0;

   if (sa-sg == 1) {
      if (adata[sa-1] > ((_ntl_limb_t) 1)) return 1;
      carry = 1;
   }

   i = sg-1;
   u = 0;
   v = 0;
   while (i >= 0 && u == v) {
      u = (carry << (NTL_ZZ_NBITS-1)) + (adata[i] >> 1);
      v = gdata[i];
      carry = (adata[i] & 1);
      i--;
   }

   if (u == v) {
      if (carry) return 1;
      return (SIZE(g) > 0);
   }
   else
      return (u > v);
}





#if (NTL_NAIL_BITS == 0)

/* DIRT: this routine will not work with non-empty "nails" */
/* and assumes NTL_ZZ_NBITS is a multiple of 8 */

#if (NTL_ZZ_NBITS % 8 != 0)
#error "assumption that NTL_ZZ_NBITS % 8 != 0"
#endif

void _ntl_gfrombytes(_ntl_gbigint *x, const unsigned char *p, long n)
{
   long lw, r, i, j;
   _ntl_limb_t *xp, t;

   while (n > 0 && p[n-1] == 0) n--;
   if (n <= 0) {
      _ntl_gzero(x);
      return;
   }

   const long BytesPerLimb = NTL_ZZ_NBITS/8;


   lw = n/BytesPerLimb;
   r = n - lw*BytesPerLimb;

   if (r != 0) 
      lw++;
   else
      r = BytesPerLimb;

   _ntl_gsetlength(x, lw); 
   xp = DATA(*x);

   for (i = 0; i < lw-1; i++) {
      t = 0;
      for (j = 0; j < BytesPerLimb; j++) {
         t >>= 8;
         t += (((_ntl_limb_t)(*p)) & ((_ntl_limb_t) 255)) << ((BytesPerLimb-1)*8);
         p++;
      }
      xp[i] = t;
   }

   t = 0;
   for (j = 0; j < r; j++) {
      t >>= 8;
      t += (((_ntl_limb_t)(*p)) & ((_ntl_limb_t) 255)) << ((BytesPerLimb-1)*8);
      p++;
   }

   t >>= (BytesPerLimb-r)*8;
   xp[lw-1] = t;

   // strip not necessary here
   // STRIP(lw, xp);
   SIZE(*x) = lw; 
}

void _ntl_gbytesfromz(unsigned char *p, _ntl_gbigint a, long n)
{
   long lbits, lbytes, min_bytes, min_words, r;
   long i, j;
   _ntl_limb_t *ap, t;

   if (n < 0) n = 0;

   const long BytesPerLimb = NTL_ZZ_NBITS/8;

   lbits = _ntl_g2log(a);
   lbytes = (lbits+7)/8;

   min_bytes = (lbytes < n) ? lbytes : n;

   min_words = min_bytes/BytesPerLimb;

   r = min_bytes - min_words*BytesPerLimb;
   if (r != 0)
      min_words++;
   else
      r = BytesPerLimb;

   if (a)
      ap = DATA(a);
   else
      ap = 0;


   for (i = 0; i < min_words-1; i++) {
      t = ap[i];
      for (j = 0; j < BytesPerLimb; j++) {
         *p = t & ((_ntl_limb_t) 255);
         t >>= 8;
         p++;
      }
   }

   if (min_words > 0) {
      t = ap[min_words-1];
      for (j = 0; j < r; j++) {
         *p = t & ((_ntl_limb_t) 255);
         t >>= 8;
         p++;
      }
   }

   for (j = min_bytes; j < n; j++) {
      *p = 0;
      p++;
   }
}



#else

void _ntl_gfrombytes(_ntl_gbigint *x, const unsigned char *p, long n)
{
   long sz;
   long i;
   _ntl_limb_t *xdata;
   _ntl_limb_t carry, tmp;

   long bitpos, wordpos, bitoffset, diff;
   long nbits;

   while (n > 0 && p[n-1] == 0) n--;
   if (n <= 0) {
      _ntl_gzero(x);
      return;
   }

   if (n > (NTL_MAX_LONG-(NTL_ZZ_NBITS-1))/8)
      ResourceError("ZZFromBytes: excessive length");

   nbits = 0;
   tmp = p[n-1];
   while (tmp) {
      tmp >>= 1;
      nbits++;
   }

   sz = ((n-1)*8 + nbits + NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS;

   _ntl_gsetlength(x, sz);

   xdata = DATA(*x);

   for (i = 0; i < sz; i++)
      xdata[i] = 0;

   carry = 0;
   for (i = 0; i < n; i++) {
      bitpos = i*8;
      wordpos = bitpos/NTL_ZZ_NBITS;
      bitoffset = bitpos - wordpos*NTL_ZZ_NBITS;
      diff = NTL_ZZ_NBITS-bitoffset;

      tmp = _ntl_limb_t(p[i]) & _ntl_limb_t(255); 

      xdata[wordpos] |= carry | CLIP(tmp << bitoffset);
      carry = tmp >> diff;
   }

   xdata[sz-1] |= carry;
   SIZE(*x) = sz;
}




void _ntl_gbytesfromz(unsigned char *p, _ntl_gbigint a, long nn)
{
   long k = _ntl_g2log(a);
   long n = (k+7)/8;
   long sz = _ntl_gsize(a);
   long min_n = min(n, nn); 
   _ntl_limb_t *ap;
   long i;


   if (a)
      ap = DATA(a);
   else
      ap = 0;


   for (i = 0; i < min_n; i++) {
      long bitpos = i*8;
      long wordpos = bitpos/NTL_ZZ_NBITS;
      long bitoffset = bitpos - wordpos*NTL_ZZ_NBITS;
      long diff;

      p[i] = (ap[wordpos] >> bitoffset) & _ntl_limb_t(255);

      diff = NTL_ZZ_NBITS - bitoffset;

      if (diff < 8 && wordpos < sz-1) {
         _ntl_limb_t msk = (_ntl_limb_t(1) << (8-diff))-_ntl_limb_t(1);
         p[i] |= ((ap[wordpos+1] & msk) << diff);
      }
   }

   for (i = min_n; i < nn; i++)
      p[i] = 0;
}


#endif





long _ntl_gblock_construct_alloc(_ntl_gbigint *x, long d, long n)
{
   long d1, sz, AllocAmt, m, j, alloc;
   char *p;
   _ntl_gbigint t;


   /* check n value */

   if (n <= 0)
      LogicError("block construct: n must be positive");



   /* check d value */

   if (d <= 0)
      LogicError("block construct: d must be positive");

   if (NTL_OVERFLOW(d, NTL_ZZ_NBITS, NTL_ZZ_NBITS))
      ResourceError("block construct: d too large");

   d1 = d + 1;

#ifdef NTL_SMALL_MP_SIZE_T
   /* this makes sure that numbers don't get too big for GMP */
   if (d1 >= (1L << (NTL_BITS_PER_INT-4)))
      ResourceError("size too big for GMP");
#endif


   if (STORAGE_OVF(d1))
      ResourceError("block construct: d too large");



   sz = STORAGE(d1);

   AllocAmt = NTL_MAX_ALLOC_BLOCK/sz;
   if (AllocAmt == 0) AllocAmt = 1;

   if (AllocAmt < n)
      m = AllocAmt;
   else
      m = n;

   p = (char *) NTL_SNS_MALLOC(m, sz, 0);
   if (!p) MemoryError();

   *x = (_ntl_gbigint) p;

   for (j = 0; j < m; j++) {
      t = (_ntl_gbigint) p;
      alloc = (d1 << 2) | 1;
      if (j < m-1) alloc |= 2;
      ALLOC(t) = alloc;
      SIZE(t) = 0;
      p += sz;
   }

   return m;
}


void _ntl_gblock_construct_set(_ntl_gbigint x, _ntl_gbigint *y, long i)
{
   long d1, sz;


   d1 = ALLOC(x) >> 2;
   sz = STORAGE(d1);

   *y = (_ntl_gbigint) (((char *) x) + i*sz);
}


long _ntl_gblock_destroy(_ntl_gbigint x)
{
   long d1, sz, alloc, m;
   char *p;
   _ntl_gbigint t;

   
   d1 = ALLOC(x) >> 2;
   sz = STORAGE(d1);

   p = (char *) x;

   m = 1;

   for (;;) {
      t = (_ntl_gbigint) p;
      alloc = ALLOC(t);

      // NOTE: this must not throw 
      if ((alloc & 1) == 0) 
         TerminalError("corrupted memory detected in _ntl_gblock_destroy");

      if ((alloc & 2) == 0) break;
      m++;
      p += sz;
   }

   free(x);
   return m;
}


long _ntl_gblock_storage(long d)
{
   long d1, sz; 

   d1 = d + 1;
   sz = STORAGE(d1) + sizeof(_ntl_gbigint);

   return sz;
}



static
long SpecialPower(long e, long p)
{
   long a;
   long x, y;

   a = (long) ((((_ntl_limb_t) 1) << (NTL_ZZ_NBITS-2)) % ((_ntl_limb_t) p));
   a = MulMod(a, 2, p);
   a = MulMod(a, 2, p);

   x = 1;
   y = a;
   while (e) {
      if (e & 1) x = MulMod(x, y, p);
      y = MulMod(y, y, p);
      e = e >> 1;
   }

   return x;
}


static
void sp_ext_eucl(long *dd, long *ss, long *tt, long a, long b)
{
   long  u, v, u0, v0, u1, v1, u2, v2, q, r;

   long aneg = 0, bneg = 0;

   if (a < 0) {
      if (a < -NTL_MAX_LONG) ResourceError("integer overflow");
      a = -a;
      aneg = 1;
   }

   if (b < 0) {
      if (b < -NTL_MAX_LONG) ResourceError("integer overflow");
      b = -b;
      bneg = 1;
   }

   u1=1; v1=0;
   u2=0; v2=1;
   u = a; v = b;

   while (v != 0) {
      q = u / v;
      r = u % v;
      u = v;
      v = r;
      u0 = u2;
      v0 = v2;
      u2 =  u1 - q*u2;
      v2 = v1- q*v2;
      u1 = u0;
      v1 = v0;
   }

   if (aneg)
      u1 = -u1;

   if (bneg)
      v1 = -v1;

   *dd = u;
   *ss = u1;
   *tt = v1;
}

static
long sp_inv_mod(long a, long n)
{
   long d, s, t;

   sp_ext_eucl(&d, &s, &t, a, n);
   if (d != 1) ArithmeticError("inverse undefined");
   if (s < 0)
      return s + n;
   else
      return s;
}




class _ntl_tmp_vec_crt_fast : public  _ntl_tmp_vec {
public:
   UniqueArray<_ntl_gbigint_wrapped> rem_vec;
   UniqueArray<_ntl_gbigint_wrapped> temps;
   UniqueArray<long> val_vec;

};


class _ntl_crt_struct_basic : public _ntl_crt_struct {
public:
   UniqueArray<_ntl_gbigint_wrapped> v;
   long sbuf;
   long n;

   bool special();
   void insert(long i, _ntl_gbigint m);
   _ntl_tmp_vec *extract();
   _ntl_tmp_vec *fetch();
   void eval(_ntl_gbigint *x, const long *b, _ntl_tmp_vec *tmp_vec);
};


#if (defined(NTL_TBL_CRT))

class _ntl_crt_struct_tbl : public _ntl_crt_struct {
public:
   Unique2DArray<_ntl_limb_t> v;
   long n;
   long sz;

   bool special();
   void insert(long i, _ntl_gbigint m);
   _ntl_tmp_vec *extract();
   _ntl_tmp_vec *fetch();
   void eval(_ntl_gbigint *x, const long *b, _ntl_tmp_vec *tmp_vec);

};

#endif




class _ntl_crt_struct_fast : public _ntl_crt_struct {
public:
   long n;
   long levels;
   UniqueArray<long> primes;
   UniqueArray<long> inv_vec;
   UniqueArray<long> index_vec;
   UniqueArray<_ntl_gbigint_wrapped> prod_vec;
   UniqueArray<_ntl_gbigint_wrapped> coeff_vec;
   _ntl_gbigint_wrapped modulus;
   UniquePtr<_ntl_tmp_vec_crt_fast> stored_tmp_vec;

   bool special();
   void insert(long i, _ntl_gbigint m);
   _ntl_tmp_vec *extract();
   _ntl_tmp_vec *fetch();
   void eval(_ntl_gbigint *x, const long *b, _ntl_tmp_vec *tmp_vec);
};






#define GCRT_TMPS (2)


_ntl_crt_struct * 
_ntl_crt_struct_build(long n, _ntl_gbigint p, long (*primes)(long))
{
#ifdef NTL_GMP_LIP
   if (n > 800)
#else
   if (0)
   // NOTE: without GMP, this does not seem to help
#endif
   {
      UniqueArray<long> q;
      UniqueArray<long> inv_vec;
      UniqueArray<long> index_vec;
      UniqueArray<_ntl_gbigint_wrapped> prod_vec, rem_vec, coeff_vec;
      UniqueArray<_ntl_gbigint_wrapped> temps;

      long i, j;
      long levels, vec_len;

      levels = 0;
      while ((n >> levels) >= 16) levels++;
      vec_len = (1L << levels) - 1;

      temps.SetLength(GCRT_TMPS);
      rem_vec.SetLength(vec_len);

      q.SetLength(n);
      for (i = 0; i < n; i++)
         q[i] = primes(i);

      inv_vec.SetLength(n);


      index_vec.SetLength(vec_len+1);
      prod_vec.SetLength(vec_len);
      coeff_vec.SetLength(n);

      index_vec[0] = 0;
      index_vec[1] = n;

      for (i = 0; i <= levels-2; i++) {
         long start = (1L << i) - 1;
         long finish = (1L << (i+1)) - 2;
         for (j = finish; j >= start; j--) {
            index_vec[2*j+2] = index_vec[j] + (index_vec[j+1] - index_vec[j])/2;
            index_vec[2*j+1] = index_vec[j];
         }
         index_vec[2*finish+3] = n;
      }

      for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
         /* multiply primes index_vec[i]..index_vec[i+1]-1 into 
          * prod_vec[i]
          */

         _ntl_gone(&prod_vec[i]);
         for (j = index_vec[i]; j < index_vec[i+1]; j++)
            _ntl_gsmul(prod_vec[i], q[j], &prod_vec[i]);
      }

      for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
         for (j = index_vec[i]; j < index_vec[i+1]; j++)
            _ntl_gsdiv(prod_vec[i], q[j], &coeff_vec[j]);
      }

      for (i = (1L << (levels-1)) - 2; i >= 0; i--)
         _ntl_gmul(prod_vec[2*i+1], prod_vec[2*i+2], &prod_vec[i]);

     /*** new asymptotically fast code to compute inv_vec ***/

      _ntl_gone(&rem_vec[0]);
      for (i = 0; i < (1L << (levels-1)) - 1; i++) {
         _ntl_gmod(rem_vec[i], prod_vec[2*i+1], &temps[0]);
         _ntl_gmul(temps[0], prod_vec[2*i+2], &temps[1]);
         _ntl_gmod(temps[1], prod_vec[2*i+1], &rem_vec[2*i+1]);

         _ntl_gmod(rem_vec[i], prod_vec[2*i+2], &temps[0]);
         _ntl_gmul(temps[0], prod_vec[2*i+1], &temps[1]);
         _ntl_gmod(temps[1], prod_vec[2*i+2], &rem_vec[2*i+2]);
      }

      for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
         for (j = index_vec[i]; j < index_vec[i+1]; j++) {
            long tt, tt1, tt2;
            _ntl_gsdiv(prod_vec[i], q[j], &temps[0]);
            tt = _ntl_gsmod(temps[0], q[j]);
            tt1 = _ntl_gsmod(rem_vec[i], q[j]);
            tt2 = MulMod(tt, tt1, q[j]);
            inv_vec[j] = sp_inv_mod(tt2, q[j]);
         }
      }


      UniquePtr<_ntl_crt_struct_fast> C;
      C.make();

      C->n = n;
      C->primes.move(q);
      C->inv_vec.move(inv_vec);
      C->levels = levels;
      C->index_vec.move(index_vec);
      C->prod_vec.move(prod_vec);
      C->coeff_vec.move(coeff_vec);

      _ntl_gcopy(p, &C->modulus);

      C->stored_tmp_vec.make();
      C->stored_tmp_vec->rem_vec.move(rem_vec);
      C->stored_tmp_vec->temps.move(temps);
      C->stored_tmp_vec->val_vec.SetLength(n);

      return C.release();
   }


#if (defined(NTL_TBL_CRT))

// assert: defined(NTL_CRT_ALTCODE) ||  defined(NTL_CRT_ALTCODE_SMALL)
// we use the alternative CRT code, either unconditionally,
// or only for small moduli.

#if (!defined(NTL_CRT_ALTCODE)) 
   if (n <= 16)
#endif
   {
      UniquePtr<_ntl_crt_struct_tbl> C;
      C.make();
      C->n = n;
      C->sz = SIZE(p);
      C->v.SetDims(C->sz, C->n);

      return C.release();
   }
#endif

// as a fallback, we use the basic CRT code

   {
      UniquePtr<_ntl_crt_struct_basic> C;
      C.make();


      C->n = n;
      C->v.SetLength(n);
      C->sbuf = SIZE(p)+2;

      return C.release();
   }

}

/* extracts existing tmp_vec, if possible -- read/write operation */

_ntl_tmp_vec *_ntl_crt_struct_basic::extract()
{
   return 0;
}

#if (defined(NTL_TBL_CRT))
_ntl_tmp_vec *_ntl_crt_struct_tbl::extract()
{
   return 0;
}
#endif

_ntl_tmp_vec *_ntl_crt_struct_fast::extract()
{
   if (stored_tmp_vec) 
      return stored_tmp_vec.release();
   else
      return fetch();
}


/* read only operation */

_ntl_tmp_vec *_ntl_crt_struct_basic::fetch()
{
   return 0;
}

#if (defined(NTL_TBL_CRT))
_ntl_tmp_vec *_ntl_crt_struct_tbl::fetch()
{
   return 0;
}
#endif

_ntl_tmp_vec *_ntl_crt_struct_fast::fetch()
{
   long vec_len = (1L << levels) - 1;

   UniquePtr<_ntl_tmp_vec_crt_fast> res;
   res.make();
   res->temps.SetLength(GCRT_TMPS);
   res->rem_vec.SetLength(vec_len);
   res->val_vec.SetLength(n);

   return res.release();
}


void _ntl_crt_struct_basic::insert(long i, _ntl_gbigint m)
{
   _ntl_gcopy(m, &v[i]);
}

#if (defined(NTL_TBL_CRT))
void _ntl_crt_struct_tbl::insert(long i, _ntl_gbigint m)
{
   if (i < 0 || i >= n) LogicError("insert: bad args");

   if (!m) 
      for (long j = 0; j < sz; j++) v[j][i] = 0;
   else {
      long sm = SIZE(m);
      if (sm < 0 || sm > sz) LogicError("insert: bad args");
      const _ntl_limb_t *mdata = DATA(m);
      for (long j = 0; j < sm; j++) 
         v[j][i] = mdata[j];
      for (long j = sm; j < sz; j++)
         v[j][i] = 0;
   }
}
#endif

void _ntl_crt_struct_fast::insert(long i, _ntl_gbigint m)
{
   LogicError("insert called improperly");
}


void _ntl_crt_struct_basic::eval(_ntl_gbigint *x, const long *b, _ntl_tmp_vec *generic_tmp_vec)
{
   _ntl_limb_t *xx, *yy; 
   _ntl_gbigint x1;
   long i, sx;
   long sy;
   _ntl_limb_t carry;

   sx = sbuf;
   _ntl_gsetlength(x, sx);
   x1 = *x;
   xx = DATA(x1);

   for (i = 0; i < sx; i++)
      xx[i] = 0;

   for (i = 0; i < n; i++) {
      if (!v[i]) continue;

      yy = DATA(v[i]);
      sy = SIZE(v[i]); 

      if (!sy || !b[i]) continue;

      carry = NTL_MPN(addmul_1)(xx, yy, sy, b[i]);
      yy = xx + sy;
      *yy = CLIP(*yy + carry);

      if (*yy < carry) { /* unsigned comparison! */
         do {
            yy++;
            *yy = CLIP(*yy + 1);
         } while (*yy == 0);
      }
   }

   STRIP(sx, xx);
   SIZE(x1) = sx;
}


#if (defined(NTL_TBL_CRT))

#define CRT_ALTCODE_UNROLL (1)

void _ntl_crt_struct_tbl::eval(_ntl_gbigint *x, const long *b, _ntl_tmp_vec *generic_tmp_vec)
{
   long sx;
   _ntl_gbigint x1;
   long i, j;

   // quick test for zero vector
   // most likely, they are either all zero (if we are working 
   // with some sparse polynomials) or none of them are zero,
   // so in the general case, this should go fast
   if (!b[0]) {
      i = 1;
      while (i < n && !b[i]) i++;
      if (i >= n) {
         _ntl_gzero(x);
         return;
      }
   }

   sx = sz + 2;
   _ntl_gsetlength(x, sx);
   x1 = *x;
   _ntl_limb_t * NTL_RESTRICT xx = DATA(x1);


   const long Bnd = 1L << (NTL_BITS_PER_LONG-NTL_SP_NBITS);

   if (n <= Bnd) {
      _ntl_limb_t carry=0;

      for (i = 0; i < sz; i++) {
         const _ntl_limb_t *row = v[i];

         ll_type acc;
         ll_mul(acc, row[0], b[0]);

#if (CRT_ALTCODE_UNROLL && NTL_BITS_PER_LONG-NTL_SP_NBITS == 4)
         switch (n) {
         case 16: ll_mul_add(acc, row[16-1], b[16-1]);
         case 15: ll_mul_add(acc, row[15-1], b[15-1]);
         case 14: ll_mul_add(acc, row[14-1], b[14-1]);
         case 13: ll_mul_add(acc, row[13-1], b[13-1]);
         case 12: ll_mul_add(acc, row[12-1], b[12-1]);
         case 11: ll_mul_add(acc, row[11-1], b[11-1]);
         case 10: ll_mul_add(acc, row[10-1], b[10-1]);
         case 9: ll_mul_add(acc, row[9-1], b[9-1]);
         case 8: ll_mul_add(acc, row[8-1], b[8-1]);
         case 7: ll_mul_add(acc, row[7-1], b[7-1]);
         case 6: ll_mul_add(acc, row[6-1], b[6-1]);
         case 5: ll_mul_add(acc, row[5-1], b[5-1]);
         case 4: ll_mul_add(acc, row[4-1], b[4-1]);
         case 3: ll_mul_add(acc, row[3-1], b[3-1]);
         case 2: ll_mul_add(acc, row[2-1], b[2-1]);
         }
#elif (CRT_ALTCODE_UNROLL)
         long j = n;
         for (; j > 16; j -= 16) {
            ll_mul_add(acc, row[j-1], b[j-1]);
            ll_mul_add(acc, row[j-2], b[j-2]);
            ll_mul_add(acc, row[j-3], b[j-3]);
            ll_mul_add(acc, row[j-4], b[j-4]);
            ll_mul_add(acc, row[j-5], b[j-5]);
            ll_mul_add(acc, row[j-6], b[j-6]);
            ll_mul_add(acc, row[j-7], b[j-7]);
            ll_mul_add(acc, row[j-8], b[j-8]);
            ll_mul_add(acc, row[j-9], b[j-9]);
            ll_mul_add(acc, row[j-10], b[j-10]);
            ll_mul_add(acc, row[j-11], b[j-11]);
            ll_mul_add(acc, row[j-12], b[j-12]);
            ll_mul_add(acc, row[j-13], b[j-13]);
            ll_mul_add(acc, row[j-14], b[j-14]);
            ll_mul_add(acc, row[j-15], b[j-15]);
            ll_mul_add(acc, row[j-16], b[j-16]);
         }
         switch (j) {
         case 16:  ll_mul_add(acc, row[16-1], b[16-1]);
         case 15:  ll_mul_add(acc, row[15-1], b[15-1]);
         case 14:  ll_mul_add(acc, row[14-1], b[14-1]);
         case 13:  ll_mul_add(acc, row[13-1], b[13-1]);
         case 12:  ll_mul_add(acc, row[12-1], b[12-1]);
         case 11:  ll_mul_add(acc, row[11-1], b[11-1]);
         case 10:  ll_mul_add(acc, row[10-1], b[10-1]);
         case 9:  ll_mul_add(acc, row[9-1], b[9-1]);
         case 8:  ll_mul_add(acc, row[8-1], b[8-1]);
         case 7:  ll_mul_add(acc, row[7-1], b[7-1]);
         case 6:  ll_mul_add(acc, row[6-1], b[6-1]);
         case 5:  ll_mul_add(acc, row[5-1], b[5-1]);
         case 4:  ll_mul_add(acc, row[4-1], b[4-1]);
         case 3:  ll_mul_add(acc, row[3-1], b[3-1]);
         case 2:  ll_mul_add(acc, row[2-1], b[2-1]);
         }

#else
         for (j = 1; j < n; j++) 
            ll_mul_add(acc, row[j], b[j]);
#endif

         ll_add(acc, carry);
         xx[i] = ll_get_lo(acc);
         carry = ll_get_hi(acc);
      }

      xx[sz] = carry;
      xx[sz+1] = 0;
   }
   else {
      ll_type carry;
      ll_init(carry, 0);

      for (i = 0; i < sz; i++) {
         const _ntl_limb_t *row = v[i];

         ll_type acc21;
         _ntl_limb_t acc0;

         {
            ll_type sum;
            ll_mul(sum, row[0], b[0]);

#if (CRT_ALTCODE_UNROLL && NTL_BITS_PER_LONG-NTL_SP_NBITS == 4)
            ll_mul_add(sum, row[1], b[1]);
            ll_mul_add(sum, row[2], b[2]);
            ll_mul_add(sum, row[3], b[3]);
            ll_mul_add(sum, row[4], b[4]);
            ll_mul_add(sum, row[5], b[5]);
            ll_mul_add(sum, row[6], b[6]);
            ll_mul_add(sum, row[7], b[7]);
            ll_mul_add(sum, row[8], b[8]);
            ll_mul_add(sum, row[9], b[9]);
            ll_mul_add(sum, row[10], b[10]);
            ll_mul_add(sum, row[11], b[11]);
            ll_mul_add(sum, row[12], b[12]);
            ll_mul_add(sum, row[13], b[13]);
            ll_mul_add(sum, row[14], b[14]);
            ll_mul_add(sum, row[15], b[15]);
#elif (CRT_ALTCODE_UNROLL && NTL_BITS_PER_LONG-NTL_SP_NBITS == 2)
            ll_mul_add(sum, row[1], b[1]);
            ll_mul_add(sum, row[2], b[2]);
            ll_mul_add(sum, row[3], b[3]);
#else
            for (j = 1; j < Bnd; j++)
               ll_mul_add(sum, row[j], b[j]);
#endif

       
            ll_init(acc21, ll_get_hi(sum));
            acc0 = ll_get_lo(sum);
         }

         const _ntl_limb_t *ap = row;
         const long *tp = b;

#if (CRT_ALTCODE_UNROLL && NTL_BITS_PER_LONG-NTL_SP_NBITS == 2)
         long m = n - 4;
         ap += 4;
         tp += 4;

         for (; m >= 8; m -= 8, ap += 8, tp += 8) {
            {
               ll_type sum;
               ll_mul(sum, ap[0], tp[0]);
               ll_mul_add(sum, ap[1], tp[1]);
               ll_mul_add(sum, ap[2], tp[2]);
               ll_mul_add(sum, ap[3], tp[3]);

               ll_add(sum, acc0);
               acc0 = ll_get_lo(sum);
               ll_add(acc21, ll_get_hi(sum));
            }
            {
               ll_type sum;
               ll_mul(sum, ap[4+0], tp[4+0]);
               ll_mul_add(sum, ap[4+1], tp[4+1]);
               ll_mul_add(sum, ap[4+2], tp[4+2]);
               ll_mul_add(sum, ap[4+3], tp[4+3]);

               ll_add(sum, acc0);
               acc0 = ll_get_lo(sum);
               ll_add(acc21, ll_get_hi(sum));
            }
         }

         for (; m >= 4; m -= 4, ap += 4, tp += 4) {
	    ll_type sum;
	    ll_mul(sum, ap[0], tp[0]);
            ll_mul_add(sum, ap[1], tp[1]);
            ll_mul_add(sum, ap[2], tp[2]);
            ll_mul_add(sum, ap[3], tp[3]);

	    ll_add(sum, acc0);
	    acc0 = ll_get_lo(sum);
	    ll_add(acc21, ll_get_hi(sum));
         }


#else
         long m;
         for (m = n-Bnd, ap += Bnd, tp += Bnd; m >= Bnd; m -= Bnd, ap += Bnd, tp += Bnd) {

	    ll_type sum;
	    ll_mul(sum, ap[0], tp[0]);

#if (CRT_ALTCODE_UNROLL && NTL_BITS_PER_LONG-NTL_SP_NBITS == 4)
            ll_mul_add(sum, ap[1], tp[1]);
            ll_mul_add(sum, ap[2], tp[2]);
            ll_mul_add(sum, ap[3], tp[3]);
            ll_mul_add(sum, ap[4], tp[4]);
            ll_mul_add(sum, ap[5], tp[5]);
            ll_mul_add(sum, ap[6], tp[6]);
            ll_mul_add(sum, ap[7], tp[7]);
            ll_mul_add(sum, ap[8], tp[8]);
            ll_mul_add(sum, ap[9], tp[9]);
            ll_mul_add(sum, ap[10], tp[10]);
            ll_mul_add(sum, ap[11], tp[11]);
            ll_mul_add(sum, ap[12], tp[12]);
            ll_mul_add(sum, ap[13], tp[13]);
            ll_mul_add(sum, ap[14], tp[14]);
            ll_mul_add(sum, ap[15], tp[15]);
#else
            for (long j = 1; j < Bnd; j++)
               ll_mul_add(sum, ap[j], tp[j]);
#endif

	    ll_add(sum, acc0);
	    acc0 = ll_get_lo(sum);
	    ll_add(acc21, ll_get_hi(sum));
         }
#endif

         if (m > 0) {
	    ll_type sum;
	    ll_mul(sum, ap[0], tp[0]);

#if (CRT_ALTCODE_UNROLL && NTL_BITS_PER_LONG-NTL_SP_NBITS == 4)
            switch (m) {
            case 15:  ll_mul_add(sum, ap[15-1], tp[15-1]);
            case 14:  ll_mul_add(sum, ap[14-1], tp[14-1]);
            case 13:  ll_mul_add(sum, ap[13-1], tp[13-1]);
            case 12:  ll_mul_add(sum, ap[12-1], tp[12-1]);
            case 11:  ll_mul_add(sum, ap[11-1], tp[11-1]);
            case 10:  ll_mul_add(sum, ap[10-1], tp[10-1]);
            case 9:  ll_mul_add(sum, ap[9-1], tp[9-1]);
            case 8:  ll_mul_add(sum, ap[8-1], tp[8-1]);
            case 7:  ll_mul_add(sum, ap[7-1], tp[7-1]);
            case 6:  ll_mul_add(sum, ap[6-1], tp[6-1]);
            case 5:  ll_mul_add(sum, ap[5-1], tp[5-1]);
            case 4:  ll_mul_add(sum, ap[4-1], tp[4-1]);
            case 3:  ll_mul_add(sum, ap[3-1], tp[3-1]);
            case 2:  ll_mul_add(sum, ap[2-1], tp[2-1]);
            }
#else
            for (m--, ap++, tp++; m > 0; m--, ap++, tp++)
               ll_mul_add(sum, ap[0], tp[0]);
#endif
	    ll_add(sum, acc0);
	    acc0 = ll_get_lo(sum);
	    ll_add(acc21, ll_get_hi(sum));

         }

         ll_add(carry, acc0);
         xx[i] = ll_get_lo(carry);
         ll_add(acc21, ll_get_hi(carry));
         carry = acc21;
      }

      xx[sz] = ll_get_lo(carry);
      xx[sz+1] = ll_get_hi(carry);
   }


   STRIP(sx, xx);
   SIZE(x1) = sx;
}
#endif

static
void gadd_mul_many(_ntl_gbigint *res, _ntl_gbigint *a, long *b, 
                      long n, long sz)

{
   _ntl_limb_t *xx, *yy; 
   long i, sx;
   long sy;
   _ntl_limb_t carry;

   sx = sz + 2;
   if (MustAlloc(*res, sx))
      _ntl_gsetlength(res, sx);

   xx = DATA(*res);

   for (i = 0; i < sx; i++)
      xx[i] = 0;

   for (i = 0; i < n; i++) {
      if (!a[i]) continue;

      yy = DATA(a[i]);
      sy = SIZE(a[i]); 

      if (!sy || !b[i]) continue;

      carry = NTL_MPN(addmul_1)(xx, yy, sy, b[i]);
      yy = xx + sy;
      *yy = CLIP(*yy + carry);

      if (*yy < carry) { /* unsigned comparison! */
         do {
            yy++;
            *yy = CLIP(*yy + 1);
         } while (*yy == 0);
      }
   }

   STRIP(sx, xx);
   SIZE(*res) = sx;
}

void _ntl_crt_struct_fast::eval(_ntl_gbigint *x, const long *b, _ntl_tmp_vec *generic_tmp_vec)
{
   _ntl_tmp_vec_crt_fast *tmp_vec = static_cast<_ntl_tmp_vec_crt_fast*> (generic_tmp_vec);

   long *val_vec = tmp_vec->val_vec.get();
   _ntl_gbigint_wrapped *temps = tmp_vec->temps.get();
   _ntl_gbigint_wrapped *rem_vec = tmp_vec->rem_vec.get();

   long vec_len = (1L << levels) - 1;

   long i;

   for (i = 0; i < n; i++) {
      val_vec[i] = MulMod(b[i], inv_vec[i], primes[i]);
   }

   for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
      long j1 = index_vec[i];
      long j2 = index_vec[i+1];
      gadd_mul_many(&rem_vec[i], &coeff_vec[j1], &val_vec[j1], j2-j1, 
                       SIZE(prod_vec[i]));
   }

   for (i = (1L << (levels-1)) - 2; i >= 0; i--) {
      _ntl_gmul(prod_vec[2*i+1], rem_vec[2*i+2], &temps[0]);
      _ntl_gmul(rem_vec[2*i+1], prod_vec[2*i+2], &temps[1]);
      _ntl_gadd(temps[0], temps[1], &rem_vec[i]);
   }

   /* temps[0] = rem_vec[0] mod prod_vec[0] (least absolute residue) */
   _ntl_gmod(rem_vec[0], prod_vec[0], &temps[0]);
   _ntl_gsub(temps[0], prod_vec[0], &temps[1]);
   _ntl_gnegate(&temps[1]);
   if (_ntl_gcompare(temps[0], temps[1]) > 0) {
      _ntl_gnegate(&temps[1]);
      _ntl_gcopy(temps[1], &temps[0]);
   }

   _ntl_gmod(temps[0], modulus, &temps[1]);
   _ntl_gcopy(temps[1], x);
}


bool _ntl_crt_struct_basic::special()  { return false; }

#if (defined(NTL_TBL_CRT))
bool _ntl_crt_struct_tbl::special()  { return false; }
#endif


bool _ntl_crt_struct_fast::special()   { return true; }



// ************** rem code


#ifdef NTL_HAVE_LL_TYPE

// This is the same logic as in sp_arith.h, but assumes 
// NumBits(d) == NTL_SP_NBITS


static inline
unsigned long tbl_red_inv(long d)
{
   return (unsigned long) ( ((_ntl_ulonglong(1) << (NTL_SP_NBITS+NTL_BITS_PER_LONG))-1UL) / _ntl_ulonglong(d) );
}

// assumes hi < d
static inline 
long tbl_red_21(unsigned long hi, unsigned long lo, long d, unsigned long dinv)
{
   unsigned long H = (hi << (NTL_BITS_PER_LONG-NTL_SP_NBITS)) | (lo >> NTL_SP_NBITS);
   unsigned long Q = ll_mul_hi(H, dinv) + H;
   unsigned long rr = lo - Q*cast_unsigned(d); // rr in [0..4*d)
   long r = sp_CorrectExcess(rr, 2*d); // r in [0..2*d)
   r = sp_CorrectExcess(r, d);
   return r;
}


// assumes x2 < d
static inline
unsigned long tbl_red_31(unsigned long x2, unsigned long x1, unsigned long x0,
                     long d, unsigned long dinv)
{
   long carry = tbl_red_21(x2, x1, d, dinv);
   return tbl_red_21(carry, x0, d, dinv);
}

#endif


class _ntl_tmp_vec_rem_impl : public  _ntl_tmp_vec {
public:
   UniqueArray<_ntl_gbigint_wrapped> rem_vec;
};






class _ntl_rem_struct_basic : public _ntl_rem_struct {
public:
   long n;
   UniqueArray<long> primes;

   void eval(long *x, _ntl_gbigint a, _ntl_tmp_vec *tmp_vec);
   _ntl_tmp_vec *fetch();
};


class _ntl_rem_struct_fast : public _ntl_rem_struct {
public:
   long n;
   long levels;
   UniqueArray<long> primes;
   UniqueArray<long> index_vec;
   UniqueArray<_ntl_gbigint_wrapped> prod_vec;
   long modulus_size;

   void eval(long *x, _ntl_gbigint a, _ntl_tmp_vec *tmp_vec);
   _ntl_tmp_vec *fetch();
};


class _ntl_rem_struct_medium : public _ntl_rem_struct {
public:
   long n;
   long levels;
   UniqueArray<long> primes;
   UniqueArray<long> index_vec;
   UniqueArray<long> len_vec;
   UniqueArray<_ntl_limb_t> inv_vec;
   UniqueArray<long> corr_vec;
   UniqueArray<mulmod_precon_t> corraux_vec;
   UniqueArray<_ntl_gbigint_wrapped> prod_vec;

   void eval(long *x, _ntl_gbigint a, _ntl_tmp_vec *tmp_vec);
   _ntl_tmp_vec *fetch();
};



#ifdef NTL_TBL_REM


#define NTL_GAP_BITS (2*NTL_BITS_PER_LONG-NTL_SP_NBITS-NTL_ZZ_NBITS)

// NOTE: do not allow NTL_GAP_BITS to exceed 28.
// This is largely academic, but it avoids some potential
// integer overflow issues.
#if (NTL_GAP_BITS > 28)
#undef NTL_GAP_BITS
#define NTL_GAP_BITS (28)
#endif


class _ntl_rem_struct_tbl : public _ntl_rem_struct {
public:
   long n;
   UniqueArray<long> primes;
   UniqueArray<_ntl_limb_t> inv_primes;
   Unique2DArray<_ntl_limb_t> tbl;

   void eval(long *x, _ntl_gbigint a, _ntl_tmp_vec *tmp_vec);
   _ntl_tmp_vec *fetch();

};

#endif



_ntl_rem_struct *_ntl_rem_struct_build(long n, _ntl_gbigint modulus, long (*p)(long))
{

#ifdef NTL_TBL_REM

// FIXME: I should incorporate the logic from _ntl_general_rem_one_struct_apply
// to keep the table sizes smaller

#ifdef NTL_GMP_LIP
   if (n <= 800) 
#else
   if (1) 
   // NOTE: without GMP, this is always the fastest
#endif
   {
      UniqueArray<long> q;
      UniqueArray<_ntl_limb_t> inv_primes;
      Unique2DArray<_ntl_limb_t> tbl;
      long i, j;
      long qq, t, t1;
      long sz = SIZE(modulus);

      q.SetLength(n);
      for (i = 0; i < n; i++)
         q[i] = p(i);

      inv_primes.SetLength(n);
      for (i = 0; i < n; i++) 
         inv_primes[i] = tbl_red_inv(q[i]);

      tbl.SetDims(n, sz);

      for (i = 0; i < n; i++) {
         qq = q[i];
         t = 1;
         for (j = 0; j < NTL_ZZ_NBITS; j++) {
            t += t;
            if (t >= qq) t -= qq;
         }
         t1 = 1;
         tbl[i][0] = 1;
         for (j = 1; j < sz; j++) {
            t1 = MulMod(t1, t, qq);
            tbl[i][j] = t1;
         }
      }

      UniquePtr<_ntl_rem_struct_tbl> R;
      R.make();
 
      R->n = n;
      R->primes.move(q);
      R->inv_primes.move(inv_primes);
      R->tbl.move(tbl);

      return R.release();
   }
#endif

#ifdef NTL_GMP_LIP
   if (0)
   // NOTE: this does not seem useful with GMP
#else
   if (n > 600)
   // NOTE: this seems to be useful without GMP, but only if TBL_REM
   // does not work
#endif
   {

      UniqueArray<long> q;
      long i, j;
      long levels, vec_len;
      UniqueArray<long> index_vec;
      UniqueArray<long> len_vec, corr_vec;
      UniqueArray<mulmod_precon_t> corraux_vec;
      UniqueArray<_ntl_limb_t> inv_vec;
      UniqueArray<_ntl_gbigint_wrapped> prod_vec;

   
      q.SetLength(n);
      for (i = 0; i < n; i++)
         q[i] = p(i);

      levels = 0;
      while ((n >> levels) >= 4) levels++;

      vec_len = (1L << levels) - 1;

      index_vec.SetLength(vec_len+1);
      len_vec.SetLength(vec_len);
      inv_vec.SetLength(vec_len);

      corr_vec.SetLength(n);
      corraux_vec.SetLength(n);

      prod_vec.SetLength(vec_len);

      index_vec[0] = 0;
      index_vec[1] = n;

      for (i = 0; i <= levels-2; i++) {
         long start = (1L << i) - 1;
         long finish = (1L << (i+1)) - 2;
         for (j = finish; j >= start; j--) {
            index_vec[2*j+2] = index_vec[j] + (index_vec[j+1] - index_vec[j])/2;
            index_vec[2*j+1] = index_vec[j];
         }
         index_vec[2*finish+3] = n;
      }

      for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
         /* multiply primes index_vec[i]..index_vec[i+1]-1 into 
          * prod_vec[i]
          */

         _ntl_gone(&prod_vec[i]);
         for (j = index_vec[i]; j < index_vec[i+1]; j++)
            _ntl_gsmul(prod_vec[i], q[j], &prod_vec[i]); 
      }

      for (i = (1L << (levels-1)) - 2; i >= 3; i--)
         _ntl_gmul(prod_vec[2*i+1], prod_vec[2*i+2], &prod_vec[i]);

      
      for (i = 3; i < vec_len; i++)
         len_vec[i] = _ntl_gsize(prod_vec[i]);

      /* Set len_vec[1] = len_vec[2] = 
       *    max(_ntl_gsize(modulus), len_vec[3..6]).
       * This is a bit paranoid, but it makes the code
       * more robust. */

      j = _ntl_gsize(modulus);
      for (i = 3; i <= 6; i++)
         if (len_vec[i] > j) j = len_vec[i];

      len_vec[1] = len_vec[2] = j;

      for (i = 3; i < vec_len; i++)
         inv_vec[i] = neg_inv_mod_limb(DATA(prod_vec[i])[0]);


      for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
         for (j = index_vec[i]; j < index_vec[i+1]; j++) {
            corr_vec[j] = SpecialPower(len_vec[1] - len_vec[i], q[j]);
            corraux_vec[j] = PrepMulModPrecon(corr_vec[j], q[j]);
         }
      }



      UniquePtr<_ntl_rem_struct_medium> R;
      R.make();

      R->n = n;
      R->levels = levels;
      R->primes.move(q);
      R->index_vec.move(index_vec);
      R->len_vec.move(len_vec);
      R->inv_vec.move(inv_vec);
      R->corr_vec.move(corr_vec);
      R->corraux_vec.move(corraux_vec);
      R->prod_vec.move(prod_vec);

      return R.release();
   }


   if (n > 800) 
   {
      UniqueArray<long> q;
      long i, j;
      long levels, vec_len;
      UniqueArray<long> index_vec;
      UniqueArray<_ntl_gbigint_wrapped> prod_vec;
   
      q.SetLength(n);
      for (i = 0; i < n; i++)
         q[i] = p(i);

      levels = 0;
      while ((n >> levels) >= 4) levels++;

      vec_len = (1L << levels) - 1;

      index_vec.SetLength(vec_len+1);
      prod_vec.SetLength(vec_len);

      index_vec[0] = 0;
      index_vec[1] = n;

      for (i = 0; i <= levels-2; i++) {
         long start = (1L << i) - 1;
         long finish = (1L << (i+1)) - 2;
         for (j = finish; j >= start; j--) {
            index_vec[2*j+2] = index_vec[j] + (index_vec[j+1] - index_vec[j])/2;
            index_vec[2*j+1] = index_vec[j];
         }
         index_vec[2*finish+3] = n;
      }

      for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
         /* multiply primes index_vec[i]..index_vec[i+1]-1 into 
          * prod_vec[i]
          */

         _ntl_gone(&prod_vec[i]);
         for (j = index_vec[i]; j < index_vec[i+1]; j++)
            _ntl_gsmul(prod_vec[i], q[j], &prod_vec[i]); 
      }

      for (i = (1L << (levels-1)) - 2; i >= 3; i--)
         _ntl_gmul(prod_vec[2*i+1], prod_vec[2*i+2], &prod_vec[i]);


      
      UniquePtr<_ntl_rem_struct_fast> R;
      R.make();

      R->n = n;
      R->levels = levels;
      R->primes.move(q);
      R->index_vec.move(index_vec);
      R->prod_vec.move(prod_vec);
      R->modulus_size = _ntl_gsize(modulus);

      return R.release();
   }

   {
      // basic case

      UniqueArray<long> q;
      long i;

      UniquePtr<_ntl_rem_struct_basic> R;
      R.make();

      R->n = n;
      R->primes.SetLength(n);
      for (i = 0; i < n; i++)
         R->primes[i] = p(i);

      return R.release();
   }
}

_ntl_tmp_vec *_ntl_rem_struct_basic::fetch()
{
   return 0;
}


#ifdef NTL_TBL_REM

_ntl_tmp_vec *_ntl_rem_struct_tbl::fetch()
{
   return 0;
}

#endif

_ntl_tmp_vec *_ntl_rem_struct_fast::fetch()
{
   long vec_len = (1L << levels) - 1;
   UniquePtr<_ntl_tmp_vec_rem_impl> res;
   res.make();
   res->rem_vec.SetLength(vec_len);
   _ntl_gbigint_wrapped *rem_vec = res->rem_vec.get();

   long i;

   /* allocate length in advance to streamline eval code */

   _ntl_gsetlength(&rem_vec[1], modulus_size);
   _ntl_gsetlength(&rem_vec[2], modulus_size);

   for (i = 1; i < (1L << (levels-1)) - 1; i++) {
      _ntl_gsetlength(&rem_vec[2*i+1], _ntl_gsize(prod_vec[2*i+1]));
      _ntl_gsetlength(&rem_vec[2*i+2], _ntl_gsize(prod_vec[2*i+2]));
   }

   return res.release();
}

_ntl_tmp_vec *_ntl_rem_struct_medium::fetch()
{
   long vec_len = (1L << levels) - 1;
   UniquePtr<_ntl_tmp_vec_rem_impl> res;
   res.make();
   res->rem_vec.SetLength(vec_len);
   _ntl_gbigint_wrapped *rem_vec = res->rem_vec.get();

   long i;

   /* allocate length in advance to streamline eval code */

   _ntl_gsetlength(&rem_vec[0], len_vec[1]); /* a special temp */

   for (i = 1; i < vec_len; i++)
      _ntl_gsetlength(&rem_vec[i], len_vec[i]);

   return res.release();
}





#ifdef NTL_TBL_REM


#if (NTL_GAP_BITS == 2)

// special case, some loop unrolling: slightly faster


void _ntl_rem_struct_tbl::eval(long *x, _ntl_gbigint a, 
                                 _ntl_tmp_vec *generic_tmp_vec)
{
   if (ZEROP(a)) {
      long i;
      for (i = 0; i < n; i++) x[i] = 0;
      return;
   }

   long sa = SIZE(a);
   _ntl_limb_t *adata = DATA(a);

   if (sa <= 4) {
      long i;
      for (i = 0; i < n; i++) {
         _ntl_limb_t *tp = tbl[i]; 
         ll_type acc;
         ll_init(acc, adata[0]);
         long j;
         for (j = 1; j < sa; j++)
            ll_mul_add(acc, adata[j], tp[j]);

         _ntl_limb_t accvec[2];
         x[i] = tbl_red_31(0, ll_get_hi(acc), ll_get_lo(acc), primes[i], inv_primes[i]);
      }
   }
   else {
      long i;
      for (i = 0; i < n; i++) {
         _ntl_limb_t *ap = adata;
         _ntl_limb_t *tp = tbl[i]; 

         ll_type acc21;
         _ntl_limb_t acc0;

         {
            ll_type sum;
            ll_init(sum, ap[0]);

            ll_mul_add(sum, ap[1], tp[1]);
            ll_mul_add(sum, ap[2], tp[2]);
            ll_mul_add(sum, ap[3], tp[3]);

            ll_init(acc21,  ll_get_hi(sum));
            acc0 = ll_get_lo(sum);
         }

         long m=sa-4;
         ap += 4;
         tp += 4;

         for (; m >= 8; m -= 8, ap += 8, tp += 8) {
            {
               ll_type sum;
               ll_mul(sum, ap[0], tp[0]);
               ll_mul_add(sum, ap[1], tp[1]);
               ll_mul_add(sum, ap[2], tp[2]);
               ll_mul_add(sum, ap[3], tp[3]);
   
               ll_add(sum, acc0);
               acc0 = ll_get_lo(sum);
               ll_add(acc21,  ll_get_hi(sum));
            }
            {
   
               ll_type sum;
               ll_mul(sum, ap[4+0], tp[4+0]);
               ll_mul_add(sum, ap[4+1], tp[4+1]);
               ll_mul_add(sum, ap[4+2], tp[4+2]);
               ll_mul_add(sum, ap[4+3], tp[4+3]);
   
               ll_add(sum, acc0);
               acc0 = ll_get_lo(sum);
               ll_add(acc21,  ll_get_hi(sum));
            }
         }

         for (; m >= 4; m -= 4, ap += 4, tp += 4) {
            ll_type sum;
            ll_mul(sum, ap[0], tp[0]);
            ll_mul_add(sum, ap[1], tp[1]);
            ll_mul_add(sum, ap[2], tp[2]);
            ll_mul_add(sum, ap[3], tp[3]);
   
	    ll_add(sum, acc0);
	    acc0 = ll_get_lo(sum);
	    ll_add(acc21,  ll_get_hi(sum));
         }

         if (m > 0) {
            ll_type sum;
            ll_mul(sum, ap[0], tp[0]);
            for (m--, ap++, tp++; m > 0; m--, ap++, tp++)
               ll_mul_add(sum, ap[0], tp[0]);

   
	    ll_add(sum, acc0);
	    acc0 = ll_get_lo(sum);
	    ll_add(acc21,  ll_get_hi(sum));
         }

         x[i] = tbl_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, primes[i], inv_primes[i]);
      }
   }
}

#else

// General case: some loop unrolling (also using "Duff's Device")
// for the case where BPL-SPNBITS == 4: this is the common
// case on 64-bit machines.  The loop unrolling and Duff seems
// to shave off 5-10%

#define TBL_UNROLL (1)

void _ntl_rem_struct_tbl::eval(long *x, _ntl_gbigint a, 
                                 _ntl_tmp_vec *generic_tmp_vec)
{
   if (ZEROP(a)) {
      long i;
      for (i = 0; i < n; i++) x[i] = 0;
      return;
   }

   long sa = SIZE(a);
   _ntl_limb_t *adata = DATA(a);

   const long Bnd =  1L << NTL_GAP_BITS;

   if (sa <= Bnd) {
      long i;
      for (i = 0; i < n; i++) {
         _ntl_limb_t *tp = tbl[i]; 


         ll_type acc;
         ll_init(acc, adata[0]);

#if (TBL_UNROLL && NTL_GAP_BITS == 4)
         switch (sa) {
         case 16:  ll_mul_add(acc, adata[16-1], tp[16-1]);
         case 15:  ll_mul_add(acc, adata[15-1], tp[15-1]);
         case 14:  ll_mul_add(acc, adata[14-1], tp[14-1]);
         case 13:  ll_mul_add(acc, adata[13-1], tp[13-1]);
         case 12:  ll_mul_add(acc, adata[12-1], tp[12-1]);
         case 11:  ll_mul_add(acc, adata[11-1], tp[11-1]);
         case 10:  ll_mul_add(acc, adata[10-1], tp[10-1]);
         case 9:  ll_mul_add(acc, adata[9-1], tp[9-1]);
         case 8:  ll_mul_add(acc, adata[8-1], tp[8-1]);
         case 7:  ll_mul_add(acc, adata[7-1], tp[7-1]);
         case 6:  ll_mul_add(acc, adata[6-1], tp[6-1]);
         case 5:  ll_mul_add(acc, adata[5-1], tp[5-1]);
         case 4:  ll_mul_add(acc, adata[4-1], tp[4-1]);
         case 3:  ll_mul_add(acc, adata[3-1], tp[3-1]);
         case 2:  ll_mul_add(acc, adata[2-1], tp[2-1]);
         }

#elif (TBL_UNROLL)
         long j = sa;
         for (; j > 16; j -= 16) {
            ll_mul_add(acc, adata[j-1], tp[j-1]);
            ll_mul_add(acc, adata[j-2], tp[j-2]);
            ll_mul_add(acc, adata[j-3], tp[j-3]);
            ll_mul_add(acc, adata[j-4], tp[j-4]);
            ll_mul_add(acc, adata[j-5], tp[j-5]);
            ll_mul_add(acc, adata[j-6], tp[j-6]);
            ll_mul_add(acc, adata[j-7], tp[j-7]);
            ll_mul_add(acc, adata[j-8], tp[j-8]);
            ll_mul_add(acc, adata[j-9], tp[j-9]);
            ll_mul_add(acc, adata[j-10], tp[j-10]);
            ll_mul_add(acc, adata[j-11], tp[j-11]);
            ll_mul_add(acc, adata[j-12], tp[j-12]);
            ll_mul_add(acc, adata[j-13], tp[j-13]);
            ll_mul_add(acc, adata[j-14], tp[j-14]);
            ll_mul_add(acc, adata[j-15], tp[j-15]);
            ll_mul_add(acc, adata[j-16], tp[j-16]);
         }
         switch (j) {
         case 16:  ll_mul_add(acc, adata[16-1], tp[16-1]);
         case 15:  ll_mul_add(acc, adata[15-1], tp[15-1]);
         case 14:  ll_mul_add(acc, adata[14-1], tp[14-1]);
         case 13:  ll_mul_add(acc, adata[13-1], tp[13-1]);
         case 12:  ll_mul_add(acc, adata[12-1], tp[12-1]);
         case 11:  ll_mul_add(acc, adata[11-1], tp[11-1]);
         case 10:  ll_mul_add(acc, adata[10-1], tp[10-1]);
         case 9:  ll_mul_add(acc, adata[9-1], tp[9-1]);
         case 8:  ll_mul_add(acc, adata[8-1], tp[8-1]);
         case 7:  ll_mul_add(acc, adata[7-1], tp[7-1]);
         case 6:  ll_mul_add(acc, adata[6-1], tp[6-1]);
         case 5:  ll_mul_add(acc, adata[5-1], tp[5-1]);
         case 4:  ll_mul_add(acc, adata[4-1], tp[4-1]);
         case 3:  ll_mul_add(acc, adata[3-1], tp[3-1]);
         case 2:  ll_mul_add(acc, adata[2-1], tp[2-1]);
         }

#else
         long j;
         for (j = 1; j < sa; j++)
            ll_mul_add(acc, adata[j], tp[j]);
#endif

         x[i] = tbl_red_31(0, ll_get_hi(acc), ll_get_lo(acc), primes[i], inv_primes[i]);
      }
   }
   else {
      long i;
      for (i = 0; i < n; i++) {
         _ntl_limb_t *ap = adata;
         _ntl_limb_t *tp = tbl[i]; 

         ll_type acc21;
         _ntl_limb_t acc0;

         {
            ll_type sum;
            ll_init(sum, ap[0]);

#if (TBL_UNROLL && NTL_GAP_BITS == 4)
            ll_mul_add(sum, ap[1], tp[1]);
            ll_mul_add(sum, ap[2], tp[2]);
            ll_mul_add(sum, ap[3], tp[3]);
            ll_mul_add(sum, ap[4], tp[4]);
            ll_mul_add(sum, ap[5], tp[5]);
            ll_mul_add(sum, ap[6], tp[6]);
            ll_mul_add(sum, ap[7], tp[7]);
            ll_mul_add(sum, ap[8], tp[8]);
            ll_mul_add(sum, ap[9], tp[9]);
            ll_mul_add(sum, ap[10], tp[10]);
            ll_mul_add(sum, ap[11], tp[11]);
            ll_mul_add(sum, ap[12], tp[12]);
            ll_mul_add(sum, ap[13], tp[13]);
            ll_mul_add(sum, ap[14], tp[14]);
            ll_mul_add(sum, ap[15], tp[15]);
#else
            for (long j = 1; j < Bnd; j++)
               ll_mul_add(sum, ap[j], tp[j]);
#endif

            ll_init(acc21, ll_get_hi(sum));
	    acc0 = ll_get_lo(sum);
         }

         long m;
         for (m = sa-Bnd, ap += Bnd, tp += Bnd; m >= Bnd; m -= Bnd, ap += Bnd, tp += Bnd) {

            ll_type sum;
            ll_mul(sum, ap[0], tp[0]);

#if (TBL_UNROLL && NTL_GAP_BITS == 4)
            ll_mul_add(sum, ap[1], tp[1]);
            ll_mul_add(sum, ap[2], tp[2]);
            ll_mul_add(sum, ap[3], tp[3]);
            ll_mul_add(sum, ap[4], tp[4]);
            ll_mul_add(sum, ap[5], tp[5]);
            ll_mul_add(sum, ap[6], tp[6]);
            ll_mul_add(sum, ap[7], tp[7]);
            ll_mul_add(sum, ap[8], tp[8]);
            ll_mul_add(sum, ap[9], tp[9]);
            ll_mul_add(sum, ap[10], tp[10]);
            ll_mul_add(sum, ap[11], tp[11]);
            ll_mul_add(sum, ap[12], tp[12]);
            ll_mul_add(sum, ap[13], tp[13]);
            ll_mul_add(sum, ap[14], tp[14]);
            ll_mul_add(sum, ap[15], tp[15]);
#else
            for (long j = 1; j < Bnd; j++)
               ll_mul_add(sum, ap[j], tp[j]);
#endif
            ll_add(sum, acc0); 
            acc0 = ll_get_lo(sum);
            ll_add(acc21, ll_get_hi(sum));
         }

         if (m > 0) {
            ll_type sum;
            ll_mul(sum, ap[0], tp[0]);

#if (TBL_UNROLL && NTL_GAP_BITS == 4)
            switch (m) {
            case 15:  ll_mul_add(sum, ap[15-1], tp[15-1]);
            case 14:  ll_mul_add(sum, ap[14-1], tp[14-1]);
            case 13:  ll_mul_add(sum, ap[13-1], tp[13-1]);
            case 12:  ll_mul_add(sum, ap[12-1], tp[12-1]);
            case 11:  ll_mul_add(sum, ap[11-1], tp[11-1]);
            case 10:  ll_mul_add(sum, ap[10-1], tp[10-1]);
            case 9:  ll_mul_add(sum, ap[9-1], tp[9-1]);
            case 8:  ll_mul_add(sum, ap[8-1], tp[8-1]);
            case 7:  ll_mul_add(sum, ap[7-1], tp[7-1]);
            case 6:  ll_mul_add(sum, ap[6-1], tp[6-1]);
            case 5:  ll_mul_add(sum, ap[5-1], tp[5-1]);
            case 4:  ll_mul_add(sum, ap[4-1], tp[4-1]);
            case 3:  ll_mul_add(sum, ap[3-1], tp[3-1]);
            case 2:  ll_mul_add(sum, ap[2-1], tp[2-1]);
            }
#else
            for (m--, ap++, tp++; m > 0; m--, ap++, tp++)
               ll_mul_add(sum, ap[0], tp[0]);
#endif
            ll_add(sum, acc0); 
            acc0 = ll_get_lo(sum);
            ll_add(acc21, ll_get_hi(sum));
         }

         x[i] = tbl_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, 
                           primes[i], inv_primes[i]);
      }
   }
}

#endif


#endif


void _ntl_rem_struct_basic::eval(long *x, _ntl_gbigint a, 
                                 _ntl_tmp_vec *generic_tmp_vec)
{
   long *q = primes.get();

   long j;
   _ntl_limb_t *adata;
   long sa;

   if (!a) 
      sa = 0;
   else
      sa = SIZE(a);

   if (sa == 0) {
      for (j = 0; j < n; j++)
         x[j] = 0;

      return;
   }

   adata = DATA(a);

   for (j = 0; j < n; j++)
      x[j] = NTL_MPN(mod_1)(adata, sa, q[j]);

}

void _ntl_rem_struct_fast::eval(long *x, _ntl_gbigint a, 
                                _ntl_tmp_vec *generic_tmp_vec)
{
   long *q = primes.get();
   _ntl_gbigint_wrapped *rem_vec = 
      (static_cast<_ntl_tmp_vec_rem_impl *> (generic_tmp_vec))->rem_vec.get();
   long vec_len = (1L << levels) - 1;

   long i, j;

   if (ZEROP(a)) {
      for (j = 0; j < n; j++)
         x[j] = 0;

      return;
   }

   _ntl_gcopy(a, &rem_vec[1]);
   _ntl_gcopy(a, &rem_vec[2]);

   for (i = 1; i < (1L << (levels-1)) - 1; i++) {
      gmod_simple(rem_vec[i], prod_vec[2*i+1], &rem_vec[2*i+1]);
      gmod_simple(rem_vec[i], prod_vec[2*i+2], &rem_vec[2*i+2]);
   }

   for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
      long lo = index_vec[i];
      long hi = index_vec[i+1];
      _ntl_limb_t *s1p = DATA(rem_vec[i]);
      long s1size = SIZE(rem_vec[i]);
      if (s1size == 0) {
         for (j = lo; j <hi; j++)
            x[j] = 0;
      }
      else {
         for (j = lo; j < hi; j++)
            x[j] = NTL_MPN(mod_1)(s1p, s1size, q[j]);
      }
   }
}

void _ntl_rem_struct_medium::eval(long *x, _ntl_gbigint a, 
                                  _ntl_tmp_vec *generic_tmp_vec)
{
   long *q = primes.get();
   _ntl_gbigint_wrapped *rem_vec = 
      (static_cast<_ntl_tmp_vec_rem_impl *> (generic_tmp_vec))->rem_vec.get();
   long vec_len = (1L << levels) - 1;

   long i, j;

   if (ZEROP(a)) {
      for (j = 0; j < n; j++)
         x[j] = 0;

      return;
   }

   _ntl_gcopy(a, &rem_vec[1]);
   _ntl_gcopy(a, &rem_vec[2]);

   for (i = 1; i < (1L << (levels-1)) - 1; i++) {
      _ntl_gcopy(rem_vec[i], &rem_vec[0]);
      redc(rem_vec[0], prod_vec[2*i+1], len_vec[i]-len_vec[2*i+1],
           inv_vec[2*i+1], rem_vec[2*i+1]);
      redc(rem_vec[i], prod_vec[2*i+2], len_vec[i]-len_vec[2*i+2],
           inv_vec[2*i+2], rem_vec[2*i+2]);
   }

   for (i = (1L << (levels-1)) - 1; i < vec_len; i++) {
      long lo = index_vec[i];
      long hi = index_vec[i+1];
      _ntl_limb_t *s1p = DATA(rem_vec[i]);
      long s1size = SIZE(rem_vec[i]);
      if (s1size == 0) {
         for (j = lo; j < hi; j++)
            x[j] = 0;
      }
      else {
         for (j = lo; j < hi; j++) {
            long t = NTL_MPN(mod_1)(s1p, s1size, q[j]);
            x[j] = MulModPrecon(t, corr_vec[j], q[j], corraux_vec[j]);
         }
      }
   }
}



/* routines for x += a*b for multi-precision b  
 */
   

void
_ntl_gaorsmul(_ntl_gbigint x, _ntl_gbigint y, long sub,  _ntl_gbigint *ww)
{
   GRegister(tmp);

   _ntl_gmul(x, y, &tmp);
   if (sub)
      _ntl_gsub(*ww, tmp, ww);
   else
      _ntl_gadd(*ww, tmp, ww);
}


void
_ntl_gaddmul(_ntl_gbigint x, _ntl_gbigint y,  _ntl_gbigint *ww)
{
  _ntl_gaorsmul(x, y, 0, ww);
}

void
_ntl_gsubmul(_ntl_gbigint x, _ntl_gbigint y,  _ntl_gbigint *ww)
{
  _ntl_gaorsmul(x, y, 1, ww);
}


/* routines for x += a*b for single-precision b 
 * Lightly massaged code taken from GMP's mpz routines */


static inline 
void _ntl_mpn_com_n(_ntl_limb_t *d, _ntl_limb_t *s, long n) 
{
  do {
    *d++ = CLIP(~ *s++); 
  } while (--n); 
}

#if 0
#define _ntl_mpn_com_n(d,s,n) \
  do { \
    _ntl_limb_t *  __d = (d); \
    _ntl_limb_t *  __s = (s); \
    long  __n = (n); \
    do \
      *__d++ = CLIP(~ *__s++); \
    while (--__n); \
  } while (0)
#endif



static inline 
void _ntl_MPN_MUL_1C(_ntl_limb_t& cout, _ntl_limb_t *dst, 
                     _ntl_limb_t *src, long size, _ntl_limb_t n, 
                     _ntl_limb_t cin) 
{
    _ntl_limb_t cy; 
    cy = NTL_MPN(mul_1) (dst, src, size, n); 
    cout = CLIP(cy + NTL_MPN(add_1) (dst, dst, size, cin)); 
}



#if 0
#define _ntl_MPN_MUL_1C(cout, dst, src, size, n, cin) \
  do { \
    _ntl_limb_t __cy; \
    __cy = NTL_MPN(mul_1) (dst, src, size, n); \
    (cout) = CLIP(__cy + NTL_MPN(add_1) (dst, dst, size, cin)); \
  } while (0)
#endif




static inline
void _ntl_g_inc(_ntl_limb_t *p, long n)
{
    while (n > 0) {  
       *p = CLIP(*p + 1); 
       if (*p != 0) break;  
       p++;  
       n--;  
    }
}

#if 0
#define _ntl_g_inc(p, n)   \
  do {   \
    _ntl_limb_t * __p = (p);  \
    long __n = (n);  \
    while (__n > 0) {  \
       *__p = CLIP(*__p + 1); \
       if (*__p != 0) break;  \
       __p++;  \
       __n--;  \
    }  \
  } while (0);
#endif

static inline
void _ntl_g_inc_carry(_ntl_limb_t& c, _ntl_limb_t *p, long n)   
{
   long addc = 1; 
   while (n > 0) {  
      *p = CLIP(*p + 1); 
      if (*p != 0) { addc = 0; break; }  
      p++;  
      n--;  
   }  
   c = CLIP(c + addc); 
}

#if 0
#define _ntl_g_inc_carry(c, p, n)   \
  do {   \
    _ntl_limb_t * __p = (p);  \
    long __n = (n);  \
    long __addc = 1; \
    while (__n > 0) {  \
       *__p = CLIP(*__p + 1); \
       if (*__p != 0) { __addc = 0; break; }  \
       __p++;  \
       __n--;  \
    }  \
    c = CLIP(c + __addc); \
  } while (0);
#endif 


static inline
void _ntl_g_dec(_ntl_limb_t *p, long n)   
{
   _ntl_limb_t tmp; 
   while (n > 0) {  
      tmp = *p; 
      *p = CLIP(*p - 1); 
      if (tmp != 0) break;  
      p++;  
      n--;  
   }  
}


#if 0
#define _ntl_g_dec(p, n)   \
  do {   \
    _ntl_limb_t * __p = (p);  \
    _ntl_limb_t __tmp; \
    long __n = (n);  \
    while (__n > 0) {  \
       __tmp = *__p; \
       *__p = CLIP(*__p - 1); \
       if (__tmp != 0) break;  \
       __p++;  \
       __n--;  \
    }  \
  } while (0);
#endif
  


/* sub==0 means an addmul w += x*y, sub==1 means a submul w -= x*y. */
void
_ntl_gaorsmul_1(_ntl_gbigint x, long yy, long sub, _ntl_gbigint *ww)
{
  long  xsize, xneg, wsize, wneg, new_wsize, min_size, dsize;
  _ntl_gbigint w;
  _ntl_limb_t *xp;
  _ntl_limb_t *wp;
  _ntl_limb_t  cy;
  _ntl_limb_t  y;

  if (ZEROP(x) || yy == 0)
    return;

  if (ZEROP(*ww)) {
    _ntl_gsmul(x, yy, ww);
    if (sub) SIZE(*ww) = -SIZE(*ww);
    return;
  }

  if (yy == 1) {
    if (sub)
      _ntl_gsub(*ww, x, ww);
    else
      _ntl_gadd(*ww, x, ww);
    return;
  }

  if (yy == -1) {
    if (sub)
      _ntl_gadd(*ww, x, ww);
    else
      _ntl_gsub(*ww, x, ww);
    return;
  }

  if (*ww == x) {
    GRegister(tmp);
    _ntl_gsmul(x, yy, &tmp);
    if (sub)
       _ntl_gsub(*ww, tmp, ww);
    else
       _ntl_gadd(*ww, tmp, ww);
    return;
  }

  y = ABS(yy);
  if (XCLIP(y)) {
    GRegister(xyy);
    _ntl_gintoz(yy, &xyy);
    _ntl_gaorsmul(x, xyy, sub, ww);
    return;
  }

  GET_SIZE_NEG(xsize, xneg, x);
  sub = XOR(sub, xneg);
  sub = XOR(sub, yy < 0);

  w = *ww;

  GET_SIZE_NEG(wsize, wneg, w);
  sub = XOR(sub, wneg);

  new_wsize = max(wsize, xsize);
  min_size = min(wsize, xsize);

  if (MustAlloc(w, new_wsize+1)) {
    _ntl_gsetlength(&w, new_wsize+1);
    *ww = w;
  }

  wp = DATA(w);
  xp = DATA(x);

  if (sub == 0)
    {
      /* addmul of absolute values */

      cy = NTL_MPN(addmul_1) (wp, xp, min_size, y);
      wp += min_size;
      xp += min_size;

      dsize = xsize - wsize;
      if (dsize != 0)
        {
          _ntl_limb_t  cy2;
          if (dsize > 0) {
            cy2 = NTL_MPN(mul_1) (wp, xp, dsize, y);
          }
          else {
            dsize = -dsize;
            cy2 = 0;
          }
          cy = CLIP(cy2 + NTL_MPN(add_1) (wp, wp, dsize, cy));
        }

      wp[dsize] = cy;
      new_wsize += (cy != 0);
    }
  else
    {
      /* submul of absolute values */

      cy = NTL_MPN(submul_1) (wp, xp, min_size, y);
      if (wsize >= xsize)
        {
          /* if w bigger than x, then propagate borrow through it */
          if (wsize != xsize) {
            cy = NTL_MPN(sub_1) (wp+xsize, wp+xsize, wsize-xsize, cy);
          }

          if (cy != 0)
            {
              /* Borrow out of w, take twos complement negative to get
                 absolute value, flip sign of w.  */
              wp[new_wsize] = CLIP(~-cy);  /* extra limb is 0-cy */
              _ntl_mpn_com_n (wp, wp, new_wsize);
              new_wsize++;
              _ntl_g_inc(wp, new_wsize);
              wneg = XOR(wneg, 1); 
            }
        }
      else /* wsize < xsize */
        {
          /* x bigger than w, so want x*y-w.  Submul has given w-x*y, so
             take twos complement and use an mpn_mul_1 for the rest.  */

          _ntl_limb_t  cy2;

          /* -(-cy*b^n + w-x*y) = (cy-1)*b^n + ~(w-x*y) + 1 */
          _ntl_mpn_com_n (wp, wp, wsize);
          _ntl_g_inc_carry(cy, wp, wsize);
          cy = CLIP(cy-1);

          /* If cy-1 == -1 then hold that -1 for latter.  mpn_submul_1 never
             returns cy==MP_LIMB_T_MAX so that value always indicates a -1. */
          cy2 = (cy == CLIP(_ntl_limb_t(-1)));
          cy = CLIP(cy + cy2);
          _ntl_MPN_MUL_1C (cy, wp+wsize, xp+wsize, xsize-wsize, y, cy);
          wp[new_wsize] = cy;
          new_wsize += (cy != 0);

          /* Apply any -1 from above.  The value at wp+wsize is non-zero
             because y!=0 and the high limb of x will be non-zero.  */
          if (cy2) {
            _ntl_g_dec(wp+wsize, new_wsize-wsize);
          }

          wneg = XOR(wneg, 1);
        }

      /* submul can produce high zero limbs due to cancellation, both when w
         has more limbs or x has more  */
      STRIP(new_wsize, wp);
    }

  if (wneg) new_wsize = -new_wsize;
  SIZE(w) = new_wsize;
}


void
_ntl_gsaddmul(_ntl_gbigint x, long yy,  _ntl_gbigint *ww)
{
  _ntl_gaorsmul_1(x, yy, 0, ww);
}

void
_ntl_gssubmul(_ntl_gbigint x, long yy,  _ntl_gbigint *ww)
{
  _ntl_gaorsmul_1(x, yy, 1, ww);
}




// general preconditioned remainder



#ifndef NTL_VIABLE_LL


struct _ntl_general_rem_one_struct  { };

_ntl_general_rem_one_struct *
_ntl_general_rem_one_struct_build(long p)
{
   return 0;
}

long 
_ntl_general_rem_one_struct_apply(_ntl_gbigint a, long p, _ntl_general_rem_one_struct *pinfo)
{
   return _ntl_gsmod(a, p);
}

void
_ntl_general_rem_one_struct_delete(_ntl_general_rem_one_struct *pinfo) 
{
}


#else


#define REM_ONE_SZ (128)

struct _ntl_general_rem_one_struct  {
   sp_ll_reduce_struct red_struct;
   long Bnd;
   UniqueArray<_ntl_limb_t> tbl;
};



_ntl_general_rem_one_struct *
_ntl_general_rem_one_struct_build(long p)
{
   if (p < 2 || p >= NTL_SP_BOUND)
      LogicError("_ntl_general_rem_one_struct_build: bad args (p)");

   UniquePtr<_ntl_general_rem_one_struct> pinfo;
   pinfo.make();

   pinfo->red_struct = make_sp_ll_reduce_struct(p);

   long pbits = _ntl_g2logs(p);
   long gapbits = min(28, 2*NTL_BITS_PER_LONG - pbits - NTL_ZZ_NBITS);
   // hold gapbits to a max of 28 to avoid some potential overflow
   // issues

   pinfo->Bnd = 1L << gapbits;

   pinfo->tbl.SetLength(REM_ONE_SZ+3);

   long t = 1;
   for (long j = 0; j < NTL_ZZ_NBITS; j++) {
      t += t;
      if (t >= p) t -= p;
   }

   long t2 = t;
   for (long j = NTL_ZZ_NBITS; j < NTL_BITS_PER_LONG; j++) {
      t2 += t2;
      if (t2 >= p) t2 -= p;
   }

   long t1 = 1;
   pinfo->tbl[0] = 1;
   for (long j = 1; j <= REM_ONE_SZ; j++) {
      t1 = MulMod(t1, t, p);
      pinfo->tbl[j] = t1;
   }

   // careful! for non-empty nails, we have to initialize
   // the last two table entries differently

   for (long j = REM_ONE_SZ+1; j < REM_ONE_SZ+3; j++) {
      t1 = MulMod(t1, t2, p);
      pinfo->tbl[j] = t1;
   }

   return pinfo.release();
}


long 
_ntl_general_rem_one_struct_apply1(_ntl_limb_t *a_data, long a_sz, long a_neg, long p, 
                                   _ntl_general_rem_one_struct *pinfo)
{
   sp_ll_reduce_struct red_struct = pinfo->red_struct;
   long Bnd = pinfo->Bnd;
   _ntl_limb_t *tbl = pinfo->tbl.elts();

   long idx = ((cast_unsigned(a_sz+REM_ONE_SZ-1)/REM_ONE_SZ)-1)*REM_ONE_SZ;
   ll_type leftover;
   long sz = a_sz-idx;
   a_data += idx;

   for ( ; ; sz = REM_ONE_SZ, a_data -= REM_ONE_SZ, idx -= REM_ONE_SZ) {
      if (sz <= Bnd) {
	 ll_type acc;
	 ll_init(acc, 0);

	 {
	    long j = 0;

	    for (; j <= sz-16; j += 16) {
	       ll_mul_add(acc, a_data[j+0], tbl[j+0]);
	       ll_mul_add(acc, a_data[j+1], tbl[j+1]);
	       ll_mul_add(acc, a_data[j+2], tbl[j+2]);
	       ll_mul_add(acc, a_data[j+3], tbl[j+3]);
	       ll_mul_add(acc, a_data[j+4], tbl[j+4]);
	       ll_mul_add(acc, a_data[j+5], tbl[j+5]);
	       ll_mul_add(acc, a_data[j+6], tbl[j+6]);
	       ll_mul_add(acc, a_data[j+7], tbl[j+7]);
	       ll_mul_add(acc, a_data[j+8], tbl[j+8]);
	       ll_mul_add(acc, a_data[j+9], tbl[j+9]);
	       ll_mul_add(acc, a_data[j+10], tbl[j+10]);
	       ll_mul_add(acc, a_data[j+11], tbl[j+11]);
	       ll_mul_add(acc, a_data[j+12], tbl[j+12]);
	       ll_mul_add(acc, a_data[j+13], tbl[j+13]);
	       ll_mul_add(acc, a_data[j+14], tbl[j+14]);
	       ll_mul_add(acc, a_data[j+15], tbl[j+15]);
	    }

	    for (; j <= sz-4; j += 4) {
	       ll_mul_add(acc, a_data[j+0], tbl[j+0]);
	       ll_mul_add(acc, a_data[j+1], tbl[j+1]);
	       ll_mul_add(acc, a_data[j+2], tbl[j+2]);
	       ll_mul_add(acc, a_data[j+3], tbl[j+3]);
	    }

	    for (; j < sz; j++)
	       ll_mul_add(acc, a_data[j+0], tbl[j+0]);
	 }

         if (idx + REM_ONE_SZ >= a_sz) { // first time
            if (idx == 0) { // last time
	      long res = sp_ll_red_31(0, ll_get_hi(acc), ll_get_lo(acc), p, red_struct);
	      if (a_neg) res = NegateMod(res, p);
	      return res;
            }
            else {
               ll_mul(leftover, ll_get_lo(acc), tbl[REM_ONE_SZ]);
               ll_mul_add(leftover, ll_get_hi(acc), tbl[REM_ONE_SZ+1]);
            }
         }
         else {
	    ll_type acc21;
	    _ntl_limb_t acc0;

	    ll_add(leftover, ll_get_lo(acc));
	    acc0 = ll_get_lo(leftover);
	    ll_init(acc21, ll_get_hi(leftover));
	    ll_add(acc21, ll_get_hi(acc));

            if (idx == 0) { // last time
	       long res = sp_ll_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, p, red_struct);
	       if (a_neg) res = NegateMod(res, p);
	       return res;
            }
            else {
               ll_mul(leftover, acc0, tbl[REM_ONE_SZ]);
               ll_mul_add(leftover, ll_get_lo(acc21), tbl[REM_ONE_SZ+1]);
               ll_mul_add(leftover, ll_get_hi(acc21), tbl[REM_ONE_SZ+2]);
            }
         }
      }
      else {
	 ll_type acc21;
	 ll_init(acc21, 0);
	 _ntl_limb_t acc0 = 0;

	 if (Bnd > 16) {
	    long jj = 0;
	    for (; jj <= sz-Bnd; jj += Bnd) {
	       ll_type acc;
	       ll_init(acc, acc0);

	       long j = jj;

	       for (; j <= jj+Bnd-16; j += 16) {
		  ll_mul_add(acc, a_data[j+0], tbl[j+0]);
		  ll_mul_add(acc, a_data[j+1], tbl[j+1]);
		  ll_mul_add(acc, a_data[j+2], tbl[j+2]);
		  ll_mul_add(acc, a_data[j+3], tbl[j+3]);
		  ll_mul_add(acc, a_data[j+4], tbl[j+4]);
		  ll_mul_add(acc, a_data[j+5], tbl[j+5]);
		  ll_mul_add(acc, a_data[j+6], tbl[j+6]);
		  ll_mul_add(acc, a_data[j+7], tbl[j+7]);
		  ll_mul_add(acc, a_data[j+8], tbl[j+8]);
		  ll_mul_add(acc, a_data[j+9], tbl[j+9]);
		  ll_mul_add(acc, a_data[j+10], tbl[j+10]);
		  ll_mul_add(acc, a_data[j+11], tbl[j+11]);
		  ll_mul_add(acc, a_data[j+12], tbl[j+12]);
		  ll_mul_add(acc, a_data[j+13], tbl[j+13]);
		  ll_mul_add(acc, a_data[j+14], tbl[j+14]);
		  ll_mul_add(acc, a_data[j+15], tbl[j+15]);
	       }

	       acc0 = ll_get_lo(acc);
	       ll_add(acc21, ll_get_hi(acc));
	    }

	    if (jj < sz) {
	       ll_type acc;
	       ll_init(acc, acc0);

	       long j = jj;

	       for (; j <= sz-4; j += 4) {
		  ll_mul_add(acc, a_data[j+0], tbl[j+0]);
		  ll_mul_add(acc, a_data[j+1], tbl[j+1]);
		  ll_mul_add(acc, a_data[j+2], tbl[j+2]);
		  ll_mul_add(acc, a_data[j+3], tbl[j+3]);
	       }

	       for (; j < sz; j++)
		  ll_mul_add(acc, a_data[j+0], tbl[j+0]);

	       acc0 = ll_get_lo(acc);
	       ll_add(acc21, ll_get_hi(acc));
	    }
	 }
	 else if (Bnd == 16) {

	    long jj = 0;
	    for (; jj <= sz-16; jj += 16) {
	       ll_type acc;

	       long j = jj;

	       ll_mul(acc, a_data[j+0], tbl[j+0]);
	       ll_mul_add(acc, a_data[j+1], tbl[j+1]);
	       ll_mul_add(acc, a_data[j+2], tbl[j+2]);
	       ll_mul_add(acc, a_data[j+3], tbl[j+3]);
	       ll_mul_add(acc, a_data[j+4], tbl[j+4]);
	       ll_mul_add(acc, a_data[j+5], tbl[j+5]);
	       ll_mul_add(acc, a_data[j+6], tbl[j+6]);
	       ll_mul_add(acc, a_data[j+7], tbl[j+7]);
	       ll_mul_add(acc, a_data[j+8], tbl[j+8]);
	       ll_mul_add(acc, a_data[j+9], tbl[j+9]);
	       ll_mul_add(acc, a_data[j+10], tbl[j+10]);
	       ll_mul_add(acc, a_data[j+11], tbl[j+11]);
	       ll_mul_add(acc, a_data[j+12], tbl[j+12]);
	       ll_mul_add(acc, a_data[j+13], tbl[j+13]);
	       ll_mul_add(acc, a_data[j+14], tbl[j+14]);
	       ll_mul_add(acc, a_data[j+15], tbl[j+15]);

	       ll_add(acc, acc0);
	       acc0 = ll_get_lo(acc);
	       ll_add(acc21, ll_get_hi(acc));
	    }

	    if (jj < sz) {
	       ll_type acc;
	       ll_init(acc, acc0);

	       long j = jj;

	       for (; j <= sz-4; j += 4) {
		  ll_mul_add(acc, a_data[j+0], tbl[j+0]);
		  ll_mul_add(acc, a_data[j+1], tbl[j+1]);
		  ll_mul_add(acc, a_data[j+2], tbl[j+2]);
		  ll_mul_add(acc, a_data[j+3], tbl[j+3]);
	       }

	       for (; j < sz; j++)
		  ll_mul_add(acc, a_data[j+0], tbl[j+0]);

	       acc0 = ll_get_lo(acc);
	       ll_add(acc21, ll_get_hi(acc));
	    }
	 }
	 else if (Bnd == 8)  {
	    long jj = 0;
	    for (; jj <= sz-8; jj += 8) {
	       ll_type acc;

	       long j = jj;

	       ll_mul(acc, a_data[j+0], tbl[j+0]);
	       ll_mul_add(acc, a_data[j+1], tbl[j+1]);
	       ll_mul_add(acc, a_data[j+2], tbl[j+2]);
	       ll_mul_add(acc, a_data[j+3], tbl[j+3]);
	       ll_mul_add(acc, a_data[j+4], tbl[j+4]);
	       ll_mul_add(acc, a_data[j+5], tbl[j+5]);
	       ll_mul_add(acc, a_data[j+6], tbl[j+6]);
	       ll_mul_add(acc, a_data[j+7], tbl[j+7]);

	       ll_add(acc, acc0);
	       acc0 = ll_get_lo(acc);
	       ll_add(acc21, ll_get_hi(acc));
	    }

	    if (jj < sz) {
	       ll_type acc;
	       ll_init(acc, acc0);

	       long j = jj;

	       for (; j < sz; j++)
		  ll_mul_add(acc, a_data[j+0], tbl[j+0]);

	       acc0 = ll_get_lo(acc);
	       ll_add(acc21, ll_get_hi(acc));
	    }
	 }
	 else /* Bnd == 4 */  {
	    long jj = 0;
	    for (; jj <= sz-4; jj += 4) {
	       ll_type acc;

	       long j = jj;

	       ll_mul(acc, a_data[j+0], tbl[j+0]);
	       ll_mul_add(acc, a_data[j+1], tbl[j+1]);
	       ll_mul_add(acc, a_data[j+2], tbl[j+2]);
	       ll_mul_add(acc, a_data[j+3], tbl[j+3]);


	       ll_add(acc, acc0);
	       acc0 = ll_get_lo(acc);
	       ll_add(acc21, ll_get_hi(acc));
	    }

	    if (jj < sz) {
	       ll_type acc;
	       ll_init(acc, acc0);

	       long j = jj;

	       for (; j < sz; j++)
		  ll_mul_add(acc, a_data[j+0], tbl[j+0]);


	       acc0 = ll_get_lo(acc);
	       ll_add(acc21, ll_get_hi(acc));
	    }
	 }

	 if (idx + REM_ONE_SZ < a_sz) { // not first time
	    ll_add(leftover, acc0);
	    acc0 = ll_get_lo(leftover);
	    ll_add(acc21, ll_get_hi(leftover));
	 }

	 if (idx == 0) { // last time
	    long res = sp_ll_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, p, red_struct);
	    if (a_neg) res = NegateMod(res, p);
	    return res;
	 }
	 else {
	    ll_mul(leftover, acc0, tbl[REM_ONE_SZ]);
	    ll_mul_add(leftover, ll_get_lo(acc21), tbl[REM_ONE_SZ+1]);
	    ll_mul_add(leftover, ll_get_hi(acc21), tbl[REM_ONE_SZ+2]);
	 }
      }
   }
}


long 
_ntl_general_rem_one_struct_apply(_ntl_gbigint a, long p, _ntl_general_rem_one_struct *pinfo)
{
   if (ZEROP(a)) return 0;

   if (!pinfo) {
      return _ntl_gsmod(a, p);
   }

   sp_ll_reduce_struct red_struct = pinfo->red_struct;
   long Bnd = pinfo->Bnd;
   _ntl_limb_t *tbl = pinfo->tbl.elts();

   long a_sz, a_neg;
   _ntl_limb_t *a_data;
   GET_SIZE_NEG(a_sz, a_neg, a);
   a_data = DATA(a);

   if (a_sz > REM_ONE_SZ) {
      return _ntl_general_rem_one_struct_apply1(a_data, a_sz, a_neg, p, pinfo);
   }

   if (a_sz <= Bnd) {
      ll_type acc;
      ll_init(acc, 0);

      {
         long j = 0;

         for (; j <= a_sz-16; j += 16) {
            ll_mul_add(acc, a_data[j+0], tbl[j+0]);
            ll_mul_add(acc, a_data[j+1], tbl[j+1]);
            ll_mul_add(acc, a_data[j+2], tbl[j+2]);
            ll_mul_add(acc, a_data[j+3], tbl[j+3]);
            ll_mul_add(acc, a_data[j+4], tbl[j+4]);
            ll_mul_add(acc, a_data[j+5], tbl[j+5]);
            ll_mul_add(acc, a_data[j+6], tbl[j+6]);
            ll_mul_add(acc, a_data[j+7], tbl[j+7]);
            ll_mul_add(acc, a_data[j+8], tbl[j+8]);
            ll_mul_add(acc, a_data[j+9], tbl[j+9]);
            ll_mul_add(acc, a_data[j+10], tbl[j+10]);
            ll_mul_add(acc, a_data[j+11], tbl[j+11]);
            ll_mul_add(acc, a_data[j+12], tbl[j+12]);
            ll_mul_add(acc, a_data[j+13], tbl[j+13]);
            ll_mul_add(acc, a_data[j+14], tbl[j+14]);
            ll_mul_add(acc, a_data[j+15], tbl[j+15]);
         }

         for (; j <= a_sz-4; j += 4) {
            ll_mul_add(acc, a_data[j+0], tbl[j+0]);
            ll_mul_add(acc, a_data[j+1], tbl[j+1]);
            ll_mul_add(acc, a_data[j+2], tbl[j+2]);
            ll_mul_add(acc, a_data[j+3], tbl[j+3]);
         }

	 for (; j < a_sz; j++)
            ll_mul_add(acc, a_data[j+0], tbl[j+0]);
      }


      long res = sp_ll_red_31(0, ll_get_hi(acc), ll_get_lo(acc), p, red_struct);
      if (a_neg) res = NegateMod(res, p);
      return res;
   }
   else if (Bnd > 16) {
      ll_type acc21;
      ll_init(acc21, 0);
      _ntl_limb_t acc0 = 0;

      long jj = 0;
      for (; jj <= a_sz-Bnd; jj += Bnd) {
         ll_type acc;
         ll_init(acc, acc0);

         long j = jj;

         for (; j <= jj+Bnd-16; j += 16) {
            ll_mul_add(acc, a_data[j+0], tbl[j+0]);
            ll_mul_add(acc, a_data[j+1], tbl[j+1]);
            ll_mul_add(acc, a_data[j+2], tbl[j+2]);
            ll_mul_add(acc, a_data[j+3], tbl[j+3]);
            ll_mul_add(acc, a_data[j+4], tbl[j+4]);
            ll_mul_add(acc, a_data[j+5], tbl[j+5]);
            ll_mul_add(acc, a_data[j+6], tbl[j+6]);
            ll_mul_add(acc, a_data[j+7], tbl[j+7]);
            ll_mul_add(acc, a_data[j+8], tbl[j+8]);
            ll_mul_add(acc, a_data[j+9], tbl[j+9]);
            ll_mul_add(acc, a_data[j+10], tbl[j+10]);
            ll_mul_add(acc, a_data[j+11], tbl[j+11]);
            ll_mul_add(acc, a_data[j+12], tbl[j+12]);
            ll_mul_add(acc, a_data[j+13], tbl[j+13]);
            ll_mul_add(acc, a_data[j+14], tbl[j+14]);
            ll_mul_add(acc, a_data[j+15], tbl[j+15]);
         }

         acc0 = ll_get_lo(acc);
         ll_add(acc21, ll_get_hi(acc));
      }

      if (jj < a_sz) {
         ll_type acc;
         ll_init(acc, acc0);

         long j = jj;

         for (; j <= a_sz-4; j += 4) {
            ll_mul_add(acc, a_data[j+0], tbl[j+0]);
            ll_mul_add(acc, a_data[j+1], tbl[j+1]);
            ll_mul_add(acc, a_data[j+2], tbl[j+2]);
            ll_mul_add(acc, a_data[j+3], tbl[j+3]);
         }

	 for (; j < a_sz; j++)
            ll_mul_add(acc, a_data[j+0], tbl[j+0]);

         acc0 = ll_get_lo(acc);
         ll_add(acc21, ll_get_hi(acc));
      }

      long res = sp_ll_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, p, red_struct);
      if (a_neg) res = NegateMod(res, p);
      return res;
   }
   else if (Bnd == 16) {
      ll_type acc21;
      ll_init(acc21, 0);
      _ntl_limb_t acc0 = 0;

      long jj = 0;
      for (; jj <= a_sz-16; jj += 16) {
         ll_type acc;

         long j = jj;

         ll_mul(acc, a_data[j+0], tbl[j+0]);
         ll_mul_add(acc, a_data[j+1], tbl[j+1]);
         ll_mul_add(acc, a_data[j+2], tbl[j+2]);
         ll_mul_add(acc, a_data[j+3], tbl[j+3]);
         ll_mul_add(acc, a_data[j+4], tbl[j+4]);
         ll_mul_add(acc, a_data[j+5], tbl[j+5]);
         ll_mul_add(acc, a_data[j+6], tbl[j+6]);
         ll_mul_add(acc, a_data[j+7], tbl[j+7]);
         ll_mul_add(acc, a_data[j+8], tbl[j+8]);
         ll_mul_add(acc, a_data[j+9], tbl[j+9]);
         ll_mul_add(acc, a_data[j+10], tbl[j+10]);
         ll_mul_add(acc, a_data[j+11], tbl[j+11]);
         ll_mul_add(acc, a_data[j+12], tbl[j+12]);
         ll_mul_add(acc, a_data[j+13], tbl[j+13]);
         ll_mul_add(acc, a_data[j+14], tbl[j+14]);
         ll_mul_add(acc, a_data[j+15], tbl[j+15]);

         ll_add(acc, acc0);
         acc0 = ll_get_lo(acc);
         ll_add(acc21, ll_get_hi(acc));
      }

      if (jj < a_sz) {
         ll_type acc;
         ll_init(acc, acc0);

         long j = jj;

         for (; j <= a_sz-4; j += 4) {
	    ll_mul_add(acc, a_data[j+0], tbl[j+0]);
	    ll_mul_add(acc, a_data[j+1], tbl[j+1]);
	    ll_mul_add(acc, a_data[j+2], tbl[j+2]);
	    ll_mul_add(acc, a_data[j+3], tbl[j+3]);
         }

	 for (; j < a_sz; j++)
	    ll_mul_add(acc, a_data[j+0], tbl[j+0]);

         acc0 = ll_get_lo(acc);
         ll_add(acc21, ll_get_hi(acc));
      }

#if (NTL_NAIL_BITS == 0 && NTL_BITS_PER_LONG-NTL_SP_NBITS==4)
// DIRT: only works if no nails
// NOTE: this is a very minor optimization
      long res = sp_ll_red_31_normalized(ll_get_hi(acc21), ll_get_lo(acc21), acc0, p, red_struct);
#else
      long res = sp_ll_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, p, red_struct);
#endif
      if (a_neg) res = NegateMod(res, p);
      return res;
   }
   else if (Bnd == 8)  {
      ll_type acc21;
      ll_init(acc21, 0);
      _ntl_limb_t acc0 = 0;

      long jj = 0;
      for (; jj <= a_sz-8; jj += 8) {
         ll_type acc;

         long j = jj;

         ll_mul(acc, a_data[j+0], tbl[j+0]);
         ll_mul_add(acc, a_data[j+1], tbl[j+1]);
         ll_mul_add(acc, a_data[j+2], tbl[j+2]);
         ll_mul_add(acc, a_data[j+3], tbl[j+3]);
         ll_mul_add(acc, a_data[j+4], tbl[j+4]);
         ll_mul_add(acc, a_data[j+5], tbl[j+5]);
         ll_mul_add(acc, a_data[j+6], tbl[j+6]);
         ll_mul_add(acc, a_data[j+7], tbl[j+7]);

         ll_add(acc, acc0);
         acc0 = ll_get_lo(acc);
         ll_add(acc21, ll_get_hi(acc));
      }

      if (jj < a_sz) {
         ll_type acc;
         ll_init(acc, acc0);

         long j = jj;

	 for (; j < a_sz; j++)
	    ll_mul_add(acc, a_data[j+0], tbl[j+0]);

         acc0 = ll_get_lo(acc);
         ll_add(acc21, ll_get_hi(acc));
      }

      long res = sp_ll_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, p, red_struct);
      if (a_neg) res = NegateMod(res, p);
      return res;
   }
   else /* Bnd == 4 */  {
      ll_type acc21;
      ll_init(acc21, 0);
      _ntl_limb_t acc0 = 0;

      long jj = 0;
      for (; jj <= a_sz-4; jj += 4) {
         ll_type acc;

         long j = jj;

         ll_mul(acc, a_data[j+0], tbl[j+0]);
         ll_mul_add(acc, a_data[j+1], tbl[j+1]);
         ll_mul_add(acc, a_data[j+2], tbl[j+2]);
         ll_mul_add(acc, a_data[j+3], tbl[j+3]);


         ll_add(acc, acc0);
         acc0 = ll_get_lo(acc);
         ll_add(acc21, ll_get_hi(acc));
      }

      if (jj < a_sz) {
         ll_type acc;
         ll_init(acc, acc0);

         long j = jj;

	 for (; j < a_sz; j++)
	    ll_mul_add(acc, a_data[j+0], tbl[j+0]);


         acc0 = ll_get_lo(acc);
         ll_add(acc21, ll_get_hi(acc));
      }

#if (NTL_NAIL_BITS == 0 && NTL_BITS_PER_LONG-NTL_SP_NBITS==2)
// DIRT: only works if no nails
// NOTE: this is a very minor optimization
      long res = sp_ll_red_31_normalized(ll_get_hi(acc21), ll_get_lo(acc21), acc0, p, red_struct);
#else
      long res = sp_ll_red_31(ll_get_hi(acc21), ll_get_lo(acc21), acc0, p, red_struct);
#endif
      if (a_neg) res = NegateMod(res, p);
      return res;
   }
}

void
_ntl_general_rem_one_struct_delete(_ntl_general_rem_one_struct *pinfo) 
{
   delete pinfo;
}


#endif


void
_ntl_quick_accum_begin(_ntl_gbigint *xp, long sz)
{
   long sbuf = sz+2;
   _ntl_gbigint x = *xp;
   if (MustAlloc(x, sbuf)) {
      _ntl_gsetlength(&x, sbuf);
      *xp = x;
   }

   _ntl_limb_t *xx = DATA(x);
   for (long i = 0; i < sbuf; i++) xx[i] = 0;
   SIZE(x) = sbuf;
}

void 
_ntl_quick_accum_muladd(_ntl_gbigint x, _ntl_gbigint y, long b)
{
   if (!y) return;

   _ntl_limb_t *yy = DATA(y);
   long sy = SIZE(y);
   if (!sy || !b) return;

   _ntl_limb_t *xx = DATA(x);

   _ntl_limb_t carry = NTL_MPN(addmul_1)(xx, yy, sy, b);
   yy = xx + sy;
   *yy = CLIP(*yy + carry);

   if (*yy < carry) { /* unsigned comparison! */
      do {
	 yy++;
	 *yy = CLIP(*yy + 1);
      } while (*yy == 0);
   }
}

void
_ntl_quick_accum_end(_ntl_gbigint x)
{
   _ntl_limb_t *xx = DATA(x);
   long sx = SIZE(x);
   STRIP(sx, xx);
   SIZE(x) = sx;
}


#ifdef NTL_PROVIDES_SS_LIP_IMPL

void
_ntl_leftrotate(_ntl_gbigint *a, const _ntl_gbigint *b, long e,
                _ntl_gbigint p, long n, _ntl_gbigint *scratch)
{
   if (e == 0 || ZEROP(*b)) {
      _ntl_gcopy(*b, a);
      return;
   }

   long sb, nwords;

   if (a == b || ((unsigned long) n) % NTL_ZZ_NBITS != 0 ||
       (sb = SIZE(*b)) == 1 + (nwords = ((unsigned long) n) / NTL_ZZ_NBITS)) {

      _ntl_grshift(*b, n-e, scratch);
      _ntl_glowbits(*b, n-e, a);
      _ntl_glshift(*a, e, a);

      if (_ntl_gcompare(*a, *scratch) < 0) {
         _ntl_gswitchbit(a, n);
         _ntl_gsadd(*a, 1, a);
         _ntl_gsubpos(*a, *scratch, a);
      }
      else {
         _ntl_gsubpos(*a, *scratch, a);
      }

      return;
   }

   long ewords = ((unsigned long) e) / NTL_ZZ_NBITS;
   long ebits  = ((unsigned long) e) % NTL_ZZ_NBITS;

   if (MustAlloc(*a, nwords+1)) _ntl_gsetlength(a, nwords+1);

   _ntl_limb_t *adata = DATA(*a);
   _ntl_limb_t *bdata = DATA(*b);


   long special_carry = 0;
   long sa = 0;

   if (ewords) {
      long hiwords = sb - (nwords-ewords);
      if (hiwords > 0) {

         _ntl_limb_t borrow = NTL_MPN(neg)(adata, bdata + (nwords-ewords),
                                           hiwords); 
         if (hiwords < ewords) {
            if (borrow) {
               for (long i = hiwords; i < ewords; i++) 
                  adata[i] = _ntl_limb_t(-1); 
            }
            else {
               for (long i = hiwords; i < ewords; i++) 
                  adata[i] = 0;
            }
         }

         if (borrow) {
            borrow = NTL_MPN(sub_1)(adata + ewords, bdata, nwords-ewords, 1);
            if (borrow) {
               special_carry = NTL_MPN(add_1)(adata, adata, nwords, 1);
               // special case: result so far is 2^n
            }
         }
         else {
            for (long i = 0; i < nwords-ewords; i++) adata[i+ewords] = bdata[i];
         }

         sa = nwords;         
      }
      else {
         for (long i = 0; i < ewords; i++) adata[i] = 0;
         for (long i = 0; i < sb; i++) adata[i+ewords] = bdata[i];

         sa = ewords + sb;
      }
   }
   else {
      for (long i = 0; i < sb; i++) adata[i] = bdata[i];
      sa = sb;
   }

   long here = 0;

   if (ebits) {
      if (special_carry) {
         NTL_MPN(sub_1)(adata, adata, nwords, (1L << ebits) - 1L);
      }
      else if (sa == nwords) {
         _ntl_limb_t shout = NTL_MPN(lshift)(adata, adata, sa, ebits);
         if (shout) {
            _ntl_limb_t borrow = NTL_MPN(sub_1)(adata, adata, sa, shout);
            if (borrow) {
               _ntl_limb_t carry = NTL_MPN(add_1)(adata, adata, sa, 1);
               if (carry) {
                  adata[sa] = 1;
                  sa++;
               }
            }
         }
      }
      else { // sa < nwords
         _ntl_limb_t shout = NTL_MPN(lshift)(adata, adata, sa, ebits);
         if (shout) {
            adata[sa] = shout;
            sa++;
         }
      }
   }
   else {
      if (special_carry) {
         adata[sa] = 1;
         sa++;
      }
   }

   STRIP(sa, adata);
   SIZE(*a) = sa;

}

void 
_ntl_ss_addmod(_ntl_gbigint *x, const _ntl_gbigint *a,
               const _ntl_gbigint *b, _ntl_gbigint p, long n)
{
   if (((unsigned long) n) % NTL_ZZ_NBITS != 0) { 
      _ntl_gadd(*a, *b, x);
      if (_ntl_gcompare(*x, p) >= 0) {
         _ntl_gsadd(*x, -1, x);
         _ntl_gswitchbit(x, n);
      }
   }
   else {
      _ntl_gadd(*a, *b, x);
      long sx, nwords;
      if (!*x ||
          (sx = SIZE(*x)) <= (nwords = ((unsigned long) n) / NTL_ZZ_NBITS))
         return;

      _ntl_limb_t *xdata = DATA(*x);
      if (xdata[nwords] == 2) {
         for (long i = 0; i < nwords; i++) xdata[i] = _ntl_limb_t(-1);
         SIZE(*x) = nwords;
         return;
      }

      long i = nwords-1;
      while (i >= 0 && xdata[i] == 0) i--;
      if (i < 0) return;

      NTL_MPN(sub_1)(xdata, xdata, nwords, 1);
      sx = nwords;
      STRIP(sx, xdata);
      SIZE(*x) = sx;
   }
}


void 
_ntl_ss_submod(_ntl_gbigint *x, const _ntl_gbigint *a,
               const _ntl_gbigint *b, _ntl_gbigint p, long n)
{
   if (((unsigned long) n) % NTL_ZZ_NBITS != 0) {
      if (_ntl_gcompare(*a, *b) < 0) {
         _ntl_gadd(*a, p, x);
         _ntl_gsubpos(*x, *b, x);
      }
      else {
         _ntl_gsubpos(*a, *b, x);
      }
   }
   else {
      if (ZEROP(*b)) {
         _ntl_gcopy(*a, x);
         return;
      }

      long sb = SIZE(*b);
      _ntl_limb_t *bdata = DATA(*b);

      long sa;

      if (!*a) 
         sa = 0;
      else
         sa = SIZE(*a);

      long nwords = ((unsigned long) n) / NTL_ZZ_NBITS;
      if (MustAlloc(*x, nwords+1)) _ntl_gsetlength(x, nwords+1);
      _ntl_limb_t *xdata = DATA(*x);

      if (sa >= sb) {
         _ntl_limb_t *adata = DATA(*a);
         _ntl_limb_t borrow = NTL_MPN(sub)(xdata, adata, sa, bdata, sb);
         if (borrow) {
            for (long i = sa; i < nwords; i++) xdata[i] = _ntl_limb_t(-1);
            _ntl_limb_t carry = NTL_MPN(add_1)(xdata, xdata, nwords, 1);
            if (carry) {
               xdata[nwords] = 1;
               SIZE(*x) = nwords+1;
            }
            else {
               long sx = nwords;
               STRIP(sx, xdata);
               SIZE(*x) = sx;
            }
         }
         else {
            long sx = sa;
            STRIP(sx, xdata);
            SIZE(*x) = sx;
         }
      }
      else {
         if (sa == 0) {
            xdata[0] = 1;
         }
         else {
            _ntl_limb_t *adata = DATA(*a); 
            xdata[sa] = NTL_MPN(add_1)(xdata, adata, sa, 1);
         }
         for (long i = sa+1; i <= nwords; i++) xdata[i] = 0;
         xdata[nwords]++;
         _ntl_limb_t borrow = NTL_MPN(sub_n)(xdata, xdata, bdata, sb);
         if (borrow) {
            NTL_MPN(sub_1)(xdata+sb, xdata+sb, nwords+1-sb, 1);
         }
         long sx = nwords+1;
         STRIP(sx, xdata);
         SIZE(*x) = sx;
      }
   }
}

#endif




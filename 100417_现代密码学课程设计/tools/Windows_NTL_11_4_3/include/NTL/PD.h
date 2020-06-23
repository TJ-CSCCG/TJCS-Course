#ifndef NTL_PD__H
#define NTL_PD__H

#include <NTL/tools.h>
#include <immintrin.h>

NTL_OPEN_NNS


template<int N>
struct PD {
private:
   PD();
};


// FIXME: should distinguish more carefully:
//   AVX512DQ for long/double conversions
//   AVX512VL for certain ops applied to shorter types:
//      long/double conversions and mask ops
// may need to translate long/double conversions for non-AVXDQ512



//=================== PD<8> implementation ===============

#ifdef NTL_HAVE_AVX512F

template<>
struct PD<8> {
   __m512d data;

   enum { size = 8};

   PD() { }
   PD(double x) : data(_mm512_set1_pd(x)) { }
   PD(__m512d _data) : data(_data) { }

   PD(double d0, double d1, double d2, double d3,
      double d4, double d5, double d6, double d7)
      : data(_mm512_set_pd(d7, d6, d5, d4, d3, d2, d1, d0)) { }

   static PD load(const double *p) { return _mm512_load_pd(p); } 

   // load from unaligned address
   static PD loadu(const double *p) { return _mm512_loadu_pd(p); } 
};

inline void
load(PD<8>& x, const double *p) 
{ x = PD<8>::load(p); }

// load from unaligned address
inline void
loadu(PD<8>& x, const double *p) 
{ x = PD<8>::loadu(p); }

inline void 
store(double *p, PD<8> a) 
{ _mm512_store_pd(p, a.data); }

// store to unaligned address
inline void 
storeu(double *p, PD<8> a) 
{ _mm512_storeu_pd(p, a.data); }

// load and convert
inline void
load(PD<8>& x, const long *p)
{ __m512i a = _mm512_load_epi64(p);  x = _mm512_cvtepi64_pd(a); }

// load unaligned and convert
inline void
loadu(PD<8>& x, const long *p)
{ __m512i a = _mm512_loadu_si512(p);  x = _mm512_cvtepi64_pd(a); }

// convert and store
inline void 
store(long *p, PD<8> a)
{  __m512i b = _mm512_cvtpd_epi64(a.data); _mm512_store_epi64(p, b); }

// convert and store unaligned
inline void 
storeu(long *p, PD<8> a)
{  __m512i b = _mm512_cvtpd_epi64(a.data); _mm512_storeu_si512(p, b); }


// swap even/odd slots
// e.g., 01234567 -> 10325476
inline PD<8> 
swap2(PD<8> a) 
{ return _mm512_permute_pd(a.data, 0x55); }

// swap even/odd slot-pairs
// e.g., 01234567 -> 23016745
inline PD<8> 
swap4(PD<8> a) 
{ return _mm512_permutex_pd(a.data, 0x4e); }

// 01234567 -> 00224466
inline PD<8> 
dup2even(PD<8> a)
{ return _mm512_permute_pd(a.data, 0);   }

// 01234567 -> 11335577
inline PD<8> 
dup2odd(PD<8> a)
{ return _mm512_permute_pd(a.data, 0xff);   }

// 01234567 -> 01014545
inline PD<8> 
dup4even(PD<8> a)
{ return _mm512_permutex_pd(a.data, 0x44);   }

// 01234567 -> 23236767
inline PD<8> 
dup4odd(PD<8> a)
{ return _mm512_permutex_pd(a.data, 0xee);   }

// blend even/odd slots
// 01234567, 89abcdef -> 092b4d6f
inline PD<8> 
blend2(PD<8> a, PD<8> b)
{ return _mm512_mask_blend_pd(0xaa, a.data, b.data); }
// FIXME: why isn't there an intrinsic that doesn't require a mask register?

// blend even/odd slot-pairs
// 01234567, 89abcdef -> 01ab45ef
inline PD<8>
blend4(PD<8> a, PD<8> b)
{ return _mm512_mask_blend_pd(0xcc, a.data, b.data); }
// FIXME: why isn't there an intrinsic that doesn't require a mask register?

// res[i] = a[i] < b[i] ? a[i] : a[i]-b[i]
inline PD<8>
correct_excess(PD<8> a, PD<8> b)
{ 
   __mmask8 k = _mm512_cmp_pd_mask(a.data, b.data, _CMP_GE_OQ);
   return _mm512_mask_sub_pd(a.data, k, a.data, b.data); 
}

// res[i] = a[i] >= 0 ? a[i] : a[i]+b[i]
inline PD<8>
correct_deficit(PD<8> a, PD<8> b)
{ 
   __mmask8 k = _mm512_cmp_pd_mask(a.data, _mm512_setzero_pd(), _CMP_LT_OQ);
   return _mm512_mask_add_pd(a.data, k, a.data, b.data); 
}

inline void 
clear(PD<8>& x) 
{ x.data = _mm512_setzero_pd(); }

inline PD<8> 
operator+(PD<8> a, PD<8> b) 
{ return _mm512_add_pd(a.data, b.data); }

inline PD<8> 
operator-(PD<8> a, PD<8> b) 
{ return _mm512_sub_pd(a.data, b.data); }

inline PD<8> 
operator*(PD<8> a, PD<8> b) 
{ return _mm512_mul_pd(a.data, b.data); }

inline PD<8> 
operator/(PD<8> a, PD<8> b) 
{ return _mm512_div_pd(a.data, b.data); }

inline PD<8>&
operator+=(PD<8>& a, PD<8> b)
{ a = a + b; return a; }

inline PD<8>&
operator-=(PD<8>& a, PD<8> b)
{ a = a - b; return a; }

inline PD<8>&
operator*=(PD<8>& a, PD<8> b)
{ a = a * b; return a; }

inline PD<8>&
operator/=(PD<8>& a, PD<8> b)
{ a = a / b; return a; }

// a*b+c (fused)
inline PD<8> 
fused_muladd(PD<8> a, PD<8> b, PD<8> c) 
{ return _mm512_fmadd_pd(a.data, b.data, c.data); }

// a*b-c (fused)
inline PD<8> 
fused_mulsub(PD<8> a, PD<8> b, PD<8> c) 
{ return _mm512_fmsub_pd(a.data, b.data, c.data); }

// -a*b+c (fused)
inline PD<8> 
fused_negmuladd(PD<8> a, PD<8> b, PD<8> c) 
{ return _mm512_fnmadd_pd(a.data, b.data, c.data); }

#endif

//=================== PD<4> implementation ===============

#if (defined(NTL_HAVE_AVX2) && defined(NTL_HAVE_FMA))

template<>
struct PD<4> {
   __m256d data;

   enum { size = 4};

   PD() { }
   PD(double x) : data(_mm256_set1_pd(x)) { }
   PD(__m256d _data) : data(_data) { }
   PD(double d0, double d1, double d2, double d3)
      : data(_mm256_set_pd(d3, d2, d1, d0)) { }

   static PD load(const double *p) { return _mm256_load_pd(p); } 

   // load from unaligned address
   static PD loadu(const double *p) { return _mm256_loadu_pd(p); } 
};

inline void
load(PD<4>& x, const double *p) 
{ x = PD<4>::load(p); }

// load from unaligned address
inline void
loadu(PD<4>& x, const double *p) 
{ x = PD<4>::loadu(p); }

inline void 
store(double *p, PD<4> a) 
{ _mm256_store_pd(p, a.data); }

// store to unaligned address
inline void 
storeu(double *p, PD<4> a) 
{ _mm256_storeu_pd(p, a.data); }





// The following assume all numbers are integers
// in the range [0, 2^52).   The idea is taken from here:
// https://stackoverflow.com/questions/41144668/how-to-efficiently-perform-double-int64-conversions-with-sse-avx


// Some of the Intel intrinsics for loading and storing packed
// integers from memory require casting between long* and __m256i*.
// Strictly speaking, this can break strict aliasing rules, but
// this is hopefully not a problem. 
// See discussion here:
// https://stackoverflow.com/questions/24787268/how-to-implement-mm-storeu-epi64-without-aliasing-problems


// load and convert
inline void
load(PD<4>& x, const long *p)
{ 
#ifdef NTL_HAVE_AVX512F
   __m256i a = _mm256_load_si256((const __m256i*)p);  
   x = _mm256_cvtepi64_pd(a); 
#else
   __m256i a = _mm256_load_si256((const __m256i*)p);
   a = _mm256_or_si256(a, _mm256_castpd_si256(_mm256_set1_pd(1L << 52)));
   x = _mm256_sub_pd(_mm256_castsi256_pd(a), _mm256_set1_pd(1L << 52));
#endif
}

// load unaligned and convert
inline void
loadu(PD<4>& x, const long *p)
{ 
#ifdef NTL_HAVE_AVX512F
   __m256i a = _mm256_loadu_si256((const __m256i*)p);  x = _mm256_cvtepi64_pd(a); 
#else
   __m256i a = _mm256_loadu_si256((const __m256i*)p);
   a = _mm256_or_si256(a, _mm256_castpd_si256(_mm256_set1_pd(1L << 52)));
   x = _mm256_sub_pd(_mm256_castsi256_pd(a), _mm256_set1_pd(1L << 52));
#endif
}

// convert and store
inline void 
store(long *p, PD<4> a)
{  
#ifdef NTL_HAVE_AVX512F
   __m256i b = _mm256_cvtpd_epi64(a.data); 
#ifdef __clang__
   _mm256_store_si256((__m256i*)p, b);
#else
// clang doesn't define this...why??
   _mm256_store_epi64(p, b); 
#endif
#else
    __m256d x = a.data;
    x = _mm256_add_pd(x, _mm256_set1_pd(1L << 52));
    __m256i b = _mm256_xor_si256(
        _mm256_castpd_si256(x),
        _mm256_castpd_si256(_mm256_set1_pd(1L << 52)));
    _mm256_store_si256((__m256i*)p, b);
#endif
}

// convert and store unaligned
inline void 
storeu(long *p, PD<4> a)
{  
#ifdef NTL_HAVE_AVX512F
   __m256i b = _mm256_cvtpd_epi64(a.data); 
  _mm256_storeu_si256((__m256i*)p, b); 
#else
    __m256d x = a.data;
    x = _mm256_add_pd(x, _mm256_set1_pd(1L << 52));
    __m256i b = _mm256_xor_si256(
        _mm256_castpd_si256(x),
        _mm256_castpd_si256(_mm256_set1_pd(1L << 52)));
    _mm256_storeu_si256((__m256i*)p, b);
#endif
}


// swap even/odd slots
// e.g., 0123 -> 1032
inline PD<4> 
swap2(PD<4> a) 
{ return _mm256_permute_pd(a.data, 0x5); }

// 0123 -> 0022
inline PD<4> 
dup2even(PD<4> a)
{ return _mm256_permute_pd(a.data, 0);   }

// 0123 -> 1133
inline PD<4> 
dup2odd(PD<4> a)
{ return _mm256_permute_pd(a.data, 0xf);   }

// blend even/odd slots
// 0123, 4567 -> 0527
inline PD<4> 
blend2(PD<4> a, PD<4> b)
{ return _mm256_blend_pd(a.data, b.data, 0xa); }

// res[i] = a[i] < b[i] ? a[i] : a[i]-b[i]
inline PD<4>
correct_excess(PD<4> a, PD<4> b)
{ 
#ifdef NTL_HAVE_AVX512F
   __mmask8 k = _mm256_cmp_pd_mask(a.data, b.data, _CMP_GE_OQ);
   return _mm256_mask_sub_pd(a.data, k, a.data, b.data); 
#else
   __m256d mask = _mm256_cmp_pd(a.data, b.data, _CMP_GE_OQ);
   __m256d corrected = _mm256_sub_pd(a.data, b.data);
   return _mm256_blendv_pd(a.data, corrected, mask);
#endif
}

// res[i] = a[i] >= 0 ? a[i] : a[i]+b[i]
inline PD<4>
correct_deficit(PD<4> a, PD<4> b)
{ 
#ifdef NTL_HAVE_AVX512F
   __mmask8 k = _mm256_cmp_pd_mask(a.data, _mm256_setzero_pd(), _CMP_LT_OQ);
   return _mm256_mask_add_pd(a.data, k, a.data, b.data); 
#else
   __m256d mask = _mm256_cmp_pd(a.data, _mm256_setzero_pd(), _CMP_LT_OQ);
   __m256d corrected = _mm256_add_pd(a.data, b.data);
   return _mm256_blendv_pd(a.data, corrected, mask);
#endif
}

inline void 
clear(PD<4>& x) 
{ x.data = _mm256_setzero_pd(); }

inline PD<4> 
operator+(PD<4> a, PD<4> b) 
{ return _mm256_add_pd(a.data, b.data); }

inline PD<4> 
operator-(PD<4> a, PD<4> b) 
{ return _mm256_sub_pd(a.data, b.data); }

inline PD<4> 
operator*(PD<4> a, PD<4> b) 
{ return _mm256_mul_pd(a.data, b.data); }

inline PD<4> 
operator/(PD<4> a, PD<4> b) 
{ return _mm256_div_pd(a.data, b.data); }

inline PD<4>&
operator+=(PD<4>& a, PD<4> b)
{ a = a + b; return a; }

inline PD<4>&
operator-=(PD<4>& a, PD<4> b)
{ a = a - b; return a; }

inline PD<4>&
operator*=(PD<4>& a, PD<4> b)
{ a = a * b; return a; }

inline PD<4>&
operator/=(PD<4>& a, PD<4> b)
{ a = a / b; return a; }

// a*b+c (fused)
inline PD<4> 
fused_muladd(PD<4> a, PD<4> b, PD<4> c) 
{ return _mm256_fmadd_pd(a.data, b.data, c.data); }

// a*b-c (fused)
inline PD<4> 
fused_mulsub(PD<4> a, PD<4> b, PD<4> c) 
{ return _mm256_fmsub_pd(a.data, b.data, c.data); }

// -a*b+c (fused)
inline PD<4> 
fused_negmuladd(PD<4> a, PD<4> b, PD<4> c) 
{ return _mm256_fnmadd_pd(a.data, b.data, c.data); }


//=================== PD<2> implementation ===============


template<>
struct PD<2> {
   __m128d data;

   enum { size = 2};

   PD() { }
   PD(double x) : data(_mm_set1_pd(x)) { }
   PD(__m128d _data) : data(_data) { }
   PD(double d0, double d1)
      : data(_mm_set_pd(d1, d0)) { }

   static PD load(const double *p) { return _mm_load_pd(p); } 

   // load from unaligned address
   static PD loadu(const double *p) { return _mm_loadu_pd(p); } 
};

inline void
load(PD<2>& x, const double *p) 
{ x = PD<2>::load(p); }

// load from unaligned address
inline void
loadu(PD<2>& x, const double *p) 
{ x = PD<2>::loadu(p); }

inline void 
store(double *p, PD<2> a) 
{ _mm_store_pd(p, a.data); }

// store to unaligned address
inline void 
storeu(double *p, PD<2> a) 
{ _mm_storeu_pd(p, a.data); }





// The following assume all numbers are integers
// in the range [0, 2^52).   The idea is taken from here:
// https://stackoverflow.com/questions/41144668/how-to-efficiently-perform-double-int64-conversions-with-sse-avx

// load and convert
inline void
load(PD<2>& x, const long *p)
{ 
#ifdef NTL_HAVE_AVX512F
   __m128i a = _mm_load_si128((const __m128i*)p);  
   x = _mm_cvtepi64_pd(a); 
#else
   __m128i a = _mm_load_si128((const __m128i*)p);
   a = _mm_or_si128(a, _mm_castpd_si128(_mm_set1_pd(1L << 52)));
   x = _mm_sub_pd(_mm_castsi128_pd(a), _mm_set1_pd(1L << 52));
#endif
}

// load unaligned and convert
inline void
loadu(PD<2>& x, const long *p)
{ 
#ifdef NTL_HAVE_AVX512F
   __m128i a = _mm_loadu_si128((const __m128i*)p);  x = _mm_cvtepi64_pd(a); 
#else
   __m128i a = _mm_loadu_si128((const __m128i*)p);
   a = _mm_or_si128(a, _mm_castpd_si128(_mm_set1_pd(1L << 52)));
   x = _mm_sub_pd(_mm_castsi128_pd(a), _mm_set1_pd(1L << 52));
#endif
}

// convert and store
inline void 
store(long *p, PD<2> a)
{  
#ifdef NTL_HAVE_AVX512F
   __m128i b = _mm_cvtpd_epi64(a.data); 
#ifdef __clang__
   _mm_store_si128((__m128i*)p, b);
#else
// clang doesn't define this...why??
   _mm_store_epi64(p, b); 
#endif
#else
    __m128d x = a.data;
    x = _mm_add_pd(x, _mm_set1_pd(1L << 52));
    __m128i b = _mm_xor_si128(
        _mm_castpd_si128(x),
        _mm_castpd_si128(_mm_set1_pd(1L << 52)));
    _mm_store_si128((__m128i*)p, b);
#endif
}

// convert and store unaligned
inline void 
storeu(long *p, PD<2> a)
{  
#ifdef NTL_HAVE_AVX512F
   __m128i b = _mm_cvtpd_epi64(a.data); 
  _mm_storeu_si128((__m128i*)p, b); 
#else
    __m128d x = a.data;
    x = _mm_add_pd(x, _mm_set1_pd(1L << 52));
    __m128i b = _mm_xor_si128(
        _mm_castpd_si128(x),
        _mm_castpd_si128(_mm_set1_pd(1L << 52)));
    _mm_storeu_si128((__m128i*)p, b);
#endif
}


// res[i] = a[i] < b[i] ? a[i] : a[i]-b[i]
inline PD<2>
correct_excess(PD<2> a, PD<2> b)
{ 
#ifdef NTL_HAVE_AVX512F
   __mmask8 k = _mm_cmp_pd_mask(a.data, b.data, _CMP_GE_OQ);
   return _mm_mask_sub_pd(a.data, k, a.data, b.data); 
#else
   __m128d mask = _mm_cmp_pd(a.data, b.data, _CMP_GE_OQ);
   __m128d corrected = _mm_sub_pd(a.data, b.data);
   return _mm_blendv_pd(a.data, corrected, mask);
#endif
}

// res[i] = a[i] >= 0 ? a[i] : a[i]+b[i]
inline PD<2>
correct_deficit(PD<2> a, PD<2> b)
{ 
#ifdef NTL_HAVE_AVX512F
   __mmask8 k = _mm_cmp_pd_mask(a.data, _mm_setzero_pd(), _CMP_LT_OQ);
   return _mm_mask_add_pd(a.data, k, a.data, b.data); 
#else
   __m128d mask = _mm_cmp_pd(a.data, _mm_setzero_pd(), _CMP_LT_OQ);
   __m128d corrected = _mm_add_pd(a.data, b.data);
   return _mm_blendv_pd(a.data, corrected, mask);
#endif
}

inline void 
clear(PD<2>& x) 
{ x.data = _mm_setzero_pd(); }

inline PD<2> 
operator+(PD<2> a, PD<2> b) 
{ return _mm_add_pd(a.data, b.data); }

inline PD<2> 
operator-(PD<2> a, PD<2> b) 
{ return _mm_sub_pd(a.data, b.data); }

inline PD<2> 
operator*(PD<2> a, PD<2> b) 
{ return _mm_mul_pd(a.data, b.data); }

inline PD<2> 
operator/(PD<2> a, PD<2> b) 
{ return _mm_div_pd(a.data, b.data); }

inline PD<2>&
operator+=(PD<2>& a, PD<2> b)
{ a = a + b; return a; }

inline PD<2>&
operator-=(PD<2>& a, PD<2> b)
{ a = a - b; return a; }

inline PD<2>&
operator*=(PD<2>& a, PD<2> b)
{ a = a * b; return a; }

inline PD<2>&
operator/=(PD<2>& a, PD<2> b)
{ a = a / b; return a; }

// a*b+c (fused)
inline PD<2> 
fused_muladd(PD<2> a, PD<2> b, PD<2> c) 
{ return _mm_fmadd_pd(a.data, b.data, c.data); }

// a*b-c (fused)
inline PD<2> 
fused_mulsub(PD<2> a, PD<2> b, PD<2> c) 
{ return _mm_fmsub_pd(a.data, b.data, c.data); }

// -a*b+c (fused)
inline PD<2> 
fused_negmuladd(PD<2> a, PD<2> b, PD<2> c) 
{ return _mm_fnmadd_pd(a.data, b.data, c.data); }




//================== PD<8>/PD<4> conversions ================

#ifdef NTL_HAVE_AVX512F 

// 0123, 4567 -> 01234567
inline PD<8> 
join(PD<4> a, PD<4> b)
{ 
   __m512d c = _mm512_castpd256_pd512(a.data);
   return _mm512_insertf64x4(c, b.data, 1);
}

// 01234567 -> 0123
inline PD<4>
get_lo(PD<8> a)
{  return _mm512_extractf64x4_pd(a.data, 0); }

// 01234567 -> 4567
inline PD<4>
get_hi(PD<8> a)
{  return _mm512_extractf64x4_pd(a.data, 1); }

#endif

//================== PD<4>/PD<2> conversions ================

// 01, 23 -> 0123
inline PD<4> 
join(PD<2> a, PD<2> b)
#if 0
// some versions of gcc are buggy and don't define this function
{ return _mm256_set_m128d(b.data, a.data); }
#else
{ return _mm256_insertf128_pd(_mm256_castpd128_pd256(a.data), b.data, 1); }
#endif


// 0123 -> 01
inline PD<2>
get_lo(PD<4> a)
{  return _mm256_extractf128_pd(a.data, 0); }

// 0123 -> 23
inline PD<2>
get_hi(PD<4> a)
{  return _mm256_extractf128_pd(a.data, 1); }


#endif


NTL_CLOSE_NNS


#endif

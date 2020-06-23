
#ifndef NTL_ctools__H
#define NTL_ctools__H

#include <NTL/config.h>
#include <NTL/mach_desc.h>

#include <NTL/ALL_FEATURES.h>

#include <NTL/PackageInfo.h>

#if (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
#define NTL_GNUC_INTEL
#endif

#if (!defined(NTL_HAVE_LL_TYPE) && defined(NTL_WINPACK) &&  (defined(_MSC_VER) || defined(NTL_GNUC_INTEL)))
// for the windows distribution, 
//   we assume LL_TYPE works for MSVC++ (which is true for both x86 and ARM)
//   and for GNUC/Intel platforms (e.g., Code Blocks)
#define NTL_HAVE_LL_TYPE
#endif

// Define the working C++ standard.
// Both NTL_STD_CXX14 and NTL_STD_CXX11, and we take the highest one

#if defined(NTL_STD_CXX14)
#define NTL_CXX_STANDARD (2014)
#elif defined(NTL_STD_CXX11)
#define NTL_CXX_STANDARD (2011)
#else
#define NTL_CXX_STANDARD (1998)
#endif

// define some macros regarding noexcept declarations

#if (NTL_CXX_STANDARD >= 2011)

#define NTL_NOEXCEPT noexcept

#ifdef NTL_EXCEPTIONS
#define NTL_FAKE_NOEXCEPT
#else
#define NTL_FAKE_NOEXCEPT noexcept
#endif

#else

#define NTL_NOEXCEPT 
#define NTL_FAKE_NOEXCEPT

#endif


/*
 * Resolve double-word integer type.
 *
 * Unfortunately, there is no "standard" way to do this.
 * On 32-bit machines, 'long long' usually works (but not
 * on MSVC++ or BORLAND), and on 64-bit machines, there is
 * no standard.  However, most compilers do offer *some*
 * non-standard double-word type.  
 *
 * Note that C99 creates a standard header <stdint.h>,
 * but it is not clear how widely this is implemented,
 * and for example, older versions of GCC does not provide a type int128_t 
 * in <stdint.h> on 64-bit machines.
 */



#if (defined(NTL_UNSIGNED_LONG_LONG_TYPE))

#define NTL_ULL_TYPE NTL_UNSIGNED_LONG_LONG_TYPE

#elif (NTL_BITS_PER_LONG == 64 && defined(__GNUC__))

#define NTL_ULL_TYPE __uint128_t 

#elif (NTL_BITS_PER_LONG == 32 && (defined(_MSC_VER) || defined(__BORLANDC__)))

#define NTL_ULL_TYPE unsigned __int64

#elif (NTL_BITS_PER_LONG == 64 && (defined(_MSC_VER) || defined(__BORLANDC__)))

#define NTL_ULL_TYPE unsigned __int128

#endif

#if (!defined(NTL_ULL_TYPE))

#define NTL_ULL_TYPE unsigned long long

#endif


#ifdef NTL_HAVE_LL_TYPE

typedef NTL_ULL_TYPE _ntl_ulonglong;
// typenames are more convenient than macros

#else

#undef NTL_ULL_TYPE
// prevent any use of these macros

class _ntl_ulonglong { private: _ntl_ulonglong() { } };
// cannot create variables of these types


#endif

/********************************************************/



// Define an unsigned type with at least 32 bits
// there is no truly portable way to do this, yet...


#if (NTL_BITS_PER_INT >= 32)

typedef unsigned int _ntl_uint32; // 32-bit word
#define NTL_BITS_PER_INT32 NTL_BITS_PER_INT

#else

// NOTE: C++ standard guarantees longs are at least 32-bits wide,
// and this is also explicitly checked at builod time

typedef unsigned long _ntl_uint32; // 32-bit word
#define NTL_BITS_PER_INT32 NTL_BITS_PER_LONG

#endif



// The usual token pasting stuff...

#define NTL_PASTE_TOKENS2(a,b) a ## b
#define NTL_PASTE_TOKENS(a,b) NTL_PASTE_TOKENS2(a,b)

#define NTL_STRINGIFY(x) NTL_STRINGIFY_AUX(x)
#define NTL_STRINGIFY_AUX(x) #x






#define NTL_OVFBND (1L << (NTL_BITS_PER_LONG-4))

/*
 * NTL_OVFBND is the general bound used throughout NTL to keep various
 * integer values comfortably bounded away from an integer overflow
 * condition.  Do not change this value!
 */





#if ((NTL_BITS_PER_SIZE_T-1) < (NTL_BITS_PER_LONG-4))
#define NTL_OVFBND1 (1L << (NTL_BITS_PER_SIZE_T-1))
#else
#define NTL_OVFBND1 NTL_OVFBND
#endif

/*
 * NTL_OVFBND1 is a smaller bound than NTL_OVF when size_t is
 * narrower than long.  This prevents overflow on calls to malloc
 * and realloc.
 */






#define NTL_OVERFLOW(n, a, b) \
   (((b) >= NTL_OVFBND) || (((long) (n)) > 0 && (((a) >= NTL_OVFBND) || \
    (((long) (n)) >= (NTL_OVFBND-((long)(b))+((long)(a))-1)/((long)(a))))))

/*
 * NTL_OVERFLOW(n, a, b) returns 1 if n*a + b >= NTL_OVFBND,
 * and returns 0 otherwise.  The value n is effectively treated as type long,
 * while the values a and b may be *any* integral type.  It is assumed that
 * n >= 0, a > 0, and b >= 0.  Care is taken to ensure that overflow does
 * not occur. If a and b are constants, and n has no side effects,
 * a good optimizing compiler will * translate this into a single test 
 * of the form n >= c, where c is a constant.
 */






#define NTL_OVERFLOW1(n, a, b) \
   (((b) >= NTL_OVFBND1) || (((long) (n)) > 0 && (((a) >= NTL_OVFBND1) || \
    (((long) (n)) >= (NTL_OVFBND1-((long)(b))+((long)(a))-1)/((long)(a))))))

/*
 * NTL_OVERFLOW1 is the same as NTL_OVERFLOW, except that it uses the
 * bound NTL_OVFBND1 instead of NTL_OVFBND.
 */




#ifdef NTL_TEST_EXCEPTIONS

extern unsigned long exception_counter;

#define NTL_BASIC_MALLOC(n, a, b) \
   (NTL_OVERFLOW1(n, a, b) ? ((void *) 0) : \
    ((void *) malloc(((long)(n))*((long)(a)) + ((long)(b)))))

#define NTL_MALLOC(n, a, b) \
   (--exception_counter == 0 ? (void *) 0 : NTL_BASIC_MALLOC(n, a, b))

#else

#define NTL_MALLOC(n, a, b) \
   (NTL_OVERFLOW1(n, a, b) ? ((void *) 0) : \
    ((void *) malloc(((long)(n))*((long)(a)) + ((long)(b)))))


#endif

/*
 * NTL_MALLOC(n, a, b) returns 0 if a*n + b >= NTL_OVFBND1, and otherwise
 * returns malloc(n*a + b). 
 * The programmer must ensure that the name "malloc" is visible
 * at the point in the source code where this macro is expanded.
 */


#ifdef NTL_TEST_EXCEPTIONS

#define NTL_BASIC_SNS_MALLOC(n, a, b) \
   (NTL_OVERFLOW1(n, a, b) ? ((void *) 0) : \
    ((void *) NTL_SNS malloc(((long)(n))*((long)(a)) + ((long)(b)))))


#define NTL_SNS_MALLOC(n, a, b) \
   (--exception_counter == 0 ? (void *) 0 : NTL_BASIC_SNS_MALLOC(n, a, b))


#else

#define NTL_SNS_MALLOC(n, a, b) \
   (NTL_OVERFLOW1(n, a, b) ? ((void *) 0) : \
    ((void *) NTL_SNS malloc(((long)(n))*((long)(a)) + ((long)(b)))))

#endif

/*
 * NTL_SNS_MALLOC is the same as NTL_MALLOC, except that the call
 * to malloc is prefixed by NTL_SNS.
 */








#define NTL_REALLOC(p, n, a, b) \
   (NTL_OVERFLOW1(n, a, b) ? ((void *) 0) : \
    ((void *) realloc((p), ((long)(n))*((long)(a)) + ((long)(b)))))

/*
 * NTL_REALLOC(n, a, b) returns 0 if a*n + b >= NTL_OVFBND1, and otherwise
 * returns realloc(p, n*a + b).
 * The programmer must ensure that the name "realloc" is visible
 * at the point in the source code where this macro is expanded.
 */






#define NTL_SNS_REALLOC(p, n, a, b) \
   (NTL_OVERFLOW1(n, a, b) ? ((void *) 0) : \
    ((void *) NTL_SNS realloc((p), ((long)(n))*((long)(a)) + ((long)(b)))))

/*
 * NTL_SNS_REALLOC is the same as NTL_REALLOC, except that the call
 * to realloc is prefixed by NTL_SNS.
 */





#define NTL_MAX_ALLOC_BLOCK (40000)

/*
 * NTL_MAX_ALLOC_BLOCK is the number of bytes that are allocated in
 * a single block in a number of places throughout NTL (for
 * vec_ZZ_p, ZZVec, vec_GF2X, and GF2XVec).
 */


#define NTL_ULONG_TO_LONG(a) \
   ((((unsigned long) a) >> (NTL_BITS_PER_LONG-1)) ? \
    (((long) (((unsigned long) a) ^ ((unsigned long) NTL_MIN_LONG))) ^ \
       NTL_MIN_LONG) : \
    ((long) a))

/* 
 * This macro converts from unsigned long to signed long.  It is portable
 * among platforms for which a long has a 2's complement representation
 * of the same width as an unsigned long.  While it avoids assumptions
 * about the behavior of non-standard conversions,  a good optimizing
 * compiler should turn it into the identity function.
 */


#define NTL_UINT_TO_INT(a) \
   ((((unsigned int) a) >> (NTL_BITS_PER_INT-1)) ? \
    (((int) (((unsigned int) a) ^ ((unsigned int) NTL_MIN_INT))) ^ \
       NTL_MIN_INT) : \
    ((int) a))

/* 
 * This macro converts from unsigned int to signed int.  It is portable
 * among platforms for which an int has a 2's complement representation
 * of the same width as an unsigned int.  While it avoids assumptions
 * about the behavior of non-standard conversions,  a good optimizing
 * compiler should turn it into the identity function.
 */


#ifdef NTL_THREADS

#define NTL_THREAD_LOCAL thread_local 

#ifdef __GNUC__
#define NTL_CHEAP_THREAD_LOCAL __thread
#else
#define NTL_CHEAP_THREAD_LOCAL thread_local
#endif

#else

#define NTL_THREAD_LOCAL 
#define NTL_CHEAP_THREAD_LOCAL 

#endif


#define NTL_RELEASE_THRESH (128)

/*
 * threshold for releasing scratch memory.
 */



double _ntl_GetWallTime();


long _ntl_IsFinite(double *p);
/* This forces a double into memory, and tests if it is "normal";
   that means, not NaN, not +/- infinity, not denormalized, etc.
   Forcing into memory is sometimes necessary on machines 
   with "extended" double precision registers (e.g., Intel x86s)
   to force the standard IEEE format. */

void _ntl_ForceToMem(double *p);
/* This is do-nothing routine that has the effect of forcing
   a double into memory (see comment above). */

double _ntl_ldexp(double x, long e);


#define NTL_DEFINE_SWAP(T)\
inline void _ntl_swap(T& a, T& b)\
{\
   T t = a; a = b; b = t;\
}

NTL_DEFINE_SWAP(long)
NTL_DEFINE_SWAP(int)
NTL_DEFINE_SWAP(short)
NTL_DEFINE_SWAP(char)

NTL_DEFINE_SWAP(unsigned long)
NTL_DEFINE_SWAP(unsigned int)
NTL_DEFINE_SWAP(unsigned short)
NTL_DEFINE_SWAP(unsigned char)

NTL_DEFINE_SWAP(double)
NTL_DEFINE_SWAP(float)

   
template<class T>
void _ntl_swap(T*& a, T*& b)
{
   T* t = a; a = b; b = t;
}

/* These are convenience routines.  I don't want it to overload
   the std library's swap function, nor do I want to rely on the latter,
   as the C++ standard is kind of broken on the issue of where
   swap is defined. And I also only want it defined for built-in types.
 */


// The following do for "move" what the above does for swap

#define NTL_DEFINE_SCALAR_MOVE(T)\
inline T _ntl_scalar_move(T& a)\
{\
   T t = a; a = 0; return t;\
}

NTL_DEFINE_SCALAR_MOVE(long)
NTL_DEFINE_SCALAR_MOVE(int)
NTL_DEFINE_SCALAR_MOVE(short)
NTL_DEFINE_SCALAR_MOVE(char)

NTL_DEFINE_SCALAR_MOVE(unsigned long)
NTL_DEFINE_SCALAR_MOVE(unsigned int)
NTL_DEFINE_SCALAR_MOVE(unsigned short)
NTL_DEFINE_SCALAR_MOVE(unsigned char)

NTL_DEFINE_SCALAR_MOVE(double)
NTL_DEFINE_SCALAR_MOVE(float)

   
template<class T>
T* _ntl_scalar_move(T*& a)
{
   T *t = a; a = 0; return t;
}





// The following routine increments a pointer so that
// it is properly aligned.  
// It is assumed that align > 0.
// If align is a constant power of 2, it compiles
// into a small handful of simple instructions.

#if (NTL_BIG_POINTERS)

#define NTL_UPTRINT_T unsigned long long
// DIRT: this should really be std::uintptr_t, defined
// in <cstdint>; however, that header is not widely available,
// and even if it were, std::uintptr_t is not guaranteed
// to be defined.  Of course, unsigned long long may not
// be defined in pre-C++11.  

#else

#define NTL_UPTRINT_T unsigned long 

#endif


#ifdef NTL_HAVE_ALIGNED_ARRAY

inline
char *_ntl_make_aligned(char *p, long align)
{
   unsigned long r =  (unsigned long) (((NTL_UPTRINT_T) (p)) % ((NTL_UPTRINT_T) (align)));
   return p + ((((unsigned long) (align)) - r) % ((unsigned long) (align)));
}

#else


inline
char *_ntl_make_aligned(char *p, long align)
{
   return p;
}


#endif





// The following is for aligning small local arrays
// Equivalent to type x[n], but aligns to align bytes
// Only works for POD types
// NOTE: the gcc aligned attribute might work, but there is
// some chatter on the web that this was (at some point) buggy.
// Not clear what the current status is.
// Anyway, this is only intended for use with gcc on intel
// machines, so it should be OK.


#define NTL_ALIGNED_LOCAL_ARRAY(align, x, type, n) \
   char x##__ntl_hidden_variable_storage[n*sizeof(type)+align]; \
   type *x = (type *) _ntl_make_aligned(&x##__ntl_hidden_variable_storage[0], align);


#define NTL_AVX_BYTE_ALIGN (32)
#define NTL_AVX_DBL_ALIGN (NTL_AVX_BYTE_ALIGN/long(sizeof(double)))

#define NTL_AVX_LOCAL_ARRAY(x, type, n) NTL_ALIGNED_LOCAL_ARRAY(NTL_AVX_BYTE_ALIGN, x, type, n)

#define NTL_AVX512_BYTE_ALIGN (64)

#define NTL_AVX512_LOCAL_ARRAY(x, type, n) NTL_ALIGNED_LOCAL_ARRAY(NTL_AVX512_BYTE_ALIGN, x, type, n)


#define NTL_DEFAULT_ALIGN (64)
// this should be big enough to satisfy any SIMD instructions,
// and it should also be as big as a cache line



#ifdef NTL_HAVE_BUILTIN_CLZL

inline long 
_ntl_count_bits(unsigned long x)
{
   return x ? (NTL_BITS_PER_LONG - __builtin_clzl(x)) : 0;
}

#else

inline long 
_ntl_count_bits(unsigned long x)
{
   if (!x) return 0;

   long res = NTL_BITS_PER_LONG;
   while (x < (1UL << (NTL_BITS_PER_LONG-1))) {
      x <<= 1;
      res--;
   }

   return res;
}

#endif




#if (!defined(NTL_CLEAN_INT) && NTL_ARITH_RIGHT_SHIFT && (NTL_BITS_PER_LONG == (1 << (NTL_NUMBITS_BPL-1))))



inline void
_ntl_bpl_divrem(long a, long& q, long& r)
{
   q = a >> (NTL_NUMBITS_BPL-1);
   r = a & (NTL_BITS_PER_LONG-1);
}

#else

inline void
_ntl_bpl_divrem(long a, long& q, long& r)
{
   q = a / NTL_BITS_PER_LONG;
   r = a % NTL_BITS_PER_LONG;
   if (r < 0) {
      q--;
      r += NTL_BITS_PER_LONG;
   }
}

#endif

inline void
_ntl_bpl_divrem(unsigned long a, long& q, long& r)
{
   q = a / NTL_BITS_PER_LONG;
   r = a % NTL_BITS_PER_LONG;
}


// vectors are grown by a factor of 1.5
inline long _ntl_vec_grow(long n)
{ return n + n/2; }


template <class T>
struct _ntl_is_char_pointer
{
 enum {value = false};
};

template <>
struct _ntl_is_char_pointer<char*>
{
 enum {value = true};
};

template <>
struct _ntl_is_char_pointer<const char*>
{
 enum {value = true};
};

template <bool, typename T = void>
struct _ntl_enable_if
{};

template <typename T>
struct _ntl_enable_if<true, T> {
  typedef T type;
};





#endif

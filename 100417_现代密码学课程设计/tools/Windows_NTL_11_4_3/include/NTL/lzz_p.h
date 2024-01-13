
#ifndef NTL_zz_p__H
#define NTL_zz_p__H

#include <NTL/ZZ.h>
#include <NTL/FFT.h>
#include <NTL/SmartPtr.h>
#include <NTL/vector.h>




NTL_OPEN_NNS


class zz_pInfoT {
private:
   zz_pInfoT();                      // disabled
   zz_pInfoT(const zz_pInfoT&);  // disabled
   void operator=(const zz_pInfoT&); // disabled
public:
   zz_pInfoT(long NewP, long maxroot);
   zz_pInfoT(INIT_FFT_TYPE, FFTPrimeInfo *info);
   zz_pInfoT(INIT_USER_FFT_TYPE, long q);

   long p;
   mulmod_t pinv;

   sp_reduce_struct red_struct;
   sp_ll_reduce_struct ll_red_struct;
   sp_ZZ_reduce_struct ZZ_red_struct;

   FFTPrimeInfo* p_info; // non-null means we are directly using 
                         // an FFT prime

   UniquePtr<FFTPrimeInfo> p_info_owner;
   // for user-defined FFT primes, we store the corresponding
   // FFTPrimeInfo object here
   

   long PrimeCnt;    // 0 for FFT prime;  otherwise same as NumPrimes
                     // used for establishing crossover points

   long NumPrimes;

   long MaxRoot;

   long MinusMModP;  //  -M mod p, M = product of primes
   mulmod_precon_t MinusMModPpinv;

   // the following arrays are indexed 0..NumPrimes-1
   // q = FFTPrime[i]


   Vec<long> CoeffModP;    // coeff mod p
   Vec<mulmod_precon_t> CoeffModPpinv; 

   Vec<double> x;               // u/q, where u = (M/q)^{-1} mod q
   Vec<long> u;                 // u, as above
   Vec<mulmod_precon_t> uqinv;  // MulModPrecon for u
};

extern 
NTL_CHEAP_THREAD_LOCAL 
zz_pInfoT *zz_pInfo;  
// current modulus, initially null


class zz_pContext {
private:
SmartPtr<zz_pInfoT> ptr;

public:

zz_pContext() { }

// copy constructor, assignment, destructor: default

explicit zz_pContext(long p, long maxroot=NTL_FFTMaxRoot);
zz_pContext(INIT_FFT_TYPE, long index);
zz_pContext(INIT_USER_FFT_TYPE, long q);

void save();
void restore() const;


// some hooks that are useful in helib and elsewhere
// FIXME: generalize these to other context classes
// and document

bool null() const { return ptr == 0; } 
bool equals(const zz_pContext& other) const { return ptr == other.ptr; } 
long modulus() const { return ptr->p; }
mulmod_t ModulusInverse() const { return ptr->pinv; }
const sp_ZZ_reduce_struct& ZZ_red_struct() const { return ptr->ZZ_red_struct; } 
sp_reduce_struct red_struct() const { return ptr->red_struct; } 


};


// should be in FFT.h, but because of some weirdess involving NTL_WIZARD_HACK,
// it has to be here
inline const sp_ZZ_reduce_struct& GetFFT_ZZ_red_struct(long i) 
{
   return FFTTables[i]->zz_p_context->ZZ_red_struct;
}


class zz_pBak {
private:
zz_pContext c;
bool MustRestore;

zz_pBak(const zz_pBak&); // disabled
void operator=(const zz_pBak&); // disabled

public:
void save();
void restore();

zz_pBak() : MustRestore(false) { }

~zz_pBak();


};


class zz_pPush {
private:
zz_pBak bak;

zz_pPush(const zz_pPush&); // disabled
void operator=(const zz_pPush&); // disabled

public:
zz_pPush() { bak.save(); }
explicit zz_pPush(const zz_pContext& context) { bak.save(); context.restore(); }

explicit zz_pPush(long p, long maxroot=NTL_FFTMaxRoot) 
   { bak.save(); zz_pContext c(p, maxroot); c.restore(); }

zz_pPush(INIT_FFT_TYPE, long index) 
   { bak.save(); zz_pContext c(INIT_FFT, index); c.restore(); }

zz_pPush(INIT_USER_FFT_TYPE, long q)
   { bak.save(); zz_pContext c(INIT_USER_FFT, q); c.restore(); }

};




#define NTL_zz_pRegister(x) zz_p x


class zz_pX; // forward declaration

class zz_p {
public:
typedef long rep_type;
typedef zz_pContext context_type;
typedef zz_pBak bak_type;
typedef zz_pPush push_type;
typedef zz_pX poly_type;



long _zz_p__rep;


static void init(long NewP, long maxroot=NTL_FFTMaxRoot);
static void FFTInit(long index);
static void UserFFTInit(long q);



// ****** constructors and assignment

zz_p() : _zz_p__rep(0) {  }

explicit zz_p(long a) : _zz_p__rep(0) { *this = a;  }

zz_p(const zz_p& a) : _zz_p__rep(a._zz_p__rep) { }  


zz_p& operator=(const zz_p& a) { _zz_p__rep = a._zz_p__rep; return *this; }

inline zz_p& operator=(long a);

// a loop-hole for direct access to _zz_p__rep
long& LoopHole() { return _zz_p__rep; }

static long modulus() { return zz_pInfo->p; }
static zz_p zero() { return zz_p(); }
static mulmod_t ModulusInverse() { return zz_pInfo->pinv; }
static sp_reduce_struct red_struct() { return zz_pInfo->red_struct; }
static sp_ll_reduce_struct ll_red_struct() { return zz_pInfo->ll_red_struct; }
static const sp_ZZ_reduce_struct& ZZ_red_struct() { return zz_pInfo->ZZ_red_struct; }
static long PrimeCnt() { return zz_pInfo->PrimeCnt; }


static long storage() { return sizeof(long); }

static bool IsFFTPrime() { return zz_pInfo->p_info != 0; }

zz_p(long a, INIT_LOOP_HOLE_TYPE) { _zz_p__rep = a; }

// for consistency
zz_p(INIT_NO_ALLOC_TYPE) : _zz_p__rep(0) { } 
zz_p(INIT_ALLOC_TYPE) : _zz_p__rep(0) { } 
void allocate() { }


};



NTL_DECLARE_RELOCATABLE((zz_p*))

inline
zz_p to_zz_p(long a) 
{
   return zz_p(rem(a, zz_pInfo->p, zz_pInfo->red_struct), INIT_LOOP_HOLE);
}

inline
void conv(zz_p& x, long a)
{
   x._zz_p__rep = rem(a, zz_pInfo->p, zz_pInfo->red_struct);
}

inline void VectorConv(long k, zz_p *x, const long *a)
{
   if (k <= 0) return;
   sp_reduce_struct red_struct = zz_p::red_struct();
   long p = zz_p::modulus();
   for (long i = 0; i < k; i++) x[i].LoopHole() = rem(a[i], p, red_struct);
}

inline zz_p& zz_p::operator=(long a) { conv(*this, a); return *this; }

inline
zz_p to_zz_p(const ZZ& a)
{
   return zz_p(zz_p::ZZ_red_struct().rem(a), INIT_LOOP_HOLE);
}

inline
void conv(zz_p& x, const ZZ& a)
{
   x._zz_p__rep = zz_p::ZZ_red_struct().rem(a);
}


inline void VectorConv(long k, zz_p *x, const ZZ *a)
{
   if (k <= 0) return;
   const sp_ZZ_reduce_struct& ZZ_red_struct = zz_p::ZZ_red_struct();
   for (long i = 0; i < k; i++) x[i].LoopHole() = ZZ_red_struct.rem(a[i]);
}

// read-only access to _zz_p__representation
inline long rep(zz_p a) { return a._zz_p__rep; }

inline void clear(zz_p& x)
// x = 0
   { x._zz_p__rep = 0; }

inline void set(zz_p& x)
// x = 1
   { x._zz_p__rep = 1; }

inline void swap(zz_p& x, zz_p& y)
// swap x and y

   { long t;  t = x._zz_p__rep; x._zz_p__rep = y._zz_p__rep; y._zz_p__rep = t; }

// ****** addition

inline void add(zz_p& x, zz_p a, zz_p b)
// x = a + b

   { x._zz_p__rep = AddMod(a._zz_p__rep, b._zz_p__rep, zz_p::modulus()); }

inline void sub(zz_p& x, zz_p a, zz_p b)
// x = a - b

   { x._zz_p__rep = SubMod(a._zz_p__rep, b._zz_p__rep, zz_p::modulus()); }


inline void negate(zz_p& x, zz_p a)
// x = -a

   { x._zz_p__rep = SubMod(0, a._zz_p__rep, zz_p::modulus()); }

// scalar versions

inline void add(zz_p& x, zz_p a, long b) { add(x, a, to_zz_p(b)); }
inline void add(zz_p& x, long a, zz_p b) { add(x, to_zz_p(a), b); }

inline void sub(zz_p& x, zz_p a, long b) { sub(x, a, to_zz_p(b)); }
inline void sub(zz_p& x, long a, zz_p b) { sub(x, to_zz_p(a), b); }

inline zz_p operator+(zz_p a, zz_p b)
    { zz_p x; add(x, a, b); return x; }

inline zz_p operator+(zz_p a, long b)
    { zz_p x; add(x, a, b); return x; }

inline zz_p operator+(long a, zz_p b)
    { zz_p x; add(x, a, b); return x; }

inline zz_p operator-(zz_p a, zz_p b)
    { zz_p x; sub(x, a, b); return x; }

inline zz_p operator-(zz_p a, long b)
    { zz_p x; sub(x, a, b); return x; }

inline zz_p operator-(long a, zz_p b)
    { zz_p x; sub(x, a, b); return x; }



inline zz_p operator-(zz_p a)
   { zz_p x; negate(x, a); return x; }



inline zz_p& operator+=(zz_p& x, zz_p b)
   { add(x, x, b); return x; }

inline zz_p& operator+=(zz_p& x, long b)
   { add(x, x, b); return x; }



inline zz_p& operator-=(zz_p& x, zz_p b)
   { sub(x, x, b); return x; }

inline zz_p& operator-=(zz_p& x, long b)
   { sub(x, x, b); return x; }

inline zz_p& operator++(zz_p& x) { add(x, x, 1); return x; }
inline void operator++(zz_p& x, int) { add(x, x, 1); }
inline zz_p& operator--(zz_p& x) { sub(x, x, 1); return x; }
inline void operator--(zz_p& x, int) { sub(x, x, 1); }

// ****** multiplication

inline void mul(zz_p& x, zz_p a, zz_p b)
// x = a*b

   { x._zz_p__rep = MulMod(a._zz_p__rep, b._zz_p__rep, zz_p::modulus(), zz_p::ModulusInverse()); }

inline void mul(zz_p& x, zz_p a, long b) { mul(x, a, to_zz_p(b)); }
inline void mul(zz_p& x, long a, zz_p b) { mul(x, to_zz_p(a), b); }

inline zz_p operator*(zz_p a, zz_p b)
    { zz_p x; mul(x, a, b); return x; }

inline zz_p operator*(zz_p a, long b)
    { zz_p x; mul(x, a, b); return x; }

inline zz_p operator*(long a, zz_p b)
    { zz_p x; mul(x, a, b); return x; }


inline zz_p& operator*=(zz_p& x, zz_p b)
   { mul(x, x, b); return x; }

inline zz_p& operator*=(zz_p& x, long b)
   { mul(x, x, b); return x; }



inline void sqr(zz_p& x, zz_p a)
// x = a^2

   { x._zz_p__rep = MulMod(a._zz_p__rep, a._zz_p__rep, zz_p::modulus(), zz_p::ModulusInverse()); }

inline zz_p sqr(zz_p a)
   { zz_p x; sqr(x, a); return x; }



// ****** division

inline void div(zz_p& x, zz_p a, zz_p b)
// x = a/b

   { x._zz_p__rep = MulMod(a._zz_p__rep, InvMod(b._zz_p__rep, zz_p::modulus()), zz_p::modulus(),
                    zz_p::ModulusInverse()); }

inline void inv(zz_p& x, zz_p a)
// x = 1/a

   { x._zz_p__rep = InvMod(a._zz_p__rep, zz_p::modulus()); }

inline zz_p inv(zz_p a)
   { zz_p x; inv(x, a); return x; }

inline void div(zz_p& x, zz_p a, long b) { div(x, a, to_zz_p(b)); }
inline void div(zz_p& x, long a, zz_p b) { div(x, to_zz_p(a), b); }

inline zz_p operator/(zz_p a, zz_p b)
    { zz_p x; div(x, a, b); return x; }

inline zz_p operator/(zz_p a, long b)
    { zz_p x; div(x, a, b); return x; }

inline zz_p operator/(long a, zz_p b)
    { zz_p x; div(x, a, b); return x; }


inline zz_p& operator/=(zz_p& x, zz_p b)
   { div(x, x, b); return x; }

inline zz_p& operator/=(zz_p& x, long b)
   { div(x, x, b); return x; }


// ****** exponentiation

inline void power(zz_p& x, zz_p a, long e)
// x = a^e

   { x._zz_p__rep = PowerMod(a._zz_p__rep, e, zz_p::modulus()); }

inline zz_p power(zz_p a, long e)
   { zz_p x; power(x, a, e); return x; }

// ****** comparison

inline long IsZero(zz_p a)
   { return a._zz_p__rep == 0; }

inline long IsOne(zz_p a)
   { return a._zz_p__rep == 1; }

inline long operator==(zz_p a, zz_p b)
   { return a._zz_p__rep == b._zz_p__rep; }

inline long operator!=(zz_p a, zz_p b)
   { return !(a == b); }

inline long operator==(zz_p a, long b) { return a == to_zz_p(b); }
inline long operator==(long a, zz_p b) { return to_zz_p(a) == b; }

inline long operator!=(zz_p a, long b) { return !(a == b); }
inline long operator!=(long a, zz_p b) { return !(a == b); }

// ****** random numbers

inline void random(zz_p& x)
// x = random element in zz_p

   { x._zz_p__rep = RandomBnd(zz_p::modulus()); }

inline zz_p random_zz_p()
   { zz_p x; random(x); return x; }

inline void VectorRandom(long k, zz_p* x)
{
   if (k <= 0) return;
   RandomBndGenerator gen(zz_p::modulus());
   for (long i = 0; i < k; i++) x[i].LoopHole() = gen.next();
}



// ****** input/output

NTL_SNS ostream& operator<<(NTL_SNS ostream& s, zz_p a);
   
NTL_SNS istream& operator>>(NTL_SNS istream& s, zz_p& x);


void conv(Vec<zz_p>& x, const Vec<ZZ>& a);
void conv(Vec<zz_p>& x, const Vec<long>& a);
// explicit instantiation of more efficient versions,
// defined in vec_lzz_p.c



/* additional legacy conversions for v6 conversion regime */

inline void conv(int& x, zz_p a) { conv(x, rep(a)); }
inline void conv(unsigned int& x, zz_p a) { conv(x, rep(a)); }
inline void conv(long& x, zz_p a) { conv(x, rep(a)); }
inline void conv(unsigned long& x, zz_p a) { conv(x, rep(a)); }
inline void conv(ZZ& x, zz_p a) { conv(x, rep(a)); }


inline void conv(zz_p& x, zz_p a) { x = a; }

/* ------------------------------------- */


// *********************************************************
// *** specialized inner-product routines, for internal consumption
// *********************************************************

#ifdef NTL_HAVE_LL_TYPE
long 
InnerProd_LL(const long *ap, const zz_p *bp, long n, long d, 
          sp_ll_reduce_struct dinv);

long
InnerProd_LL(const zz_p *ap, const zz_p *bp, long n, long d, 
          sp_ll_reduce_struct dinv);

inline bool
InnerProd_L_viable(long n, long d)
{
   if (n < 2) n = 2;  
   // this ensures cast_unsigned(-1)/d^2 is at least 2, which
   // streamlines things
   if (n > 128) n = 128;
   return cast_unsigned(n) <= cast_unsigned(-1L)/cast_unsigned(d) && 
          cast_unsigned(n)*cast_unsigned(d) <= cast_unsigned(-1L)/cast_unsigned(d);
}

inline long 
InnerProd_L_bound(long d)
{
   return cast_unsigned(-1L)/(cast_unsigned(d)*cast_unsigned(d));
   // This calculation ensures that the return value does not sign-overflow,
   // and that InnerProd_L itself can accumulate an extra term.
}

long 
InnerProd_L(const long *ap, const zz_p *bp, long n, long d, 
          sp_reduce_struct dinv, long bound);

long 
InnerProd_L(const zz_p *ap, const zz_p *bp, long n, long d, 
          sp_reduce_struct dinv, long bound);

#endif

NTL_CLOSE_NNS

#endif

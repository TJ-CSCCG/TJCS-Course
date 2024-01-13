

#ifndef NTL_ZZ_p__H
#define NTL_ZZ_p__H

#include <NTL/ZZ.h>
#include <NTL/ZZVec.h>
#include <NTL/SmartPtr.h>
#include <NTL/Lazy.h>

NTL_OPEN_NNS


// ZZ_p representation:  each ZZ_p is represented by a ZZ in the range 0..p-1.


class ZZ_pFFTInfoT {
private:
   ZZ_pFFTInfoT(const ZZ_pFFTInfoT&); // disabled
   void operator=(const ZZ_pFFTInfoT&); // disabled

public:
   ZZ_pFFTInfoT() { }

   long NumPrimes;
   long MaxRoot;  
   ZZ MinusMModP;  //  -M mod p, M = product of primes
   ZZ_CRTStructAdapter crt_struct;
   ZZ_RemStructAdapter rem_struct;


   // the following arrays are indexed 0..NumPrimes-1
   // q[i] = FFTPrime[i]
   Vec<long> prime;  // prime[i] = q[i]
   Vec<double> prime_recip;  // prime_recip[i] = 1/double(q[i])
   Vec<long> u;  // u[i] = (M/q[i])^{-1} mod q[i]
   Vec<mulmod_precon_t> uqinv;

   ZZ_ReduceStructAdapter reduce_struct;

};


#ifndef NTL_WIZARD_HACK

struct MatPrime_crt_helper;
void MatPrime_crt_helper_deleter(MatPrime_crt_helper*);

#endif


class ZZ_pInfoT {
private:
   ZZ_pInfoT();                       // disabled
   ZZ_pInfoT(const ZZ_pInfoT&);   // disabled
   void operator=(const ZZ_pInfoT&);  // disabled
public:
   ZZ_pInfoT(const ZZ& NewP);

   ZZ p;      // the modulus
   long size;  // p.size()
   long ExtendedModulusSize;

   Lazy<ZZ_pFFTInfoT> FFTInfo;

#ifndef NTL_WIZARD_HACK

   struct MatPrime_crt_helper_deleter_policy {
      static void deleter(MatPrime_crt_helper *p) { MatPrime_crt_helper_deleter(p); }
   };


   Lazy<MatPrime_crt_helper,MatPrime_crt_helper_deleter_policy> MatPrime_crt_helper_info;
   // PIMPL 

#endif

};



// auxilliary data structures to store space for temporaries
// used by the crt and rem routines in the low-level lip module.
// These used to be stored in data structures managed by the
// lip module, but to achieve thread-safety, they have to be
// externally on a per-thread basis.

class ZZ_pTmpSpaceT {
public:
  ZZ_TmpVecAdapter crt_tmp_vec;
  ZZ_TmpVecAdapter rem_tmp_vec;
};


extern 
NTL_CHEAP_THREAD_LOCAL
ZZ_pInfoT *ZZ_pInfo; 
// info for current modulus, initially null
// plain pointer for faster TLS access

extern 
NTL_CHEAP_THREAD_LOCAL
ZZ_pTmpSpaceT *ZZ_pTmpSpace;  
// space for temps associated with current modulus, 
// plain pointer for faster TLS access

extern 
NTL_CHEAP_THREAD_LOCAL
bool ZZ_pInstalled;
// flag indicating if current modulus is fully installed




class ZZ_pContext {
private:
SmartPtr<ZZ_pInfoT> ptr;

public:

ZZ_pContext() { }
explicit ZZ_pContext(const ZZ& p) : ptr(MakeSmart<ZZ_pInfoT>(p)) { }

// copy constructor, assignment, destructor: default

void save();
void restore() const;

};


class ZZ_pBak {
private:
ZZ_pContext c;
bool MustRestore;

ZZ_pBak(const ZZ_pBak&); // disabled
void operator=(const ZZ_pBak&); // disabled

public:
void save();
void restore();

ZZ_pBak() : MustRestore(false) { }

~ZZ_pBak();


};

class ZZ_pPush {
private:
ZZ_pBak bak;

ZZ_pPush(const ZZ_pPush&); // disabled
void operator=(const ZZ_pPush&); // disabled

public:
ZZ_pPush() { bak.save(); }
explicit ZZ_pPush(const ZZ_pContext& context) { bak.save(); context.restore(); }
explicit ZZ_pPush(const ZZ& p) { bak.save(); ZZ_pContext c(p); c.restore(); }


};


class ZZ_pX; // forward declaration

class ZZ_p {

public:
typedef ZZ rep_type;
typedef ZZ_pContext context_type;
typedef ZZ_pBak bak_type;
typedef ZZ_pPush push_type;
typedef ZZ_pX poly_type;



ZZ _ZZ_p__rep;


static void init(const ZZ&);


typedef void (*DivHandlerPtr)(const ZZ_p& a);   // error-handler for division

static 
NTL_CHEAP_THREAD_LOCAL 
DivHandlerPtr DivHandler;


// ****** constructors and assignment

ZZ_p() { } // NO_ALLOC
explicit ZZ_p(long a) { *this = a; }

ZZ_p(INIT_NO_ALLOC_TYPE) { }  // allocates no space
ZZ_p(INIT_ALLOC_TYPE) { _ZZ_p__rep.SetSize(ZZ_pInfo->size); }  // allocates space



inline ZZ_p& operator=(long a);

// You can always access the _ZZ_p__representation directly...if you dare.
ZZ& LoopHole() { return _ZZ_p__rep; }

ZZ_p(ZZ_p& x, INIT_TRANS_TYPE) : _ZZ_p__rep(x._ZZ_p__rep, INIT_TRANS) { }



static const ZZ& modulus() { return ZZ_pInfo->p; }
static long ModulusSize() { return ZZ_pInfo->size; }
static long storage() { return ZZ_storage(ZZ_pInfo->size); }
static long ExtendedModulusSize() { return ZZ_pInfo->ExtendedModulusSize; }

static const ZZ_p& zero();

static void DoInstall();

static void install()
{
   // we test and set ZZ_pInstalled here, to allow better
   // inlining and optimization
   if (!ZZ_pInstalled) { DoInstall(); ZZ_pInstalled = true; } 
}

static const ZZ_pFFTInfoT* GetFFTInfo() 
{ 
   install();
   return ZZ_pInfo->FFTInfo.get();
}

static ZZ_pTmpSpaceT* GetTmpSpace()
{
   install();
   return ZZ_pTmpSpace;
}


ZZ_p(INIT_VAL_TYPE, const ZZ& a);
ZZ_p(INIT_VAL_TYPE, long a);


void swap(ZZ_p& x)
{
   _ZZ_p__rep.swap(x._ZZ_p__rep);
}



void allocate() 
{ 
   long sz = ZZ_pInfo->size;
   if (_ZZ_p__rep.MaxAlloc() < sz) 
      _ZZ_p__rep.SetSize(sz);
}

// mainly for internal consumption by the ZZ_pWatcher class below

void KillBig() { _ZZ_p__rep.KillBig(); }


};



NTL_DECLARE_RELOCATABLE((ZZ_p*))



// read-only access to _ZZ_p__representation
inline const ZZ& rep(const ZZ_p& a) { return a._ZZ_p__rep; }

// ****** conversion

inline void conv(ZZ_p& x, const ZZ& a)
   { rem(x._ZZ_p__rep, a, ZZ_p::modulus()); }


inline ZZ_p to_ZZ_p(const ZZ& a)
   { return ZZ_p(INIT_VAL, a); }


void conv(ZZ_p& x, long a);


inline ZZ_p to_ZZ_p(long a)
   { return ZZ_p(INIT_VAL, a); }




// ****** some basics


inline void clear(ZZ_p& x)
// x = 0
   { clear(x._ZZ_p__rep); }

inline void set(ZZ_p& x)
// x = 1
   { set(x._ZZ_p__rep); }

inline void swap(ZZ_p& x, ZZ_p& y)
// swap x and y

   { x.swap(y); }

// ****** addition

inline void add(ZZ_p& x, const ZZ_p& a, const ZZ_p& b)
// x = a + b

   { AddMod(x._ZZ_p__rep, a._ZZ_p__rep, b._ZZ_p__rep, ZZ_p::modulus()); }

inline void sub(ZZ_p& x, const ZZ_p& a, const ZZ_p& b)
// x = a - b

   { SubMod(x._ZZ_p__rep, a._ZZ_p__rep, b._ZZ_p__rep, ZZ_p::modulus()); }


inline void negate(ZZ_p& x, const ZZ_p& a)
// x = -a

   { NegateMod(x._ZZ_p__rep, a._ZZ_p__rep, ZZ_p::modulus()); }


// scalar versions

void add(ZZ_p& x, const ZZ_p& a, long b);
inline void add(ZZ_p& x, long a, const ZZ_p& b) { add(x, b, a); }

void sub(ZZ_p& x, const ZZ_p& a, long b);
void sub(ZZ_p& x, long a, const ZZ_p& b);


// ****** multiplication

inline void mul(ZZ_p& x, const ZZ_p& a, const ZZ_p& b)
// x = a*b

   { MulMod(x._ZZ_p__rep, a._ZZ_p__rep, b._ZZ_p__rep, ZZ_p::modulus()); }


inline void sqr(ZZ_p& x, const ZZ_p& a)
// x = a^2

   { SqrMod(x._ZZ_p__rep, a._ZZ_p__rep, ZZ_p::modulus()); }

inline ZZ_p sqr(const ZZ_p& a)
   { ZZ_p x; sqr(x, a); NTL_OPT_RETURN(ZZ_p, x); }


// scalar versions

void mul(ZZ_p& x, const ZZ_p& a, long b);
inline void mul(ZZ_p& x, long a, const ZZ_p& b) { mul(x, b, a); }

// ****** division


void div(ZZ_p& x, const ZZ_p& a, const ZZ_p& b);
// x = a/b
// If b != 0 & b not invertible & DivHandler != 0,
// then DivHandler will be called with the offending b.
// In this case, of course, p is not really prime, and one
// can factor p by taking a gcd with rep(b).
// Otherwise, if b is not invertible, an error occurs.

void inv(ZZ_p& x, const ZZ_p& a);
// x = 1/a
// Error handling is the same as above.

inline ZZ_p inv(const ZZ_p& a)
   { ZZ_p x; inv(x, a); NTL_OPT_RETURN(ZZ_p, x); }

void div(ZZ_p& x, const ZZ_p& a, long b);
void div(ZZ_p& x, long a, const ZZ_p& b);


// operator notation:

inline ZZ_p operator+(const ZZ_p& a, const ZZ_p& b)
   { ZZ_p x; add(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p operator+(const ZZ_p& a, long b)
   { ZZ_p x; add(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p operator+(long a, const ZZ_p& b)
   { ZZ_p x; add(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p& operator+=(ZZ_p& x, const ZZ_p& b)
   { add(x, x, b); return x; } 

inline ZZ_p& operator+=(ZZ_p& x, long b)
   { add(x, x, b); return x; } 



inline ZZ_p operator-(const ZZ_p& a, const ZZ_p& b)
   { ZZ_p x; sub(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p operator-(const ZZ_p& a, long b)
   { ZZ_p x; sub(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p operator-(long a, const ZZ_p& b)
   { ZZ_p x; sub(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p& operator-=(ZZ_p& x, const ZZ_p& b)
   { sub(x, x, b); return x; } 

inline ZZ_p& operator-=(ZZ_p& x, long b)
   { sub(x, x, b); return x; } 



inline ZZ_p operator*(const ZZ_p& a, const ZZ_p& b)
   { ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p operator*(const ZZ_p& a, long b)
   { ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p operator*(long a, const ZZ_p& b)
   { ZZ_p x; mul(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p& operator*=(ZZ_p& x, const ZZ_p& b)
   { mul(x, x, b); return x; } 

inline ZZ_p& operator*=(ZZ_p& x, long b)
   { mul(x, x, b); return x; } 


inline ZZ_p operator/(const ZZ_p& a, const ZZ_p& b)
   { ZZ_p x; div(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p operator/(const ZZ_p& a, long b)
   { ZZ_p x; div(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p operator/(long a, const ZZ_p& b)
   { ZZ_p x; div(x, a, b); NTL_OPT_RETURN(ZZ_p, x); }

inline ZZ_p& operator/=(ZZ_p& x, const ZZ_p& b)
   { div(x, x, b); return x; } 

inline ZZ_p& operator/=(ZZ_p& x, long b)
   { div(x, x, b); return x; } 


inline ZZ_p operator-(const ZZ_p& a)
   { ZZ_p x; negate(x, a); NTL_OPT_RETURN(ZZ_p, x); }


inline ZZ_p& operator++(ZZ_p& x) { add(x, x, 1); return x; }
inline void operator++(ZZ_p& x, int) { add(x, x, 1); }
inline ZZ_p& operator--(ZZ_p& x) { sub(x, x, 1); return x; }
inline void operator--(ZZ_p& x, int) { sub(x, x, 1); }


// ****** exponentiation

inline void power(ZZ_p& x, const ZZ_p& a, const ZZ& e)
   { PowerMod(x._ZZ_p__rep, a._ZZ_p__rep, e, ZZ_p::modulus()); }

inline ZZ_p power(const ZZ_p& a, const ZZ& e)
   { ZZ_p x; power(x, a, e); NTL_OPT_RETURN(ZZ_p, x); }

inline void power(ZZ_p& x, const ZZ_p& a, long e)
   { PowerMod(x._ZZ_p__rep, a._ZZ_p__rep, e, ZZ_p::modulus()); }

inline ZZ_p power(const ZZ_p& a, long e)
   { ZZ_p x; power(x, a, e); NTL_OPT_RETURN(ZZ_p, x); }


// ****** comparison

inline long IsZero(const ZZ_p& a)
   { return IsZero(a._ZZ_p__rep); }


inline long IsOne(const ZZ_p& a)
   { return IsOne(a._ZZ_p__rep); }

inline long operator==(const ZZ_p& a, const ZZ_p& b)
   { return a._ZZ_p__rep == b._ZZ_p__rep; }

inline long operator!=(const ZZ_p& a, const ZZ_p& b)
   { return !(a == b); }

long operator==(const ZZ_p& a, long b);
inline long operator==(long a, const ZZ_p& b) { return b == a; }

inline long operator!=(const ZZ_p& a, long b) { return !(a == b); }
inline long operator!=(long a, const ZZ_p& b) { return !(a == b); }


// ****** random numbers

inline void random(ZZ_p& x)
// x = random element in ZZ_p

   { RandomBnd(x._ZZ_p__rep, ZZ_p::modulus()); }

inline ZZ_p random_ZZ_p()
   { ZZ_p x; random(x); NTL_OPT_RETURN(ZZ_p, x); }


// ****** input/output

inline NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const ZZ_p& a)
   { return s << a._ZZ_p__rep; }
   
NTL_SNS istream& operator>>(NTL_SNS istream& s, ZZ_p& x);


inline ZZ_p& ZZ_p::operator=(long a) { conv(*this, a); return *this; }



/* additional legacy conversions for v6 conversion regime */

inline void conv(int& x, const ZZ_p& a) { conv(x, rep(a)); }
inline void conv(unsigned int& x, const ZZ_p& a) { conv(x, rep(a)); }
inline void conv(long& x, const ZZ_p& a) { conv(x, rep(a)); }
inline void conv(unsigned long& x, const ZZ_p& a) { conv(x, rep(a)); }
inline void conv(ZZ& x, const ZZ_p& a) { conv(x, rep(a)); }

inline void conv(ZZ_p& x, const ZZ_p& a) { x = a; }

/* ------------------------------------- */


// Thread-boosted conversion. Defined in vec_zz_p.cpp.
void conv(Vec<ZZ_p>& x, const Vec<ZZ>& a);

/* ------------------------------------- */

// overload these functions for Vec<ZZ_p>.
// They are defined in vec_ZZ_p.c
void BlockConstruct(ZZ_p* p, long n);
void BlockConstructFromVec(ZZ_p* p, long n, const ZZ_p* q);
void BlockConstructFromObj(ZZ_p* p, long n, const ZZ_p& q);
void BlockDestroy(ZZ_p* p, long n);


// ZZ_p scratch variables



class ZZ_pWatcher {
public:
   ZZ_p& watched;
   explicit
   ZZ_pWatcher(ZZ_p& _watched) : watched(_watched) { }

   ~ZZ_pWatcher() { watched.KillBig(); }
};

#define NTL_ZZ_pRegister(x) NTL_TLS_LOCAL(ZZ_p, x); ZZ_pWatcher _WATCHER__ ## x(x); x.allocate()

// FIXME: register variables that are allocated with respect to one modulus
// and then reused with another modulus may have initial values that are
// not in the correct range.  This should not cause any problems, though,
// as these register values should always be written to before being read.
// Note also that the underlying integer reps may have space
// allocated that is smaller or *bigger* than the current modulus.
// This may impact future interface design changes --- especially
// one that tries to make "out of context" copy constructors
// safe by reading the allocated space of the source.



NTL_CLOSE_NNS

#endif

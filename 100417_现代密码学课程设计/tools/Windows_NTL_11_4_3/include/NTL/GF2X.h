
#ifndef NTL_GF2X__H
#define NTL_GF2X__H

#include <NTL/vector.h>
#include <NTL/ZZ.h>
#include <NTL/WordVector.h>
#include <NTL/vec_GF2.h>
#include <NTL/Lazy.h>

NTL_OPEN_NNS

class GF2E; // forward declaration
class GF2XModulus;

class GF2X {
public:
typedef GF2 coeff_type;
typedef GF2E residue_type;
typedef GF2XModulus modulus_type;


WordVector xrep;

typedef vec_GF2 VectorBaseType;


GF2X() { }

explicit GF2X(long a) { *this = a; }
explicit GF2X(GF2 a) { *this = a; }


// we have to explicitly declare copy constructor and assignment
// in case we also declare the corresponding move operations

GF2X(const GF2X& a) : xrep(a.xrep) { }
GF2X& operator=(const GF2X& a) { xrep = a.xrep; return *this; }

#if (NTL_CXX_STANDARD >= 2011 && !defined(NTL_DISABLE_MOVE))

GF2X(GF2X&& a) NTL_FAKE_NOEXCEPT
{
   if (a.xrep.pinned()) {
      *this = a;
   }
   else {
      xrep.unpinned_move(a.xrep);
   }
}

#ifndef NTL_DISABLE_MOVE_ASSIGN
GF2X& operator=(GF2X&& a) NTL_FAKE_NOEXCEPT
{
   if (xrep.pinned() || a.xrep.pinned()) {
      *this = a;
   }
   else {
      xrep.unpinned_move(a.xrep);
   }

   return *this;
}
#endif


#endif


GF2X(INIT_SIZE_TYPE, long n);


inline GF2X& operator=(GF2 a);
inline GF2X& operator=(long a);

void normalize();

static const GF2X& zero();

void kill() { xrep.kill(); }

void SetMaxLength(long n);



void SetLength(long n);

ref_GF2 operator[](long i) 
{
#ifdef NTL_RANGE_CHECK
   if (i < 0) LogicError("index out of range in Vec");
   long q, r;
   _ntl_bpl_divrem(cast_unsigned(i), q, r);
   if (q >= xrep.length()) LogicError("index out of range in Vec");
#endif

   vec_GF2::iterator t(INIT_LOOP_HOLE, xrep.elts(), i);
   return *t;
}


const GF2 operator[](long i) const
{
#ifdef NTL_RANGE_CHECK
   if (i < 0) LogicError("index out of range in Vec");
   long q, r;
   _ntl_bpl_divrem(cast_unsigned(i), q, r);
   if (q >= xrep.length()) LogicError("index out of range in Vec");
#endif

   vec_GF2::const_iterator t(INIT_LOOP_HOLE, xrep.elts(), i);
   return *t;
}



static NTL_CHEAP_THREAD_LOCAL long HexOutput;

inline GF2X(long i, GF2 c);
inline GF2X(long i, long c);

inline GF2X(INIT_MONO_TYPE, long i, GF2 c);
inline GF2X(INIT_MONO_TYPE, long i, long c);
inline GF2X(INIT_MONO_TYPE, long i);

GF2X(GF2X& x, INIT_TRANS_TYPE) : xrep(x.xrep, INIT_TRANS) { }
// This should only be used for simple, local variables
// that are not be subject to special memory management.


void swap(GF2X& x) { xrep.swap(x.xrep); }





// mainly for internal consumption by GF2XWatcher

void KillBig() { xrep.KillBig(); }

};


NTL_DECLARE_RELOCATABLE((GF2X*))


long IsZero(const GF2X& a);

long IsOne(const GF2X& a); 

long IsX(const GF2X& a);

const GF2 coeff(const GF2X& a, long i);

const GF2 LeadCoeff(const GF2X& a);

const GF2 ConstTerm(const GF2X& a);


inline void clear(GF2X& x) 
{ x.xrep.ZeroLength(); }

void set(GF2X& x);

void SetX(GF2X& x);

void SetCoeff(GF2X& x, long i);

void SetCoeff(GF2X& x, long i, GF2 a);
void SetCoeff(GF2X& x, long i, long a);

inline GF2X::GF2X(long i, GF2 a) { SetCoeff(*this, i, a); }
inline GF2X::GF2X(long i, long a) { SetCoeff(*this, i, a); }

inline GF2X::GF2X(INIT_MONO_TYPE, long i, GF2 a) { SetCoeff(*this, i, a); }
inline GF2X::GF2X(INIT_MONO_TYPE, long i, long a) { SetCoeff(*this, i, a); }
inline GF2X::GF2X(INIT_MONO_TYPE, long i) { SetCoeff(*this, i); }


inline void swap(GF2X& a, GF2X& b) { a.swap(b); }

long deg(const GF2X& aa);

long weight(const GF2X& a);
   
long operator==(const GF2X& a, const GF2X& b);

inline long operator!=(const GF2X& a, const GF2X& b)
   { return !(a == b); }

long operator==(const GF2X& a, GF2 b);
long operator==(const GF2X& a, long b);

inline long operator==(GF2 a, const GF2X& b) { return b == a; }
inline long operator==(long a, const GF2X& b) { return b == a; }

inline long operator!=(const GF2X& a, GF2 b) { return !(a == b); }
inline long operator!=(const GF2X& a, long b) { return !(a == b); }
inline long operator!=(GF2 a, const GF2X& b) { return !(a == b); }
inline long operator!=(long a, const GF2X& b) { return !(a == b); }


NTL_SNS istream & operator>>(NTL_SNS istream& s, GF2X& a);

NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const GF2X& a);




void random(GF2X& x, long n);
inline GF2X random_GF2X(long n)
   { GF2X x; random(x, n); NTL_OPT_RETURN(GF2X, x); }




void add(GF2X& x, const GF2X& a, const GF2X& b);
void add(GF2X& x, const GF2X& a, GF2 b);
void add(GF2X& x, const GF2X& a, long b);

inline void add(GF2X& x, GF2 a, const GF2X& b) { add(x, b, a); }
inline void add(GF2X& x, long a, const GF2X& b) { add(x, b, a); }

inline void sub(GF2X& x, const GF2X& a, const GF2X& b) { add(x, a, b); }
inline void sub(GF2X& x, const GF2X& a, GF2 b) { add(x, a, b); }
inline void sub(GF2X& x, const GF2X& a, long b) { add(x, a, b); }
inline void sub(GF2X& x, GF2 a, const GF2X& b) { add(x, a, b); }
inline void sub(GF2X& x, long a, const GF2X& b) { add(x, a, b); }

inline void negate(GF2X& x, const GF2X& a) { x = a; }

inline GF2X operator+(const GF2X& a, const GF2X& b)
   { GF2X x; add(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator+(const GF2X& a, GF2 b)
   { GF2X x; add(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator+(const GF2X& a, long b)
   { GF2X x; add(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator+(GF2 a, const GF2X& b)
   { GF2X x; add(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator+(long a, const GF2X& b)
   { GF2X x; add(x, a, b); NTL_OPT_RETURN(GF2X, x); }


inline GF2X operator-(const GF2X& a, const GF2X& b)
   { GF2X x; sub(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator-(const GF2X& a, GF2 b)
   { GF2X x; sub(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator-(const GF2X& a, long b)
   { GF2X x; sub(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator-(GF2 a, const GF2X& b)
   { GF2X x; sub(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator-(long a, const GF2X& b)
   { GF2X x; sub(x, a, b); NTL_OPT_RETURN(GF2X, x); }


inline GF2X& operator+=(GF2X& x, const GF2X& b)
   { add(x, x, b); return x; }

inline GF2X& operator+=(GF2X& x, GF2 b)
   { add(x, x, b); return x; }

inline GF2X& operator+=(GF2X& x, long b)
   { add(x, x, b); return x; }

inline GF2X& operator-=(GF2X& x, const GF2X& b)
   { sub(x, x, b); return x; }

inline GF2X& operator-=(GF2X& x, GF2 b)
   { sub(x, x, b); return x; }

inline GF2X& operator-=(GF2X& x, long b)
   { sub(x, x, b); return x; }


inline GF2X operator-(const GF2X& a)
   { GF2X x; negate(x, a); NTL_OPT_RETURN(GF2X, x); }

inline GF2X& operator++(GF2X& x) { add(x, x, 1); return x; }
inline void operator++(GF2X& x, int) { add(x, x, 1); }
inline GF2X& operator--(GF2X& x) { sub(x, x, 1); return x; }
inline void operator--(GF2X& x, int) { sub(x, x, 1); }


void mul(GF2X& c, const GF2X& a, const GF2X& b);
void OldMul(GF2X& c, const GF2X& a, const GF2X& b);

void mul(GF2X& x, const GF2X& a, GF2 b);
void mul(GF2X& x, const GF2X& a, long b);

inline void mul(GF2X& x, GF2 a, const GF2X& b) { mul(x, b, a); }
inline void mul(GF2X& x, long a, const GF2X& b) { mul(x, b, a); }

void MulByX(GF2X& x, const GF2X& a);
inline GF2X MulByX(const GF2X& a) 
   { GF2X x; MulByX(x, a); NTL_OPT_RETURN(GF2X, x); }


void sqr(GF2X& c, const GF2X& a);

inline GF2X sqr(const GF2X& a)
   { GF2X x; sqr(x, a); NTL_OPT_RETURN(GF2X, x); }

void trunc(GF2X& x, const GF2X& a, long m);
inline GF2X trunc(const GF2X& a, long m)
   { GF2X x; trunc(x, a, m); NTL_OPT_RETURN(GF2X, x); }


inline GF2X operator*(const GF2X& a, const GF2X& b)
   { GF2X x; mul(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator*(const GF2X& a, GF2 b)
   { GF2X x; mul(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator*(const GF2X& a, long b)
   { GF2X x; mul(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator*(GF2 a, const GF2X& b)
   { GF2X x; mul(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator*(long a, const GF2X& b)
   { GF2X x; mul(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X& operator*=(GF2X& x, const GF2X& b)
   { mul(x, x, b); return x; }

inline GF2X& operator*=(GF2X& x, GF2 b)
   { mul(x, x, b); return x; }

inline GF2X& operator*=(GF2X& x, long b)
   { mul(x, x, b); return x; }

void power(GF2X& x, const GF2X& a, long e);  // x = a^e (e >= 0)
inline GF2X power(const GF2X& a, long e)
   { GF2X x; power(x, a, e); NTL_OPT_RETURN(GF2X, x); }



typedef Vec<GF2X> vec_GF2X;

void LeftShift(GF2X& c, const GF2X& a, long n);
inline GF2X LeftShift(const GF2X& a, long n)
   {  GF2X x; LeftShift(x, a, n); NTL_OPT_RETURN(GF2X, x); }

void ShiftAdd(GF2X& c, const GF2X& a, long n);


void RightShift(GF2X& c, const GF2X& a, long n);
inline GF2X RightShift(const GF2X& a, long n)
   {  GF2X x; RightShift(x, a, n); NTL_OPT_RETURN(GF2X, x); }


#ifndef NTL_TRANSITION

inline GF2X operator>>(const GF2X& a, long n)
   { GF2X x; RightShift(x, a, n); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator<<(const GF2X& a, long n)
   { GF2X x; LeftShift(x, a, n); NTL_OPT_RETURN(GF2X, x); }

inline GF2X& operator<<=(GF2X& x, long n)
   { LeftShift(x, x, n); return x; }

inline GF2X& operator>>=(GF2X& x, long n)
   { RightShift(x, x, n); return x; }

#endif



void CopyReverse(GF2X& c, const GF2X& a, long hi);
// c[0..hi] = reverse(a[0..hi]), with zero fill as necessary

inline void reverse(GF2X& c, const GF2X& a, long hi)
{  CopyReverse(c, a, hi); }

inline GF2X reverse(const GF2X& a, long hi)
   { GF2X x; reverse(x, a, hi); NTL_OPT_RETURN(GF2X, x); }

inline void reverse(GF2X& c, const GF2X& a)
{  CopyReverse(c, a, deg(a)); }


inline GF2X reverse(const GF2X& a)
   { GF2X x; reverse(x, a); NTL_OPT_RETURN(GF2X, x); }

void InvTrunc(GF2X& c, const GF2X& a, long e);


inline GF2X InvTrunc(const GF2X& a, long e)
   { GF2X x; InvTrunc(x, a, e); NTL_OPT_RETURN(GF2X, x); }


class GF2XModulus {

public:
   GF2XModulus();

   GF2XModulus(const GF2XModulus&);  
   GF2XModulus& operator=(const GF2XModulus&); 

   GF2XModulus(const GF2X& ff);

   GF2X f;   // the modulus

   operator const GF2X& () const { return f; }
   const GF2X& val() const { return f; }

   long n; //  deg(f)
   long sn; //  f.xrep.length()
   long posn; //  n - NTL_BITS_PER_LONG*(sn-1);

   long k3; // used for trinomials and pentanomials
   long k2; 
   long k1;

   long size; // word length of residues

   long WordLength() const { return size; }

   _ntl_ulong msk; // mask of high bits of residues

   long method; 

   vec_GF2X stab;

   UniqueArray<unsigned long *> stab_ptr;
   UniqueArray<long> stab_cnt;
   UniqueArray<unsigned long> stab1;


   GF2X h0, f0;

   OptionalVal< Lazy<vec_GF2> > tracevec;

}; 

NTL_DECLARE_RELOCATABLE((GF2XModulus*))

inline long deg(const GF2XModulus& F) { return F.n; }


void build(GF2XModulus& F, const GF2X& f);

void rem(GF2X& r, const GF2X& a, const GF2XModulus& F);
   
void DivRem(GF2X& q, GF2X& r, const GF2X& a, const GF2XModulus& F);

void div(GF2X& q, const GF2X& a, const GF2XModulus& F);

void PlainDivRem(GF2X& q, GF2X& r, const GF2X& a, const GF2X& b);
void PlainDiv(GF2X& q, const GF2X& a, const GF2X& b);
void PlainRem(GF2X& r, const GF2X& a, const GF2X& b);


void MulMod(GF2X& c, const GF2X& a, const GF2X& b, const GF2XModulus& F);
inline GF2X MulMod(const GF2X& a, const GF2X& b, const GF2XModulus& F)
   { GF2X x; MulMod(x, a, b, F); NTL_OPT_RETURN(GF2X, x); }

void SqrMod(GF2X& c, const GF2X& a, const GF2XModulus& F);
inline GF2X SqrMod(const GF2X& a, const GF2XModulus& F)
   { GF2X x; SqrMod(x, a, F); NTL_OPT_RETURN(GF2X, x); }

void MulByXMod(GF2X& c, const GF2X& a, const GF2XModulus& F);
inline GF2X MulByXMod(const GF2X& a, const GF2XModulus& F)
   { GF2X x; MulByXMod(x, a, F); NTL_OPT_RETURN(GF2X, x); }



void MulMod(GF2X& c, const GF2X& a, const GF2X& b, const GF2X& f);
inline GF2X MulMod(const GF2X& a, const GF2X& b, const GF2X& f)
   { GF2X x; MulMod(x, a, b, f); NTL_OPT_RETURN(GF2X, x); } 

void SqrMod(GF2X& c, const GF2X& a, const GF2X& f);
inline GF2X SqrMod(const GF2X& a, const GF2X& f)
   { GF2X x; SqrMod(x, a, f); NTL_OPT_RETURN(GF2X, x); } 

void MulByXMod(GF2X& c, const GF2X& a, const GF2X& f);
inline GF2X MulByXMod(const GF2X& a, const GF2X& f)
   { GF2X x; MulByXMod(x, a, f); NTL_OPT_RETURN(GF2X, x); } 


void InvMod(GF2X& c, const GF2X& a, const GF2X& f);
inline GF2X InvMod(const GF2X& a, const GF2X& f)
   { GF2X x; InvMod(x, a, f); NTL_OPT_RETURN(GF2X, x); } 

long InvModStatus(GF2X& c, const GF2X& a, const GF2X& f);

inline long InvModStatus(GF2X& c, const GF2X& a, const GF2XModulus& F)
   { return InvModStatus(c, a, F.f); }


void PowerMod(GF2X& h, const GF2X& g, const ZZ& e, const GF2XModulus& F);
inline void PowerMod(GF2X& x, const GF2X& g, long e, const GF2XModulus& F)
   { PowerMod(x, g, ZZ_expo(e), F); } 

void PowerXMod(GF2X& hh, const ZZ& e, const GF2XModulus& F);
inline void PowerXMod(GF2X& x, long e, const GF2XModulus& F)
   { PowerXMod(x, ZZ_expo(e), F); } 

inline GF2X PowerMod(const GF2X& g, const ZZ& e, const GF2XModulus& F)
   { GF2X x; PowerMod(x, g, e, F); NTL_OPT_RETURN(GF2X, x); }

inline GF2X PowerMod(const GF2X& g, long e, const GF2XModulus& F)
   { GF2X x; PowerMod(x, g, e, F); NTL_OPT_RETURN(GF2X, x); }

inline GF2X PowerXMod(const ZZ& e, const GF2XModulus& F)
   { GF2X x; PowerXMod(x, e, F); NTL_OPT_RETURN(GF2X, x); }

inline GF2X PowerXMod(long e, const GF2XModulus& F)
   { GF2X x; PowerXMod(x, e, F); NTL_OPT_RETURN(GF2X, x); }



inline GF2X operator%(const GF2X& a, const GF2XModulus& F)
   { GF2X x; rem(x, a, F); NTL_OPT_RETURN(GF2X, x); }

inline GF2X& operator%=(GF2X& x, const GF2XModulus& F)
   { rem(x, x, F); return x; }


inline GF2X operator/(const GF2X& a, const GF2XModulus& F)
   { GF2X x; div(x, a, F); NTL_OPT_RETURN(GF2X, x); }

inline GF2X& operator/=(GF2X& x, const GF2XModulus& F)
   { div(x, x, F); return x; }



void DivRem(GF2X& q, GF2X& r, const GF2X& a, const GF2X& b);

void div(GF2X& q, const GF2X& a, const GF2X& b);

void div(GF2X& q, const GF2X& a, GF2 b);
void div(GF2X& q, const GF2X& a, long b);

void rem(GF2X& r, const GF2X& a, const GF2X& b);




inline GF2X operator/(const GF2X& a, const GF2X& b)
   { GF2X x; div(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator/(const GF2X& a, GF2 b)
   { GF2X x; div(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X operator/(const GF2X& a, long b)
   { GF2X x; div(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X& operator/=(GF2X& x, GF2 b)
   { div(x, x, b); return x; }

inline GF2X& operator/=(GF2X& x, long b)
   { div(x, x, b); return x; }

inline GF2X& operator/=(GF2X& x, const GF2X& b)
   { div(x, x, b); return x; }


inline GF2X operator%(const GF2X& a, const GF2X& b)
   { GF2X x; rem(x, a, b); NTL_OPT_RETURN(GF2X, x); }

inline GF2X& operator%=(GF2X& x, const GF2X& b)
   { rem(x, x, b); return x; }


void GCD(GF2X& d, const GF2X& a, const GF2X& b);
inline GF2X GCD(const GF2X& a, const GF2X& b)
   { GF2X x; GCD(x, a, b); NTL_OPT_RETURN(GF2X, x); }

void OldGCD(GF2X& d, const GF2X& a, const GF2X& b);


void XGCD(GF2X& d, GF2X& s, GF2X& t, const GF2X& a, const GF2X& b);

void OldXGCD(GF2X& d, GF2X& s, GF2X& t, const GF2X& a, const GF2X& b);

   
void diff(GF2X& c, const GF2X& a);
inline GF2X diff(const GF2X& a)
   { GF2X x; diff(x, a); NTL_OPT_RETURN(GF2X, x); }

void conv(GF2X& c, long a);
void conv(GF2X& c, GF2 a);
void conv(GF2X& x, const vec_GF2& a);
inline void conv(GF2X& x, const ZZ& a)
   { conv(x, to_GF2(a)); }

void conv(vec_GF2& x, const GF2X& a);

inline GF2X to_GF2X(long a)
   { GF2X x; conv(x, a); NTL_OPT_RETURN(GF2X, x); }

inline GF2X to_GF2X(GF2 a)
   { GF2X x; conv(x, a); NTL_OPT_RETURN(GF2X, x); }

inline GF2X to_GF2X(const vec_GF2& a)
   { GF2X x; conv(x, a); NTL_OPT_RETURN(GF2X, x); }

inline GF2X to_GF2X(const ZZ& a)
   { GF2X x; conv(x, a); NTL_OPT_RETURN(GF2X, x); }

inline vec_GF2 to_vec_GF2(const GF2X& a)
   { vec_GF2 x; conv(x, a); NTL_OPT_RETURN(vec_GF2, x); }



/* additional legacy conversions for v6 conversion regime */

inline void conv(GF2X& x, const GF2X& a)
   { x = a; }

class ZZX;
void conv(GF2X& x, const ZZX& a);
void conv(ZZX& x, const GF2X& a);


/* ------------------------------------- */





inline GF2X& GF2X::operator=(long a)
   { conv(*this, a); return *this; }

inline GF2X& GF2X::operator=(GF2 a)
   { conv(*this, a); return *this; }

void VectorCopy(vec_GF2& x, const GF2X& a, long n);

inline vec_GF2 VectorCopy(const GF2X& a, long n)
   { vec_GF2 x; VectorCopy(x, a, n); NTL_OPT_RETURN(vec_GF2, x);  }


void MulTrunc(GF2X& c, const GF2X& a, const GF2X& b, long n);
inline GF2X MulTrunc(const GF2X& a, const GF2X& b, long n)
   { GF2X x; MulTrunc(x, a, b, n); NTL_OPT_RETURN(GF2X, x); }

void SqrTrunc(GF2X& c, const GF2X& a, long n);
inline GF2X SqrTrunc(const GF2X& a, long n)
   { GF2X x; SqrTrunc(x, a, n); NTL_OPT_RETURN(GF2X, x); }

long divide(GF2X& q, const GF2X& a, const GF2X& b);

long divide(const GF2X& a, const GF2X& b);


/*** modular composition routines and data structures ***/

struct GF2XArgument {
   vec_GF2X H;
};


void CompMod(GF2X& x, const GF2X& g, 
             const GF2XArgument& A, const GF2XModulus& F);

inline GF2X CompMod(const GF2X& g, 
             const GF2XArgument& A, const GF2XModulus& F)
   { GF2X x; CompMod(x, g, A, F); NTL_OPT_RETURN(GF2X, x); }

void build(GF2XArgument& A, const GF2X& h, const GF2XModulus& F, long m);

void CompMod(GF2X& x, const GF2X& g, const GF2X& h, const GF2XModulus& F);
inline GF2X CompMod(const GF2X& g, const GF2X& h, const GF2XModulus& F)
   { GF2X x; CompMod(x, g, h, F); NTL_OPT_RETURN(GF2X, x); }

void Comp2Mod(GF2X& x1, GF2X& x2, const GF2X& g1, const GF2X& g2,
              const GF2X& h, const GF2XModulus& F);

void Comp3Mod(GF2X& x1, GF2X& x2, GF2X& x3, 
              const GF2X& g1, const GF2X& g2, const GF2X& g3,
              const GF2X& h, const GF2XModulus& F);


void MinPolySeq(GF2X& h, const vec_GF2& a, long m);
inline GF2X MinPolySeq(const vec_GF2& a, long m)
   { GF2X x; MinPolySeq(x, a, m); NTL_OPT_RETURN(GF2X, x); }

void ProbMinPolyMod(GF2X& hh, const GF2X& g, const GF2XModulus& F);
inline GF2X ProbMinPolyMod(const GF2X& g, const GF2XModulus& F)
   { GF2X x; ProbMinPolyMod(x, g, F); NTL_OPT_RETURN(GF2X, x); }

void ProbMinPolyMod(GF2X& hh, const GF2X& g, const GF2XModulus& F, long m);
inline GF2X ProbMinPolyMod(const GF2X& g, const GF2XModulus& F, long m)
   { GF2X x; ProbMinPolyMod(x, g, F, m); NTL_OPT_RETURN(GF2X, x); }

void MinPolyMod(GF2X& hh, const GF2X& g, const GF2XModulus& F);
inline GF2X MinPolyMod(const GF2X& g, const GF2XModulus& F)
   { GF2X x; MinPolyMod(x, g, F); NTL_OPT_RETURN(GF2X, x); }

void MinPolyMod(GF2X& hh, const GF2X& g, const GF2XModulus& F, long m);
inline GF2X MinPolyMod(const GF2X& g, const GF2XModulus& F, long m)
   { GF2X x; MinPolyMod(x, g, F, m); NTL_OPT_RETURN(GF2X, x); }

void IrredPolyMod(GF2X& h, const GF2X& g, const GF2XModulus& F);
inline GF2X IrredPolyMod(const GF2X& g, const GF2XModulus& F)
   { GF2X x; IrredPolyMod(x, g, F); NTL_OPT_RETURN(GF2X, x); }

void IrredPolyMod(GF2X& h, const GF2X& g, const GF2XModulus& F, long m);
inline GF2X IrredPolyMod(const GF2X& g, const GF2XModulus& F, long m)
   { GF2X x; IrredPolyMod(x, g, F, m); NTL_OPT_RETURN(GF2X, x); }


// undocumented stuff:

void MinPolyInternal(GF2X& h, const GF2X& x, long m);

void OldMinPolyInternal(GF2X& h, const GF2X& x, long m);



struct GF2XTransMultiplier {
   GF2X f0, fbi, b;
   long shamt, shamt_fbi, shamt_b;
};

void build(GF2XTransMultiplier& B, const GF2X& b, const GF2XModulus& F);

void UpdateMap(vec_GF2& x, const vec_GF2& a, const GF2XTransMultiplier& B,
               const GF2XModulus& F);

inline vec_GF2 UpdateMap(const vec_GF2& a, 
                       const GF2XTransMultiplier& B, const GF2XModulus& F)
   { vec_GF2 x; UpdateMap(x, a, B, F); NTL_OPT_RETURN(vec_GF2, x); }

inline void project(ref_GF2 x, const vec_GF2& a, const GF2X& b)
   { x = to_GF2(InnerProduct(a.rep, b.xrep)); }

inline GF2 project(const vec_GF2& a, const GF2X& b)
   { return to_GF2(InnerProduct(a.rep, b.xrep)); }


void ProjectPowers(vec_GF2& x, const vec_GF2& a, long k, 
                   const GF2XArgument& H, const GF2XModulus& F);

inline vec_GF2 ProjectPowers(const vec_GF2& a, long k, 
                          const GF2XArgument& H, const GF2XModulus& F)
   { vec_GF2 x; ProjectPowers(x, a, k, H, F); 
     NTL_OPT_RETURN(vec_GF2, x); }

void ProjectPowers(vec_GF2& x, const vec_GF2& a, long k, const GF2X& h, 
                   const GF2XModulus& F);

inline vec_GF2 ProjectPowers(const vec_GF2& a, long k, 
                          const GF2X& H, const GF2XModulus& F)
   { vec_GF2 x; ProjectPowers(x, a, k, H, F); 
     NTL_OPT_RETURN(vec_GF2, x); }

void TraceVec(vec_GF2& S, const GF2X& f);

inline vec_GF2 TraceVec(const GF2X& f)
   { vec_GF2 x; TraceVec(x, f); NTL_OPT_RETURN(vec_GF2, x); }


void TraceMod(ref_GF2 x, const GF2X& a, const GF2XModulus& F);

inline GF2 TraceMod(const GF2X& a, const GF2XModulus& F)
   { GF2 x; TraceMod(x, a, F); return x; }

void TraceMod(ref_GF2 x, const GF2X& a, const GF2X& f);

inline GF2 TraceMod(const GF2X& a, const GF2X& f)
   { GF2 x; TraceMod(x, a, f); return x; }



void GF2XFromBytes(GF2X& x, const unsigned char *p, long n);
inline GF2X GF2XFromBytes(const unsigned char *p, long n)
   { GF2X x; GF2XFromBytes(x, p, n); NTL_OPT_RETURN(GF2X, x); }

void BytesFromGF2X(unsigned char *p, const GF2X& a, long n);

inline long NumBits(const GF2X& a)
   { return deg(a) + 1; }

inline long NumBytes(const GF2X& a)
   { return (NumBits(a) + 7)/8; }



// GF2X scratch variabes


class GF2XWatcher {
public:
   GF2X& watched;
   explicit
   GF2XWatcher(GF2X& _watched) : watched(_watched) {}

   ~GF2XWatcher() { watched.KillBig(); } 
};

#define NTL_GF2XRegister(x) NTL_TLS_LOCAL(GF2X, x); GF2XWatcher _WATCHER__ ## x(x)



// RAII for HexOutput

class GF2XHexOutputPush {
private:
   long OldHexOutput;

   GF2XHexOutputPush(const GF2XHexOutputPush&); // disable
   void operator=(const GF2XHexOutputPush&); // disable

public:
   GF2XHexOutputPush() : OldHexOutput(GF2X::HexOutput) { }
   ~GF2XHexOutputPush() { GF2X::HexOutput = OldHexOutput; }
};


NTL_CLOSE_NNS

#endif

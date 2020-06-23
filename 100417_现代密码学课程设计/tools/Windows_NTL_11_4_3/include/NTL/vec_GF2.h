
#ifndef NTL_vec_GF2__H
#define NTL_vec_GF2__H

#include <NTL/WordVector.h>
#include <NTL/GF2.h>

NTL_OPEN_NNS


// Vec<GF2> is an explicit specialization of Vec<T>.
// Vec<GF2> is declared, but not defined, in GF2.h,
// to prevent the generic Vec from being used.

template<> 
class Vec<GF2> {

public:

// these should be private, but they are not

   WordVector rep;

   long _len;  // length (in bits)
   long _maxlen;  // (MaxLength << 1) | (fixed)

   // invariants: rep.length() "tracks" length() ( = _len)
   //             All bits in positions >= length are zero.

   // Note:       rep.MaxLength() may exceed the value
   //             indicated by MaxLength().
   

//the following are "really" public


   Vec() : _len(0), _maxlen(0) {}
   Vec(INIT_SIZE_TYPE, long n) : _len(0), _maxlen(0) { SetLength(n); }
   Vec(const Vec<GF2>& a) : _len(0), _maxlen(0) { *this = a; }

   Vec& operator=(const Vec<GF2>& a);

   ~Vec() {}

#if (NTL_CXX_STANDARD >= 2011 && !defined(NTL_DISABLE_MOVE))

Vec(Vec&& a) NTL_FAKE_NOEXCEPT : Vec()
{
   if (a.fixed()) {
      *this = a;
   }
   else {
      rep.unpinned_move(a.rep);
      _len = _ntl_scalar_move(a._len);
      _maxlen = _ntl_scalar_move(a._maxlen);
   }
}

#ifndef NTL_DISABLE_MOVE_ASSIGN
Vec& operator=(Vec&& a) NTL_FAKE_NOEXCEPT
{
   if (fixed() || a.fixed()) {
      *this = a;
   }
   else {
      rep.unpinned_move(a.rep);
      _len = _ntl_scalar_move(a._len);
      _maxlen = _ntl_scalar_move(a._maxlen);
   }

   return *this;
}
#endif


#endif

   void kill();

   void SetLength(long n);
   void SetLength(long n, GF2 a);

   void SetMaxLength(long n);
   void FixLength(long n);
   void FixAtCurrentLength();

   long length() const { return _len; }
   long MaxLength() const { return _maxlen >> 1; }  
   long allocated() const { return rep.MaxLength() * NTL_BITS_PER_LONG; }
   long fixed() const { return _maxlen & 1; }


   Vec(Vec<GF2>& x, INIT_TRANS_TYPE) : 
      rep(x.rep, INIT_TRANS), _len(x._len), _maxlen(x._maxlen) { }



   ref_GF2 operator[](long i) 
   {
#ifdef NTL_RANGE_CHECK
      if (i < 0 || i >= _len) LogicError("index out of range in Vec");
#endif

      iterator t(INIT_LOOP_HOLE, rep.elts(), i);
      return *t;
   }

   const GF2 operator[](long i) const
   {
#ifdef NTL_RANGE_CHECK
      if (i < 0 || i >= _len) LogicError("index out of range in Vec");
#endif

      const_iterator t(INIT_LOOP_HOLE, rep.elts(), i);
      return *t;
   }

   ref_GF2 at(long i) 
   {
      if (i < 0 || i >= _len) LogicError("index out of range in Vec");
      iterator t(INIT_LOOP_HOLE, rep.elts(), i);
      return *t;
   }

   const GF2 at(long i) const
   {
      if (i < 0 || i >= _len) LogicError("index out of range in Vec");
      const_iterator t(INIT_LOOP_HOLE, rep.elts(), i);
      return *t;
   }

   void put(long i, GF2 a) { (*this)[i] = a; }
   void put(long i, long a) { put(i, to_GF2(a)); }

   const GF2 get(long i) const
   {
      return (*this)[i];
   }

   ref_GF2 operator()(long i) 
      { return (*this)[i-1]; }

   const GF2 operator()(long i) const 
      { return (*this)[i-1]; }

   void swap(Vec<GF2>& y);
   void move(Vec<GF2>& y);
   void append(GF2 a);
   void append(const Vec<GF2>& w);




// Some partial STL compatibility...also used
// to interface with the Matrix template class

   typedef GF2 value_type;
   typedef ref_GF2 reference;
   typedef const GF2 const_reference;

// The following makes it possible to use range-based for-loops
// However, the only safe ways to use it are
//    for (auto x: vec)
//    for (auto&& x : vec)
// The following:  
//    for (const auto& x : vec)
// will unfortunately allow elements of vec to be modified 

   template<class T> // T is either unsigned long or const unsigned long
   struct proxy_iterator_impl {

      T *ptr;
      long idx;

      proxy_iterator_impl() : ptr(0), idx(0) { }
      proxy_iterator_impl(T *_ptr, long _idx) : ptr(_ptr), idx(_idx) { }

      template <class X>
      proxy_iterator_impl(const proxy_iterator_impl<X>& other)
      { ptr = other.ptr; idx = other.idx; }

      void add(long x)
      {
         idx += x;
      }

      void inc() { idx++; }

      void dec() { idx--; }

      ref_GF2 make_ref_GF2() const
      {
         long q, r;
         _ntl_bpl_divrem(cast_unsigned(idx), q, r);
         return ref_GF2(INIT_LOOP_HOLE, ptr+q, r);
      }

      const GF2  make_GF2() const
      {
         long q, r;
         _ntl_bpl_divrem(cast_unsigned(idx), q, r);
	 return GF2(INIT_LOOP_HOLE, (ptr[q] >> r) & 1);
      }

      long diff(const proxy_iterator_impl& other) const
      {
         return (this->idx - other.idx);
      }

      bool eq(const proxy_iterator_impl& other) const
      { return ptr == other.ptr && idx == other.idx; }

   };


   struct const_proxy_iterator {

      proxy_iterator_impl<const unsigned long> rep;

      const_proxy_iterator() { }
      const_proxy_iterator(INIT_LOOP_HOLE_TYPE, const unsigned long *ptr, long idx)
         : rep(ptr, idx) { }

      const_proxy_iterator& operator++() { rep.inc(); return *this; }
      const_proxy_iterator  operator++(int) { const_proxy_iterator t = *this; rep.inc(); return t; }

      const_proxy_iterator& operator--() { rep.dec(); return *this; }
      const_proxy_iterator  operator--(int) { const_proxy_iterator t = *this; rep.dec(); return t; }

      const_proxy_iterator& operator+=(long x) { rep.add(x); return *this; }
      const_proxy_iterator& operator-=(long x) { rep.add(-x); return *this; }

      const GF2 operator*() const { return rep.make_GF2(); }

      const GF2 operator[](long x) const 
      { const_proxy_iterator t = *this; t.rep.add(x); return *t; }

   };

   typedef const_proxy_iterator const_iterator;


   const_iterator begin() const { return const_iterator(INIT_LOOP_HOLE, rep.elts(), 0); }
   const_iterator end() const { 
      return const_iterator(INIT_LOOP_HOLE, rep.elts(), _len);
   }


   struct proxy_iterator {

      proxy_iterator_impl<unsigned long> rep;

      proxy_iterator() { }
      proxy_iterator(INIT_LOOP_HOLE_TYPE, unsigned long *ptr, long idx)
         : rep(ptr, idx) { }

      proxy_iterator& operator++() { rep.inc(); return *this; }
      proxy_iterator  operator++(int) { proxy_iterator t = *this; rep.inc(); return t; }

      proxy_iterator& operator--() { rep.dec(); return *this; }
      proxy_iterator  operator--(int) { proxy_iterator t = *this; rep.dec(); return t; }

      proxy_iterator& operator+=(long x) { rep.add(x); return *this; }
      proxy_iterator& operator-=(long x) { rep.add(-x); return *this; }

      ref_GF2 operator*() const { return rep.make_ref_GF2(); }

      ref_GF2 operator[](long x) const 
      { proxy_iterator t = *this; t.rep.add(x); return *t; }

      operator const_proxy_iterator() const 
      { const_proxy_iterator t; t.rep = rep; return t; }

   };

   typedef proxy_iterator iterator;

   iterator begin() { return iterator(INIT_LOOP_HOLE, rep.elts(), 0); }
   iterator end() { 
      return iterator(INIT_LOOP_HOLE, rep.elts(), _len);
   }

 };

typedef Vec<GF2> vec_GF2;



inline
bool operator==(const vec_GF2::const_proxy_iterator& a, const vec_GF2::const_proxy_iterator& b) 
{ return a.rep.eq(b.rep); }

inline
bool operator!=(const vec_GF2::const_proxy_iterator& a, const vec_GF2::const_proxy_iterator& b) 
{ return !(a == b); }

inline
vec_GF2::const_proxy_iterator operator+(const vec_GF2::const_proxy_iterator& a, long x)
{ vec_GF2::const_proxy_iterator t = a; t.rep.add(x); return t; }

inline
vec_GF2::const_proxy_iterator operator+(long x, const vec_GF2::const_proxy_iterator& a)
{ return a + x; }

inline
vec_GF2::const_proxy_iterator operator-(const vec_GF2::const_proxy_iterator& a, long x)
{ vec_GF2::const_proxy_iterator t = a; t.rep.add(-x); return t; }

inline
vec_GF2::const_proxy_iterator operator-(long x, const vec_GF2::const_proxy_iterator& a)
{ return a - x; }

inline
long operator-(const vec_GF2::const_proxy_iterator& a, const vec_GF2::const_proxy_iterator& b)
{ return a.rep.diff(b.rep); }









inline
bool operator==(const vec_GF2::proxy_iterator& a, const vec_GF2::proxy_iterator& b) 
{ return a.rep.eq(b.rep); }

inline
bool operator!=(const vec_GF2::proxy_iterator& a, const vec_GF2::proxy_iterator& b) 
{ return !(a == b); }

inline
vec_GF2::proxy_iterator operator+(const vec_GF2::proxy_iterator& a, long x)
{ vec_GF2::proxy_iterator t = a; t.rep.add(x); return t; }

inline
vec_GF2::proxy_iterator operator+(long x, const vec_GF2::proxy_iterator& a)
{ return a + x; }

inline
vec_GF2::proxy_iterator operator-(const vec_GF2::proxy_iterator& a, long x)
{ vec_GF2::proxy_iterator t = a; t.rep.add(-x); return t; }

inline
vec_GF2::proxy_iterator operator-(long x, const vec_GF2::proxy_iterator& a)
{ return a - x; }

inline
long operator-(const vec_GF2::proxy_iterator& a, const vec_GF2::proxy_iterator& b)
{ return a.rep.diff(b.rep); }


// sepcialized conversion
inline void conv(vec_GF2& x, const vec_GF2& a)
{  x = a; }


inline void swap(vec_GF2& x, vec_GF2& y) { x.swap(y); }
inline void append(vec_GF2& v, const GF2 a) { v.append(a); }
inline void append(vec_GF2& v, const vec_GF2& a) { v.append(a); }

long operator==(const vec_GF2& a, const vec_GF2& b);
inline long operator!=(const vec_GF2& a, const vec_GF2& b)
   { return !(a == b); }

NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const vec_GF2& a);
NTL_SNS istream& operator>>(NTL_SNS istream& s, vec_GF2& a);

void shift(vec_GF2& x, const vec_GF2& a, long n);
// x = a shifted n places, i.e., if l = a.length(),
//    x.length() = l, x[i] = a[i-n] for 0 <= i-n < l,
//    and x[i] = 0 for all other i such that 0 <= i < l.

inline vec_GF2 shift(const vec_GF2& a, long n)
   { vec_GF2 x; shift(x, a, n); NTL_OPT_RETURN(vec_GF2, x); }

void reverse(vec_GF2& x, const vec_GF2& a);

inline vec_GF2 reverse(const vec_GF2& a)
   { vec_GF2 x; reverse(x, a); NTL_OPT_RETURN(vec_GF2, x); }

void random(vec_GF2& x, long n);
inline vec_GF2 random_vec_GF2(long n)
   { vec_GF2 x; random(x, n); NTL_OPT_RETURN(vec_GF2, x); }

long weight(const vec_GF2& a);

void mul(vec_GF2& x, const vec_GF2& a, GF2 b);
inline void mul(vec_GF2& x, GF2 a, const vec_GF2& b)
   { mul(x, b, a); }

inline void mul(vec_GF2& x, const vec_GF2& a, long b)
   { mul(x, a, to_GF2(b)); }
inline void mul(vec_GF2& x, long a, const vec_GF2& b)
   { mul(x, b, a); }

void add(vec_GF2& x, const vec_GF2& a, const vec_GF2& b);

inline void sub(vec_GF2& x, const vec_GF2& a, const vec_GF2& b)
   { add(x, a, b); }

void clear(vec_GF2& x);

inline void negate(vec_GF2& x, const vec_GF2& a)
   { x = a; }

inline void InnerProduct(ref_GF2 x, const vec_GF2& a, const vec_GF2& b)
   { x = to_GF2(InnerProduct(a.rep, b.rep)); }

long IsZero(const vec_GF2& a);

vec_GF2 operator+(const vec_GF2& a, const vec_GF2& b);

vec_GF2 operator-(const vec_GF2& a, const vec_GF2& b);

inline vec_GF2 operator-(const vec_GF2& a)
   { return a; }

inline vec_GF2 operator*(const vec_GF2& a, GF2 b)
   { vec_GF2 x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2, x); }

inline vec_GF2 operator*(const vec_GF2& a, long b)
   { vec_GF2 x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2, x); }

inline vec_GF2 operator*(GF2 a, const vec_GF2& b)
   { vec_GF2 x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2, x); }

inline vec_GF2 operator*(long a, const vec_GF2& b)
   { vec_GF2 x; mul(x, a, b); NTL_OPT_RETURN(vec_GF2, x); }


inline GF2 operator*(const vec_GF2& a, const vec_GF2& b)
   { return to_GF2(InnerProduct(a.rep, b.rep)); }

// assignment operator notation:

inline vec_GF2& operator+=(vec_GF2& x, const vec_GF2& a)
{ 
   add(x, x, a);
   return x;
}

inline vec_GF2& operator-=(vec_GF2& x, const vec_GF2& a)
{ 
   sub(x, x, a);
   return x;
}

inline vec_GF2& operator*=(vec_GF2& x, GF2 a)
{ 
   mul(x, x, a);
   return x;
}

inline vec_GF2& operator*=(vec_GF2& x, long a)
{ 
   mul(x, x, a);
   return x;
}

void VectorCopy(vec_GF2& x, const vec_GF2& a, long n);
inline vec_GF2 VectorCopy(const vec_GF2& a, long n)
   { vec_GF2 x; VectorCopy(x, a, n); NTL_OPT_RETURN(vec_GF2, x); }

NTL_CLOSE_NNS


#endif



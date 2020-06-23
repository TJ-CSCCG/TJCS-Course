
#ifndef NTL_WordVector__H
#define NTL_WordVector__H

/**************************************************************

  A WordVector is functionally similar to
  a  generic NTL vector of _ntl_ulong.  

  Be careful! the MaxLength() function does not return 
    the max length ever set, but rather the max space allocated,
    which *may* be more.

  The FixLength() facility is not available.

  The reason for special-casing is efficiency (of course).

**************************************************************/



#include <NTL/tools.h>
#include <NTL/ZZ.h>

NTL_OPEN_NNS



#ifndef NTL_RANGE_CHECK
#define NTL_WV_RANGE_CHECK_CODE 
#else
#define NTL_WV_RANGE_CHECK_CODE if (i < 0 || !rep || i >= long(rep[-1])) LogicError("index out of range in WordVector");
#endif

// vectors are allocated in chunks of this size

#ifndef NTL_WordVectorMinAlloc
#define NTL_WordVectorMinAlloc (4)
#endif

// controls initialization during input

#ifndef NTL_WordVectorInputBlock
#define NTL_WordVectorInputBlock 50
#endif


class WordVector {  
public:  
   _ntl_ulong *rep;  

   WordVector(WordVector& x, INIT_TRANS_TYPE) { rep = x.rep; x.rep = 0; }


  
   WordVector() : rep(0) { }  
   WordVector(INIT_SIZE_TYPE, long n) : rep(0) { DoSetLength(n); }  
   WordVector(const WordVector& a) : rep(0) { *this = a; }     

   WordVector& operator=(const WordVector& a);  

   bool pinned() const 
   {
      return rep && (rep[-2] & 1);
   }

   // assumes *this and other are not pinned
   void unpinned_swap(WordVector& other)
   {
      _ntl_swap(this->rep, other.rep);
   }

   // assumes *this and other are not pinned
   void unpinned_move(WordVector& other)
   {
      WordVector tmp;
      tmp.unpinned_swap(other);
      tmp.unpinned_swap(*this);
   }

   ~WordVector();  
   void kill(); 

   void KillBig() { if (MaxLength() > NTL_RELEASE_THRESH) kill(); }
   // this conditinally kills the vector, if its size is excessive

   void DoSetLength(long n);
  
   void SetLength(long n)
   {
      _ntl_ulong *x = rep;
      if (x && long(x[-2] >> 1) >= n && n >= 0)
         x[-1] = n;
      else
         DoSetLength(n);
   }

   void ZeroLength() { if (rep) rep[-1] = 0; }
         
   void SetMaxLength(long n); 
   void QuickSetLength(long n) { rep[-1] = _ntl_ulong(n); } 
  
   long length() const { return (!rep) ?  0 : long(rep[-1]); }  
   long MaxLength() const 
   { return (!rep) ?  0 : long(rep[-2] >> 1); } 
  
   _ntl_ulong& operator[](long i)   
   {  
      NTL_WV_RANGE_CHECK_CODE  
      return rep[i];  
   }  
  
   const _ntl_ulong& operator[](long i) const 
   {  
      NTL_WV_RANGE_CHECK_CODE  
      return rep[i];  
   }  
  
   _ntl_ulong& operator()(long i) { return (*this)[i-1]; }  
   const _ntl_ulong& operator()(long i) const { return (*this)[i-1]; } 
   
  
   const _ntl_ulong* elts() const { return rep; }  
   _ntl_ulong* elts() { return rep; }  
         
   void swap(WordVector& y);  
   void append(_ntl_ulong a); 
   void append(const WordVector& w); 
}; 




class WordVectorWatcher {
public:
   WordVector& watched;
   explicit
   WordVectorWatcher(WordVector& _watched) : watched(_watched) {}

   ~WordVectorWatcher() { watched.KillBig(); }
};


inline void swap(WordVector& x, WordVector& y) 
   { x.swap(y); }

inline void append(WordVector& v, _ntl_ulong a)
   { v.append(a); }

inline void append(WordVector& v, const WordVector& w)
   { v.append(w); }


NTL_SNS istream& operator>>(NTL_SNS istream&, WordVector&);  
NTL_SNS ostream& operator<<(NTL_SNS ostream&, const WordVector&);  


long operator==(const WordVector& a, const WordVector& b);  
long operator!=(const WordVector& a, const WordVector& b);


long InnerProduct(const WordVector& a, const WordVector& b);

void ShiftAdd(_ntl_ulong *cp, const _ntl_ulong* ap, long sa, long n);
// cp = cp + (a << n)


long WV_BlockConstructAlloc(WordVector& x, long d, long n);
 
void WV_BlockConstructSet(WordVector& x, WordVector& y, long i);
 
long WV_BlockDestroy(WordVector& x);

long WV_storage(long d);





NTL_CLOSE_NNS

#endif

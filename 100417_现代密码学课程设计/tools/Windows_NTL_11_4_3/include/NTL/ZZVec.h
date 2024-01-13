#ifndef NTL_ZZVec__H
#define NTL_ZZVec__H

#include <NTL/ZZ.h>

NTL_OPEN_NNS

/*****************************************************************

The class ZZVec implements vectors of fixed-length ZZ's.
You can allocate a vector of ZZ's of a specified length, where
the maximum size of each ZZ is also specified.
These parameters can be specified once, either with a constructor,
or with SetSize.
It is an error to try to re-size a vector, or store a ZZ that
doesn't fit.
The space can be released with "kill", and then you are free to 
call SetSize again.
If you want more flexible---but less efficient---vectors, 
use vec_ZZ.

*****************************************************************/



class ZZVec {

private:
   ZZ* v;
   long len;
   long bsize;


public:
   ZZVec& operator=(const ZZVec&); 
   ZZVec(const ZZVec&); 

   long length() const { return len; }
   long BaseSize() const { return bsize; }
   void SetSize(long n, long d);
   void kill();

   ZZVec() : v(0), len(0), bsize(0) { }
   ZZVec(long n, long d) : v(0), len(0), bsize(0)  { SetSize(n, d); }
   ~ZZVec() { kill(); };

   ZZ* elts() { return v; }
   const ZZ* elts() const { return v; }

   ZZ& operator[](long i) { return v[i]; }
   const ZZ& operator[](long i) const { return v[i]; }


   void swap(ZZVec& x)
   {
      _ntl_swap(v, x.v);
      _ntl_swap(len, x.len);
      _ntl_swap(bsize, x.bsize);
   }

   void move(ZZVec& other) 
   { 
      ZZVec tmp;
      tmp.swap(other);
      tmp.swap(*this);
   }


#if (NTL_CXX_STANDARD >= 2011 && !defined(NTL_DISABLE_MOVE))

   ZZVec(ZZVec&& other) noexcept : ZZVec() 
   {
      this->move(other);
   }

   ZZVec& operator=(ZZVec&& other) noexcept
   {
      this->move(other);
      return *this;
   }

#endif


};


NTL_DECLARE_RELOCATABLE((ZZVec*))


inline void swap(ZZVec& x, ZZVec& y) { x.swap(y); }

NTL_CLOSE_NNS

#endif

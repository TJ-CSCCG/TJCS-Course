#ifndef NTL_matrix__H
#define NTL_matrix__H

#include <NTL/tools.h>
#include <NTL/vector.h>


// matrix templates

NTL_OPEN_NNS


template<class T> 
class Mat {  
private:

   struct Fixer {
      long m;

      explicit Fixer(long _m) : m(_m) { }
      void operator()(Vec<T>& v) { v.FixLength(m); }
   };

public:  
  
   // pseudo-private fields
   Vec< Vec<T> > _mat__rep;  
   long _mat__numcols;  



   // really public fields

   typedef typename Vec<T>::value_type value_type;
   typedef typename Vec<T>::reference reference;
   typedef typename Vec<T>::const_reference const_reference;
  
  
   Mat() : _mat__numcols(0) { }  
   Mat(const Mat& a);  
   Mat& operator=(const Mat& a);  
  
   Mat(INIT_SIZE_TYPE, long n, long m);  
  
   void kill();  
  
   void SetDims(long n, long m);  
  
   long NumRows() const { return _mat__rep.length(); }  
   long NumCols() const { return _mat__numcols; }  
  
   Vec<T>& operator[](long i) { return _mat__rep[i]; }  
   const Vec<T>& operator[](long i) const { return _mat__rep[i]; }  
  
   Vec<T>& operator()(long i) { return _mat__rep[i-1]; }  
   const Vec<T>& operator()(long i) const { return _mat__rep[i-1]; }  
  
   reference operator()(long i, long j) { return _mat__rep[i-1][j-1]; }  
   const_reference operator()(long i, long j) const   
      { return _mat__rep[i-1][j-1]; }  

   const_reference get(long i, long j) const { return _mat__rep[i].get(j); }
   void put(long i, long j, const T& a) { _mat__rep[i].put(j, a); }

   template <class U>
   void put(long i, long j, const U& a) { _mat__rep[i].put(j, a); }

  
   long position(const Vec<T>& a) const { return _mat__rep.position(a); } 
   long position1(const Vec<T>& a) const { return _mat__rep.position1(a); } 
   long alias(const Vec<T>& a) const 
   {
      return a.fixed() && a.length() == NumCols() && position1(a) != -1; 
   }

   Mat(Mat& x, INIT_TRANS_TYPE) :  
    _mat__rep(x._mat__rep, INIT_TRANS), _mat__numcols(x._mat__numcols) { }  

   void swap(Mat& other)
   {
      _mat__rep.swap(other._mat__rep);
      _ntl_swap(_mat__numcols, other._mat__numcols);
   }

   void move(Mat& other) 
   { 
      Mat tmp;
      tmp.swap(other);
      tmp.swap(*this);
   }


#if (NTL_CXX_STANDARD >= 2011 && !defined(NTL_DISABLE_MOVE))

   Mat(Mat&& other) noexcept : Mat() 
   {
      this->move(other);
   }

#ifndef NTL_DISABLE_MOVE_ASSIGN
   Mat& operator=(Mat&& other) noexcept
   {
      this->move(other);
      return *this;
   }
#endif

#endif


};  


template<class T> NTL_DECLARE_RELOCATABLE((Mat<T>*))
 
template<class T> 
inline const Vec< Vec<T> >& rep(const Mat<T>& a)  
   { return a._mat__rep; }  
  

template<class T>
Mat<T>::Mat(const Mat& src) : 
   _mat__rep(src._mat__rep), _mat__numcols(src._mat__numcols)
{  
   long i, nrows;

   nrows = _mat__rep.length();
   for (i = 0; i < nrows; i++)
      _mat__rep[i].FixAtCurrentLength();
}  
  
template<class T>
Mat<T>& Mat<T>::operator=(const Mat& src)  
{  
   if (this == &src) return *this;

   if (src.NumCols() == 0)
      SetDims(src.NumRows(), src.NumCols());
   else if (NumCols() != src.NumCols()) {
      Mat<T> tmp(src);
      this->swap(tmp);
   }
   else {
      long i, init, len;

      init = _mat__rep.MaxLength();
      len = src._mat__rep.length();

      _mat__rep = src._mat__rep;

      for (i = init; i < len; i++)
         _mat__rep[i].FixAtCurrentLength();
   }

   return *this;
}  
  
template<class T>
Mat<T>::Mat(INIT_SIZE_TYPE, long n, long m) : _mat__numcols(0)
{  
   SetDims(n, m);  
}  
  
template<class T>
void Mat<T>::kill()  
{  
   Mat<T> tmp;
   this->swap(tmp);
}  
  

// This is designed to provide strong ES
template<class T>
void Mat<T>::SetDims(long n, long m)  
{  
   if (n < 0 || m < 0)  
      LogicError("SetDims: bad args");  

   long init = _mat__rep.MaxLength();  

   if (init > 0 && m != _mat__numcols) {
      Mat<T> tmp;
      tmp._mat__rep.SetLengthAndApply(n, Fixer(m));
      tmp._mat__numcols = m;
      this->swap(tmp);
   }
   else {
      _mat__rep.SetLengthAndApply(n, Fixer(m));
      _mat__numcols = m;
   }
  
}  
     
        
template<class T>
void MakeMatrix(Mat<T>& x, const Vec< Vec<T> >& a)  
{  
   long n = a.length();  
  
   if (n == 0) {  
      x.SetDims(0, 0);  
      return;  
   }  
  
   long m = a[0].length();  
   long i;  
  
   for (i = 1; i < n; i++)  
      if (a[i].length() != m)  
         LogicError("nonrectangular matrix");  
  
   x.SetDims(n, m);  
   for (i = 0; i < n; i++)  
      x[i] = a[i];  
}  

template<class T>
bool MakeMatrixStatus(Mat<T>& x, const Vec< Vec<T> >& a)  
{  
   long n = a.length();  
  
   if (n == 0) {  
      x.SetDims(0, 0);  
      return false;  
   }  
  
   long m = a[0].length();  
   long i;  
  
   for (i = 1; i < n; i++)  
      if (a[i].length() != m)  
         return true;
  
   x.SetDims(n, m);  
   for (i = 0; i < n; i++)  
      x[i] = a[i];  

   return false;
}  
  
template<class T>
void swap(Mat<T>& X, Mat<T>& Y)  
{  
   X.swap(Y);
}  
  
template<class T>
long operator==(const Mat<T>& a, const Mat<T>& b)  
{  
   if (a.NumCols() != b.NumCols())  
      return 0;  
  
   if (a.NumRows() != b.NumRows())  
      return 0;  
  
   long n = a.NumRows();  
   long i;  
  
   for (i = 0; i < n; i++)  
      if (a[i] != b[i])  
         return 0;  
  
   return 1;  
}  
  
  
template<class T>
long operator!=(const Mat<T>& a, const Mat<T>& b)  
{  
   return !(a == b);  
}  


template<class T>
NTL_SNS istream& operator>>(NTL_SNS istream& s, Mat<T>& x)  
{  
   Vec< Vec<T> > buf;  
   NTL_INPUT_CHECK_RET(s, s >> buf);  
   if (MakeMatrixStatus(x, buf)) 
      NTL_INPUT_ERROR(s, "non-rectangular matrix detected on input");
   return s;  
}  
  
template<class T>
NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const Mat<T>& a)  
{  
   long n = a.NumRows();  
   long i;  
   s << "[";  
   for (i = 0; i < n; i++) {  
      s << a[i]; 
      s << "\n"; 
   }  
   s << "]";  
   return s;  
}  


// conversion

template<class T, class S>
void conv(Mat<T>& x, const Mat<S>& a)
{  
   x.SetDims(a.NumRows(), a.NumCols());  
   conv(x._mat__rep, a._mat__rep);  
}  



NTL_CLOSE_NNS


#endif

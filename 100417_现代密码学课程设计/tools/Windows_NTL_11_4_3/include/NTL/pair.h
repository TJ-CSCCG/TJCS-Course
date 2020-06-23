
#ifndef NTL_pair__H
#define NTL_pair__H

#include <NTL/tools.h>

// pair templates

NTL_OPEN_NNS

template<class S, class T>
class Pair {  
public:  
   S a;  
   T b;  
  
   Pair() { }  
   Pair(const S& x, const T& y) : a(x), b(y) { }  

};  

template<class S, class T> NTL_DECLARE_RELOCATABLE_WHEN((Pair<S,T>*))
   { return DeclareRelocatableType((S*)0) &&
            DeclareRelocatableType((T*)0); }
// FIXME: remove CV-qualifiers and S and T? 

  
template<class S, class T>
inline Pair<S,T> cons(const S& x, const T& y) { return Pair<S,T>(x, y); } 



template<class S, class T>
inline long operator==(const Pair<S,T>& x, const Pair<S,T>& y)  
   { return x.a == y.a && x.b == y.b; }  

template<class S, class T>
inline long operator!=(const Pair<S,T>& x, const Pair<S,T>& y) 
   { return !(x == y); }  



template<class S, class T>
NTL_SNS istream& operator>>(NTL_SNS istream& s, Pair<S,T>& x)  
{  
   long c;  
   S a;
   T b;
  
   if (!s) NTL_INPUT_ERROR(s, "bad pair input");  
  
   c = s.peek();  
   while (IsWhiteSpace(c)) {  
      s.get();  
      c = s.peek();  
   }  
  
   if (c != '[')  
      NTL_INPUT_ERROR(s, "bad pair input");  
  
   s.get();  
  
   if (!(s >> a))   
      NTL_INPUT_ERROR(s, "bad pair input");  
   if (!(s >> b))  
      NTL_INPUT_ERROR(s, "bad pair input");  
  
   c = s.peek();  
   while (IsWhiteSpace(c)) {  
      s.get();  
      c = s.peek();  
   }  
  
   if (c != ']')  
      NTL_INPUT_ERROR(s, "bad pair input");  
  
   s.get();  

   x.a = a;
   x.b = b;
   return s;  
}  
  
template<class S, class T>
NTL_SNS ostream& operator<<(NTL_SNS ostream& s, const Pair<S,T>& x)  
{  
   return s << "[" << x.a << " " << x.b << "]";  
}  


NTL_CLOSE_NNS


#endif


#include <NTL/WordVector.h>

#include <cstdio>

NTL_START_IMPL



void WordVector::DoSetLength(long n)   
{   
   long m;  
  
   if (n < 0) {  
      LogicError("negative length in vector::SetLength");  
   }  

   if (NTL_OVERFLOW(n, NTL_BITS_PER_LONG, 0)) 
      ResourceError("length too big in vector::SetLength");
      
   if (n == 0) {  
      if (rep) rep[-1] = 0;  
      return;  
   }  
  
   if (!rep) {  
      m = ((n+NTL_WordVectorMinAlloc-1)/NTL_WordVectorMinAlloc) * NTL_WordVectorMinAlloc; 

      if (NTL_OVERFLOW(m, NTL_BITS_PER_LONG, 0))
         ResourceError("length too big in vector::SetLength");

      _ntl_ulong *p = (_ntl_ulong *) 
                      NTL_SNS_MALLOC(m, sizeof(_ntl_ulong), 2*sizeof(_ntl_ulong));

      if (!p) {  
	 MemoryError();  
      }  

      rep = p+2;

      rep[-1] = n;
      rep[-2] = m << 1;
 
      return;
   }  

   long max_length = (rep[-2] >> 1);

   if (n <= max_length) {  
      rep[-1] = n;  
      return;
   }  

   long frozen = (rep[-2] & 1);

   if (frozen) LogicError("Cannot grow this WordVector");
      
   m = max(n, _ntl_vec_grow(max_length));

   m = ((m+NTL_WordVectorMinAlloc-1)/NTL_WordVectorMinAlloc)*NTL_WordVectorMinAlloc; 
   _ntl_ulong *p = rep - 2;

   if (NTL_OVERFLOW(m, NTL_BITS_PER_LONG, 0))
      ResourceError("length too big in vector::SetLength");

   p = (_ntl_ulong *) 
       NTL_SNS_REALLOC(p, m, sizeof(_ntl_ulong), 2*sizeof(_ntl_ulong)); 

   if (!p) {  
      MemoryError();  
   }  

   rep = p+2;

   rep[-1] = n;
   rep[-2] = m << 1;
}  
 
 
void WordVector::SetMaxLength(long n) 
{ 
   long OldLength = length(); 
   DoSetLength(n); 
   if (rep) rep[-1] = OldLength;
} 
 
  
WordVector& WordVector::operator=(const WordVector& a)  
{  
   long i, n;  
   _ntl_ulong *p;  
   const _ntl_ulong *ap;  
  
   if (this == &a) return *this;  
  
   n = a.length();  
   ap = a.elts();  
  
   SetLength(n);  
   p = elts();  
  
   for (i = 0; i < n; i++)  
      p[i] = ap[i];  

   return *this;
}  
       
  
WordVector::~WordVector()  
{  
   if (!rep) return;  
   if (rep[-2] & 1) TerminalError("Cannot free this WordVector");
   free(rep-2);
}  
   
void WordVector::kill()  
{  
   if (!rep) return;  
   if (rep[-2] & 1) LogicError("Cannot free this WordVector");
   free(rep-2);
   rep = 0; 
}  
  
void CopySwap(WordVector& x, WordVector& y)
{
   NTL_TLS_LOCAL(WordVector, t);
   WordVectorWatcher watch_t(t);

   long sz_x = x.length();
   long sz_y = y.length();
   long sz = (sz_x > sz_y) ? sz_x : sz_y;

   x.SetMaxLength(sz);
   y.SetMaxLength(sz);

   // EXCEPTIONS: all of the above ensures that swap provides strong ES

   t = x;
   x = y;
   y = t;
}
 
void WordVector::swap(WordVector& y)  
{  
   if ((this->rep && (this->rep[-2] & 1)) ||
       (y.rep && (y.rep[-2] & 1))) {
      CopySwap(*this, y);
      return;
   }

   _ntl_swap(this->rep, y.rep);
} 
 
void WordVector::append(_ntl_ulong a)  
{  
   long l = this->length();
   this->SetLength(l+1);  
   (*this)[l] = a;  
}  
  
void WordVector::append(const WordVector& w)  
{  
   long l = this->length();  
   long m = w.length();  
   long i;  
   this->SetLength(l+m);  
   for (i = 0; i < m; i++)  
      (*this)[l+i] = w[i];  
}


istream & operator>>(istream& s, WordVector& a)   
{   
   WordVector ibuf;  
   long c;   
   long n;   
   if (!s) NTL_INPUT_ERROR(s, "bad vector input"); 
   
   c = s.peek();  
   while (IsWhiteSpace(c)) {  
      s.get();  
      c = s.peek();  
   }  
   if (c != '[') {  
      NTL_INPUT_ERROR(s, "bad vector input");  
   }  
 
   n = 0;   
   ibuf.SetLength(0);  
      
   s.get();  
   c = s.peek();  
   while (IsWhiteSpace(c)) {  
      s.get();  
      c = s.peek();  
   }  
   while (c != ']' && c != EOF) {   
      if (n % NTL_WordVectorInputBlock == 0) ibuf.SetMaxLength(n + NTL_WordVectorInputBlock); 
      n++;   
      ibuf.SetLength(n);   
      if (!(s >> ibuf[n-1])) NTL_INPUT_ERROR(s, "bad vector input");   
      c = s.peek();  
      while (IsWhiteSpace(c)) {  
         s.get();  
         c = s.peek();  
      }  
   }   
   if (c == EOF) NTL_INPUT_ERROR(s, "bad vector input");  
   s.get(); 
   
   a = ibuf; 
   return s;   
}    
   
   
ostream& operator<<(ostream& s, const WordVector& a)   
{   
   long i, n;   
  
   n = a.length();  
   
   s << '[';   
   
   for (i = 0; i < n; i++) {   
      s << a[i];   
      if (i < n-1) s << " ";   
   }   
   
   s << ']';   
      
   return s;   
}   

long operator==(const WordVector& a, const WordVector& b) 
{  
   long n = a.length();  
   if (b.length() != n) return 0;  
   const _ntl_ulong* ap = a.elts(); 
   const _ntl_ulong* bp = b.elts(); 
   long i;  
   for (i = 0; i < n; i++) if (ap[i] != bp[i]) return 0;  
   return 1;  
} 

long operator!=(const WordVector& a, const WordVector& b) 
{  return !(a == b); }

   



long InnerProduct(const WordVector& a, const WordVector& b)
{
   long n = min(a.length(), b.length());
   const _ntl_ulong *ap = a.elts();
   const _ntl_ulong *bp = b.elts();

   _ntl_ulong acc;
   long i;

   acc = 0;
   for (i = 0; i < n; i++)
      acc ^= ap[i] & bp[i];

#if (NTL_BITS_PER_LONG == 32)
   acc ^= acc >> 16;
   acc ^= acc >> 8;
   acc ^= acc >> 4;
   acc ^= acc >> 2;
   acc ^= acc >> 1;
   acc &= 1;
#elif (NTL_BITS_PER_LONG == 64)
   acc ^= acc >> 32;
   acc ^= acc >> 16;
   acc ^= acc >> 8;
   acc ^= acc >> 4;
   acc ^= acc >> 2;
   acc ^= acc >> 1;
   acc &= 1;
#else
   _ntl_ulong t = acc;
   while (t) {
      t = t >> 8;
      acc ^= t;
   }

   acc ^= acc >> 4;
   acc ^= acc >> 2;
   acc ^= acc >> 1;
   acc &= 1;
#endif

   return long(acc);
}


void ShiftAdd(_ntl_ulong *cp, const _ntl_ulong* ap, long sa, long n)
// c = c + (a << n)
{
   if (sa == 0) return;

   long i;

   long wn = n/NTL_BITS_PER_LONG;
   long bn = n - wn*NTL_BITS_PER_LONG;

   if (bn == 0) {
      for (i = sa+wn-1; i >= wn; i--)
         cp[i] ^= ap[i-wn];
   }
   else {
      _ntl_ulong t = ap[sa-1] >> (NTL_BITS_PER_LONG-bn);
      if (t) cp[sa+wn] ^= t;
      for (i = sa+wn-1; i >= wn+1; i--)
         cp[i] ^= (ap[i-wn] << bn) | (ap[i-wn-1] >> (NTL_BITS_PER_LONG-bn));
      cp[wn] ^= ap[0] << bn;
   }
}

long WV_BlockConstructAlloc(WordVector& x, long d, long n)
{
   long nwords, nbytes, AllocAmt, m, j; 
   _ntl_ulong *p, *q;


   /* check n value */

   if (n <= 0)
      LogicError("block construct: n must be positive");

   /* check d value */

   if (d <= 0) 
      LogicError("block construct: d must be positive");

   if (NTL_OVERFLOW(d, NTL_BITS_PER_LONG, 0) || 
       NTL_OVERFLOW(d, sizeof(_ntl_ulong), 2*sizeof(_ntl_ulong)))
      ResourceError("block construct: d too large");

   nwords = d + 2;
   nbytes = nwords*sizeof(_ntl_ulong);
   
   AllocAmt = (NTL_MAX_ALLOC_BLOCK - sizeof(_ntl_ulong)) / nbytes;
   if (AllocAmt == 0) AllocAmt = 1;

   if (AllocAmt < n)
      m = AllocAmt;
   else
      m = n;

   p = (_ntl_ulong *) NTL_SNS_MALLOC(m, nbytes, sizeof(_ntl_ulong));
   if (!p) MemoryError();

   *p = m;

   q = p+3;
   x.rep = q;
   
   for (j = 0; j < m; j++) {
      q[-2] = (d << 1) | 1;
      q[-1] = 0;
      q += nwords;
   }

   return m;
}

void WV_BlockConstructSet(WordVector& x, WordVector& y, long i)
{
   long d, size;
 
   d = x.rep[-2] >> 1;
   size = d + 2;
 
   y.rep = x.rep + i*size;
}

long WV_BlockDestroy(WordVector& x)
{
   long m;
   _ntl_ulong *p;
 
   p = x.rep - 3;
   m = (long) *p;
   free(p);
   return m;
}

long WV_storage(long d)
{
   return (d + 2)*sizeof(_ntl_ulong) + sizeof(WordVector);
}




NTL_END_IMPL




#include <NTL/ZZ_pEX.h>
#include <NTL/vec_vec_ZZ_p.h>
#include <NTL/ZZX.h>


NTL_START_IMPL


const ZZ_pEX& ZZ_pEX::zero()
{
   static const ZZ_pEX z; // GLOBAL (assumes C++11 thread-safe init)
   return z;
}


istream& operator>>(istream& s, ZZ_pEX& x)
{
   NTL_INPUT_CHECK_RET(s, s >> x.rep);
   x.normalize();
   return s;
}

ostream& operator<<(ostream& s, const ZZ_pEX& a)
{
   return s << a.rep;
}


void ZZ_pEX::normalize()
{
   long n;
   const ZZ_pE* p;

   n = rep.length();
   if (n == 0) return;
   p = rep.elts() + n;
   while (n > 0 && IsZero(*--p)) {
      n--;
   }
   rep.SetLength(n);
}


long IsZero(const ZZ_pEX& a)
{
   return a.rep.length() == 0;
}


long IsOne(const ZZ_pEX& a)
{
    return a.rep.length() == 1 && IsOne(a.rep[0]);
}

long operator==(const ZZ_pEX& a, long b)
{
   if (b == 0)
      return IsZero(a);

   if (b == 1)
      return IsOne(a);

   long da = deg(a);

   if (da > 0) return 0;

   NTL_ZZ_pRegister(bb);
   bb = b;

   if (da < 0)
      return IsZero(bb);

   return a.rep[0] == bb;
}

long operator==(const ZZ_pEX& a, const ZZ_p& b)
{
   if (IsZero(b))
      return IsZero(a);

   long da = deg(a);

   if (da != 0)
      return 0;

   return a.rep[0] == b;
}

long operator==(const ZZ_pEX& a, const ZZ_pE& b)
{
   if (IsZero(b))
      return IsZero(a);

   long da = deg(a);

   if (da != 0)
      return 0;

   return a.rep[0] == b;
}




void SetCoeff(ZZ_pEX& x, long i, const ZZ_pE& a)
{
   long j, m;

   if (i < 0) 
      LogicError("SetCoeff: negative index");

   if (NTL_OVERFLOW(i, 1, 0))
      ResourceError("overflow in SetCoeff");

   m = deg(x);


   if (i > m && IsZero(a)) return; 

   if (i > m) {
      /* careful: a may alias a coefficient of x */

      long alloc = x.rep.allocated();

      if (alloc > 0 && i >= alloc) {
         ZZ_pE aa = a;
         x.rep.SetLength(i+1);
         x.rep[i] = aa;
      }
      else {
         x.rep.SetLength(i+1);
         x.rep[i] = a;
      }

      for (j = m+1; j < i; j++)
         clear(x.rep[j]);
   }
   else
      x.rep[i] = a;

   x.normalize();
}

void SetCoeff(ZZ_pEX& x, long i, const ZZ_p& aa)
{
   long j, m;

   if (i < 0)
      LogicError("SetCoeff: negative index");

   if (NTL_OVERFLOW(i, 1, 0))
      ResourceError("overflow in SetCoeff");

   NTL_ZZ_pRegister(a);  // watch out for aliases!
   a = aa;

   m = deg(x);

   if (i > m && IsZero(a)) return; 

   if (i > m) {
      x.rep.SetLength(i+1);
      for (j = m+1; j < i; j++)
         clear(x.rep[j]);
   }
   x.rep[i] = a;
   x.normalize();
}

void SetCoeff(ZZ_pEX& x, long i, long a)
{
   if (a == 1)
      SetCoeff(x, i);
   else {
      NTL_ZZ_pRegister(T);
      T = a;
      SetCoeff(x, i, T);
   }
}



void SetCoeff(ZZ_pEX& x, long i)
{
   long j, m;

   if (i < 0) 
      LogicError("coefficient index out of range");

   if (NTL_OVERFLOW(i, 1, 0))
      ResourceError("overflow in SetCoeff");

   m = deg(x);

   if (i > m) {
      x.rep.SetLength(i+1);
      for (j = m+1; j < i; j++)
         clear(x.rep[j]);
   }
   set(x.rep[i]);
   x.normalize();
}


void SetX(ZZ_pEX& x)
{
   clear(x);
   SetCoeff(x, 1);
}


long IsX(const ZZ_pEX& a)
{
   return deg(a) == 1 && IsOne(LeadCoeff(a)) && IsZero(ConstTerm(a));
}
      
      

const ZZ_pE& coeff(const ZZ_pEX& a, long i)
{
   if (i < 0 || i > deg(a))
      return ZZ_pE::zero();
   else
      return a.rep[i];
}


const ZZ_pE& LeadCoeff(const ZZ_pEX& a)
{
   if (IsZero(a))
      return ZZ_pE::zero();
   else
      return a.rep[deg(a)];
}

const ZZ_pE& ConstTerm(const ZZ_pEX& a)
{
   if (IsZero(a))
      return ZZ_pE::zero();
   else
      return a.rep[0];
}



void conv(ZZ_pEX& x, const ZZ_pE& a)
{
   if (IsZero(a))
      x.rep.SetLength(0);
   else {
      x.rep.SetLength(1);
      x.rep[0] = a;
   }
}

void conv(ZZ_pEX& x, long a)
{
   if (a == 0) 
      clear(x);
   else if (a == 1)
      set(x);
   else {
      NTL_ZZ_pRegister(T);
      T = a;
      conv(x, T);
   }
}

void conv(ZZ_pEX& x, const ZZ& a)
{
   NTL_ZZ_pRegister(T);
   conv(T, a);
   conv(x, T);
}

void conv(ZZ_pEX& x, const ZZ_p& a)
{
   if (IsZero(a)) 
      clear(x);
   else if (IsOne(a))
      set(x);
   else {
      x.rep.SetLength(1);
      conv(x.rep[0], a);
      x.normalize();
   }
}

void conv(ZZ_pEX& x, const ZZ_pX& aa)
{
   ZZ_pX a = aa; // in case a aliases the rep of a coefficient of x

   long n = deg(a)+1;
   long i;

   x.rep.SetLength(n);
   for (i = 0; i < n; i++)
      conv(x.rep[i], coeff(a, i));
}


void conv(ZZ_pEX& x, const vec_ZZ_pE& a)
{
   x.rep = a;
   x.normalize();
}



/* additional legacy conversions for v6 conversion regime */

void conv(ZZ_pEX& x, const ZZX& a)
{
   long n = a.rep.length();
   long i;

   x.rep.SetLength(n);
   for (i = 0; i < n; i++)
      conv(x.rep[i], a.rep[i]);

   x.normalize();
}


/* ------------------------------------- */


void add(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const ZZ_pE *ap, *bp; 
   ZZ_pE* xp;

   for (i = minab+1, ap = a.rep.elts(), bp = b.rep.elts(), xp = x.rep.elts();
        i; i--, ap++, bp++, xp++)
      add(*xp, (*ap), (*bp));

   if (da > minab && &x != &a)
      for (i = da-minab; i; i--, xp++, ap++)
         *xp = *ap;
   else if (db > minab && &x != &b)
      for (i = db-minab; i; i--, xp++, bp++)
         *xp = *bp;
   else
      x.normalize();
}


void add(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pE& b)
{
   long n = a.rep.length();
   if (n == 0) {
      conv(x, b);
   }
   else if (&x == &a) {
      add(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else if (x.rep.MaxLength() == 0) {
      x = a;
      add(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else {
      // ugly...b could alias a coeff of x

      ZZ_pE *xp = x.rep.elts();
      add(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ_pE *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}

void add(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_p& b)
{
   long n = a.rep.length();
   if (n == 0) {
      conv(x, b);
   }
   else if (&x == &a) {
      add(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else if (x.rep.MaxLength() == 0) {
      x = a;
      add(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else {
      // ugly...b could alias a coeff of x

      ZZ_pE *xp = x.rep.elts();
      add(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ_pE *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}


void add(ZZ_pEX& x, const ZZ_pEX& a, long b)
{
   if (a.rep.length() == 0) {
      conv(x, b);
   }
   else {
      if (&x != &a) x = a;
      add(x.rep[0], x.rep[0], b);
      x.normalize();
   }
}


void sub(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const ZZ_pE *ap, *bp; 
   ZZ_pE* xp;

   for (i = minab+1, ap = a.rep.elts(), bp = b.rep.elts(), xp = x.rep.elts();
        i; i--, ap++, bp++, xp++)
      sub(*xp, (*ap), (*bp));

   if (da > minab && &x != &a)
      for (i = da-minab; i; i--, xp++, ap++)
         *xp = *ap;
   else if (db > minab)
      for (i = db-minab; i; i--, xp++, bp++)
         negate(*xp, *bp);
   else
      x.normalize();
}


void sub(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pE& b)
{
   long n = a.rep.length();
   if (n == 0) {
      conv(x, b);
      negate(x, x);
   }
   else if (&x == &a) {
      sub(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else if (x.rep.MaxLength() == 0) {
      x = a;
      sub(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else {
      // ugly...b could alias a coeff of x

      ZZ_pE *xp = x.rep.elts();
      sub(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ_pE *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}

void sub(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_p& b)
{
   long n = a.rep.length();
   if (n == 0) {
      conv(x, b);
      negate(x, x);
   }
   else if (&x == &a) {
      sub(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else if (x.rep.MaxLength() == 0) {
      x = a;
      sub(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else {
      // ugly...b could alias a coeff of x

      ZZ_pE *xp = x.rep.elts();
      sub(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ_pE *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}


void sub(ZZ_pEX& x, const ZZ_pEX& a, long b)
{
   if (a.rep.length() == 0) {
      conv(x, b);
      negate(x, x);
   }
   else {
      if (&x != &a) x = a;
      sub(x.rep[0], x.rep[0], b);
      x.normalize();
   }
}

void sub(ZZ_pEX& x, const ZZ_pE& b, const ZZ_pEX& a)
{
   long n = a.rep.length();
   if (n == 0) {
      conv(x, b);
   }
   else if (x.rep.MaxLength() == 0) {
      negate(x, a);
      add(x.rep[0], x.rep[0], b);
      x.normalize();
   }
   else {
      // ugly...b could alias a coeff of x

      ZZ_pE *xp = x.rep.elts();
      sub(xp[0], b, a.rep[0]);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ_pE *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         negate(xp[i], ap[i]);
      x.normalize();
   }
}


void sub(ZZ_pEX& x, const ZZ_p& a, const ZZ_pEX& b)
{
   NTL_ZZ_pRegister(T);   // avoids aliasing problems
   T = a;
   negate(x, b);
   add(x, x, T);
}

void sub(ZZ_pEX& x, long a, const ZZ_pEX& b)
{
   NTL_ZZ_pRegister(T); 
   T = a;
   negate(x, b);
   add(x, x, T);
}

void mul(ZZ_pEX& c, const ZZ_pEX& a, const ZZ_pEX& b)
{
   if (&a == &b) {
      sqr(c, a);
      return;
   }

   if (IsZero(a) || IsZero(b)) {
      clear(c);
      return;
   }

   if (deg(a) == 0) {
      mul(c, b, ConstTerm(a));
      return;
   } 

   if (deg(b) == 0) {
      mul(c, a, ConstTerm(b));
      return;
   }

   // general case...Kronecker subst

   ZZ_pX A, B, C;

   long da = deg(a);
   long db = deg(b);

   long n = ZZ_pE::degree();
   long n2 = 2*n-1;

   if (NTL_OVERFLOW(da+db+1, n2, 0))
      ResourceError("overflow in ZZ_pEX mul");

   long i, j;

   A.rep.SetLength((da+1)*n2);

   for (i = 0; i <= da; i++) {
      const ZZ_pX& coeff = rep(a.rep[i]);
      long dcoeff = deg(coeff);
      for (j = 0; j <= dcoeff; j++)
         A.rep[n2*i + j] = coeff.rep[j]; 
   }

   A.normalize();

   B.rep.SetLength((db+1)*n2);

   for (i = 0; i <= db; i++) {
      const ZZ_pX& coeff = rep(b.rep[i]);
      long dcoeff = deg(coeff);
      for (j = 0; j <= dcoeff; j++)
         B.rep[n2*i + j] = coeff.rep[j]; 
   }

   B.normalize();

   mul(C, A, B);

   long Clen = C.rep.length();
   long lc = (Clen + n2 - 1)/n2;
   long dc = lc - 1;

   c.rep.SetLength(dc+1);

   ZZ_pX tmp;
   
   for (i = 0; i <= dc; i++) {
      tmp.rep.SetLength(n2);
      for (j = 0; j < n2 && n2*i + j < Clen; j++)
         tmp.rep[j] = C.rep[n2*i + j];
      for (; j < n2; j++)
         clear(tmp.rep[j]);
      tmp.normalize();
      conv(c.rep[i], tmp);
   }
  
   c.normalize();
}


void mul(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pE& b)
{
   if (IsZero(b)) {
      clear(x);
      return;
   }

   ZZ_pE t;
   t = b;

   long i, da;

   const ZZ_pE *ap;
   ZZ_pE* xp;

   da = deg(a);
   x.rep.SetLength(da+1);
   ap = a.rep.elts();
   xp = x.rep.elts();

   for (i = 0; i <= da; i++)
      mul(xp[i], ap[i], t);

   x.normalize();
}



void mul(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_p& b)
{
   if (IsZero(b)) {
      clear(x);
      return;
   }

   NTL_ZZ_pRegister(t);
   t = b;

   long i, da;

   const ZZ_pE *ap;
   ZZ_pE* xp;

   da = deg(a);
   x.rep.SetLength(da+1);
   ap = a.rep.elts();
   xp = x.rep.elts();

   for (i = 0; i <= da; i++)
      mul(xp[i], ap[i], t);

   x.normalize();
}


void mul(ZZ_pEX& x, const ZZ_pEX& a, long b)
{
   NTL_ZZ_pRegister(t);
   t = b;
   mul(x, a, t);
}

void sqr(ZZ_pEX& c, const ZZ_pEX& a)
{
   if (IsZero(a)) {
      clear(c);
      return;
   }

   if (deg(a) == 0) {
      ZZ_pE res;
      sqr(res, ConstTerm(a));
      conv(c, res);
      return;
   } 

   // general case...Kronecker subst

   ZZ_pX A, C;

   long da = deg(a);

   long n = ZZ_pE::degree();
   long n2 = 2*n-1;

   if (NTL_OVERFLOW(2*da+1, n2, 0))
      ResourceError("overflow in ZZ_pEX sqr");

   long i, j;

   A.rep.SetLength((da+1)*n2);

   for (i = 0; i <= da; i++) {
      const ZZ_pX& coeff = rep(a.rep[i]);
      long dcoeff = deg(coeff);
      for (j = 0; j <= dcoeff; j++)
         A.rep[n2*i + j] = coeff.rep[j]; 
   }

   A.normalize();

   sqr(C, A);

   long Clen = C.rep.length();
   long lc = (Clen + n2 - 1)/n2;
   long dc = lc - 1;

   c.rep.SetLength(dc+1);

   ZZ_pX tmp;
   
   for (i = 0; i <= dc; i++) {
      tmp.rep.SetLength(n2);
      for (j = 0; j < n2 && n2*i + j < Clen; j++)
         tmp.rep[j] = C.rep[n2*i + j];
      for (; j < n2; j++)
         clear(tmp.rep[j]);
      tmp.normalize();
      conv(c.rep[i], tmp);
   }
  
  
   c.normalize();
}


void MulTrunc(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b, long n)
{
   if (n < 0) LogicError("MulTrunc: bad args");

   ZZ_pEX t;
   mul(t, a, b);
   trunc(x, t, n);
}

void SqrTrunc(ZZ_pEX& x, const ZZ_pEX& a, long n)
{
   if (n < 0) LogicError("SqrTrunc: bad args");

   ZZ_pEX t;
   sqr(t, a);
   trunc(x, t, n);
}


void CopyReverse(ZZ_pEX& x, const ZZ_pEX& a, long hi)

   // x[0..hi] = reverse(a[0..hi]), with zero fill
   // input may not alias output

{
   long i, j, n, m;

   n = hi+1;
   m = a.rep.length();

   x.rep.SetLength(n);

   const ZZ_pE* ap = a.rep.elts();
   ZZ_pE* xp = x.rep.elts();

   for (i = 0; i < n; i++) {
      j = hi-i;
      if (j < 0 || j >= m)
         clear(xp[i]);
      else
         xp[i] = ap[j];
   }

   x.normalize();
} 


void trunc(ZZ_pEX& x, const ZZ_pEX& a, long m)

// x = a % X^m, output may alias input 

{
   if (m < 0) LogicError("trunc: bad args");

   if (&x == &a) {
      if (x.rep.length() > m) {
         x.rep.SetLength(m);
         x.normalize();
      }
   }
   else {
      long n;
      long i;
      ZZ_pE* xp;
      const ZZ_pE* ap;

      n = min(a.rep.length(), m);
      x.rep.SetLength(n);

      xp = x.rep.elts();
      ap = a.rep.elts();

      for (i = 0; i < n; i++) xp[i] = ap[i];

      x.normalize();
   }
}


void random(ZZ_pEX& x, long n)
{
   long i;

   x.rep.SetLength(n);

   for (i = 0; i < n; i++)
      random(x.rep[i]);

   x.normalize();
}

void negate(ZZ_pEX& x, const ZZ_pEX& a)
{
   long n = a.rep.length();
   x.rep.SetLength(n);

   const ZZ_pE* ap = a.rep.elts();
   ZZ_pE* xp = x.rep.elts();
   long i;

   for (i = n; i; i--, ap++, xp++)
      negate((*xp), (*ap));
}



static
void MulByXModAux(ZZ_pEX& h, const ZZ_pEX& a, const ZZ_pEX& f)
{
   long i, n, m;
   ZZ_pE* hh;
   const ZZ_pE *aa, *ff;

   ZZ_pE t, z;

   n = deg(f);
   m = deg(a);

   if (m >= n || n == 0) LogicError("MulByXMod: bad args");

   if (m < 0) {
      clear(h);
      return;
   }

   if (m < n-1) {
      h.rep.SetLength(m+2);
      hh = h.rep.elts();
      aa = a.rep.elts();
      for (i = m+1; i >= 1; i--)
         hh[i] = aa[i-1];
      clear(hh[0]);
   }
   else {
      h.rep.SetLength(n);
      hh = h.rep.elts();
      aa = a.rep.elts();
      ff = f.rep.elts();
      negate(z, aa[n-1]);
      if (!IsOne(ff[n]))
         div(z, z, ff[n]);
      for (i = n-1; i >= 1; i--) {
         mul(t, z, ff[i]);
         add(hh[i], aa[i-1], t);
      }
      mul(hh[0], z, ff[0]);
      h.normalize();
   }
}

void MulByXMod(ZZ_pEX& h, const ZZ_pEX& a, const ZZ_pEX& f)
{
   if (&h == &f) {
      ZZ_pEX hh;
      MulByXModAux(hh, a, f);
      h = hh;
   }
   else
      MulByXModAux(h, a, f);
}



void PlainMul(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long da = deg(a);
   long db = deg(b);

   if (da < 0 || db < 0) {
      clear(x);
      return;
   }

   long d = da+db;



   const ZZ_pE *ap, *bp;
   ZZ_pE *xp;
   
   ZZ_pEX la, lb;

   if (&x == &a) {
      la = a;
      ap = la.rep.elts();
   }
   else
      ap = a.rep.elts();

   if (&x == &b) {
      lb = b;
      bp = lb.rep.elts();
   }
   else
      bp = b.rep.elts();

   x.rep.SetLength(d+1);

   xp = x.rep.elts();

   long i, j, jmin, jmax;
   ZZ_pX t, accum;

   for (i = 0; i <= d; i++) {
      jmin = max(0, i-db);
      jmax = min(da, i);
      clear(accum);
      for (j = jmin; j <= jmax; j++) {
         mul(t, rep(ap[j]), rep(bp[i-j]));
         add(accum, accum, t);
      }
      conv(xp[i], accum);
   }
   x.normalize();
}

void SetSize(vec_ZZ_pX& x, long n, long m)
{
   x.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      x[i].rep.SetMaxLength(m);
}



void PlainDivRem(ZZ_pEX& q, ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_pE *bp;
   ZZ_pE *qp;
   ZZ_pX *xp;


   ZZ_pE LCInv, t;
   ZZ_pX s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pEX: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   ZZ_pEX lb;

   if (&q == &b) {
      lb = b;
      bp = lb.rep.elts();
   }
   else
      bp = b.rep.elts();

   if (IsOne(bp[db]))
      LCIsOne = 1;
   else {
      LCIsOne = 0;
      inv(LCInv, bp[db]);
   }

   vec_ZZ_pX x;

   SetSize(x, da+1, 2*ZZ_pE::degree());

   for (i = 0; i <= da; i++) 
      x[i] = rep(a.rep[i]);

   xp = x.elts();

   dq = da - db;
   q.rep.SetLength(dq+1);
   qp = q.rep.elts();

   for (i = dq; i >= 0; i--) {
      conv(t, xp[i+db]);
      if (!LCIsOne)
         mul(t, t, LCInv);
      qp[i] = t;
      negate(t, t);

      for (j = db-1; j >= 0; j--) {
         mul(s, rep(t), rep(bp[j]));
         add(xp[i+j], xp[i+j], s);
      }
   }

   r.rep.SetLength(db);
   for (i = 0; i < db; i++)
      conv(r.rep[i], xp[i]);
   r.normalize();
}


void PlainRem(ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b, vec_ZZ_pX& x)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_pE *bp;
   ZZ_pX *xp;


   ZZ_pE LCInv, t;
   ZZ_pX s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pEX: division by zero");

   if (da < db) {
      r = a;
      return;
   }

   bp = b.rep.elts();

   if (IsOne(bp[db]))
      LCIsOne = 1;
   else {
      LCIsOne = 0;
      inv(LCInv, bp[db]);
   }

   for (i = 0; i <= da; i++)
      x[i] = rep(a.rep[i]);

   xp = x.elts();

   dq = da - db;

   for (i = dq; i >= 0; i--) {
      conv(t, xp[i+db]);
      if (!LCIsOne)
         mul(t, t, LCInv);
      negate(t, t);

      for (j = db-1; j >= 0; j--) {
         mul(s, rep(t), rep(bp[j]));
         add(xp[i+j], xp[i+j], s);
      }
   }

   r.rep.SetLength(db);
   for (i = 0; i < db; i++)
      conv(r.rep[i], xp[i]);
   r.normalize();
}


void PlainDivRem(ZZ_pEX& q, ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b, 
     vec_ZZ_pX& x)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_pE *bp;
   ZZ_pE *qp;
   ZZ_pX *xp;


   ZZ_pE LCInv, t;
   ZZ_pX s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pEX: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   ZZ_pEX lb;

   if (&q == &b) {
      lb = b;
      bp = lb.rep.elts();
   }
   else
      bp = b.rep.elts();

   if (IsOne(bp[db]))
      LCIsOne = 1;
   else {
      LCIsOne = 0;
      inv(LCInv, bp[db]);
   }

   for (i = 0; i <= da; i++)
      x[i] = rep(a.rep[i]);

   xp = x.elts();

   dq = da - db;
   q.rep.SetLength(dq+1);
   qp = q.rep.elts();

   for (i = dq; i >= 0; i--) {
      conv(t, xp[i+db]);
      if (!LCIsOne)
         mul(t, t, LCInv);
      qp[i] = t;
      negate(t, t);

      for (j = db-1; j >= 0; j--) {
         mul(s, rep(t), rep(bp[j]));
         add(xp[i+j], xp[i+j], s);
      }
   }

   r.rep.SetLength(db);
   for (i = 0; i < db; i++)
      conv(r.rep[i], xp[i]);
   r.normalize();
}


void PlainDiv(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_pE *bp;
   ZZ_pE *qp;
   ZZ_pX *xp;


   ZZ_pE LCInv, t;
   ZZ_pX s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pEX: division by zero");

   if (da < db) {
      clear(q);
      return;
   }

   ZZ_pEX lb;

   if (&q == &b) {
      lb = b;
      bp = lb.rep.elts();
   }
   else
      bp = b.rep.elts();

   if (IsOne(bp[db]))
      LCIsOne = 1;
   else {
      LCIsOne = 0;
      inv(LCInv, bp[db]);
   }

   vec_ZZ_pX x;
   SetSize(x, da+1-db, 2*ZZ_pE::degree());

   for (i = db; i <= da; i++)
      x[i-db] = rep(a.rep[i]);

   xp = x.elts();

   dq = da - db;
   q.rep.SetLength(dq+1);
   qp = q.rep.elts();

   for (i = dq; i >= 0; i--) {
      conv(t, xp[i]);
      if (!LCIsOne)
         mul(t, t, LCInv);
      qp[i] = t;
      negate(t, t);

      long lastj = max(0, db-i);

      for (j = db-1; j >= lastj; j--) {
         mul(s, rep(t), rep(bp[j]));
         add(xp[i+j-db], xp[i+j-db], s);
      }
   }
}

void PlainRem(ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_pE *bp;
   ZZ_pX *xp;


   ZZ_pE LCInv, t;
   ZZ_pX s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pEX: division by zero");

   if (da < db) {
      r = a;
      return;
   }

   bp = b.rep.elts();

   if (IsOne(bp[db]))
      LCIsOne = 1;
   else {
      LCIsOne = 0;
      inv(LCInv, bp[db]);
   }

   vec_ZZ_pX x;
   SetSize(x, da + 1, 2*ZZ_pE::degree());

   for (i = 0; i <= da; i++)
      x[i] = rep(a.rep[i]);

   xp = x.elts();

   dq = da - db;

   for (i = dq; i >= 0; i--) {
      conv(t, xp[i+db]);
      if (!LCIsOne)
         mul(t, t, LCInv);
      negate(t, t);

      for (j = db-1; j >= 0; j--) {
         mul(s, rep(t), rep(bp[j]));
         add(xp[i+j], xp[i+j], s);
      }
   }

   r.rep.SetLength(db);
   for (i = 0; i < db; i++)
      conv(r.rep[i], xp[i]);
   r.normalize();
}



void RightShift(ZZ_pEX& x, const ZZ_pEX& a, long n)
{
   if (IsZero(a)) {
      clear(x);
      return;
   }

   if (n < 0) {
      if (n < -NTL_MAX_LONG) ResourceError("overflow in RightShift");
      LeftShift(x, a, -n);
      return;
   }

   long da = deg(a);
   long i;
 
   if (da < n) {
      clear(x);
      return;
   }

   if (&x != &a)
      x.rep.SetLength(da-n+1);

   for (i = 0; i <= da-n; i++)
      x.rep[i] = a.rep[i+n];

   if (&x == &a)
      x.rep.SetLength(da-n+1);

   x.normalize();
}

void LeftShift(ZZ_pEX& x, const ZZ_pEX& a, long n)
{
   if (IsZero(a)) {
      clear(x);
      return;
   }

   if (n < 0) {
      if (n < -NTL_MAX_LONG) 
         clear(x);
      else
         RightShift(x, a, -n);
      return;
   }

   if (NTL_OVERFLOW(n, 1, 0))
      ResourceError("overflow in LeftShift");

   long m = a.rep.length();

   x.rep.SetLength(m+n);

   long i;
   for (i = m-1; i >= 0; i--)
      x.rep[i+n] = a.rep[i];

   for (i = 0; i < n; i++)
      clear(x.rep[i]);
}



void NewtonInv(ZZ_pEX& c, const ZZ_pEX& a, long e)
{
   ZZ_pE x;

   inv(x, ConstTerm(a));

   if (e == 1) {
      conv(c, x);
      return;
   }

   vec_long E;
   E.SetLength(0);
   append(E, e);
   while (e > 1) {
      e = (e+1)/2;
      append(E, e);
   }

   long L = E.length();

   ZZ_pEX g, g0, g1, g2;


   g.rep.SetMaxLength(E[0]);
   g0.rep.SetMaxLength(E[0]);
   g1.rep.SetMaxLength((3*E[0]+1)/2);
   g2.rep.SetMaxLength(E[0]);

   conv(g, x);

   long i;

   for (i = L-1; i > 0; i--) {
      // lift from E[i] to E[i-1]

      long k = E[i];
      long l = E[i-1]-E[i];

      trunc(g0, a, k+l);

      mul(g1, g0, g);
      RightShift(g1, g1, k);
      trunc(g1, g1, l);

      mul(g2, g1, g);
      trunc(g2, g2, l);
      LeftShift(g2, g2, k);

      sub(g, g, g2);
   }

   c = g;
}

void InvTrunc(ZZ_pEX& c, const ZZ_pEX& a, long e)
{
   if (e < 0) LogicError("InvTrunc: bad args");
   if (e == 0) {
      clear(c);
      return;
   }

   if (NTL_OVERFLOW(e, 1, 0))
      ResourceError("overflow in InvTrunc");

   NewtonInv(c, a, e);
}




const long ZZ_pEX_MOD_PLAIN = 0;
const long ZZ_pEX_MOD_MUL = 1;


void build(ZZ_pEXModulus& F, const ZZ_pEX& f)
{
   long n = deg(f);

   if (n <= 0) LogicError("build(ZZ_pEXModulus,ZZ_pEX): deg(f) <= 0");

   if (NTL_OVERFLOW(n, ZZ_pE::degree(), 0))
      ResourceError("build(ZZ_pEXModulus,ZZ_pEX): overflow");
   

   F.tracevec.make();

   F.f = f;
   F.n = n;

   if (F.n < ZZ_pE::ModCross()) {
      F.method = ZZ_pEX_MOD_PLAIN;
   }
   else {
      F.method = ZZ_pEX_MOD_MUL;
      ZZ_pEX P1;
      ZZ_pEX P2;

      CopyReverse(P1, f, n);
      InvTrunc(P2, P1, n-1);
      CopyReverse(P1, P2, n-2);
      trunc(F.h0, P1, n-2);
      trunc(F.f0, f, n);
      F.hlc = ConstTerm(P2);
   }
}



ZZ_pEXModulus::ZZ_pEXModulus()
{
   n = -1;
   method = ZZ_pEX_MOD_PLAIN;
}


ZZ_pEXModulus::~ZZ_pEXModulus() 
{ 
}



ZZ_pEXModulus::ZZ_pEXModulus(const ZZ_pEX& ff)
{
   n = -1;
   method = ZZ_pEX_MOD_PLAIN;

   build(*this, ff);
}


void UseMulRem21(ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   ZZ_pEX P1;
   ZZ_pEX P2;

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   if (!IsOne(F.hlc)) mul(P1, P1, F.hlc);
   add(P2, P2, P1);
   mul(P1, P2, F.f0);
   trunc(P1, P1, F.n);
   trunc(r, a, F.n);
   sub(r, r, P1);
}

void UseMulDivRem21(ZZ_pEX& q, ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   ZZ_pEX P1;
   ZZ_pEX P2;

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   if (!IsOne(F.hlc)) mul(P1, P1, F.hlc);
   add(P2, P2, P1);
   mul(P1, P2, F.f0);
   trunc(P1, P1, F.n);
   trunc(r, a, F.n);
   sub(r, r, P1);
   q = P2;
}

void UseMulDiv21(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   ZZ_pEX P1;
   ZZ_pEX P2;

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   if (!IsOne(F.hlc)) mul(P1, P1, F.hlc);
   add(P2, P2, P1);
   q = P2;

}


void rem(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   if (F.method == ZZ_pEX_MOD_PLAIN) {
      PlainRem(x, a, F.f);
      return;
   }

   long da = deg(a);
   long n = F.n;

   if (da <= 2*n-2) {
      UseMulRem21(x, a, F);
      return;
   }

   ZZ_pEX buf(INIT_SIZE, 2*n-1);

   long a_len = da+1;

   while (a_len > 0) {
      long old_buf_len = buf.rep.length();
      long amt = min(2*n-1-old_buf_len, a_len);

      buf.rep.SetLength(old_buf_len+amt);

      long i;

      for (i = old_buf_len+amt-1; i >= amt; i--)
         buf.rep[i] = buf.rep[i-amt];

      for (i = amt-1; i >= 0; i--)
         buf.rep[i] = a.rep[a_len-amt+i];

      buf.normalize();

      UseMulRem21(buf, buf, F);

      a_len -= amt;
   }

   x = buf;
}

void DivRem(ZZ_pEX& q, ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   if (F.method == ZZ_pEX_MOD_PLAIN) {
      PlainDivRem(q, r, a, F.f);
      return;
   }

   long da = deg(a);
   long n = F.n;

   if (da <= 2*n-2) {
      UseMulDivRem21(q, r, a, F);
      return;
   }

   ZZ_pEX buf(INIT_SIZE, 2*n-1);
   ZZ_pEX qbuf(INIT_SIZE, n-1);

   ZZ_pEX qq;
   qq.rep.SetLength(da-n+1);

   long a_len = da+1;
   long q_hi = da-n+1;

   while (a_len > 0) {
      long old_buf_len = buf.rep.length();
      long amt = min(2*n-1-old_buf_len, a_len);

      buf.rep.SetLength(old_buf_len+amt);

      long i;

      for (i = old_buf_len+amt-1; i >= amt; i--)
         buf.rep[i] = buf.rep[i-amt];

      for (i = amt-1; i >= 0; i--)
         buf.rep[i] = a.rep[a_len-amt+i];

      buf.normalize();

      UseMulDivRem21(qbuf, buf, buf, F);
      long dl = qbuf.rep.length();
      a_len = a_len - amt;
      for(i = 0; i < dl; i++)
         qq.rep[a_len+i] = qbuf.rep[i];
      for(i = dl+a_len; i < q_hi; i++)
         clear(qq.rep[i]);
      q_hi = a_len;
   }

   r = buf;

   qq.normalize();
   q = qq;
}

void div(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   if (F.method == ZZ_pEX_MOD_PLAIN) {
      PlainDiv(q, a, F.f);
      return;
   }

   long da = deg(a);
   long n = F.n;

   if (da <= 2*n-2) {
      UseMulDiv21(q, a, F);
      return;
   }

   ZZ_pEX buf(INIT_SIZE, 2*n-1);
   ZZ_pEX qbuf(INIT_SIZE, n-1);

   ZZ_pEX qq;
   qq.rep.SetLength(da-n+1);

   long a_len = da+1;
   long q_hi = da-n+1;

   while (a_len > 0) {
      long old_buf_len = buf.rep.length();
      long amt = min(2*n-1-old_buf_len, a_len);

      buf.rep.SetLength(old_buf_len+amt);

      long i;

      for (i = old_buf_len+amt-1; i >= amt; i--)
         buf.rep[i] = buf.rep[i-amt];

      for (i = amt-1; i >= 0; i--)
         buf.rep[i] = a.rep[a_len-amt+i];

      buf.normalize();

      a_len = a_len - amt;
      if (a_len > 0)
         UseMulDivRem21(qbuf, buf, buf, F);
      else
         UseMulDiv21(qbuf, buf, F);

      long dl = qbuf.rep.length();
      for(i = 0; i < dl; i++)
         qq.rep[a_len+i] = qbuf.rep[i];
      for(i = dl+a_len; i < q_hi; i++)
         clear(qq.rep[i]);
      q_hi = a_len;
   }

   qq.normalize();
   q = qq;
}




void MulMod(ZZ_pEX& c, const ZZ_pEX& a, const ZZ_pEX& b, const ZZ_pEXModulus& F)
{
   if (deg(a) >= F.n || deg(b) >= F.n) LogicError("MulMod: bad args");

   ZZ_pEX t;
   mul(t, a, b);
   rem(c, t, F);
}


void SqrMod(ZZ_pEX& c, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   if (deg(a) >= F.n) LogicError("MulMod: bad args");

   ZZ_pEX t;
   sqr(t, a);
   rem(c, t, F);
}



void UseMulRem(ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b)
{
   ZZ_pEX P1;
   ZZ_pEX P2;

   long da = deg(a);
   long db = deg(b);

   CopyReverse(P1, b, db);
   InvTrunc(P2, P1, da-db+1);
   CopyReverse(P1, P2, da-db);

   RightShift(P2, a, db);
   mul(P2, P1, P2);
   RightShift(P2, P2, da-db);
   mul(P1, P2, b);
   sub(P1, a, P1);
   
   r = P1;
}

void UseMulDivRem(ZZ_pEX& q, ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b)
{
   ZZ_pEX P1;
   ZZ_pEX P2;

   long da = deg(a);
   long db = deg(b);

   CopyReverse(P1, b, db);
   InvTrunc(P2, P1, da-db+1);
   CopyReverse(P1, P2, da-db);

   RightShift(P2, a, db);
   mul(P2, P1, P2);
   RightShift(P2, P2, da-db);
   mul(P1, P2, b);
   sub(P1, a, P1);
   
   r = P1;
   q = P2;
}

void UseMulDiv(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEX& b)
{
   ZZ_pEX P1;
   ZZ_pEX P2;

   long da = deg(a);
   long db = deg(b);

   CopyReverse(P1, b, db);
   InvTrunc(P2, P1, da-db+1);
   CopyReverse(P1, P2, da-db);

   RightShift(P2, a, db);
   mul(P2, P1, P2);
   RightShift(P2, P2, da-db);
   
   q = P2;
}



void DivRem(ZZ_pEX& q, ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sb < ZZ_pE::DivCross() || sa-sb < ZZ_pE::DivCross())
      PlainDivRem(q, r, a, b);
   else if (sa < 4*sb)
      UseMulDivRem(q, r, a, b);
   else {
      ZZ_pEXModulus B;
      build(B, b);
      DivRem(q, r, a, B);
   }
}

void div(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sb < ZZ_pE::DivCross() || sa-sb < ZZ_pE::DivCross())
      PlainDiv(q, a, b);
   else if (sa < 4*sb)
      UseMulDiv(q, a, b);
   else {
      ZZ_pEXModulus B;
      build(B, b);
      div(q, a, B);
   }
}

void div(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pE& b)
{
   ZZ_pE T;
   inv(T, b);
   mul(q, a, T);
}

void div(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_p& b)
{
   NTL_ZZ_pRegister(T);
   inv(T, b);
   mul(q, a, T);
}

void div(ZZ_pEX& q, const ZZ_pEX& a, long b)
{
   NTL_ZZ_pRegister(T);
   T = b;
   inv(T, T);
   mul(q, a, T);
}

void rem(ZZ_pEX& r, const ZZ_pEX& a, const ZZ_pEX& b)
{
   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sb < ZZ_pE::DivCross() || sa-sb < ZZ_pE::DivCross())
      PlainRem(r, a, b);
   else if (sa < 4*sb)
      UseMulRem(r, a, b);
   else {
      ZZ_pEXModulus B;
      build(B, b);
      rem(r, a, B);
   }
}

void PlainGCD(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b)
{
   ZZ_pE t;

   if (IsZero(b))
      x = a;
   else if (IsZero(a))
      x = b;
   else {
      long n = max(deg(a),deg(b)) + 1;
      ZZ_pEX u(INIT_SIZE, n), v(INIT_SIZE, n);

      vec_ZZ_pX tmp;
      SetSize(tmp, n, 2*ZZ_pE::degree());

      u = a;
      v = b;
      do {
         PlainRem(u, u, v, tmp);
         swap(u, v);
      } while (!IsZero(v));

      x = u;
   }

   if (IsZero(x)) return;
   if (IsOne(LeadCoeff(x))) return;

   /* make gcd monic */


   inv(t, LeadCoeff(x)); 
   mul(x, x, t); 
}

class _NTL_ZZ_pEXMatrix {
private:

   _NTL_ZZ_pEXMatrix(const _NTL_ZZ_pEXMatrix&);  // disable
   ZZ_pEX elts[2][2];

public:

   _NTL_ZZ_pEXMatrix() { }
   ~_NTL_ZZ_pEXMatrix() { }

   void operator=(const _NTL_ZZ_pEXMatrix&);
   ZZ_pEX& operator() (long i, long j) { return elts[i][j]; }
   const ZZ_pEX& operator() (long i, long j) const { return elts[i][j]; }
};


void _NTL_ZZ_pEXMatrix::operator=(const _NTL_ZZ_pEXMatrix& M)
{
   elts[0][0] = M.elts[0][0];
   elts[0][1] = M.elts[0][1];
   elts[1][0] = M.elts[1][0];
   elts[1][1] = M.elts[1][1];
}


static
void mul(ZZ_pEX& U, ZZ_pEX& V, const _NTL_ZZ_pEXMatrix& M)
// (U, V)^T = M*(U, V)^T
{
   ZZ_pEX t1, t2, t3;

   mul(t1, M(0,0), U);
   mul(t2, M(0,1), V);
   add(t3, t1, t2);
   mul(t1, M(1,0), U);
   mul(t2, M(1,1), V);
   add(V, t1, t2);
   U = t3;
}


static
void mul(_NTL_ZZ_pEXMatrix& A, _NTL_ZZ_pEXMatrix& B, _NTL_ZZ_pEXMatrix& C)
// A = B*C, B and C are destroyed
{
   ZZ_pEX t1, t2;

   mul(t1, B(0,0), C(0,0));
   mul(t2, B(0,1), C(1,0));
   add(A(0,0), t1, t2);

   mul(t1, B(1,0), C(0,0));
   mul(t2, B(1,1), C(1,0));
   add(A(1,0), t1, t2);

   mul(t1, B(0,0), C(0,1));
   mul(t2, B(0,1), C(1,1));
   add(A(0,1), t1, t2);

   mul(t1, B(1,0), C(0,1));
   mul(t2, B(1,1), C(1,1));
   add(A(1,1), t1, t2);

   long i, j;
   for (i = 0; i < 2; i++) {
      for (j = 0; j < 2; j++) {
          B(i,j).kill();
          C(i,j).kill();
      }
   }
}


void IterHalfGCD(_NTL_ZZ_pEXMatrix& M_out, ZZ_pEX& U, ZZ_pEX& V, long d_red)
{
   M_out(0,0).SetMaxLength(d_red);
   M_out(0,1).SetMaxLength(d_red);
   M_out(1,0).SetMaxLength(d_red);
   M_out(1,1).SetMaxLength(d_red);

   set(M_out(0,0));   clear(M_out(0,1));
   clear(M_out(1,0)); set(M_out(1,1));

   long goal = deg(U) - d_red;

   if (deg(V) <= goal)
      return;

   ZZ_pEX Q, t(INIT_SIZE, d_red);

   while (deg(V) > goal) {
      PlainDivRem(Q, U, U, V);
      swap(U, V);

      mul(t, Q, M_out(1,0));
      sub(t, M_out(0,0), t);
      M_out(0,0) = M_out(1,0);
      M_out(1,0) = t;

      mul(t, Q, M_out(1,1));
      sub(t, M_out(0,1), t);
      M_out(0,1) = M_out(1,1);
      M_out(1,1) = t;
   }
}



#define NTL_ZZ_pEX_HalfGCD_CROSSOVER (25)
#define NTL_ZZ_pEX_GCD_CROSSOVER (275)


void HalfGCD(_NTL_ZZ_pEXMatrix& M_out, const ZZ_pEX& U, const ZZ_pEX& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));

      return;
   }


   long n = deg(U) - 2*d_red + 2;
   if (n < 0) n = 0;

   ZZ_pEX U1, V1;

   RightShift(U1, U, n);
   RightShift(V1, V, n);

   if (d_red <= NTL_ZZ_pEX_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U1, V1, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   _NTL_ZZ_pEXMatrix M1;

   HalfGCD(M1, U1, V1, d1);
   mul(U1, V1, M1);

   long d2 = deg(V1) - deg(U) + n + d_red;

   if (IsZero(V1) || d2 <= 0) {
      M_out = M1;
      return;
   }


   ZZ_pEX Q;
   _NTL_ZZ_pEXMatrix M2;

   DivRem(Q, U1, U1, V1);
   swap(U1, V1);

   HalfGCD(M2, U1, V1, d2);

   ZZ_pEX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

   mul(t, Q, M1(1,0));
   sub(t, M1(0,0), t);
   swap(M1(0,0), M1(1,0));
   swap(M1(1,0), t);

   t.kill();

   t.SetMaxLength(deg(M1(1,1))+deg(Q)+1);

   mul(t, Q, M1(1,1));
   sub(t, M1(0,1), t);
   swap(M1(0,1), M1(1,1));
   swap(M1(1,1), t);

   t.kill();

   mul(M_out, M2, M1);
}




void XHalfGCD(_NTL_ZZ_pEXMatrix& M_out, ZZ_pEX& U, ZZ_pEX& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));

      return;
   }

   long du = deg(U);

   if (d_red <= NTL_ZZ_pEX_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U, V, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   //ZZ_pXMatrix M1;
   _NTL_ZZ_pEXMatrix M1;

   HalfGCD(M1, U, V, d1);
   mul(U, V, M1);

   long d2 = deg(V) - du + d_red;

   if (IsZero(V) || d2 <= 0) {
      M_out = M1;
      return;
   }


   ZZ_pEX Q;
   _NTL_ZZ_pEXMatrix M2;

   DivRem(Q, U, U, V);
   swap(U, V);

   XHalfGCD(M2, U, V, d2);

   ZZ_pEX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

   mul(t, Q, M1(1,0));
   sub(t, M1(0,0), t);
   swap(M1(0,0), M1(1,0));
   swap(M1(1,0), t);

   t.kill();

   t.SetMaxLength(deg(M1(1,1))+deg(Q)+1);

   mul(t, Q, M1(1,1));
   sub(t, M1(0,1), t);
   swap(M1(0,1), M1(1,1));
   swap(M1(1,1), t);

   t.kill();

   mul(M_out, M2, M1);
}

void HalfGCD(ZZ_pEX& U, ZZ_pEX& V)
{
   long d_red = (deg(U)+1)/2;

   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      return;
   }

   long du = deg(U);


   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   _NTL_ZZ_pEXMatrix M1;

   HalfGCD(M1, U, V, d1);
   mul(U, V, M1);

   long d2 = deg(V) - du + d_red;

   if (IsZero(V) || d2 <= 0) {
      return;
   }

   M1(0,0).kill();
   M1(0,1).kill();
   M1(1,0).kill();
   M1(1,1).kill();


   ZZ_pEX Q;

   DivRem(Q, U, U, V);
   swap(U, V);

   HalfGCD(M1, U, V, d2);

   mul(U, V, M1);
}


void GCD(ZZ_pEX& d, const ZZ_pEX& u, const ZZ_pEX& v)
{
   ZZ_pEX u1, v1;

   u1 = u;
   v1 = v;

   if (deg(u1) == deg(v1)) {
      if (IsZero(u1)) {
         clear(d);
         return;
      }

      rem(v1, v1, u1);
   }
   else if (deg(u1) < deg(v1)) {
      swap(u1, v1);
   }

   // deg(u1) > deg(v1)

   while (deg(u1) > NTL_ZZ_pEX_GCD_CROSSOVER && !IsZero(v1)) {
      HalfGCD(u1, v1);

      if (!IsZero(v1)) {
         rem(u1, u1, v1);
         swap(u1, v1);
      }
   }

   PlainGCD(d, u1, v1);
}


void XGCD(ZZ_pEX& d, ZZ_pEX& s, ZZ_pEX& t, const ZZ_pEX& a, const ZZ_pEX& b)
{
    ZZ_pE w;

   if (IsZero(a) && IsZero(b)) {
      clear(d);
      set(s);
      clear(t);
      return;
   }

   ZZ_pEX U, V, Q;

   U = a;
   V = b;

   long flag = 0;

   if (deg(U) == deg(V)) {
      DivRem(Q, U, U, V);
      swap(U, V);
      flag = 1;
   }
   else if (deg(U) < deg(V)) {
      swap(U, V);
      flag = 2;
   }

   _NTL_ZZ_pEXMatrix M;

   XHalfGCD(M, U, V, deg(U)+1);

   d = U;

   if (flag == 0) {
      s = M(0,0);
      t = M(0,1);
   }
   else if (flag == 1) {
      s = M(0,1);
      mul(t, Q, M(0,1));
      sub(t, M(0,0), t);
   }
   else {  /* flag == 2 */
      s = M(0,1);
      t = M(0,0);
   }

   // normalize

   inv(w, LeadCoeff(d));
   mul(d, d, w);
   mul(s, s, w);
   mul(t, t, w);
}


void IterBuild(ZZ_pE* a, long n)
{
   long i, k;
   ZZ_pE b, t;

   if (n <= 0) return;

   negate(a[0], a[0]);

   for (k = 1; k <= n-1; k++) {
      negate(b, a[k]);
      add(a[k], b, a[k-1]);
      for (i = k-1; i >= 1; i--) {
         mul(t, a[i], b);
         add(a[i], t, a[i-1]);
      }
      mul(a[0], a[0], b);
   }
}

void BuildFromRoots(ZZ_pEX& x, const vec_ZZ_pE& a)
{
   long n = a.length();

   if (n == 0) {
      set(x);
      return;
   }

   x.rep.SetMaxLength(n+1);
   x.rep = a;
   IterBuild(&x.rep[0], n);
   x.rep.SetLength(n+1);
   SetCoeff(x, n);
}

void eval(ZZ_pE& b, const ZZ_pEX& f, const ZZ_pE& a)
// does a Horner evaluation
{
   ZZ_pE acc;
   long i;

   clear(acc);
   for (i = deg(f); i >= 0; i--) {
      mul(acc, acc, a);
      add(acc, acc, f.rep[i]);
   }

   b = acc;
}

void eval(vec_ZZ_pE& b, const ZZ_pEX& f, const vec_ZZ_pE& a)
// naive algorithm:  repeats Horner
{
   if (&b == &f.rep) {
      vec_ZZ_pE bb;
      eval(bb, f, a);
      b = bb;
      return;
   }

   long m = a.length();
   b.SetLength(m);
   long i;
   for (i = 0; i < m; i++)
      eval(b[i], f, a[i]);
}


void interpolate(ZZ_pEX& f, const vec_ZZ_pE& a, const vec_ZZ_pE& b)
{
   long m = a.length();
   if (b.length() != m) LogicError("interpolate: vector length mismatch");

   if (m == 0) {
      clear(f);
      return;
   }

   vec_ZZ_pE prod;
   prod = a;

   ZZ_pE t1, t2;

   long k, i;

   vec_ZZ_pE res;
   res.SetLength(m);

   for (k = 0; k < m; k++) {

      const ZZ_pE& aa = a[k];

      set(t1);
      for (i = k-1; i >= 0; i--) {
         mul(t1, t1, aa);
         add(t1, t1, prod[i]);
      }

      clear(t2);
      for (i = k-1; i >= 0; i--) {
         mul(t2, t2, aa);
         add(t2, t2, res[i]);
      }


      inv(t1, t1);
      sub(t2, b[k], t2);
      mul(t1, t1, t2);

      for (i = 0; i < k; i++) {
         mul(t2, prod[i], t1);
         add(res[i], res[i], t2);
      }

      res[k] = t1;

      if (k < m-1) {
         if (k == 0)
            negate(prod[0], prod[0]);
         else {
            negate(t1, a[k]);
            add(prod[k], t1, prod[k-1]);
            for (i = k-1; i >= 1; i--) {
               mul(t2, prod[i], t1);
               add(prod[i], t2, prod[i-1]);
            }
            mul(prod[0], prod[0], t1);
         }
      }
   }

   while (m > 0 && IsZero(res[m-1])) m--;
   res.SetLength(m);
   f.rep = res;
}
   
void InnerProduct(ZZ_pEX& x, const vec_ZZ_pE& v, long low, long high, 
                   const vec_ZZ_pEX& H, long n, vec_ZZ_pX& t)
{
   ZZ_pX s;
   long i, j;

   for (j = 0; j < n; j++)
      clear(t[j]);

   high = min(high, v.length()-1);
   for (i = low; i <= high; i++) {
      const vec_ZZ_pE& h = H[i-low].rep;
      long m = h.length();
      const ZZ_pX& w = rep(v[i]);

      for (j = 0; j < m; j++) {
         mul(s, w, rep(h[j]));
         add(t[j], t[j], s);
      }
   }

   x.rep.SetLength(n);
   for (j = 0; j < n; j++)
      conv(x.rep[j], t[j]);
   x.normalize();
}



void CompMod(ZZ_pEX& x, const ZZ_pEX& g, const ZZ_pEXArgument& A, 
             const ZZ_pEXModulus& F)
{
   if (deg(g) <= 0) {
      x = g;
      return;
   }


   ZZ_pEX s, t;
   vec_ZZ_pX scratch;
   SetSize(scratch, deg(F), 2*ZZ_pE::degree());

   long m = A.H.length() - 1;
   long l = ((g.rep.length()+m-1)/m) - 1;

   const ZZ_pEX& M = A.H[m];

   InnerProduct(t, g.rep, l*m, l*m + m - 1, A.H, F.n, scratch);
   for (long i = l-1; i >= 0; i--) {
      InnerProduct(s, g.rep, i*m, i*m + m - 1, A.H, F.n, scratch);
      MulMod(t, t, M, F);
      add(t, t, s);
   }

   x = t;
}


void build(ZZ_pEXArgument& A, const ZZ_pEX& h, const ZZ_pEXModulus& F, long m)
{
   long i;

   if (m <= 0 || deg(h) >= F.n)
      LogicError("build: bad args");

   if (m > F.n) m = F.n;

   if (ZZ_pEXArgBound > 0) {
      double sz = ZZ_p::storage();
      sz = sz*ZZ_pE::degree();
      sz = sz + NTL_VECTOR_HEADER_SIZE + sizeof(vec_ZZ_p);
      sz = sz*F.n;
      sz = sz + NTL_VECTOR_HEADER_SIZE + sizeof(vec_ZZ_pE);
      sz = sz/1024;
      m = min(m, long(ZZ_pEXArgBound/sz));
      m = max(m, 1);
   }


   A.H.SetLength(m+1);

   set(A.H[0]);
   A.H[1] = h;
   for (i = 2; i <= m; i++)
      MulMod(A.H[i], A.H[i-1], h, F);
}

NTL_CHEAP_THREAD_LOCAL long ZZ_pEXArgBound = 0;




void CompMod(ZZ_pEX& x, const ZZ_pEX& g, const ZZ_pEX& h, const ZZ_pEXModulus& F)
   // x = g(h) mod f
{
   long m = SqrRoot(g.rep.length());

   if (m == 0) {
      clear(x);
      return;
   }

   ZZ_pEXArgument A;

   build(A, h, F, m);

   CompMod(x, g, A, F);
}




void Comp2Mod(ZZ_pEX& x1, ZZ_pEX& x2, const ZZ_pEX& g1, const ZZ_pEX& g2,
              const ZZ_pEX& h, const ZZ_pEXModulus& F)

{
   long m = SqrRoot(g1.rep.length() + g2.rep.length());

   if (m == 0) {
      clear(x1);
      clear(x2);
      return;
   }

   ZZ_pEXArgument A;

   build(A, h, F, m);

   ZZ_pEX xx1, xx2;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);

   x1 = xx1;
   x2 = xx2;
}

void Comp3Mod(ZZ_pEX& x1, ZZ_pEX& x2, ZZ_pEX& x3, 
              const ZZ_pEX& g1, const ZZ_pEX& g2, const ZZ_pEX& g3,
              const ZZ_pEX& h, const ZZ_pEXModulus& F)

{
   long m = SqrRoot(g1.rep.length() + g2.rep.length() + g3.rep.length());

   if (m == 0) {
      clear(x1);
      clear(x2);
      clear(x3);
      return;
   }

   ZZ_pEXArgument A;

   build(A, h, F, m);

   ZZ_pEX xx1, xx2, xx3;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);
   CompMod(xx3, g3, A, F);

   x1 = xx1;
   x2 = xx2;
   x3 = xx3;
}

void build(ZZ_pEXTransMultiplier& B, const ZZ_pEX& b, const ZZ_pEXModulus& F)
{
   long db = deg(b);

   if (db >= F.n) LogicError("build TransMultiplier: bad args");

   ZZ_pEX t;

   LeftShift(t, b, F.n-1);
   div(t, t, F);

   // we optimize for low degree b

   long d;

   d = deg(t);
   if (d < 0)
      B.shamt_fbi = 0;
   else
      B.shamt_fbi = F.n-2 - d; 

   CopyReverse(B.fbi, t, d);

   // The following code optimizes the case when 
   // f = X^n + low degree poly

   trunc(t, F.f, F.n);
   d = deg(t);
   if (d < 0)
      B.shamt = 0;
   else
      B.shamt = d;

   CopyReverse(B.f0, t, d);

   if (db < 0)
      B.shamt_b = 0;
   else
      B.shamt_b = db;

   CopyReverse(B.b, b, db);
}

void TransMulMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEXTransMultiplier& B,
               const ZZ_pEXModulus& F)
{
   if (deg(a) >= F.n) LogicError("TransMulMod: bad args");

   ZZ_pEX t1, t2;

   mul(t1, a, B.b);
   RightShift(t1, t1, B.shamt_b);

   mul(t2, a, B.f0);
   RightShift(t2, t2, B.shamt);
   trunc(t2, t2, F.n-1);

   mul(t2, t2, B.fbi);
   if (B.shamt_fbi > 0) LeftShift(t2, t2, B.shamt_fbi);
   trunc(t2, t2, F.n-1);
   LeftShift(t2, t2, 1);

   sub(x, t1, t2);
}


void ShiftSub(ZZ_pEX& U, const ZZ_pEX& V, long n)
// assumes input does not alias output
{
   if (IsZero(V))
      return;

   long du = deg(U);
   long dv = deg(V);

   long d = max(du, n+dv);

   U.rep.SetLength(d+1);
   long i;

   for (i = du+1; i <= d; i++)
      clear(U.rep[i]);

   for (i = 0; i <= dv; i++)
      sub(U.rep[i+n], U.rep[i+n], V.rep[i]);

   U.normalize();
}


void UpdateMap(vec_ZZ_pE& x, const vec_ZZ_pE& a,
         const ZZ_pEXTransMultiplier& B, const ZZ_pEXModulus& F)
{
   ZZ_pEX xx;
   TransMulMod(xx, to_ZZ_pEX(a), B, F);
   x = xx.rep;
}

static
void ProjectPowers(vec_ZZ_pE& x, const ZZ_pEX& a, long k, 
                   const ZZ_pEXArgument& H, const ZZ_pEXModulus& F)
{
   if (k < 0 || deg(a) >= F.n) 
      LogicError("ProjectPowers: bad args");

   if (NTL_OVERFLOW(k, 1, 0)) 
      ResourceError("ProjectPowers: excessive args");


   long m = H.H.length()-1;
   long l = (k+m-1)/m - 1;

   ZZ_pEXTransMultiplier M;
   build(M, H.H[m], F);

   ZZ_pEX s;
   s = a;

   x.SetLength(k);

   long i;

   for (i = 0; i <= l; i++) {
      long m1 = min(m, k-i*m);
      for (long j = 0; j < m1; j++)
         InnerProduct(x[i*m+j], H.H[j].rep, s.rep);
      if (i < l)
         TransMulMod(s, s, M, F);
   }
}

static
void ProjectPowers(vec_ZZ_pE& x, const ZZ_pEX& a, long k, const ZZ_pEX& h, 
                   const ZZ_pEXModulus& F)
{
   if (k < 0 || deg(a) >= F.n || deg(h) >= F.n)
      LogicError("ProjectPowers: bad args");

   if (k == 0) {
      x.SetLength(0);;
      return;
   }

   long m = SqrRoot(k);

   ZZ_pEXArgument H;
   build(H, h, F, m);

   ProjectPowers(x, a, k, H, F);
}

void ProjectPowers(vec_ZZ_pE& x, const vec_ZZ_pE& a, long k,
                   const ZZ_pEXArgument& H, const ZZ_pEXModulus& F)
{
   ProjectPowers(x, to_ZZ_pEX(a), k, H, F);
}

void ProjectPowers(vec_ZZ_pE& x, const vec_ZZ_pE& a, long k,
                   const ZZ_pEX& h, const ZZ_pEXModulus& F)
{
   ProjectPowers(x, to_ZZ_pEX(a), k, h, F);
}




void BerlekampMassey(ZZ_pEX& h, const vec_ZZ_pE& a, long m)
{
   ZZ_pEX Lambda, Sigma, Temp;
   long L;
   ZZ_pE Delta, Delta1, t1;
   long shamt;

   // cerr << "*** " << m << "\n";

   Lambda.SetMaxLength(m+1);
   Sigma.SetMaxLength(m+1);
   Temp.SetMaxLength(m+1);

   L = 0;
   set(Lambda);
   clear(Sigma);
   set(Delta);
   shamt = 0;

   long i, r, dl;

   for (r = 1; r <= 2*m; r++) {
      // cerr << r << "--";
      clear(Delta1);
      dl = deg(Lambda);
      for (i = 0; i <= dl; i++) {
         mul(t1, Lambda.rep[i], a[r-i-1]);
         add(Delta1, Delta1, t1);
      }

      if (IsZero(Delta1)) {
         shamt++;
         // cerr << "case 1: " << deg(Lambda) << " " << deg(Sigma) << " " << shamt << "\n";
      }
      else if (2*L < r) {
         div(t1, Delta1, Delta);
         mul(Temp, Sigma, t1);
         Sigma = Lambda;
         ShiftSub(Lambda, Temp, shamt+1);
         shamt = 0;
         L = r-L;
         Delta = Delta1;
         // cerr << "case 2: " << deg(Lambda) << " " << deg(Sigma) << " " << shamt << "\n";
      }
      else {
         shamt++;
         div(t1, Delta1, Delta);
         mul(Temp, Sigma, t1);
         ShiftSub(Lambda, Temp, shamt);
         // cerr << "case 3: " << deg(Lambda) << " " << deg(Sigma) << " " << shamt << "\n";
      }
   }

   // cerr << "finished: " << L << " " << deg(Lambda) << "\n"; 

   dl = deg(Lambda);
   h.rep.SetLength(L + 1);

   for (i = 0; i < L - dl; i++)
      clear(h.rep[i]);

   for (i = L - dl; i <= L; i++)
      h.rep[i] = Lambda.rep[L - i];
}




void MinPolySeq(ZZ_pEX& h, const vec_ZZ_pE& a, long m)
{
   if (m < 0 || NTL_OVERFLOW(m, 1, 0)) LogicError("MinPoly: bad args");
   if (a.length() < 2*m) LogicError("MinPoly: sequence too short");

   BerlekampMassey(h, a, m);
}


void DoMinPolyMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m, 
               const ZZ_pEX& R)
{
   vec_ZZ_pE x;

   ProjectPowers(x, R, 2*m, g, F);
   MinPolySeq(h, x, m);
}

void ProbMinPolyMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m)
{
   long n = F.n;
   if (m < 1 || m > n) LogicError("ProbMinPoly: bad args");

   ZZ_pEX R;
   random(R, n);

   DoMinPolyMod(h, g, F, m, R);
}

void ProbMinPolyMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F)
{
   ProbMinPolyMod(h, g, F, F.n);
}

void MinPolyMod(ZZ_pEX& hh, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m)
{
   ZZ_pEX h, h1;
   long n = F.n;
   if (m < 1 || m > n) LogicError("MinPoly: bad args");

   /* probabilistically compute min-poly */

   ProbMinPolyMod(h, g, F, m);
   if (deg(h) == m) { hh = h; return; }
   CompMod(h1, h, g, F);
   if (IsZero(h1)) { hh = h; return; }

   /* not completely successful...must iterate */

   ZZ_pEX h2, h3;
   ZZ_pEX R;
   ZZ_pEXTransMultiplier H1;
   

   for (;;) {
      random(R, n);
      build(H1, h1, F);
      TransMulMod(R, R, H1, F);
      DoMinPolyMod(h2, g, F, m-deg(h), R);

      mul(h, h, h2);
      if (deg(h) == m) { hh = h; return; }
      CompMod(h3, h2, g, F);
      MulMod(h1, h3, h1, F);
      if (IsZero(h1)) { hh = h; return; }
   }
}

void IrredPolyMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m)
{
   if (m < 1 || m > F.n) LogicError("IrredPoly: bad args");

   ZZ_pEX R;
   set(R);

   DoMinPolyMod(h, g, F, m, R);
}



void IrredPolyMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F)
{
   IrredPolyMod(h, g, F, F.n);
}



void MinPolyMod(ZZ_pEX& hh, const ZZ_pEX& g, const ZZ_pEXModulus& F)
{
   MinPolyMod(hh, g, F, F.n);
}

void diff(ZZ_pEX& x, const ZZ_pEX& a)
{
   long n = deg(a);
   long i;

   if (n <= 0) {
      clear(x);
      return;
   }

   if (&x != &a)
      x.rep.SetLength(n);

   for (i = 0; i <= n-1; i++) {
      mul(x.rep[i], a.rep[i+1], i+1);
   }

   if (&x == &a)
      x.rep.SetLength(n);

   x.normalize();
}



void MakeMonic(ZZ_pEX& x)
{
   if (IsZero(x))
      return;

   if (IsOne(LeadCoeff(x)))
      return;

   ZZ_pE t;

   inv(t, LeadCoeff(x));
   mul(x, x, t);
}


long divide(ZZ_pEX& q, const ZZ_pEX& a, const ZZ_pEX& b)
{
   if (IsZero(b)) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   ZZ_pEX lq, r;
   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0; 
   q = lq;
   return 1;
}

long divide(const ZZ_pEX& a, const ZZ_pEX& b)
{
   if (IsZero(b)) return IsZero(a);
   ZZ_pEX lq, r;
   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0; 
   return 1;
}



static
long OptWinSize(long n)
// finds k that minimizes n/(k+1) + 2^{k-1}

{
   long k;
   double v, v_new;


   v = n/2.0 + 1.0;
   k = 1;

   for (;;) {
      v_new = n/(double(k+2)) + double(1L << k);
      if (v_new >= v) break;
      v = v_new;
      k++;
   }

   return k;
}
      


void PowerMod(ZZ_pEX& h, const ZZ_pEX& g, const ZZ& e, const ZZ_pEXModulus& F)
// h = g^e mod f using "sliding window" algorithm
{
   if (deg(g) >= F.n) LogicError("PowerMod: bad args");

   if (e == 0) {
      set(h);
      return;
   }

   if (e == 1) {
      h = g;
      return;
   }

   if (e == -1) {
      InvMod(h, g, F);
      return;
   }

   if (e == 2) {
      SqrMod(h, g, F);
      return;
   }

   if (e == -2) {
      SqrMod(h, g, F);
      InvMod(h, h, F);
      return;
   }


   long n = NumBits(e);

   ZZ_pEX res;
   res.SetMaxLength(F.n);
   set(res);

   long i;

   if (n < 16) {
      // plain square-and-multiply algorithm

      for (i = n - 1; i >= 0; i--) {
         SqrMod(res, res, F);
         if (bit(e, i))
            MulMod(res, res, g, F);
      }

      if (e < 0) InvMod(res, res, F);

      h = res;
      return;
   }

   long k = OptWinSize(n);
   k = min(k, 3);

   vec_ZZ_pEX v;

   v.SetLength(1L << (k-1));

   v[0] = g;
 
   if (k > 1) {
      ZZ_pEX t;
      SqrMod(t, g, F);

      for (i = 1; i < (1L << (k-1)); i++)
         MulMod(v[i], v[i-1], t, F);
   }


   long val;
   long cnt;
   long m;

   val = 0;
   for (i = n-1; i >= 0; i--) {
      val = (val << 1) | bit(e, i); 
      if (val == 0)
         SqrMod(res, res, F);
      else if (val >= (1L << (k-1)) || i == 0) {
         cnt = 0;
         while ((val & 1) == 0) {
            val = val >> 1;
            cnt++;
         }

         m = val;
         while (m > 0) {
            SqrMod(res, res, F);
            m = m >> 1;
         }

         MulMod(res, res, v[val >> 1], F);

         while (cnt > 0) {
            SqrMod(res, res, F);
            cnt--;
         }

         val = 0;
      }
   }

   if (e < 0) InvMod(res, res, F);

   h = res;
}

void InvMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvMod: bad args");

   ZZ_pEX d, xx, t;

   XGCD(d, xx, t, a, f);
   if (!IsOne(d))
      InvModError("ZZ_pEX InvMod: can't compute multiplicative inverse");

   x = xx;
}

long InvModStatus(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvModStatus: bad args");
   ZZ_pEX d, t;

   XGCD(d, x, t, a, f);
   if (!IsOne(d)) {
      x = d;
      return 1;
   }
   else
      return 0;
}


void MulMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& b, const ZZ_pEX& f)
{
   if (deg(a) >= deg(f) || deg(b) >= deg(f) || deg(f) == 0)
      LogicError("MulMod: bad args");

   ZZ_pEX t;

   mul(t, a, b);
   rem(x, t, f);
}

void SqrMod(ZZ_pEX& x, const ZZ_pEX& a, const ZZ_pEX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("SqrMod: bad args");

   ZZ_pEX t;

   sqr(t, a);
   rem(x, t, f);
}


void PowerXMod(ZZ_pEX& hh, const ZZ& e, const ZZ_pEXModulus& F)
{
   if (F.n < 0) LogicError("PowerXMod: uninitialized modulus");

   if (IsZero(e)) {
      set(hh);
      return;
   }

   long n = NumBits(e);
   long i;

   ZZ_pEX h;

   h.SetMaxLength(F.n);
   set(h);

   for (i = n - 1; i >= 0; i--) {
      SqrMod(h, h, F);
      if (bit(e, i))
         MulByXMod(h, h, F.f);
   }

   if (e < 0) InvMod(h, h, F);

   hh = h;
}


void reverse(ZZ_pEX& x, const ZZ_pEX& a, long hi)
{
   if (hi < 0) { clear(x); return; }
   if (NTL_OVERFLOW(hi, 1, 0))
      ResourceError("overflow in reverse");

   if (&x == &a) {
      ZZ_pEX tmp;
      CopyReverse(tmp, a, hi);
      x = tmp;
   }
   else
      CopyReverse(x, a, hi);
}


void power(ZZ_pEX& x, const ZZ_pEX& a, long e)
{
   if (e < 0) {
      ArithmeticError("power: negative exponent");
   }

   if (e == 0) {
      x = 1;
      return;
   }

   if (a == 0 || a == 1) {
      x = a;
      return;
   }

   long da = deg(a);

   if (da == 0) {
      x = power(ConstTerm(a), e);
      return;
   }

   if (da > (NTL_MAX_LONG-1)/e)
      ResourceError("overflow in power");

   ZZ_pEX res;
   res.SetMaxLength(da*e + 1);
   res = 1;
   
   long k = NumBits(e);
   long i;

   for (i = k - 1; i >= 0; i--) {
      sqr(res, res);
      if (bit(e, i))
         mul(res, res, a);
   }

   x = res;
}



static
void FastTraceVec(vec_ZZ_pE& S, const ZZ_pEXModulus& f)
{
   long n = deg(f);

   ZZ_pEX x = reverse(-LeftShift(reverse(diff(reverse(f)), n-1), n-1)/f, n-1);

   S.SetLength(n);
   S[0] = n;

   long i;
   for (i = 1; i < n; i++)
      S[i] = coeff(x, i);
}


void PlainTraceVec(vec_ZZ_pE& S, const ZZ_pEX& ff)
{
   if (deg(ff) <= 0)
      LogicError("TraceVec: bad args");

   ZZ_pEX f;
   f = ff;

   MakeMonic(f);

   long n = deg(f);

   S.SetLength(n);

   if (n == 0)
      return;

   long k, i;
   ZZ_pX acc, t;
   ZZ_pE t1;

   S[0] = n;

   for (k = 1; k < n; k++) {
      mul(acc, rep(f.rep[n-k]), k);

      for (i = 1; i < k; i++) {
         mul(t, rep(f.rep[n-i]), rep(S[k-i]));
         add(acc, acc, t);
      }

      conv(t1, acc);
      negate(S[k], t1);
   }
}

void TraceVec(vec_ZZ_pE& S, const ZZ_pEX& f)
{
   if (deg(f) < ZZ_pE::DivCross())
      PlainTraceVec(S, f);
   else
      FastTraceVec(S, f);
}

static

void ComputeTraceVec(vec_ZZ_pE& S, const ZZ_pEXModulus& F)
{
   if (F.method == ZZ_pEX_MOD_PLAIN) {
      PlainTraceVec(S, F.f);
   }
   else {
      FastTraceVec(S, F);
   }
}

void TraceMod(ZZ_pE& x, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   long n = F.n;

   if (deg(a) >= n)
      LogicError("trace: bad args");

   do { // NOTE: thread safe lazy init
      Lazy<vec_ZZ_pE>::Builder builder(F.tracevec.val());
      if (!builder()) break;
      UniquePtr<vec_ZZ_pE> p;
      p.make();
      ComputeTraceVec(*p, F);
      builder.move(p);
   } while (0);

   InnerProduct(x, a.rep, *F.tracevec.val());
}

void TraceMod(ZZ_pE& x, const ZZ_pEX& a, const ZZ_pEX& f)
{
   if (deg(a) >= deg(f) || deg(f) <= 0)
      LogicError("trace: bad args");

   project(x, TraceVec(f), a);
}


void PlainResultant(ZZ_pE& rres, const ZZ_pEX& a, const ZZ_pEX& b)
{
   ZZ_pE res;
 
   if (IsZero(a) || IsZero(b))
      clear(res);
   else if (deg(a) == 0 && deg(b) == 0) 
      set(res);
   else {
      long d0, d1, d2;
      ZZ_pE lc;
      set(res);

      long n = max(deg(a),deg(b)) + 1;
      ZZ_pEX u(INIT_SIZE, n), v(INIT_SIZE, n);
      vec_ZZ_pX tmp;
      SetSize(tmp, n, 2*ZZ_pE::degree());

      u = a;
      v = b;

      for (;;) {
         d0 = deg(u);
         d1 = deg(v);
         lc = LeadCoeff(v);

         PlainRem(u, u, v, tmp);
         swap(u, v);

         d2 = deg(v);
         if (d2 >= 0) {
            power(lc, lc, d0-d2);
            mul(res, res, lc);
            if (d0 & d1 & 1) negate(res, res);
         }
         else {
            if (d1 == 0) {
               power(lc, lc, d0);
               mul(res, res, lc);
            }
            else
               clear(res);
        
            break;
         }
      }
   }
   rres = res;
}

void resultant(ZZ_pE& rres, const ZZ_pEX& a, const ZZ_pEX& b)
{
   PlainResultant(rres, a, b); 
}


void NormMod(ZZ_pE& x, const ZZ_pEX& a, const ZZ_pEX& f)
{
   if (deg(f) <= 0 || deg(a) >= deg(f)) 
      LogicError("norm: bad args");

   if (IsZero(a)) {
      clear(x);
      return;
   }

   ZZ_pE t;
   resultant(t, f, a);
   if (!IsOne(LeadCoeff(f))) {
      ZZ_pE t1;
      power(t1, LeadCoeff(f), deg(a));
      inv(t1, t1);
      mul(t, t, t1);
   }

   x = t;
}



// tower stuff...



void InnerProduct(ZZ_pEX& x, const vec_ZZ_p& v, long low, long high,
                   const vec_ZZ_pEX& H, long n, vec_ZZ_pE& t)
{
   ZZ_pE s;
   long i, j;

   for (j = 0; j < n; j++)
      clear(t[j]);

   high = min(high, v.length()-1);
   for (i = low; i <= high; i++) {
      const vec_ZZ_pE& h = H[i-low].rep;
      long m = h.length();
      const ZZ_p& w = v[i];

      for (j = 0; j < m; j++) {
         mul(s, h[j], w);
         add(t[j], t[j], s);
      }
   }

   x.rep.SetLength(n);
   for (j = 0; j < n; j++)
      x.rep[j] = t[j];

   x.normalize();
}



void CompTower(ZZ_pEX& x, const ZZ_pX& g, const ZZ_pEXArgument& A,
             const ZZ_pEXModulus& F)
{
   if (deg(g) <= 0) {
      conv(x, g);
      return;
   }


   ZZ_pEX s, t;
   vec_ZZ_pE scratch;
   scratch.SetLength(deg(F));

   long m = A.H.length() - 1;
   long l = ((g.rep.length()+m-1)/m) - 1;

   const ZZ_pEX& M = A.H[m];

   InnerProduct(t, g.rep, l*m, l*m + m - 1, A.H, F.n, scratch);
   for (long i = l-1; i >= 0; i--) {
      InnerProduct(s, g.rep, i*m, i*m + m - 1, A.H, F.n, scratch);
      MulMod(t, t, M, F);
      add(t, t, s);
   }
   x = t;
}


void CompTower(ZZ_pEX& x, const ZZ_pX& g, const ZZ_pEX& h, 
             const ZZ_pEXModulus& F)
   // x = g(h) mod f
{
   long m = SqrRoot(g.rep.length());

   if (m == 0) {
      clear(x);
      return;
   }


   ZZ_pEXArgument A;

   build(A, h, F, m);

   CompTower(x, g, A, F);
}

void PrepareProjection(vec_vec_ZZ_p& tt, const vec_ZZ_pE& s,
                       const vec_ZZ_p& proj)
{
   long l = s.length();
   tt.SetLength(l);

   ZZ_pXMultiplier M;
   long i;

   for (i = 0; i < l; i++) {
      build(M, rep(s[i]), ZZ_pE::modulus());
      UpdateMap(tt[i], proj, M, ZZ_pE::modulus());
   }
}

void ProjectedInnerProduct(ZZ_p& x, const vec_ZZ_pE& a, 
                           const vec_vec_ZZ_p& b)
{
   long n = min(a.length(), b.length());

   ZZ_p t, res;

   res = 0;

   long i;
   for (i = 0; i < n; i++) {
      project(t, b[i], rep(a[i]));
      res += t;
   }

   x = res;
}


void PrecomputeProj(vec_ZZ_p& proj, const ZZ_pX& f)
{
   long n = deg(f);

   if (n <= 0) LogicError("PrecomputeProj: bad args");

   if (ConstTerm(f) != 0) {
      proj.SetLength(1);
      proj[0] = 1;
   }
   else {
      proj.SetLength(n);
      clear(proj);
      proj[n-1] = 1;
   }
}
   


void ProjectPowersTower(vec_ZZ_p& x, const vec_ZZ_pE& a, long k,
                   const ZZ_pEXArgument& H, const ZZ_pEXModulus& F,
                   const vec_ZZ_p& proj)

{
   long n = F.n;

   if (a.length() > n || k < 0) 
      LogicError("ProjectPowers: bad args");
   if (NTL_OVERFLOW(k, 1, 0)) 
      ResourceError("ProjectPowers: excessive args");


   long m = H.H.length()-1;
   long l = (k+m-1)/m - 1;

   ZZ_pEXTransMultiplier M;
   build(M, H.H[m], F);

   vec_ZZ_pE s(INIT_SIZE, n);
   s = a;

   x.SetLength(k);

   vec_vec_ZZ_p tt;

   for (long i = 0; i <= l; i++) {
      long m1 = min(m, k-i*m);
      ZZ_p* w = &x[i*m];

      PrepareProjection(tt, s, proj);

      for (long j = 0; j < m1; j++)
         ProjectedInnerProduct(w[j], H.H[j].rep, tt);
      if (i < l)
         UpdateMap(s, s, M, F);
   }
}




void ProjectPowersTower(vec_ZZ_p& x, const vec_ZZ_pE& a, long k,
                   const ZZ_pEX& h, const ZZ_pEXModulus& F,
                   const vec_ZZ_p& proj)

{
   if (a.length() > F.n || k < 0) LogicError("ProjectPowers: bad args");

   if (k == 0) {
      x.SetLength(0);
      return;
   }

   long m = SqrRoot(k);

   ZZ_pEXArgument H;

   build(H, h, F, m);
   ProjectPowersTower(x, a, k, H, F, proj);
}


void DoMinPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m,
               const vec_ZZ_pE& R, const vec_ZZ_p& proj)
{
   vec_ZZ_p x;

   ProjectPowersTower(x, R, 2*m, g, F, proj);
   
   MinPolySeq(h, x, m);
}


void ProbMinPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F, 
                      long m)
{
   long n = F.n;
   if (m < 1 || m > n*ZZ_pE::degree())
      LogicError("MinPoly: bad args");

   vec_ZZ_pE R;
   R.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      random(R[i]);

   vec_ZZ_p proj;
   PrecomputeProj(proj, ZZ_pE::modulus());

   DoMinPolyTower(h, g, F, m, R, proj);
}


void ProbMinPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F, 
                      long m, const vec_ZZ_p& proj)
{
   long n = F.n;
   if (m < 1 || m > n*ZZ_pE::degree())
      LogicError("MinPoly: bad args");

   vec_ZZ_pE R;
   R.SetLength(n);
   long i;
   for (i = 0; i < n; i++)
      random(R[i]);

   DoMinPolyTower(h, g, F, m, R, proj);
}

void MinPolyTower(ZZ_pX& hh, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m)
{
   ZZ_pX h;
   ZZ_pEX h1;
   long n = F.n;

   if (m < 1 || m > n*ZZ_pE::degree())
      LogicError("MinPoly: bad args");

   vec_ZZ_p proj;
   PrecomputeProj(proj, ZZ_pE::modulus());

   /* probabilistically compute min-poly */

   ProbMinPolyTower(h, g, F, m, proj);
   if (deg(h) == m) { hh = h; return; }
   CompTower(h1, h, g, F);
   if (IsZero(h1)) { hh = h; return; }

   /* not completely successful...must iterate */

   long i;

   ZZ_pX h2;
   ZZ_pEX h3;
   vec_ZZ_pE R;
   ZZ_pEXTransMultiplier H1;
   

   for (;;) {
      R.SetLength(n);
      for (i = 0; i < n; i++) random(R[i]);
      build(H1, h1, F);
      UpdateMap(R, R, H1, F);
      DoMinPolyTower(h2, g, F, m-deg(h), R, proj);

      mul(h, h, h2);
      if (deg(h) == m) { hh = h; return; }
      CompTower(h3, h2, g, F);
      MulMod(h1, h3, h1, F);
      if (IsZero(h1)) { hh = h; return; }
   }
}

void IrredPolyTower(ZZ_pX& h, const ZZ_pEX& g, const ZZ_pEXModulus& F, long m)
{
   if (m < 1 || m > deg(F)*ZZ_pE::degree())
      LogicError("IrredPoly: bad args");

   vec_ZZ_pE R;
   R.SetLength(1);
   R[0] = 1;

   vec_ZZ_p proj;
   proj.SetLength(1);
   proj[0] = 1;

   DoMinPolyTower(h, g, F, m, R, proj);
}

NTL_END_IMPL

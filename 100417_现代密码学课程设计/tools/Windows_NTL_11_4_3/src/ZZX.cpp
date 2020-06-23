
#include <NTL/ZZX.h>


NTL_START_IMPL



const ZZX& ZZX::zero()
{
   static const ZZX z; // GLOBAL (relies on C++11 thread-safe init)
   return z;
}



void conv(ZZ_pX& x, const ZZX& a)
{
   conv(x.rep, a.rep);
   x.normalize();
}

void conv(ZZX& x, const ZZ_pX& a)
{
   conv(x.rep, a.rep);
   x.normalize();
}


istream& operator>>(istream& s, ZZX& x)
{
   NTL_INPUT_CHECK_RET(s, s >> x.rep);
   x.normalize();
   return s;
}

ostream& operator<<(ostream& s, const ZZX& a)
{
   return s << a.rep;
}


void ZZX::normalize()
{
   long n;
   const ZZ* p;

   n = rep.length();
   if (n == 0) return;
   p = rep.elts() + n;
   while (n > 0 && IsZero(*--p)) {
      n--;
   }
   rep.SetLength(n);
}


long IsZero(const ZZX& a)
{
   return a.rep.length() == 0;
}


long IsOne(const ZZX& a)
{
    return a.rep.length() == 1 && IsOne(a.rep[0]);
}

long operator==(const ZZX& a, const ZZX& b)
{
   long i, n;
   const ZZ *ap, *bp;

   n = a.rep.length();
   if (n != b.rep.length()) return 0;

   ap = a.rep.elts();
   bp = b.rep.elts();

   for (i = 0; i < n; i++)
      if (ap[i] != bp[i]) return 0;

   return 1;
}


long operator==(const ZZX& a, long b)
{
   if (b == 0)
      return IsZero(a);

   if (deg(a) != 0)
      return 0;

   return a.rep[0] == b;
}

long operator==(const ZZX& a, const ZZ& b)
{
   if (IsZero(b))
      return IsZero(a);

   if (deg(a) != 0)
      return 0;

   return a.rep[0] == b;
}


void GetCoeff(ZZ& x, const ZZX& a, long i)
{
   if (i < 0 || i > deg(a))
      clear(x);
   else
      x = a.rep[i];
}

void SetCoeff(ZZX& x, long i, const ZZ& a)
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
         ZZ aa = a;
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


void SetCoeff(ZZX& x, long i)
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


void SetX(ZZX& x)
{
   clear(x);
   SetCoeff(x, 1);
}


long IsX(const ZZX& a)
{
   return deg(a) == 1 && IsOne(LeadCoeff(a)) && IsZero(ConstTerm(a));
}
      
      

const ZZ& coeff(const ZZX& a, long i)
{
   if (i < 0 || i > deg(a))
      return ZZ::zero();
   else
      return a.rep[i];
}


const ZZ& LeadCoeff(const ZZX& a)
{
   if (IsZero(a))
      return ZZ::zero();
   else
      return a.rep[deg(a)];
}

const ZZ& ConstTerm(const ZZX& a)
{
   if (IsZero(a))
      return ZZ::zero();
   else
      return a.rep[0];
}



void conv(ZZX& x, const ZZ& a)
{
   if (IsZero(a))
      x.rep.SetLength(0);
   else {
      x.rep.SetLength(1);
      x.rep[0] = a;
   }
}


void conv(ZZX& x, long a)
{
   if (a == 0) 
      x.rep.SetLength(0);
   else {
      x.rep.SetLength(1);
      conv(x.rep[0], a);
   }
}


void conv(ZZX& x, const vec_ZZ& a)
{
   x.rep = a;
   x.normalize();
}


void add(ZZX& x, const ZZX& a, const ZZX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const ZZ *ap, *bp; 
   ZZ* xp;

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

void add(ZZX& x, const ZZX& a, const ZZ& b)
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

      ZZ *xp = x.rep.elts(); 
      add(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}


void add(ZZX& x, const ZZX& a, long b)
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


void sub(ZZX& x, const ZZX& a, const ZZX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const ZZ *ap, *bp; 
   ZZ* xp;

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

void sub(ZZX& x, const ZZX& a, const ZZ& b)
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

      ZZ *xp = x.rep.elts();
      sub(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}

void sub(ZZX& x, const ZZX& a, long b)
{
   if (b == 0) {
      x = a;
      return;
   }

   if (a.rep.length() == 0) {
      x.rep.SetLength(1);
      conv(x.rep[0], b);
      negate(x.rep[0], x.rep[0]);
   }
   else {
      if (&x != &a) x = a;
      sub(x.rep[0], x.rep[0], b);
   }
   x.normalize();
}

void sub(ZZX& x, long a, const ZZX& b)
{
   negate(x, b);
   add(x, x, a);
}


void sub(ZZX& x, const ZZ& b, const ZZX& a)
{
   long n = a.rep.length();
   if (n == 0) {
      conv(x, b);
   }
   else if (x.rep.MaxLength() == 0) {
      negate(x, a);
      add(x.rep[0], a.rep[0], b);
      x.normalize();
   }
   else {
      // ugly...b could alias a coeff of x

      ZZ *xp = x.rep.elts();
      sub(xp[0], b, a.rep[0]);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         negate(xp[i], ap[i]);
      x.normalize();
   }
}



void negate(ZZX& x, const ZZX& a)
{
   long n = a.rep.length();
   x.rep.SetLength(n);

   const ZZ* ap = a.rep.elts();
   ZZ* xp = x.rep.elts();
   long i;

   for (i = n; i; i--, ap++, xp++)
      negate((*xp), (*ap));

}

long MaxBits(const ZZX& f)
{
   long i, m;
   m = 0;

   for (i = 0; i <= deg(f); i++) {
      m = max(m, NumBits(f.rep[i]));
   }

   return m;
}


void PlainMul(ZZX& x, const ZZX& a, const ZZX& b)
{
   if (&a == &b) {
      PlainSqr(x, a);
      return;
   }

   long da = deg(a);
   long db = deg(b);

   if (da < 0 || db < 0) {
      clear(x);
      return;
   }

   long d = da+db;



   const ZZ *ap, *bp;
   ZZ *xp;
   
   ZZX la, lb;

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
   ZZ t, accum;

   for (i = 0; i <= d; i++) {
      jmin = max(0, i-db);
      jmax = min(da, i);
      clear(accum);
      for (j = jmin; j <= jmax; j++) {
         mul(t, ap[j], bp[i-j]);
         add(accum, accum, t);
      }
      xp[i] = accum;
   }
   x.normalize();
}

void PlainSqr(ZZX& x, const ZZX& a)
{
   long da = deg(a);

   if (da < 0) {
      clear(x);
      return;
   }

   long d = 2*da;

   const ZZ *ap;
   ZZ *xp;

   ZZX la;

   if (&x == &a) {
      la = a;
      ap = la.rep.elts();
   }
   else
      ap = a.rep.elts();


   x.rep.SetLength(d+1);

   xp = x.rep.elts();

   long i, j, jmin, jmax;
   long m, m2;
   ZZ t, accum;

   for (i = 0; i <= d; i++) {
      jmin = max(0, i-da);
      jmax = min(da, i);
      m = jmax - jmin + 1;
      m2 = m >> 1;
      jmax = jmin + m2 - 1;
      clear(accum);
      for (j = jmin; j <= jmax; j++) {
         mul(t, ap[j], ap[i-j]);
         add(accum, accum, t);
      }
      add(accum, accum, accum);
      if (m & 1) {
         sqr(t, ap[jmax + 1]);
         add(accum, accum, t);
      }

      xp[i] = accum;
   }

   x.normalize();
}



static
void PlainMul(ZZ *xp, const ZZ *ap, long sa, const ZZ *bp, long sb)
{
   if (sa == 0 || sb == 0) return;

   long sx = sa+sb-1;

   long i, j, jmin, jmax;
   NTL_ZZRegister(t);
   NTL_ZZRegister(accum);

   for (i = 0; i < sx; i++) {
      jmin = max(0, i-sb+1);
      jmax = min(sa-1, i);
      clear(accum);
      for (j = jmin; j <= jmax; j++) {
         mul(t, ap[j], bp[i-j]);
         add(accum, accum, t);
      }
      xp[i] = accum;
   }
}



static
void KarFold(ZZ *T, const ZZ *b, long sb, long hsa)
{
   long m = sb - hsa;
   long i;

   for (i = 0; i < m; i++)
      add(T[i], b[i], b[hsa+i]);

   for (i = m; i < hsa; i++)
      T[i] = b[i];
}

static
void KarSub(ZZ *T, const ZZ *b, long sb)
{
   long i;

   for (i = 0; i < sb; i++)
      sub(T[i], T[i], b[i]);
}

static
void KarAdd(ZZ *T, const ZZ *b, long sb)
{
   long i;

   for (i = 0; i < sb; i++)
      add(T[i], T[i], b[i]);
}

static
void KarFix(ZZ *c, const ZZ *b, long sb, long hsa)
{
   long i;

   for (i = 0; i < hsa; i++)
      c[i] = b[i];

   for (i = hsa; i < sb; i++)
      add(c[i], c[i], b[i]);
}

static void PlainMul1(ZZ *xp, const ZZ *ap, long sa, const ZZ& b)
{
   long i;

   for (i = 0; i < sa; i++)
      mul(xp[i], ap[i], b);
}



static
void KarMul(ZZ *c, const ZZ *a, 
            long sa, const ZZ *b, long sb, ZZ *stk)
{
   if (sa < sb) {
      { long t = sa; sa = sb; sb = t; }
      { const ZZ *t = a; a = b; b = t; }
   }

   if (sb == 1) {
      if (sa == 1)
         mul(*c, *a, *b);
      else
         PlainMul1(c, a, sa, *b);

      return;
   }

   if (sb == 2 && sa == 2) {
      mul(c[0], a[0], b[0]);
      mul(c[2], a[1], b[1]);
      add(stk[0], a[0], a[1]);
      add(stk[1], b[0], b[1]);
      mul(c[1], stk[0], stk[1]);
      sub(c[1], c[1], c[0]);
      sub(c[1], c[1], c[2]);

      return;

   }

   long hsa = (sa + 1) >> 1;

   if (hsa < sb) {
      /* normal case */

      long hsa2 = hsa << 1;

      ZZ *T1, *T2, *T3;

      T1 = stk; stk += hsa;
      T2 = stk; stk += hsa;
      T3 = stk; stk += hsa2 - 1;

      /* compute T1 = a_lo + a_hi */

      KarFold(T1, a, sa, hsa);

      /* compute T2 = b_lo + b_hi */

      KarFold(T2, b, sb, hsa);

      /* recursively compute T3 = T1 * T2 */

      KarMul(T3, T1, hsa, T2, hsa, stk);

      /* recursively compute a_hi * b_hi into high part of c */
      /* and subtract from T3 */

      KarMul(c + hsa2, a+hsa, sa-hsa, b+hsa, sb-hsa, stk);
      KarSub(T3, c + hsa2, sa + sb - hsa2 - 1);


      /* recursively compute a_lo*b_lo into low part of c */
      /* and subtract from T3 */

      KarMul(c, a, hsa, b, hsa, stk);
      KarSub(T3, c, hsa2 - 1);

      clear(c[hsa2 - 1]);

      /* finally, add T3 * X^{hsa} to c */

      KarAdd(c+hsa, T3, hsa2-1);
   }
   else {
      /* degenerate case */

      ZZ *T;

      T = stk; stk += hsa + sb - 1;

      /* recursively compute b*a_hi into high part of c */

      KarMul(c + hsa, a + hsa, sa - hsa, b, sb, stk);

      /* recursively compute b*a_lo into T */

      KarMul(T, a, hsa, b, sb, stk);

      KarFix(c, T, hsa + sb - 1, hsa);
   }
}

void KarMul(ZZX& c, const ZZX& a, const ZZX& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(c);
      return;
   }

   if (&a == &b) {
      KarSqr(c, a);
      return;
   }

   vec_ZZ mem;

   const ZZ *ap, *bp;
   ZZ *cp;

   long sa = a.rep.length();
   long sb = b.rep.length();

   if (&a == &c) {
      mem = a.rep;
      ap = mem.elts();
   }
   else
      ap = a.rep.elts();

   if (&b == &c) {
      mem = b.rep;
      bp = mem.elts();
   }
   else
      bp = b.rep.elts();

   c.rep.SetLength(sa+sb-1);
   cp = c.rep.elts();

   long maxa, maxb, xover;

   maxa = MaxBits(a);
   maxb = MaxBits(b);
   xover = 2;

   if (sa < xover || sb < xover)
      PlainMul(cp, ap, sa, bp, sb);
   else {
      /* karatsuba */

      long n, hn, sp, depth;

      n = max(sa, sb);
      sp = 0;
      depth = 0;
      do {
         hn = (n+1) >> 1;
         sp += (hn << 2) - 1;
         n = hn;
         depth++;
      } while (n >= xover);

      ZZVec stk;
      stk.SetSize(sp, 
         ((maxa + maxb + NumBits(min(sa, sb)) + 2*depth + 10) 
          + NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS);

      KarMul(cp, ap, sa, bp, sb, stk.elts());
   }

   c.normalize();
}






void PlainSqr(ZZ* xp, const ZZ* ap, long sa)
{
   if (sa == 0) return;

   long da = sa-1;
   long d = 2*da;

   long i, j, jmin, jmax;
   long m, m2;
   NTL_ZZRegister(t);
   NTL_ZZRegister(accum);

   for (i = 0; i <= d; i++) {
      jmin = max(0, i-da);
      jmax = min(da, i);
      m = jmax - jmin + 1;
      m2 = m >> 1;
      jmax = jmin + m2 - 1;
      clear(accum);
      for (j = jmin; j <= jmax; j++) {
         mul(t, ap[j], ap[i-j]);
         add(accum, accum, t);
      }
      add(accum, accum, accum);
      if (m & 1) {
         sqr(t, ap[jmax + 1]);
         add(accum, accum, t);
      }

      xp[i] = accum;
   }
}


static
void KarSqr(ZZ *c, const ZZ *a, long sa, ZZ *stk)
{
   if (sa == 1) {
      sqr(*c, *a);
      return;
   }

   if (sa == 2) {
      sqr(c[0], a[0]);
      sqr(c[2], a[1]);
      mul(c[1], a[0], a[1]);
      add(c[1], c[1], c[1]);

      return;
   }

   if (sa == 3) {
      sqr(c[0], a[0]);
      mul(c[1], a[0], a[1]);
      add(c[1], c[1], c[1]);
      sqr(stk[0], a[1]);
      mul(c[2], a[0], a[2]);
      add(c[2], c[2], c[2]);
      add(c[2], c[2], stk[0]);
      mul(c[3], a[1], a[2]);
      add(c[3], c[3], c[3]);
      sqr(c[4], a[2]);

      return;
 
   }

   long hsa = (sa + 1) >> 1;
   long hsa2 = hsa << 1;

   ZZ *T1, *T2;

   T1 = stk; stk += hsa;
   T2 = stk; stk += hsa2-1;

   KarFold(T1, a, sa, hsa);
   KarSqr(T2, T1, hsa, stk);


   KarSqr(c + hsa2, a+hsa, sa-hsa, stk);
   KarSub(T2, c + hsa2, sa + sa - hsa2 - 1);


   KarSqr(c, a, hsa, stk);
   KarSub(T2, c, hsa2 - 1);

   clear(c[hsa2 - 1]);

   KarAdd(c+hsa, T2, hsa2-1);
}

      
void KarSqr(ZZX& c, const ZZX& a)
{
   if (IsZero(a)) {
      clear(c);
      return;
   }

   vec_ZZ mem;

   const ZZ *ap;
   ZZ *cp;

   long sa = a.rep.length();

   if (&a == &c) {
      mem = a.rep;
      ap = mem.elts();
   }
   else
      ap = a.rep.elts();

   c.rep.SetLength(sa+sa-1);
   cp = c.rep.elts();

   long maxa, xover;

   maxa = MaxBits(a);

   xover = 2;

   if (sa < xover)
      PlainSqr(cp, ap, sa);
   else {
      /* karatsuba */

      long n, hn, sp, depth;

      n = sa;
      sp = 0;
      depth = 0;
      do {
         hn = (n+1) >> 1;
         sp += hn+hn+hn - 1;
         n = hn;
         depth++;
      } while (n >= xover);

      ZZVec stk;
      stk.SetSize(sp, 
         ((2*maxa + NumBits(sa) + 2*depth + 10) 
          + NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS);

      KarSqr(cp, ap, sa, stk.elts());
   }

   c.normalize();
}


// specialized versions for ZZ_pX, with lower overhead
// for memory allocations...used mainly for testing purposes

void KarSqr(ZZ_pX& c, const ZZ_pX& a)
{
   if (IsZero(a)) {
      clear(c);
      return;
   }


   long sa = a.rep.length();

   long xover = 2;

   if (sa < xover) {
      PlainSqr(c, a);
      return;
   }


   ZZ *ap;
   ZZ *cp;
   
   ZZVec a_elts;
   a_elts.SetSize(sa, ZZ_p::ModulusSize());
   ap = a_elts.elts();
   for (long i = 0; i < sa; i++) ap[i] = rep(a.rep[i]);


   long sc = sa + sa - 1;

   ZZVec c_elts;
   c_elts.SetSize(sc, ZZ_p::ExtendedModulusSize());
   cp = c_elts.elts();


   long maxa, maxb;

   maxa = maxb = NumBits(ZZ_p::modulus());

   long n, hn, sp, depth;

   n = sa;
   sp = 0;
   depth = 0;
   do {
      hn = (n+1) >> 1;
      sp += (hn << 2) - 1;
      n = hn;
      depth++;
   } while (n >= xover);

   ZZVec stk;
   stk.SetSize(sp, 
      ((maxa + maxb + NumBits(sa) + 2*depth + 10) 
       + NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS);

   KarSqr(cp, ap, sa, stk.elts());

   c.rep.SetLength(sc);
   for (long i = 0; i < sc; i++) conv(c.rep[i], cp[i]);
   c.normalize();
}


void KarMul(ZZ_pX& c, const ZZ_pX& a, const ZZ_pX& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(c);
      return;
   }

   if (&a == &b) {
      KarSqr(c, a);
      return;
   }

   long sa = a.rep.length();
   long sb = b.rep.length();

   long xover = 2;

   if (sa < xover || sb < xover) {
      PlainMul(c, a, b);
      return;
   }


   ZZ *ap, *bp;
   ZZ *cp;
   
   ZZVec a_elts;
   a_elts.SetSize(sa, ZZ_p::ModulusSize());
   ap = a_elts.elts();
   for (long i = 0; i < sa; i++) ap[i] = rep(a.rep[i]);

   ZZVec b_elts;
   b_elts.SetSize(sb, ZZ_p::ModulusSize());
   bp = b_elts.elts();
   for (long i = 0; i < sb; i++) bp[i] = rep(b.rep[i]);

   long sc = sa + sb - 1;

   ZZVec c_elts;
   c_elts.SetSize(sc, ZZ_p::ExtendedModulusSize());
   cp = c_elts.elts();


   long maxa, maxb;

   maxa = maxb = NumBits(ZZ_p::modulus());

   long n, hn, sp, depth;

   n = max(sa, sb);
   sp = 0;
   depth = 0;
   do {
      hn = (n+1) >> 1;
      sp += (hn << 2) - 1;
      n = hn;
      depth++;
   } while (n >= xover);

   ZZVec stk;
   stk.SetSize(sp, 
      ((maxa + maxb + NumBits(min(sa, sb)) + 2*depth + 10) 
       + NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS);

   KarMul(cp, ap, sa, bp, sb, stk.elts());

   c.rep.SetLength(sc);
   for (long i = 0; i < sc; i++) conv(c.rep[i], cp[i]);
   c.normalize();
}



NTL_END_IMPL

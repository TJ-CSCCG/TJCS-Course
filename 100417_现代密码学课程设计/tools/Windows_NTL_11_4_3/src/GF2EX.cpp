

#include <NTL/GF2EX.h>
#include <NTL/vec_vec_GF2.h>
#include <NTL/ZZX.h>


NTL_START_IMPL



const GF2EX& GF2EX::zero()
{
   static const GF2EX z; // GLOBAL (assumes C++11 thread-safe init)
   return z;
}



istream& operator>>(istream& s, GF2EX& x)
{
   NTL_INPUT_CHECK_RET(s, s >> x.rep);
   x.normalize();
   return s;
}

ostream& operator<<(ostream& s, const GF2EX& a)
{
   return s << a.rep;
}


void GF2EX::normalize()
{
   long n;
   const GF2E* p;

   n = rep.length();
   if (n == 0) return;
   p = rep.elts() + n;
   while (n > 0 && IsZero(*--p)) {
      n--;
   }
   rep.SetLength(n);
}


long IsZero(const GF2EX& a)
{
   return a.rep.length() == 0;
}


long IsOne(const GF2EX& a)
{
    return a.rep.length() == 1 && IsOne(a.rep[0]);
}

void GetCoeff(GF2E& x, const GF2EX& a, long i)
{
   if (i < 0 || i > deg(a))
      clear(x);
   else
      x = a.rep[i];
}

void SetCoeff(GF2EX& x, long i, const GF2E& a)
{
   long j, m;

   if (i < 0) 
      LogicError("SetCoeff: negative index");

   if (NTL_OVERFLOW(i, 1, 0))
      LogicError("overflow in SetCoeff");

   m = deg(x);

   if (i > m && IsZero(a)) return; 

   if (i > m) {
      /* careful: a may alias a coefficient of x */

      long alloc = x.rep.allocated();

      if (alloc > 0 && i >= alloc) {
         GF2E aa = a;
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

void SetCoeff(GF2EX& x, long i, GF2 a)
{
   if (i < 0)
      LogicError("SetCoeff: negative index");

   if (a == 1)
      SetCoeff(x, i);
   else
      SetCoeff(x, i, GF2E::zero());
}

void SetCoeff(GF2EX& x, long i, long a)
{
   if (i < 0)
      LogicError("SetCoeff: negative index");

   if ((a & 1) == 1)
      SetCoeff(x, i);
   else
      SetCoeff(x, i, GF2E::zero());
}

void SetCoeff(GF2EX& x, long i)
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


void SetX(GF2EX& x)
{
   clear(x);
   SetCoeff(x, 1);
}


long IsX(const GF2EX& a)
{
   return deg(a) == 1 && IsOne(LeadCoeff(a)) && IsZero(ConstTerm(a));
}
      
      

const GF2E& coeff(const GF2EX& a, long i)
{
   if (i < 0 || i > deg(a))
      return GF2E::zero();
   else
      return a.rep[i];
}


const GF2E& LeadCoeff(const GF2EX& a)
{
   if (IsZero(a))
      return GF2E::zero();
   else
      return a.rep[deg(a)];
}

const GF2E& ConstTerm(const GF2EX& a)
{
   if (IsZero(a))
      return GF2E::zero();
   else
      return a.rep[0];
}



void conv(GF2EX& x, const GF2E& a)
{
   if (IsZero(a))
      x.rep.SetLength(0);
   else {
      x.rep.SetLength(1);
      x.rep[0] = a;
   }
}

void conv(GF2EX& x, long a)
{
   if (a & 1)
      set(x);
   else
      clear(x);
}

void conv(GF2EX& x, GF2 a)
{
   if (a == 1)
      set(x);
   else
      clear(x);
}

void conv(GF2EX& x, const ZZ& a)
{
   if (IsOdd(a))
      set(x);
   else
      clear(x);
}

void conv(GF2EX& x, const GF2X& aa)
{
   GF2X a = aa; // in case a aliases the rep of a coefficient of x
   
   long n = deg(a)+1;
   long i;

   x.rep.SetLength(n);
   for (i = 0; i < n; i++)
      conv(x.rep[i], coeff(a, i));
}

void conv(GF2EX& x, const vec_GF2E& a)
{
   x.rep = a;
   x.normalize();
}



/* additional legacy conversions for v6 conversion regime */

void conv(GF2EX& x, const ZZX& a)
{
   long n = a.rep.length();
   long i;

   x.rep.SetLength(n);
   for (i = 0; i < n; i++)
      conv(x.rep[i], a.rep[i]);

   x.normalize();
}


/* ------------------------------------- */




void add(GF2EX& x, const GF2EX& a, const GF2EX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const GF2E *ap, *bp; 
   GF2E* xp;

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

void add(GF2EX& x, const GF2EX& a, const GF2E& b)
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

      GF2E *xp = x.rep.elts();
      add(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const GF2E *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}

void add(GF2EX& x, const GF2EX& a, GF2 b)
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

void add(GF2EX& x, const GF2EX& a, long b)
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


void PlainMul(GF2EX& x, const GF2EX& a, const GF2EX& b)
{
   long da = deg(a);
   long db = deg(b);

   if (da < 0 || db < 0) {
      clear(x);
      return;
   }

   if (&a == &b) {
      sqr(x, a);
      return;
   }

   long d = da+db;

   const GF2E *ap, *bp;
   GF2E *xp;
   
   GF2EX la, lb;

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
   GF2X t, accum;

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


void sqr(GF2EX& x, const GF2EX& a)
{
   long da = deg(a);

   if (da < 0) {
      clear(x);
      return;
   }

   x.rep.SetLength(2*da+1);
   long i;

   for (i = da; i > 0; i--) {
      sqr(x.rep[2*i], a.rep[i]);
      clear(x.rep[2*i-1]);
   }

   sqr(x.rep[0], a.rep[0]);

   x.normalize();
}



static 
void PlainMul1(GF2X *xp, const GF2X *ap, long sa, const GF2X& b)
{
   long i;

   for (i = 0; i < sa; i++)
      mul(xp[i], ap[i], b);
}




static inline
void q_add(GF2X& x, const GF2X& a, const GF2X& b)

// This is a quick-and-dirty add routine used by the karatsuba routine.
// It assumes that the output already has enough space allocated,
// thus avoiding any procedure calls.
// WARNING: it also accesses the underlying WordVector representation
// directly...that is dirty!.
// It shaves a few percent off the running time.

{
   _ntl_ulong *xp = x.xrep.elts();
   const _ntl_ulong *ap = a.xrep.elts();
   const _ntl_ulong *bp = b.xrep.elts();

   long sa = ap[-1];
   long sb = bp[-1];

   long i;

   if (sa == sb) {
      for (i = 0; i < sa; i++)
         xp[i] = ap[i] ^ bp[i];

      i = sa-1;
      while (i >= 0 && !xp[i]) i--;
      xp[-1] = i+1;
   }
   else if (sa < sb) {
      for (i = 0; i < sa; i++)
         xp[i] = ap[i] ^ bp[i];

      for (; i < sb; i++)
         xp[i] = bp[i];

      xp[-1] = sb;
   }
   else { // sa > sb
      for (i = 0; i < sb; i++)
         xp[i] = ap[i] ^ bp[i];

      for (; i < sa; i++)
         xp[i] = ap[i];

      xp[-1] = sa;
   }
}


static inline
void q_copy(GF2X& x, const GF2X& a)
// see comments for q_add above

{
   _ntl_ulong *xp = x.xrep.elts();
   const _ntl_ulong *ap = a.xrep.elts();

   long sa = ap[-1];
   long i;

   for (i = 0; i < sa; i++)
      xp[i] = ap[i];

   xp[-1] = sa;
}



static
void KarFold(GF2X *T, const GF2X *b, long sb, long hsa)
{
   long m = sb - hsa;
   long i;

   for (i = 0; i < m; i++)
      q_add(T[i], b[i], b[hsa+i]);

   for (i = m; i < hsa; i++)
      q_copy(T[i], b[i]);
}


static
void KarAdd(GF2X *T, const GF2X *b, long sb)
{
   long i;

   for (i = 0; i < sb; i++)
      q_add(T[i], T[i], b[i]);
}

static
void KarFix(GF2X *c, const GF2X *b, long sb, long hsa)
{
   long i;

   for (i = 0; i < hsa; i++)
      q_copy(c[i], b[i]);

   for (i = hsa; i < sb; i++)
      q_add(c[i], c[i], b[i]);
}



static
void KarMul(GF2X *c, const GF2X *a, 
            long sa, const GF2X *b, long sb, GF2X *stk)
{
   if (sa < sb) {
      { long t = sa; sa = sb; sb = t; }
      { const GF2X *t = a; a = b; b = t; }
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
      q_add(stk[0], a[0], a[1]);
      q_add(stk[1], b[0], b[1]);
      mul(c[1], stk[0], stk[1]);
      q_add(c[1], c[1], c[0]);
      q_add(c[1], c[1], c[2]);
      
      return;
   }

   long hsa = (sa + 1) >> 1;

   if (hsa < sb) {
      /* normal case */

      long hsa2 = hsa << 1;

      GF2X *T1, *T2, *T3;

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
      KarAdd(T3, c + hsa2, sa + sb - hsa2 - 1);


      /* recursively compute a_lo*b_lo into low part of c */
      /* and subtract from T3 */

      KarMul(c, a, hsa, b, hsa, stk);
      KarAdd(T3, c, hsa2 - 1);

      clear(c[hsa2 - 1]);

      /* finally, add T3 * X^{hsa} to c */

      KarAdd(c+hsa, T3, hsa2-1);
   }
   else {
      /* degenerate case */

      GF2X *T;

      T = stk; stk += hsa + sb - 1;

      /* recursively compute b*a_hi into high part of c */

      KarMul(c + hsa, a + hsa, sa - hsa, b, sb, stk);

      /* recursively compute b*a_lo into T */

      KarMul(T, a, hsa, b, sb, stk);

      KarFix(c, T, hsa + sb - 1, hsa);
   }
}

void ExtractBits(_ntl_ulong *cp, const _ntl_ulong *ap, long k, long n)

// extract k bits from a at position n

{
   long sc = (k + NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG;

   long wn = n/NTL_BITS_PER_LONG;
   long bn = n - wn*NTL_BITS_PER_LONG;

   long i;

   if (bn == 0) {
      for (i = 0; i < sc; i++)
         cp[i] = ap[i+wn];
   }
   else {
      for (i = 0; i < sc-1; i++)
         cp[i] = (ap[i+wn] >> bn) | (ap[i+wn+1] << (NTL_BITS_PER_LONG - bn));

      if (k > sc*NTL_BITS_PER_LONG - bn) 
         cp[sc-1] = (ap[sc+wn-1] >> bn)|(ap[sc+wn] << (NTL_BITS_PER_LONG - bn));
      else
         cp[sc-1] = ap[sc+wn-1] >> bn;
   }

   long p = k % NTL_BITS_PER_LONG;
   if (p != 0) 
      cp[sc-1] &= ((1UL << p) - 1UL);

}


void KronSubst(GF2X& aa, const GF2EX& a)
{
   long sa = a.rep.length();
   long blocksz = 2*GF2E::degree() - 1;

   long saa = sa*blocksz;

   long wsaa = (saa + NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG;

   aa.xrep.SetLength(wsaa+1);

   _ntl_ulong *paa = aa.xrep.elts();


   long i;
   for (i = 0; i < wsaa+1; i++)
      paa[i] = 0;

   for (i = 0; i < sa; i++) 
      ShiftAdd(paa, rep(a.rep[i]).xrep.elts(), rep(a.rep[i]).xrep.length(),
               blocksz*i);

   aa.normalize(); 
}

void KronMul(GF2EX& x, const GF2EX& a, const GF2EX& b)
{
   if (a == 0 || b == 0) {
      clear(x);
      return;
   }

   GF2X aa, bb, xx;

   long sx = deg(a) + deg(b) + 1;
   long blocksz = 2*GF2E::degree() - 1;

   if (NTL_OVERFLOW(blocksz, sx, 0))
      ResourceError("overflow in GF2EX KronMul");

   KronSubst(aa, a);
   KronSubst(bb, b);
   mul(xx, aa, bb);

   GF2X c;

   long wc = (blocksz + NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG;

   x.rep.SetLength(sx);

   long i;
   for (i = 0; i < sx-1; i++) {
      c.xrep.SetLength(wc);
      ExtractBits(c.xrep.elts(), xx.xrep.elts(), blocksz, i*blocksz);
      c.normalize();
      conv(x.rep[i], c);
   }

   long last_blocksz = deg(xx) - (sx-1)*blocksz + 1;
   wc = (last_blocksz + NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG;
   c.xrep.SetLength(wc);

   ExtractBits(c.xrep.elts(), xx.xrep.elts(), last_blocksz, (sx-1)*blocksz);
   c.normalize();
   conv(x.rep[sx-1], c);

   x.normalize();
}



void mul(GF2EX& c, const GF2EX& a, const GF2EX& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(c);
      return;
   }

   if (&a == &b) {
      sqr(c, a);
      return;
   }

   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sa == 1) {
      mul(c, b, a.rep[0]);
      return;
   }

   if (sb == 1) {
      mul(c, a, b.rep[0]);
      return;
   }

   if (sa < GF2E::KarCross() || sb < GF2E::KarCross()) {
      PlainMul(c, a, b);
      return;
   }

   bool use_kron_mul = false;

   if (GF2E::WordLength() <= 1) use_kron_mul = true;

#if (defined(NTL_GF2X_LIB) && defined(NTL_HAVE_PCLMUL))
   // With gf2x library and pclmul, KronMul is better in a larger range, but
   // it is very hard to characterize that range.  The following is very
   // conservative.

   if (GF2E::WordLength() <= 4 && sa >= 50 && sb >= 50) use_kron_mul = true;
   // FIXME: figure out a larger range where KronMul is better
   // (and don't forget to recompute crossovers in GF2E.cpp).
#endif


   if (use_kron_mul) {
      KronMul(c, a, b);
      return;
   }
   

   /* karatsuba */

   long n, hn, sp;

   n = max(sa, sb);
   sp = 0;
   do {
      hn = (n+1) >> 1;
      sp += (hn << 2) - 1;
      n = hn;
   } while (n > 1);

   GF2XVec stk;
   stk.SetSize(sp + 2*(sa+sb)-1, 2*GF2E::WordLength()); 

   long i;

   for (i = 0; i < sa; i++)
      stk[i+sa+sb-1] = rep(a.rep[i]);

   for (i = 0; i < sb; i++)
      stk[i+2*sa+sb-1] = rep(b.rep[i]);

   KarMul(&stk[0], &stk[sa+sb-1], sa, &stk[2*sa+sb-1], sb, 
          &stk[2*(sa+sb)-1]);

   c.rep.SetLength(sa+sb-1);

   for (i = 0; i < sa+sb-1; i++)
      conv(c.rep[i], stk[i]);

   c.normalize();
}



#if 0
// used only for computing KarCross using GF2EXKarCross.cpp
void mul_disable_plain(GF2EX& c, const GF2EX& a, const GF2EX& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(c);
      return;
   }

   if (&a == &b) {
      sqr(c, a);
      return;
   }

   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sa == 1) {
      mul(c, b, a.rep[0]);
      return;
   }

   if (sb == 1) {
      mul(c, a, b.rep[0]);
      return;
   }

   if (0) {
   //if (sa < GF2E::KarCross() || sb < GF2E::KarCross()) {
      PlainMul(c, a, b);
      return;
   }

   if (GF2E::WordLength() <= 1) {
      KronMul(c, a, b);
      return;
   }
   

   /* karatsuba */

   long n, hn, sp;

   n = max(sa, sb);
   sp = 0;
   do {
      hn = (n+1) >> 1;
      sp += (hn << 2) - 1;
      n = hn;
   } while (n > 1);

   GF2XVec stk;
   stk.SetSize(sp + 2*(sa+sb)-1, 2*GF2E::WordLength()); 

   long i;

   for (i = 0; i < sa; i++)
      stk[i+sa+sb-1] = rep(a.rep[i]);

   for (i = 0; i < sb; i++)
      stk[i+2*sa+sb-1] = rep(b.rep[i]);

   KarMul(&stk[0], &stk[sa+sb-1], sa, &stk[2*sa+sb-1], sb, 
          &stk[2*(sa+sb)-1]);

   c.rep.SetLength(sa+sb-1);

   for (i = 0; i < sa+sb-1; i++)
      conv(c.rep[i], stk[i]);

   c.normalize();
}
#endif





void MulTrunc(GF2EX& x, const GF2EX& a, const GF2EX& b, long n)
{
   GF2EX t;
   mul(t, a, b);
   trunc(x, t, n);
}

void SqrTrunc(GF2EX& x, const GF2EX& a, long n)
{
   GF2EX t;
   sqr(t, a);
   trunc(x, t, n);
}



void PlainDivRem(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const GF2E *bp;
   GF2E *qp;
   GF2X *xp;


   GF2E LCInv, t;
   GF2X s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("GF2EX: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   GF2EX lb;

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

   GF2XVec x(da + 1, 2*GF2E::WordLength());

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


void PlainRem(GF2EX& r, const GF2EX& a, const GF2EX& b, GF2XVec& x)
{
   long da, db, dq, i, j, LCIsOne;
   const GF2E *bp;
   GF2X *xp;


   GF2E LCInv, t;
   GF2X s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("GF2EX: division by zero");

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


void PlainDivRem(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EX& b, GF2XVec& x)
{
   long da, db, dq, i, j, LCIsOne;
   const GF2E *bp;
   GF2E *qp;
   GF2X *xp;


   GF2E LCInv, t;
   GF2X s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("GF2EX: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   GF2EX lb;

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


void PlainDiv(GF2EX& q, const GF2EX& a, const GF2EX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const GF2E *bp;
   GF2E *qp;
   GF2X *xp;


   GF2E LCInv, t;
   GF2X s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("GF2EX: division by zero");

   if (da < db) {
      clear(q);
      return;
   }

   GF2EX lb;

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

   GF2XVec x(da + 1 - db, 2*GF2E::WordLength());

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

      long lastj = max(0, db-i);

      for (j = db-1; j >= lastj; j--) {
	 mul(s, rep(t), rep(bp[j]));
	 add(xp[i+j-db], xp[i+j-db], s);
      }
   }
}

void PlainRem(GF2EX& r, const GF2EX& a, const GF2EX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const GF2E *bp;
   GF2X *xp;


   GF2E LCInv, t;
   GF2X s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("GF2EX: division by zero");

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

   GF2XVec x(da + 1, 2*GF2E::WordLength());

   for (i = 0; i <= da; i++)
      x[i] = rep(a.rep[i]);

   xp = x.elts();

   dq = da - db;

   for (i = dq; i >= 0; i--) {
      conv(t, xp[i+db]);
      if (!LCIsOne)
	 mul(t, t, LCInv);

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

void mul(GF2EX& x, const GF2EX& a, const GF2E& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(x);
      return;
   }

   GF2X bb, t;
   long i, da;

   const GF2E *ap;
   GF2E* xp;

   bb = rep(b);
   da = deg(a);
   x.rep.SetLength(da+1);
   ap = a.rep.elts();
   xp = x.rep.elts();

   for (i = 0; i <= da; i++) {
      mul(t, rep(ap[i]), bb);
      conv(xp[i], t);
   }

   x.normalize();
}

void mul(GF2EX& x, const GF2EX& a, GF2 b)
{
   if (b == 0)
      clear(x);
   else
      x = a;
}

void mul(GF2EX& x, const GF2EX& a, long b)
{
   if ((b & 1) == 0)
      clear(x);
   else
      x = a;
}


void PlainGCD(GF2EX& x, const GF2EX& a, const GF2EX& b)
{
   GF2E t;

   if (IsZero(b))
      x = a;
   else if (IsZero(a))
      x = b;
   else {
      long n = max(deg(a),deg(b)) + 1;
      GF2EX u(INIT_SIZE, n), v(INIT_SIZE, n);
      GF2XVec tmp(n, 2*GF2E::WordLength());

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

class _NTL_GF2EXMatrix {
private:

   _NTL_GF2EXMatrix(const _NTL_GF2EXMatrix&);  // disable
   GF2EX elts[2][2];

public:

   _NTL_GF2EXMatrix() { }
   ~_NTL_GF2EXMatrix() { }

   void operator=(const _NTL_GF2EXMatrix&);
   GF2EX& operator() (long i, long j) { return elts[i][j]; }
   const GF2EX& operator() (long i, long j) const { return elts[i][j]; }
};


void _NTL_GF2EXMatrix::operator=(const _NTL_GF2EXMatrix& M)
{
   elts[0][0] = M.elts[0][0];
   elts[0][1] = M.elts[0][1];
   elts[1][0] = M.elts[1][0];
   elts[1][1] = M.elts[1][1];
}


static
void mul(GF2EX& U, GF2EX& V, const _NTL_GF2EXMatrix& M)
// (U, V)^T = M*(U, V)^T
{
   GF2EX t1, t2, t3;

   mul(t1, M(0,0), U);
   mul(t2, M(0,1), V);
   add(t3, t1, t2);
   mul(t1, M(1,0), U);
   mul(t2, M(1,1), V);
   add(V, t1, t2);
   U = t3;
}


static
void mul(_NTL_GF2EXMatrix& A, _NTL_GF2EXMatrix& B, _NTL_GF2EXMatrix& C)
// A = B*C, B and C are destroyed
{
   GF2EX t1, t2;

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


void IterHalfGCD(_NTL_GF2EXMatrix& M_out, GF2EX& U, GF2EX& V, long d_red)
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

   GF2EX Q, t(INIT_SIZE, d_red);

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


#define NTL_GF2EX_HalfGCD_CROSSOVER (40)


void HalfGCD(_NTL_GF2EXMatrix& M_out, const GF2EX& U, const GF2EX& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));

      return;
   }


   long n = deg(U) - 2*d_red + 2;
   if (n < 0) n = 0;

   GF2EX U1, V1;

   RightShift(U1, U, n);
   RightShift(V1, V, n);

   if (d_red <= NTL_GF2EX_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U1, V1, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   _NTL_GF2EXMatrix M1;

   HalfGCD(M1, U1, V1, d1);
   mul(U1, V1, M1);

   long d2 = deg(V1) - deg(U) + n + d_red;

   if (IsZero(V1) || d2 <= 0) {
      M_out = M1;
      return;
   }


   GF2EX Q;
   _NTL_GF2EXMatrix M2;

   DivRem(Q, U1, U1, V1);
   swap(U1, V1);

   HalfGCD(M2, U1, V1, d2);

   GF2EX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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

void XHalfGCD(_NTL_GF2EXMatrix& M_out, GF2EX& U, GF2EX& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));

      return;
   }

   long du = deg(U);

   if (d_red <= NTL_GF2EX_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U, V, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   //ZZ_pXMatrix M1;
   _NTL_GF2EXMatrix M1;

   HalfGCD(M1, U, V, d1);
   mul(U, V, M1);

   long d2 = deg(V) - du + d_red;

   if (IsZero(V) || d2 <= 0) {
      M_out = M1;
      return;
   }


   GF2EX Q;
   _NTL_GF2EXMatrix M2;

   DivRem(Q, U, U, V);
   swap(U, V);

   XHalfGCD(M2, U, V, d2);

   GF2EX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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

void HalfGCD(GF2EX& U, GF2EX& V)
{
   long d_red = (deg(U)+1)/2;

   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      return;
   }

   long du = deg(U);


   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   _NTL_GF2EXMatrix M1;

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


   GF2EX Q;

   DivRem(Q, U, U, V);
   swap(U, V);

   HalfGCD(M1, U, V, d2);

   mul(U, V, M1);
}


void GCD(GF2EX& d, const GF2EX& u, const GF2EX& v)
{
   GF2EX u1, v1;

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

   while (deg(u1) >= GF2E::GCDCross() && !IsZero(v1)) {
      HalfGCD(u1, v1);

      if (!IsZero(v1)) {
         rem(u1, u1, v1);
         swap(u1, v1);
      }
   }

   PlainGCD(d, u1, v1);
}


         


void XGCD(GF2EX& d, GF2EX& s, GF2EX& t, const GF2EX& a, const GF2EX& b)
{
   GF2E w;

   if (IsZero(a) && IsZero(b)) {
      clear(d);
      set(s);
      clear(t);
      return;
   }

   GF2EX U, V, Q;

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

   _NTL_GF2EXMatrix M;

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


void MulMod(GF2EX& x, const GF2EX& a, const GF2EX& b, const GF2EX& f)
{
   if (deg(a) >= deg(f) || deg(b) >= deg(f) || deg(f) == 0) 
      LogicError("MulMod: bad args");

   GF2EX t;

   mul(t, a, b);
   rem(x, t, f);
}

void SqrMod(GF2EX& x, const GF2EX& a, const GF2EX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("SqrMod: bad args");

   GF2EX t;

   sqr(t, a);
   rem(x, t, f);
}


void InvMod(GF2EX& x, const GF2EX& a, const GF2EX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvMod: bad args");

   GF2EX d, xx, t;

   XGCD(d, xx, t, a, f);
   if (!IsOne(d))
      InvModError("GF2EX InvMod: can't compute multiplicative inverse");

   x = xx;
}

long InvModStatus(GF2EX& x, const GF2EX& a, const GF2EX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvModStatus: bad args");

   GF2EX d, t;

   XGCD(d, x, t, a, f);
   if (!IsOne(d)) {
      x = d;
      return 1;
   }
   else
      return 0;
}




static
void MulByXModAux(GF2EX& h, const GF2EX& a, const GF2EX& f)
{
   long i, n, m;
   GF2E* hh;
   const GF2E *aa, *ff;

   GF2E t, z;

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
      z = aa[n-1];
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

void MulByXMod(GF2EX& h, const GF2EX& a, const GF2EX& f)
{
   if (&h == &f) {
      GF2EX hh;
      MulByXModAux(hh, a, f);
      h = hh;
   }
   else
      MulByXModAux(h, a, f);
}




void random(GF2EX& x, long n)
{
   long i;

   x.rep.SetLength(n);

   for (i = 0; i < n; i++)
      random(x.rep[i]); 

   x.normalize();
}


void CopyReverse(GF2EX& x, const GF2EX& a, long hi)

   // x[0..hi] = reverse(a[0..hi]), with zero fill
   // input may not alias output

{
   long i, j, n, m;

   n = hi+1;
   m = a.rep.length();

   x.rep.SetLength(n);

   const GF2E* ap = a.rep.elts();
   GF2E* xp = x.rep.elts();

   for (i = 0; i < n; i++) {
      j = hi-i;
      if (j < 0 || j >= m)
         clear(xp[i]);
      else
         xp[i] = ap[j];
   }

   x.normalize();
} 



void trunc(GF2EX& x, const GF2EX& a, long m)

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
      GF2E* xp;
      const GF2E* ap;

      n = min(a.rep.length(), m);
      x.rep.SetLength(n);

      xp = x.rep.elts();
      ap = a.rep.elts();

      for (i = 0; i < n; i++) xp[i] = ap[i];

      x.normalize();
   }
}

void NewtonInvTrunc(GF2EX& c, const GF2EX& a, long e)
{
   GF2E x;

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

   GF2EX g, g0, g1, g2;


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

      add(g, g, g2);
   }

   c = g;
}


void InvTrunc(GF2EX& c, const GF2EX& a, long e)
{
   if (e < 0) LogicError("InvTrunc: bad args");
   if (e == 0) {
      clear(c);
      return;
   }

   if (NTL_OVERFLOW(e, 1, 0))
      ResourceError("overflow in InvTrunc");

   NewtonInvTrunc(c, a, e);
}



const long GF2EX_MOD_PLAIN = 0;
const long GF2EX_MOD_MUL = 1;

void build(GF2EXModulus& F, const GF2EX& f)
{
   long n = deg(f);

   if (n <= 0) LogicError("build(GF2EXModulus,GF2EX): deg(f) <= 0");

   if (NTL_OVERFLOW(n, GF2E::degree(), 0))
      ResourceError("build(GF2EXModulus,GF2EX): overflow");

   F.tracevec.make();

   F.f = f;
   F.n = n;

   if (F.n < GF2E::ModCross()) {
      F.method = GF2EX_MOD_PLAIN;
   }
   else {
      F.method = GF2EX_MOD_MUL;
      GF2EX P1;
      GF2EX P2;

      CopyReverse(P1, f, n);
      InvTrunc(P2, P1, n-1);
      CopyReverse(P1, P2, n-2);
      trunc(F.h0, P1, n-2);
      trunc(F.f0, f, n);
      F.hlc = ConstTerm(P2);
   }
}


#if 0
// used only for computing ModCross using GF2EXModCross.cpp
void BuildPlain(GF2EXModulus& F, const GF2EX& f, bool plain)
{
   long n = deg(f);

   if (n <= 0) LogicError("build(GF2EXModulus,GF2EX): deg(f) <= 0");

   if (NTL_OVERFLOW(n, GF2E::degree(), 0))
      ResourceError("build(GF2EXModulus,GF2EX): overflow");

   F.tracevec.make();

   F.f = f;
   F.n = n;

   if (plain) {
      F.method = GF2EX_MOD_PLAIN;
   }
   else {
      F.method = GF2EX_MOD_MUL;
      GF2EX P1;
      GF2EX P2;

      CopyReverse(P1, f, n);
      InvTrunc(P2, P1, n-1);
      CopyReverse(P1, P2, n-2);
      trunc(F.h0, P1, n-2);
      trunc(F.f0, f, n);
      F.hlc = ConstTerm(P2);
   }
}
#endif

GF2EXModulus::GF2EXModulus()
{
   n = -1;
   method = GF2EX_MOD_PLAIN;
}



GF2EXModulus::GF2EXModulus(const GF2EX& ff)
{
   n = -1;
   method = GF2EX_MOD_PLAIN;

   build(*this, ff);
}





void UseMulRem21(GF2EX& r, const GF2EX& a, const GF2EXModulus& F)
{
   GF2EX P1;
   GF2EX P2;

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   if (!IsOne(F.hlc)) mul(P1, P1, F.hlc);
   add(P2, P2, P1);
   mul(P1, P2, F.f0);
   trunc(P1, P1, F.n);
   trunc(r, a, F.n);
   add(r, r, P1);
}

void UseMulDivRem21(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EXModulus& F)
{
   GF2EX P1;
   GF2EX P2;

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   if (!IsOne(F.hlc)) mul(P1, P1, F.hlc);
   add(P2, P2, P1);
   mul(P1, P2, F.f0);
   trunc(P1, P1, F.n);
   trunc(r, a, F.n);
   add(r, r, P1);
   q = P2;
}

void UseMulDiv21(GF2EX& q, const GF2EX& a, const GF2EXModulus& F)
{
   GF2EX P1;
   GF2EX P2;

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   if (!IsOne(F.hlc)) mul(P1, P1, F.hlc);
   add(P2, P2, P1);
   q = P2;

}

void rem(GF2EX& x, const GF2EX& a, const GF2EXModulus& F)
{
   if (F.method == GF2EX_MOD_PLAIN) {
      PlainRem(x, a, F.f);
      return;
   }

   long da = deg(a);
   long n = F.n;

   if (da <= 2*n-2) {
      UseMulRem21(x, a, F);
      return;
   }

   GF2EX buf(INIT_SIZE, 2*n-1);

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

void DivRem(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EXModulus& F)
{
   if (F.method == GF2EX_MOD_PLAIN) {
      PlainDivRem(q, r, a, F.f);
      return;
   }

   long da = deg(a);
   long n = F.n;

   if (da <= 2*n-2) {
      UseMulDivRem21(q, r, a, F);
      return;
   }

   GF2EX buf(INIT_SIZE, 2*n-1);
   GF2EX qbuf(INIT_SIZE, n-1);

   GF2EX qq;
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

void div(GF2EX& q, const GF2EX& a, const GF2EXModulus& F)
{
   if (F.method == GF2EX_MOD_PLAIN) {
      PlainDiv(q, a, F.f);
      return;
   }

   long da = deg(a);
   long n = F.n;

   if (da <= 2*n-2) {
      UseMulDiv21(q, a, F);
      return;
   }

   GF2EX buf(INIT_SIZE, 2*n-1);
   GF2EX qbuf(INIT_SIZE, n-1);

   GF2EX qq;
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




void MulMod(GF2EX& c, const GF2EX& a, const GF2EX& b, const GF2EXModulus& F)
{
   if (deg(a) >= F.n || deg(b) >= F.n) LogicError("MulMod: bad args");

   GF2EX t;
   mul(t, a, b);
   rem(c, t, F);
}


void SqrMod(GF2EX& c, const GF2EX& a, const GF2EXModulus& F)
{
   if (deg(a) >= F.n) LogicError("MulMod: bad args");

   GF2EX t;
   sqr(t, a);
   rem(c, t, F);
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
      


void PowerMod(GF2EX& h, const GF2EX& g, const ZZ& e, const GF2EXModulus& F)
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

   GF2EX res;
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
   k = min(k, 5);

   vec_GF2EX v;

   v.SetLength(1L << (k-1));

   v[0] = g;
 
   if (k > 1) {
      GF2EX t;
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

   


void PowerXMod(GF2EX& hh, const ZZ& e, const GF2EXModulus& F)
{
   if (F.n < 0) LogicError("PowerXMod: uninitialized modulus");

   if (IsZero(e)) {
      set(hh);
      return;
   }

   long n = NumBits(e);
   long i;

   GF2EX h;

   h.SetMaxLength(F.n+1);
   set(h);

   for (i = n - 1; i >= 0; i--) {
      SqrMod(h, h, F);
      if (bit(e, i)) {
         MulByXMod(h, h, F.f);
      }
   }

   if (e < 0) InvMod(h, h, F);

   hh = h;
}


      


void UseMulRem(GF2EX& r, const GF2EX& a, const GF2EX& b)
{
   GF2EX P1;
   GF2EX P2;

   long da = deg(a);
   long db = deg(b);

   CopyReverse(P1, b, db);
   InvTrunc(P2, P1, da-db+1);
   CopyReverse(P1, P2, da-db);

   RightShift(P2, a, db);
   mul(P2, P1, P2);
   RightShift(P2, P2, da-db);
   mul(P1, P2, b);
   add(P1, P1, a);
   
   r = P1;
}

void UseMulDivRem(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EX& b)
{
   GF2EX P1;
   GF2EX P2;

   long da = deg(a);
   long db = deg(b);

   CopyReverse(P1, b, db);
   InvTrunc(P2, P1, da-db+1);
   CopyReverse(P1, P2, da-db);

   RightShift(P2, a, db);
   mul(P2, P1, P2);
   RightShift(P2, P2, da-db);
   mul(P1, P2, b);
   add(P1, P1, a);
   
   r = P1;
   q = P2;
}

void UseMulDiv(GF2EX& q, const GF2EX& a, const GF2EX& b)
{
   GF2EX P1;
   GF2EX P2;

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



void DivRem(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EX& b)
{
   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sb < GF2E::DivCross() || sa-sb < GF2E::DivCross())
      PlainDivRem(q, r, a, b);
   else if (sa < 4*sb)
      UseMulDivRem(q, r, a, b);
   else {
      GF2EXModulus B;
      build(B, b);
      DivRem(q, r, a, B);
   }
}

#if 0
// used only for computing DivCross using GF2EXDivCross.cpp
void DivRemPlain(GF2EX& q, GF2EX& r, const GF2EX& a, const GF2EX& b, bool plain)
{
   long sa = a.rep.length();
   long sb = b.rep.length();

   if (plain)
      PlainDivRem(q, r, a, b);
   else if (sa < 4*sb)
      UseMulDivRem(q, r, a, b);
   else {
      GF2EXModulus B;
      build(B, b);
      DivRem(q, r, a, B);
   }
}
#endif

void div(GF2EX& q, const GF2EX& a, const GF2EX& b)
{
   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sb < GF2E::DivCross() || sa-sb < GF2E::DivCross())
      PlainDiv(q, a, b);
   else if (sa < 4*sb)
      UseMulDiv(q, a, b);
   else {
      GF2EXModulus B;
      build(B, b);
      div(q, a, B);
   }
}

void div(GF2EX& q, const GF2EX& a, const GF2E& b)
{
   GF2E t;
   inv(t, b);
   mul(q, a, t);
}

void div(GF2EX& q, const GF2EX& a, GF2 b)
{
   if (b == 0)
      ArithmeticError("div: division by zero");

   q = a;
}

void div(GF2EX& q, const GF2EX& a, long b)
{
   if ((b & 1) == 0)
      ArithmeticError("div: division by zero");

   q = a;
}
   


void rem(GF2EX& r, const GF2EX& a, const GF2EX& b)
{
   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sb < GF2E::DivCross() || sa-sb < GF2E::DivCross())
      PlainRem(r, a, b);
   else if (sa < 4*sb)
      UseMulRem(r, a, b);
   else {
      GF2EXModulus B;
      build(B, b);
      rem(r, a, B);
   }
}


void diff(GF2EX& x, const GF2EX& a)
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
      if ((i+1)&1)
         x.rep[i] = a.rep[i+1];
      else
         clear(x.rep[i]);
   }

   if (&x == &a)
      x.rep.SetLength(n);

   x.normalize();
}


void RightShift(GF2EX& x, const GF2EX& a, long n)
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

void LeftShift(GF2EX& x, const GF2EX& a, long n)
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


void ShiftAdd(GF2EX& U, const GF2EX& V, long n)
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
      add(U.rep[i+n], U.rep[i+n], V.rep[i]);

   U.normalize();
}


void IterBuild(GF2E* a, long n)
{
   long i, k;
   GF2E b, t;

   if (n <= 0) return;

   for (k = 1; k <= n-1; k++) {
      b = a[k];
      add(a[k], b, a[k-1]);
      for (i = k-1; i >= 1; i--) {
         mul(t, a[i], b);
         add(a[i], t, a[i-1]);
      }
      mul(a[0], a[0], b);
   }
} 



void BuildFromRoots(GF2EX& x, const vec_GF2E& a)
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



void eval(GF2E& b, const GF2EX& f, const GF2E& a)
// does a Horner evaluation
{
   GF2E acc;
   long i;

   clear(acc);
   for (i = deg(f); i >= 0; i--) {
      mul(acc, acc, a);
      add(acc, acc, f.rep[i]);
   }

   b = acc;
}



void eval(vec_GF2E& b, const GF2EX& f, const vec_GF2E& a)
// naive algorithm:  repeats Horner
{
   if (&b == &f.rep) {
      vec_GF2E bb;
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




void interpolate(GF2EX& f, const vec_GF2E& a, const vec_GF2E& b)
{
   long m = a.length();
   if (b.length() != m) LogicError("interpolate: vector length mismatch");

   if (m == 0) {
      clear(f);
      return;
   }

   vec_GF2E prod;
   prod = a;

   GF2E t1, t2;

   long k, i;

   vec_GF2E res;
   res.SetLength(m);

   for (k = 0; k < m; k++) {

      const GF2E& aa = a[k];

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

   
void InnerProduct(GF2EX& x, const vec_GF2E& v, long low, long high, 
                   const vec_GF2EX& H, long n, GF2XVec& t)
{
   GF2X s;
   long i, j;

   for (j = 0; j < n; j++)
      clear(t[j]);

   high = min(high, v.length()-1);
   for (i = low; i <= high; i++) {
      const vec_GF2E& h = H[i-low].rep;
      long m = h.length();
      const GF2X& w = rep(v[i]);

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


void CompMod(GF2EX& x, const GF2EX& g, const GF2EXArgument& A, 
             const GF2EXModulus& F)
{
   if (deg(g) <= 0) {
      x = g;
      return;
   }


   GF2EX s, t;
   GF2XVec scratch(F.n, 2*GF2E::WordLength());

   long m = A.H.length() - 1;
   long l = ((g.rep.length()+m-1)/m) - 1;

   const GF2EX& M = A.H[m];

   InnerProduct(t, g.rep, l*m, l*m + m - 1, A.H, F.n, scratch);
   for (long i = l-1; i >= 0; i--) {
      InnerProduct(s, g.rep, i*m, i*m + m - 1, A.H, F.n, scratch);
      MulMod(t, t, M, F);
      add(t, t, s);
   }

   x = t;
}


void build(GF2EXArgument& A, const GF2EX& h, const GF2EXModulus& F, long m)
{
   long i;

   if (m <= 0 || deg(h) >= F.n)
      LogicError("build GF2EXArgument: bad args");

   if (m > F.n) m = F.n;

   if (GF2EXArgBound > 0) {
      double sz = GF2E::storage();
      sz = sz*F.n;
      sz = sz + NTL_VECTOR_HEADER_SIZE + sizeof(vec_GF2E);
      sz = sz/1024;
      m = min(m, long(GF2EXArgBound/sz));
      m = max(m, 1);
   }

   A.H.SetLength(m+1);

   set(A.H[0]);
   A.H[1] = h;
   for (i = 2; i <= m; i++) 
      MulMod(A.H[i], A.H[i-1], h, F);
}




NTL_CHEAP_THREAD_LOCAL
long GF2EXArgBound = 0;


void CompMod(GF2EX& x, const GF2EX& g, const GF2EX& h, const GF2EXModulus& F)
   // x = g(h) mod f
{
   long m = SqrRoot(g.rep.length());

   if (m == 0) {
      clear(x);
      return;
   }

   GF2EXArgument A;

   build(A, h, F, m);

   CompMod(x, g, A, F);
}




void Comp2Mod(GF2EX& x1, GF2EX& x2, const GF2EX& g1, const GF2EX& g2,
              const GF2EX& h, const GF2EXModulus& F)

{
   long m = SqrRoot(g1.rep.length() + g2.rep.length());

   if (m == 0) {
      clear(x1);
      clear(x2);
      return;
   }

   GF2EXArgument A;

   build(A, h, F, m);

   GF2EX xx1, xx2;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);

   x1 = xx1;
   x2 = xx2;
}

void Comp3Mod(GF2EX& x1, GF2EX& x2, GF2EX& x3, 
              const GF2EX& g1, const GF2EX& g2, const GF2EX& g3,
              const GF2EX& h, const GF2EXModulus& F)

{
   long m = SqrRoot(g1.rep.length() + g2.rep.length() + g3.rep.length());

   if (m == 0) {
      clear(x1);
      clear(x2);
      clear(x3);
      return;
   }

   GF2EXArgument A;

   build(A, h, F, m);

   GF2EX xx1, xx2, xx3;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);
   CompMod(xx3, g3, A, F);

   x1 = xx1;
   x2 = xx2;
   x3 = xx3;
}





void build(GF2EXTransMultiplier& B, const GF2EX& b, const GF2EXModulus& F)
{
   long db = deg(b);

   if (db >= F.n) LogicError("build TransMultiplier: bad args");

   GF2EX t;

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

void TransMulMod(GF2EX& x, const GF2EX& a, const GF2EXTransMultiplier& B,
               const GF2EXModulus& F)
{
   if (deg(a) >= F.n) LogicError("TransMulMod: bad args");

   GF2EX t1, t2;

   mul(t1, a, B.b);
   RightShift(t1, t1, B.shamt_b);

   mul(t2, a, B.f0);
   RightShift(t2, t2, B.shamt);
   trunc(t2, t2, F.n-1);

   mul(t2, t2, B.fbi);
   if (B.shamt_fbi > 0) LeftShift(t2, t2, B.shamt_fbi);
   trunc(t2, t2, F.n-1);
   LeftShift(t2, t2, 1);

   add(x, t1, t2);
}


void UpdateMap(vec_GF2E& x, const vec_GF2E& a, 
         const GF2EXTransMultiplier& B, const GF2EXModulus& F)
{
   GF2EX xx;
   TransMulMod(xx, to_GF2EX(a), B, F);
   x = xx.rep;
}
   


static
void ProjectPowers(vec_GF2E& x, const GF2EX& a, long k, 
                   const GF2EXArgument& H, const GF2EXModulus& F)
{
   if (k < 0 || deg(a) >= F.n) 
      LogicError("ProjectPowers: bad args");

   if (NTL_OVERFLOW(k, 1, 0)) 
      ResourceError("ProjectPowers: excessive args");

   long m = H.H.length()-1;
   long l = (k+m-1)/m - 1;

   GF2EXTransMultiplier M;
   build(M, H.H[m], F);

   GF2EX s;
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
void ProjectPowers(vec_GF2E& x, const GF2EX& a, long k, const GF2EX& h, 
                   const GF2EXModulus& F)
{
   if (k < 0 || deg(a) >= F.n || deg(h) >= F.n)
      LogicError("ProjectPowers: bad args");

   if (k == 0) {
      x.SetLength(0);;
      return;
   }

   long m = SqrRoot(k);

   GF2EXArgument H;
   build(H, h, F, m);

   ProjectPowers(x, a, k, H, F);
}

void ProjectPowers(vec_GF2E& x, const vec_GF2E& a, long k,
                   const GF2EXArgument& H, const GF2EXModulus& F)
{
   ProjectPowers(x, to_GF2EX(a), k, H, F);
}

void ProjectPowers(vec_GF2E& x, const vec_GF2E& a, long k, 
                   const GF2EX& h, const GF2EXModulus& F)
{
   ProjectPowers(x, to_GF2EX(a), k, h, F);
}




void BerlekampMassey(GF2EX& h, const vec_GF2E& a, long m)
{
   GF2EX Lambda, Sigma, Temp;
   long L;
   GF2E Delta, Delta1, t1;
   long shamt;
   GF2X tt1, tt2;

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
      clear(tt1);
      dl = deg(Lambda);
      for (i = 0; i <= dl; i++) {
         mul(tt2, rep(Lambda.rep[i]), rep(a[r-i-1]));
         add(tt1, tt1, tt2);
      }

      conv(Delta1, tt1);

      if (IsZero(Delta1)) {
         shamt++;
         // cerr << "case 1: " << deg(Lambda) << " " << deg(Sigma) << " " << shamt << "\n";
      }
      else if (2*L < r) {
         div(t1, Delta1, Delta);
         mul(Temp, Sigma, t1);
         Sigma = Lambda;
         ShiftAdd(Lambda, Temp, shamt+1);
         shamt = 0;
         L = r-L;
         Delta = Delta1;
         // cerr << "case 2: " << deg(Lambda) << " " << deg(Sigma) << " " << shamt << "\n";
      }
      else {
         shamt++;
         div(t1, Delta1, Delta);
         mul(Temp, Sigma, t1);
         ShiftAdd(Lambda, Temp, shamt);
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


void MinPolySeq(GF2EX& h, const vec_GF2E& a, long m)
{
   if (m < 0 || NTL_OVERFLOW(m, 1, 0)) LogicError("MinPoly: bad args");
   if (a.length() < 2*m) LogicError("MinPoly: sequence too short");

   BerlekampMassey(h, a, m);
}


void DoMinPolyMod(GF2EX& h, const GF2EX& g, const GF2EXModulus& F, long m, 
               const GF2EX& R)
{
   vec_GF2E x;

   ProjectPowers(x, R, 2*m, g, F);
   MinPolySeq(h, x, m);
}

void ProbMinPolyMod(GF2EX& h, const GF2EX& g, const GF2EXModulus& F, long m)
{
   long n = F.n;
   if (m < 1 || m > n) LogicError("ProbMinPoly: bad args");

   GF2EX R;
   random(R, n);

   DoMinPolyMod(h, g, F, m, R);
}

void ProbMinPolyMod(GF2EX& h, const GF2EX& g, const GF2EXModulus& F)
{
   ProbMinPolyMod(h, g, F, F.n);
}

void MinPolyMod(GF2EX& hh, const GF2EX& g, const GF2EXModulus& F, long m)
{
   GF2EX h, h1;
   long n = F.n;
   if (m < 1 || m > n) LogicError("MinPoly: bad args");

   /* probabilistically compute min-poly */

   ProbMinPolyMod(h, g, F, m);
   if (deg(h) == m) { hh = h; return; }
   CompMod(h1, h, g, F);
   if (IsZero(h1)) { hh = h; return; }

   /* not completely successful...must iterate */


   GF2EX h2, h3;
   GF2EX R;
   GF2EXTransMultiplier H1;
   

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

void IrredPolyMod(GF2EX& h, const GF2EX& g, const GF2EXModulus& F, long m)
{
   if (m < 1 || m > F.n) LogicError("IrredPoly: bad args");

   GF2EX R;
   set(R);

   DoMinPolyMod(h, g, F, m, R);
}



void IrredPolyMod(GF2EX& h, const GF2EX& g, const GF2EXModulus& F)
{
   IrredPolyMod(h, g, F, F.n);
}



void MinPolyMod(GF2EX& hh, const GF2EX& g, const GF2EXModulus& F)
{
   MinPolyMod(hh, g, F, F.n);
}


void MakeMonic(GF2EX& x)
{
   if (IsZero(x))
      return;

   if (IsOne(LeadCoeff(x)))
      return;

   GF2E t;

   inv(t, LeadCoeff(x));
   mul(x, x, t);
}


long divide(GF2EX& q, const GF2EX& a, const GF2EX& b)
{
   if (IsZero(b)) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   GF2EX lq, r;
   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0; 
   q = lq;
   return 1;
}

long divide(const GF2EX& a, const GF2EX& b)
{
   if (IsZero(b)) return IsZero(a);
   GF2EX lq, r;
   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0; 
   return 1;
}


long operator==(const GF2EX& a, long b)
{
   if (b & 1)
      return IsOne(a);
   else
      return IsZero(a);
}


long operator==(const GF2EX& a, GF2 b)
{
   if (b == 1)
      return IsOne(a);
   else
      return IsZero(a);
}

long operator==(const GF2EX& a, const GF2E& b)
{
   if (IsZero(b))
      return IsZero(a);

   if (deg(a) != 0)
      return 0;

   return a.rep[0] == b;
}



void power(GF2EX& x, const GF2EX& a, long e)
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

   GF2EX res;
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

void reverse(GF2EX& x, const GF2EX& a, long hi)
{
   if (hi < 0) { clear(x); return; }
   if (NTL_OVERFLOW(hi, 1, 0)) ResourceError("overflow in reverse");

   if (&x == &a) {
      GF2EX tmp;
      CopyReverse(tmp, a, hi);
      x = tmp;
   }
   else
      CopyReverse(x, a, hi);
}


static
void FastTraceVec(vec_GF2E& S, const GF2EXModulus& f)
{
   long n = deg(f);

   GF2EX x = reverse(-LeftShift(reverse(diff(reverse(f)), n-1), n-1)/f, n-1);

   S.SetLength(n);
   S[0] = n;

   long i;
   for (i = 1; i < n; i++)
      S[i] = coeff(x, i);
}


void PlainTraceVec(vec_GF2E& S, const GF2EX& ff)
{
   if (deg(ff) <= 0)
      LogicError("TraceVec: bad args");

   GF2EX f;
   f = ff;

   MakeMonic(f);

   long n = deg(f);

   S.SetLength(n);

   if (n == 0)
      return;

   long k, i;
   GF2X acc, t;
   GF2E t1;

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

void TraceVec(vec_GF2E& S, const GF2EX& f)
{
   if (deg(f) < GF2E::DivCross())
      PlainTraceVec(S, f);
   else
      FastTraceVec(S, f);
}

static
void ComputeTraceVec(vec_GF2E& S, const GF2EXModulus& F)
{
   if (F.method == GF2EX_MOD_PLAIN) {
      PlainTraceVec(S, F.f);
   }
   else {
      FastTraceVec(S, F);
   }
}

void TraceMod(GF2E& x, const GF2EX& a, const GF2EXModulus& F)
{
   long n = F.n;

   if (deg(a) >= n)
      LogicError("trace: bad args");

   do { // NOTE: thread safe lazy init
      Lazy<vec_GF2E>::Builder builder(F.tracevec.val());
      if (!builder()) break;
      UniquePtr<vec_GF2E> p;
      p.make();
      ComputeTraceVec(*p, F);
      builder.move(p);
   } while (0);

   InnerProduct(x, a.rep, *F.tracevec.val());
}

void TraceMod(GF2E& x, const GF2EX& a, const GF2EX& f)
{
   if (deg(a) >= deg(f) || deg(f) <= 0)
      LogicError("trace: bad args");

   project(x, TraceVec(f), a);
}


void PlainResultant(GF2E& rres, const GF2EX& a, const GF2EX& b)
{
   GF2E res;
 
   if (IsZero(a) || IsZero(b))
      clear(res);
   else if (deg(a) == 0 && deg(b) == 0) 
      set(res);
   else {
      long d0, d1, d2;
      GF2E lc;
      set(res);

      long n = max(deg(a),deg(b)) + 1;
      GF2EX u(INIT_SIZE, n), v(INIT_SIZE, n);
      GF2XVec tmp(n, 2*GF2E::WordLength());

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

      rres = res;
   }
}

void resultant(GF2E& rres, const GF2EX& a, const GF2EX& b)
{
   PlainResultant(rres, a, b); 
}


void NormMod(GF2E& x, const GF2EX& a, const GF2EX& f)
{
   if (deg(f) <= 0 || deg(a) >= deg(f)) 
      LogicError("norm: bad args");

   if (IsZero(a)) {
      clear(x);
      return;
   }

   GF2E t;
   resultant(t, f, a);
   if (!IsOne(LeadCoeff(f))) {
      GF2E t1;
      power(t1, LeadCoeff(f), deg(a));
      inv(t1, t1);
      mul(t, t, t1);
   }

   x = t;
}



// tower stuff...

void InnerProduct(GF2EX& x, const GF2X& v, long low, long high,
                   const vec_GF2EX& H, long n, vec_GF2E& t)
{
   long i, j;

   for (j = 0; j < n; j++)
      clear(t[j]);

   high = min(high, deg(v));
   for (i = low; i <= high; i++) {
      const vec_GF2E& h = H[i-low].rep;
      long m = h.length();

      if (coeff(v, i) != 0) {
         for (j = 0; j < m; j++) {
            add(t[j], t[j], h[j]);
         }
      }
   }

   x.rep.SetLength(n);
   for (j = 0; j < n; j++)
      x.rep[j] = t[j];

   x.normalize();
}



void CompTower(GF2EX& x, const GF2X& g, const GF2EXArgument& A,
             const GF2EXModulus& F)
{
   if (deg(g) <= 0) {
      conv(x, g);
      return;
   }


   GF2EX s, t;
   vec_GF2E scratch;
   scratch.SetLength(deg(F));

   long m = A.H.length() - 1;
   long l = (((deg(g)+1)+m-1)/m) - 1;

   const GF2EX& M = A.H[m];

   InnerProduct(t, g, l*m, l*m + m - 1, A.H, F.n, scratch);
   for (long i = l-1; i >= 0; i--) {
      InnerProduct(s, g, i*m, i*m + m - 1, A.H, F.n, scratch);
      MulMod(t, t, M, F);
      add(t, t, s);
   }
   x = t;
}


void CompTower(GF2EX& x, const GF2X& g, const GF2EX& h, 
             const GF2EXModulus& F)
   // x = g(h) mod f
{
   long m = SqrRoot(deg(g)+1);

   if (m == 0) {
      clear(x);
      return;
   }


   GF2EXArgument A;

   build(A, h, F, m);

   CompTower(x, g, A, F);
}

void PrepareProjection(vec_vec_GF2& tt, const vec_GF2E& s,
                       const vec_GF2& proj)
{
   long l = s.length();
   tt.SetLength(l);

   GF2XTransMultiplier M;
   long i;

   for (i = 0; i < l; i++) {
      build(M, rep(s[i]), GF2E::modulus());
      UpdateMap(tt[i], proj, M, GF2E::modulus());
   }
}

void ProjectedInnerProduct(ref_GF2 x, const vec_GF2E& a, 
                           const vec_vec_GF2& b)
{
   long n = min(a.length(), b.length());

   GF2 t, res;

   res = 0;

   long i;
   for (i = 0; i < n; i++) {
      project(t, b[i], rep(a[i]));
      res += t;
   }

   x = res;
}



void PrecomputeProj(vec_GF2& proj, const GF2X& f)
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

void ProjectPowersTower(vec_GF2& x, const vec_GF2E& a, long k,
                   const GF2EXArgument& H, const GF2EXModulus& F,
                   const vec_GF2& proj)

{
   long n = F.n;

   if (a.length() > n || k < 0) LogicError("ProjectPowers: bad args");

   long m = H.H.length()-1;
   long l = (k+m-1)/m - 1;

   GF2EXTransMultiplier M;
   build(M, H.H[m], F);

   vec_GF2E s(INIT_SIZE, n);
   s = a;

   x.SetLength(k);

   vec_vec_GF2 tt;

   for (long i = 0; i <= l; i++) {
      long m1 = min(m, k-i*m);

      PrepareProjection(tt, s, proj);

      for (long j = 0; j < m1; j++) {
         GF2 r;
         ProjectedInnerProduct(r, H.H[j].rep, tt);
         x.put(i*m + j, r);
      }
      if (i < l)
         UpdateMap(s, s, M, F);
   }
}




void ProjectPowersTower(vec_GF2& x, const vec_GF2E& a, long k,
                   const GF2EX& h, const GF2EXModulus& F,
                   const vec_GF2& proj)

{
   if (a.length() > F.n || k < 0) LogicError("ProjectPowers: bad args");

   if (k == 0) {
      x.SetLength(0);
      return;
   }

   long m = SqrRoot(k);

   GF2EXArgument H;

   build(H, h, F, m);
   ProjectPowersTower(x, a, k, H, F, proj);
}


void DoMinPolyTower(GF2X& h, const GF2EX& g, const GF2EXModulus& F, long m,
               const vec_GF2E& R, const vec_GF2& proj)
{
   vec_GF2 x;

   ProjectPowersTower(x, R, 2*m, g, F, proj);
   
   MinPolySeq(h, x, m);
}


void ProbMinPolyTower(GF2X& h, const GF2EX& g, const GF2EXModulus& F, 
                      long m)
{
   long n = F.n;
   if (m < 1 || m > n*GF2E::degree()) LogicError("ProbMinPoly: bad args");

   vec_GF2E R;
   R.SetLength(n);
   long i;
   for (i = 0; i < n; i++) random(R[i]);

   vec_GF2 proj;
   PrecomputeProj(proj, GF2E::modulus());

   DoMinPolyTower(h, g, F, m, R, proj);
}

void ProbMinPolyTower(GF2X& h, const GF2EX& g, const GF2EXModulus& F, 
                      long m, const vec_GF2& proj)
{
   long n = F.n;
   if (m < 1 || m > n*GF2E::degree()) LogicError("ProbMinPoly: bad args");

   vec_GF2E R;
   R.SetLength(n);
   long i;
   for (i = 0; i < n; i++) random(R[i]);

   DoMinPolyTower(h, g, F, m, R, proj);
}

void MinPolyTower(GF2X& hh, const GF2EX& g, const GF2EXModulus& F, long m)
{
   GF2X h;
   GF2EX h1;
   long n = F.n;
   if (m < 1 || m > n*GF2E::degree()) {
      LogicError("MinPoly: bad args");
   }

   vec_GF2 proj;
   PrecomputeProj(proj, GF2E::modulus());

   /* probabilistically compute min-poly */

   ProbMinPolyTower(h, g, F, m, proj);
   if (deg(h) == m) { hh = h; return; }
   CompTower(h1, h, g, F);
   if (IsZero(h1)) { hh = h; return; }

   /* not completely successful...must iterate */

   long i;

   GF2X h2;
   GF2EX h3;
   vec_GF2E R;
   GF2EXTransMultiplier H1;
   

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
      if (IsZero(h1)) { 
         hh = h; 
         return; 
      }
   }
}

void IrredPolyTower(GF2X& h, const GF2EX& g, const GF2EXModulus& F, long m)
{
   if (m < 1 || m > deg(F)*GF2E::degree()) LogicError("IrredPoly: bad args");

   vec_GF2E R;
   R.SetLength(1);
   R[0] = 1;

   vec_GF2 proj;
   proj.SetLength(1);
   proj.put(0, 1);

   DoMinPolyTower(h, g, F, m, R, proj);
}

NTL_END_IMPL

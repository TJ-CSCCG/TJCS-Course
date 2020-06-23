
#include <NTL/lzz_pX.h>
#include <NTL/FFT_impl.h>


NTL_START_IMPL


// NOTE: these are declared extern in lzz_pX.h

const long zz_pX_mod_crossover[5] = {45, 45, 90, 180, 180};
const long zz_pX_mul_crossover[5] = {150, 150, 300, 500, 500};
const long zz_pX_newton_crossover[5] = {150, 150, 300, 700, 700};
const long zz_pX_div_crossover[5] = {180, 180, 350, 750, 750};
const long zz_pX_halfgcd_crossover[5] = {90, 90, 180, 350, 350};
const long zz_pX_gcd_crossover[5] = {400, 400, 800, 1400, 1400};
const long zz_pX_bermass_crossover[5] = {400, 480, 900, 1600, 1600};
const long zz_pX_trace_crossover[5] = {200, 350, 450, 800, 800};




const zz_pX& zz_pX::zero()
{
   static const zz_pX z; // GLOBAL (assumes C++11 thread-safe init)
   return z;
}



istream& operator>>(istream& s, zz_pX& x)
{
   NTL_INPUT_CHECK_RET(s, s >> x.rep);
   x.normalize();
   return s;
}

ostream& operator<<(ostream& s, const zz_pX& a)
{
   return s << a.rep;
}


void zz_pX::normalize()
{
   long n;
   const zz_p* p;

   n = rep.length();
   if (n == 0) return;
   p = rep.elts() + n;
   while (n > 0 && IsZero(*--p)) {
      n--;
   }
   rep.SetLength(n);
}


long IsZero(const zz_pX& a)
{
   return a.rep.length() == 0;
}


long IsOne(const zz_pX& a)
{
    return a.rep.length() == 1 && IsOne(a.rep[0]);
}

void GetCoeff(zz_p& x, const zz_pX& a, long i)
{
   if (i < 0 || i > deg(a))
      clear(x);
   else
      x = a.rep[i];
}

void SetCoeff(zz_pX& x, long i, zz_p a)
{
   long j, m;

   if (i < 0) 
      LogicError("SetCoeff: negative index");

   if (NTL_OVERFLOW(i, 1, 0))
      ResourceError("overflow in SetCoeff");

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

void SetCoeff(zz_pX& x, long i, long a)
{
   if (a == 1)
      SetCoeff(x, i);
   else
      SetCoeff(x, i, to_zz_p(a));
}

void SetCoeff(zz_pX& x, long i)
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


void SetX(zz_pX& x)
{
   clear(x);
   SetCoeff(x, 1);
}


long IsX(const zz_pX& a)
{
   return deg(a) == 1 && IsOne(LeadCoeff(a)) && IsZero(ConstTerm(a));
}
      
      

const zz_p coeff(const zz_pX& a, long i)
{
   if (i < 0 || i > deg(a))
      return zz_p::zero();
   else
      return a.rep[i];
}


const zz_p LeadCoeff(const zz_pX& a)
{
   if (IsZero(a))
      return zz_p::zero();
   else
      return a.rep[deg(a)];
}

const zz_p ConstTerm(const zz_pX& a)
{
   if (IsZero(a))
      return zz_p::zero();
   else
      return a.rep[0];
}



void conv(zz_pX& x, zz_p a)
{
   if (IsZero(a))
      x.rep.SetLength(0);
   else {
      x.rep.SetLength(1);
      x.rep[0] = a;
   }
}

void conv(zz_pX& x, long a)
{
   if (a == 0) {
      x.rep.SetLength(0);
      return;
   }
   
   zz_p t;

   conv(t, a);
   conv(x, t);
}

void conv(zz_pX& x, const ZZ& a)
{
   if (a == 0) {
      x.rep.SetLength(0);
      return;
   }
   
   zz_p t;

   conv(t, a);
   conv(x, t);
}


void conv(zz_pX& x, const vec_zz_p& a)
{
   x.rep = a;
   x.normalize();
}


void add(zz_pX& x, const zz_pX& a, const zz_pX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const zz_p *ap, *bp; 
   zz_p* xp;
   long p = zz_p::modulus();

   for (i = minab+1, ap = a.rep.elts(), bp = b.rep.elts(), xp = x.rep.elts();
        i; i--, ap++, bp++, xp++)
      xp->LoopHole() = AddMod(rep(*ap), rep(*bp), p);

   if (da > minab && &x != &a)
      for (i = da-minab; i; i--, xp++, ap++)
         *xp = *ap;
   else if (db > minab && &x != &b)
      for (i = db-minab; i; i--, xp++, bp++)
         *xp = *bp;
   else
      x.normalize();
}

void add(zz_pX& x, const zz_pX& a, zz_p b)
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


void sub(zz_pX& x, const zz_pX& a, const zz_pX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const zz_p *ap, *bp; 
   zz_p* xp;
   long p = zz_p::modulus();

   for (i = minab+1, ap = a.rep.elts(), bp = b.rep.elts(), xp = x.rep.elts();
        i; i--, ap++, bp++, xp++)
      xp->LoopHole() = SubMod(rep(*ap), rep(*bp), p);

   if (da > minab && &x != &a)
      for (i = da-minab; i; i--, xp++, ap++)
         *xp = *ap;
   else if (db > minab)
      for (i = db-minab; i; i--, xp++, bp++)
         xp->LoopHole() = NegateMod(rep(*bp), p);
   else
      x.normalize();

}

void sub(zz_pX& x, const zz_pX& a, zz_p b)
{
   if (a.rep.length() == 0) {
      x.rep.SetLength(1);
      negate(x.rep[0], b);
   }
   else {
      if (&x != &a) x = a;
      sub(x.rep[0], x.rep[0], b);
   }
   x.normalize();
}

void sub(zz_pX& x, zz_p a, const zz_pX& b)
{
   negate(x, b);
   add(x, x, a);
}

void negate(zz_pX& x, const zz_pX& a)
{
   long n = a.rep.length();
   x.rep.SetLength(n);

   const zz_p* ap = a.rep.elts();
   zz_p* xp = x.rep.elts();
   long i;
   long p = zz_p::modulus();

   for (i = n; i; i--, ap++, xp++)
      xp->LoopHole() = NegateMod(rep(*ap), p);
}

void mul(zz_pX& x, const zz_pX& a, const zz_pX& b)
{
   if (&a == &b) {
      sqr(x, a);
      return;
   }

   if (deg(a) > NTL_zz_pX_MUL_CROSSOVER && deg(b) > NTL_zz_pX_MUL_CROSSOVER)
      FFTMul(x, a, b);
   else
      PlainMul(x, a, b);
}

void sqr(zz_pX& x, const zz_pX& a)
{
   if (deg(a) > NTL_zz_pX_MUL_CROSSOVER)
      FFTSqr(x, a);
   else
      PlainSqr(x, a);
}

/* "plain" multiplication and squaring actually incorporates Karatsuba */

void PlainMul(zz_p *xp, const zz_p *ap, long sa, const zz_p *bp, long sb)
{
   if (sa == 0 || sb == 0) return;

   long sx = sa+sb-1;


   if (sa < sb) {
      { long t = sa; sa = sb; sb = t; }
      { const zz_p *t = ap; ap = bp; bp = t; }
   }

   long i, j;

   for (i = 0; i < sx; i++)
      clear(xp[i]);

   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();

   for (i = 0; i < sb; i++) {
      long t1 = rep(bp[i]);
      mulmod_precon_t bpinv = PrepMulModPrecon(t1, p, pinv); 
      zz_p *xp1 = xp+i;
      for (j = 0; j < sa; j++) {
         long t2;
         t2 = MulModPrecon(rep(ap[j]), t1, p, bpinv);
         xp1[j].LoopHole() = AddMod(t2, rep(xp1[j]), p);
      }
   }
}

static inline 
void reduce(zz_p& r, long a, long p, mulmod_t pinv)
{
   // DIRT: uses undocumented MulMod feature (see sp_arith.h)
   r.LoopHole() = MulMod(a, 1L, p, pinv);
}

void PlainMul_long(zz_p *xp, const zz_p *ap, long sa, const zz_p *bp, long sb)
{
   if (sa == 0 || sb == 0) return;

   long d = sa+sb-2;

   long i, j, jmin, jmax;

   long accum;

   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();

   for (i = 0; i <= d; i++) {
      jmin = max(0, i-(sb-1));
      jmax = min((sa-1), i);
      accum = 0;
      for (j = jmin; j <= jmax; j++) {
         accum += rep(ap[j])*rep(bp[i-j]);
      }
      reduce(xp[i], accum, p, pinv);
   }
}

#define KARX (16)

void KarFold(zz_p *T, const zz_p *b, long sb, long hsa)
{
   long m = sb - hsa;
   long i;
   long p = zz_p::modulus();

   for (i = 0; i < m; i++)
      T[i].LoopHole() = AddMod(rep(b[i]), rep(b[hsa+i]), p);

   for (i = m; i < hsa; i++)
      T[i] = b[i];
}

void KarSub(zz_p *T, const zz_p *b, long sb)
{
   long i;
   long p = zz_p::modulus();

   for (i = 0; i < sb; i++)
      T[i].LoopHole() = SubMod(rep(T[i]), rep(b[i]), p);
}

void KarAdd(zz_p *T, const zz_p *b, long sb)
{
   long i;
   long p = zz_p::modulus();

   for (i = 0; i < sb; i++)
      T[i].LoopHole() = AddMod(rep(T[i]), rep(b[i]), p);
}

void KarFix(zz_p *c, const zz_p *b, long sb, long hsa)
{
   long i;
   long p = zz_p::modulus();

   for (i = 0; i < hsa; i++)
      c[i] = b[i];

   for (i = hsa; i < sb; i++)
      c[i].LoopHole() = AddMod(rep(c[i]), rep(b[i]), p);
}


void KarMul(zz_p *c, const zz_p *a, long sa, const zz_p *b, long sb, zz_p *stk)
{
   if (sa < sb) {
      { long t = sa; sa = sb; sb = t; }
      { const zz_p *t = a; a = b; b = t; }
   }

   if (sb < KARX) {
      PlainMul(c, a, sa, b, sb);
      return;
   }

   long hsa = (sa + 1) >> 1;

   if (hsa < sb) {
      /* normal case */

      long hsa2 = hsa << 1;

      zz_p *T1, *T2, *T3;

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

      zz_p *T;

      T = stk; stk += hsa + sb - 1;

      /* recursively compute b*a_hi into high part of c */

      KarMul(c + hsa, a + hsa, sa - hsa, b, sb, stk);

      /* recursively compute b*a_lo into T */

      KarMul(T, a, hsa, b, sb, stk);

      KarFix(c, T, hsa + sb - 1, hsa);
   }
}

void KarMul_long(zz_p *c, const zz_p *a, long sa, const zz_p *b, long sb, zz_p *stk)
{
   if (sa < sb) {
      { long t = sa; sa = sb; sb = t; }
      { const zz_p *t = a; a = b; b = t; }
   }

   if (sb < KARX) {
      PlainMul_long(c, a, sa, b, sb);
      return;
   }

   long hsa = (sa + 1) >> 1;

   if (hsa < sb) {
      /* normal case */

      long hsa2 = hsa << 1;

      zz_p *T1, *T2, *T3;

      T1 = stk; stk += hsa;
      T2 = stk; stk += hsa;
      T3 = stk; stk += hsa2 - 1;

      /* compute T1 = a_lo + a_hi */

      KarFold(T1, a, sa, hsa);

      /* compute T2 = b_lo + b_hi */

      KarFold(T2, b, sb, hsa);

      /* recursively compute T3 = T1 * T2 */

      KarMul_long(T3, T1, hsa, T2, hsa, stk);

      /* recursively compute a_hi * b_hi into high part of c */
      /* and subtract from T3 */

      KarMul_long(c + hsa2, a+hsa, sa-hsa, b+hsa, sb-hsa, stk);
      KarSub(T3, c + hsa2, sa + sb - hsa2 - 1);


      /* recursively compute a_lo*b_lo into low part of c */
      /* and subtract from T3 */

      KarMul_long(c, a, hsa, b, hsa, stk);
      KarSub(T3, c, hsa2 - 1);

      clear(c[hsa2 - 1]);

      /* finally, add T3 * X^{hsa} to c */

      KarAdd(c+hsa, T3, hsa2-1);
   }
   else {
      /* degenerate case */

      zz_p *T;

      T = stk; stk += hsa + sb - 1;

      /* recursively compute b*a_hi into high part of c */

      KarMul_long(c + hsa, a + hsa, sa - hsa, b, sb, stk);

      /* recursively compute b*a_lo into T */

      KarMul_long(T, a, hsa, b, sb, stk);

      KarFix(c, T, hsa + sb - 1, hsa);
   }
}


void PlainMul(zz_pX& c, const zz_pX& a, const zz_pX& b)
{
   long sa = a.rep.length();
   long sb = b.rep.length();

   if (sa == 0 || sb == 0) {
      clear(c);
      return;
   }

   if (sa == 1) {
      mul(c, b, a.rep[0]);
      return;
   }

   if (sb == 1) {
      mul(c, a, b.rep[0]);
      return;
   }

   if (&a == &b) {
      PlainSqr(c, a);
      return;
   }

   vec_zz_p mem;

   const zz_p *ap, *bp;
   zz_p *cp;

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

   long p = zz_p::modulus();
   long use_long = (p < NTL_SP_BOUND/KARX && p*KARX < NTL_SP_BOUND/p);

   if (sa < KARX || sb < KARX) {
      if (use_long) 
         PlainMul_long(cp, ap, sa, bp, sb);
      else
         PlainMul(cp, ap, sa, bp, sb);
   }
   else {
      /* karatsuba */

      long n, hn, sp;

      n = max(sa, sb);
      sp = 0;
      do {
         hn = (n+1) >> 1;
         sp += (hn << 2) - 1;
         n = hn;
      } while (n >= KARX);

      vec_zz_p stk;
      stk.SetLength(sp);

      if (use_long) 
         KarMul_long(cp, ap, sa, bp, sb, stk.elts());
      else
         KarMul(cp, ap, sa, bp, sb, stk.elts());
   }

   c.normalize();
}

void PlainSqr_long(zz_p *xp, const zz_p *ap, long sa)
{
   if (sa == 0) return;

   long da = sa-1;
   long d = 2*da;

   long i, j, jmin, jmax, m, m2;

   long accum;
   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();

   for (i = 0; i <= d; i++) {
      jmin = max(0, i-da);
      jmax = min(da, i);
      m = jmax - jmin + 1;
      m2 = m >> 1;
      jmax = jmin + m2 - 1;
      accum = 0;
      for (j = jmin; j <= jmax; j++) {
         accum += rep(ap[j])*rep(ap[i-j]);
      }
      accum += accum;
      if (m & 1) {
         accum += rep(ap[jmax + 1])*rep(ap[jmax + 1]);
      }

      reduce(xp[i], accum, p, pinv);
   }
}


void PlainSqr(zz_p *xp, const zz_p *ap, long sa)
{
   if (sa == 0) return;

   long i, j, k, cnt;

   cnt = 2*sa-1;
   for (i = 0; i < cnt; i++)
      clear(xp[i]);

   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();
   long t1, t2;

   i = -1;
   for (j = 0; j <= sa-2; j++) {
      i += 2;

      t1 = MulMod(rep(ap[j]), rep(ap[j]), p, pinv);
      t2 = rep(xp[i-1]);
      t2 = AddMod(t2, t2, p);
      t2 = AddMod(t2, t1, p);
      xp[i-1].LoopHole() = t2;

      cnt = sa - 1 - j;
      const zz_p *ap1 = ap+(j+1);
      zz_p *xp1 = xp+i;
      t1 = rep(ap[j]);
      mulmod_precon_t tpinv = PrepMulModPrecon(t1, p, pinv); 

      for (k = 0; k < cnt; k++) {
         t2 = MulModPrecon(rep(ap1[k]), t1, p, tpinv);
         t2 = AddMod(t2, rep(xp1[k]), p);
         xp1[k].LoopHole() = t2;
      }
      t2 = rep(*xp1);
      t2 = AddMod(t2, t2, p);
      (*xp1).LoopHole() = t2;
   }


   t1 = rep(ap[sa-1]);
   t1 = MulMod(t1, t1, p, pinv);
   xp[2*sa-2].LoopHole() = t1;
}

#define KARSX (30)

void KarSqr(zz_p *c, const zz_p *a, long sa, zz_p *stk)
{
   if (sa < KARSX) {
      PlainSqr(c, a, sa);
      return;
   }

   long hsa = (sa + 1) >> 1;
   long hsa2 = hsa << 1;

   zz_p *T1, *T2;

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

void KarSqr_long(zz_p *c, const zz_p *a, long sa, zz_p *stk)
{
   if (sa < KARSX) {
      PlainSqr_long(c, a, sa);
      return;
   }

   long hsa = (sa + 1) >> 1;
   long hsa2 = hsa << 1;

   zz_p *T1, *T2;

   T1 = stk; stk += hsa;
   T2 = stk; stk += hsa2-1;

   KarFold(T1, a, sa, hsa);
   KarSqr_long(T2, T1, hsa, stk);


   KarSqr_long(c + hsa2, a+hsa, sa-hsa, stk);
   KarSub(T2, c + hsa2, sa + sa - hsa2 - 1);


   KarSqr_long(c, a, hsa, stk);
   KarSub(T2, c, hsa2 - 1);

   clear(c[hsa2 - 1]);

   KarAdd(c+hsa, T2, hsa2-1);
}

void PlainSqr(zz_pX& c, const zz_pX& a)
{
   if (IsZero(a)) {
      clear(c);
      return;
   }

   vec_zz_p mem;

   const zz_p *ap;
   zz_p *cp;

   long sa = a.rep.length();

   if (&a == &c) {
      mem = a.rep;
      ap = mem.elts();
   }
   else
      ap = a.rep.elts();

   c.rep.SetLength(2*sa-1);
   cp = c.rep.elts();

   long p = zz_p::modulus();
   long use_long = (p < NTL_SP_BOUND/KARSX && p*KARSX < NTL_SP_BOUND/p);

   if (sa < KARSX) {
      if (use_long) 
         PlainSqr_long(cp, ap, sa);
      else
         PlainSqr(cp, ap, sa);
   }
   else {
      /* karatsuba */

      long n, hn, sp;

      n = sa;
      sp = 0;
      do {
         hn = (n+1) >> 1;
         sp += hn+hn+hn - 1;
         n = hn;
      } while (n >= KARSX);

      vec_zz_p stk;
      stk.SetLength(sp);

      if (use_long) 
         KarSqr_long(cp, ap, sa, stk.elts());
      else
         KarSqr(cp, ap, sa, stk.elts());
   }

   c.normalize();
}


void PlainDivRem(zz_pX& q, zz_pX& r, const zz_pX& a, const zz_pX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const zz_p *bp;
   zz_p *qp;
   zz_p *xp;


   zz_p LCInv, t;
   zz_p s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("zz_pX: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   zz_pX lb;

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

   vec_zz_p x;
   if (&r == &a)
      xp = r.rep.elts();
   else {
      x = a.rep;
      xp = x.elts();
   }

   dq = da - db;
   q.rep.SetLength(dq+1);
   qp = q.rep.elts();

   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();

   for (i = dq; i >= 0; i--) {
      t = xp[i+db];
      if (!LCIsOne)
         mul(t, t, LCInv);
      qp[i] = t;
      negate(t, t);

      long T = rep(t);
      mulmod_precon_t Tpinv = PrepMulModPrecon(T, p, pinv); 

      for (j = db-1; j >= 0; j--) {
         long S = MulModPrecon(rep(bp[j]), T, p, Tpinv);
         S = AddMod(S, rep(xp[i+j]), p);
         xp[i+j].LoopHole() = S;
      }
   }

   r.rep.SetLength(db);
   if (&r != &a) {
      for (i = 0; i < db; i++)
         r.rep[i] = xp[i];
   }
   r.normalize();
}

void PlainDiv(zz_pX& q, const zz_pX& a, const zz_pX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const zz_p *bp;
   zz_p *qp;
   zz_p *xp;


   zz_p LCInv, t;
   zz_p s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("zz_pX: division by zero");

   if (da < db) {
      clear(q);
      return;
   }

   zz_pX lb;

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

   vec_zz_p x;
   x.SetLength(da+1-db);
   for (i = db; i <= da; i++)
      x[i-db] = a.rep[i];

   xp = x.elts();



   dq = da - db;
   q.rep.SetLength(dq+1);
   qp = q.rep.elts();

   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();

   for (i = dq; i >= 0; i--) {
      t = xp[i];
      if (!LCIsOne)
         mul(t, t, LCInv);
      qp[i] = t;
      negate(t, t);

      long T = rep(t);
      mulmod_precon_t Tpinv = PrepMulModPrecon(T, p, pinv); 

      long lastj = max(0, db-i);

      for (j = db-1; j >= lastj; j--) {
         long S = MulModPrecon(rep(bp[j]), T, p, Tpinv);
         S = AddMod(S, rep(xp[i+j-db]), p);
         xp[i+j-db].LoopHole() = S;
      }
   }
}


void PlainRem(zz_pX& r, const zz_pX& a, const zz_pX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const zz_p *bp;
   zz_p *xp;


   zz_p LCInv, t;
   zz_p s;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("zz_pX: division by zero");

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

   vec_zz_p x;

   if (&r == &a)
      xp = r.rep.elts();
   else {
      x = a.rep;
      xp = x.elts();
   }

   dq = da - db;

   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();

   for (i = dq; i >= 0; i--) {
      t = xp[i+db];
      if (!LCIsOne)
         mul(t, t, LCInv);
      negate(t, t);

      long T = rep(t);
      mulmod_precon_t Tpinv = PrepMulModPrecon(T, p, pinv); 

      for (j = db-1; j >= 0; j--) {
         long S = MulModPrecon(rep(bp[j]), T, p, Tpinv);
         S = AddMod(S, rep(xp[i+j]), p);
         xp[i+j].LoopHole() = S;
      }
   }

   r.rep.SetLength(db);
   if (&r != &a) {
      for (i = 0; i < db; i++)
         r.rep[i] = xp[i];
   }
   r.normalize();
}


void mul(zz_pX& x, const zz_pX& a, zz_p b)
{
   if (IsZero(b)) {
      clear(x);
      return;
   }

   if (IsOne(b)) {
      x = a;
      return;
   }

   long i, da;

   const zz_p *ap;
   zz_p* xp;

   long t;
   t = rep(b);
   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();
   mulmod_precon_t bpinv = PrepMulModPrecon(t, p, pinv); 

   da = deg(a);
   x.rep.SetLength(da+1);
   ap = a.rep.elts();
   xp = x.rep.elts();

   for (i = 0; i <= da; i++) 
      xp[i].LoopHole() = MulModPrecon(rep(ap[i]), t, p, bpinv);

   x.normalize();
}



void PlainGCD(zz_pX& x, const zz_pX& a, const zz_pX& b)
{
   zz_p t;

   if (IsZero(b))
      x = a;
   else if (IsZero(a))
      x = b;
   else {
      long n = max(deg(a),deg(b)) + 1;
      zz_pX u(INIT_SIZE, n), v(INIT_SIZE, n);

      u = a;
      v = b;
      do {
         PlainRem(u, u, v);
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



         

void PlainXGCD(zz_pX& d, zz_pX& s, zz_pX& t, const zz_pX& a, const zz_pX& b)
{
   zz_p z;


   if (IsZero(b)) {
      set(s);
      clear(t);
      d = a;
   }
   else if (IsZero(a)) {
      clear(s);
      set(t);
      d = b;
   }
   else {
      long e = max(deg(a), deg(b)) + 1;

      zz_pX temp(INIT_SIZE, e), u(INIT_SIZE, e), v(INIT_SIZE, e), u0(INIT_SIZE, e), v0(INIT_SIZE, e), 
            u1(INIT_SIZE, e), v1(INIT_SIZE, e), u2(INIT_SIZE, e), v2(INIT_SIZE, e), q(INIT_SIZE, e);


      set(u1); clear(v1);
      clear(u2); set(v2);
      u = a; v = b;

      do {
         DivRem(q, u, u, v);
         swap(u, v);
         u0 = u2;
         v0 = v2;
         mul(temp, q, u2);
         sub(u2, u1, temp);
         mul(temp, q, v2);
         sub(v2, v1, temp);
         u1 = u0;
         v1 = v0;
      } while (!IsZero(v));

      d = u;
      s = u1;
      t = v1;
   }

   if (IsZero(d)) return;
   if (IsOne(LeadCoeff(d))) return;

   /* make gcd monic */

   inv(z, LeadCoeff(d));
   mul(d, d, z);
   mul(s, s, z);
   mul(t, t, z);
}


void MulMod(zz_pX& x, const zz_pX& a, const zz_pX& b, const zz_pX& f)
{
   if (deg(a) >= deg(f) || deg(b) >= deg(f) || deg(f) == 0) 
      LogicError("MulMod: bad args");

   zz_pX t;

   mul(t, a, b);
   rem(x, t, f);
}

void SqrMod(zz_pX& x, const zz_pX& a, const zz_pX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("SqrMod: bad args");

   zz_pX t;

   sqr(t, a);
   rem(x, t, f);
}


void InvMod(zz_pX& x, const zz_pX& a, const zz_pX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvMod: bad args");

   zz_pX d, xx, t;

   XGCD(d, xx, t, a, f);
   if (!IsOne(d))
      InvModError("zz_pX InvMod: can't compute multiplicative inverse");

   x = xx;
}

long InvModStatus(zz_pX& x, const zz_pX& a, const zz_pX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvModStatus: bad args");

   zz_pX d, t;

   XGCD(d, x, t, a, f);
   if (!IsOne(d)) {
      x = d;
      return 1;
   }
   else
      return 0;
}




static
void MulByXModAux(zz_pX& h, const zz_pX& a, const zz_pX& f)
{
   long i, n, m;
   zz_p* hh;
   const zz_p *aa, *ff;

   zz_p t, z;

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

void MulByXMod(zz_pX& h, const zz_pX& a, const zz_pX& f)
{
   if (&h == &f) {
      zz_pX hh;
      MulByXModAux(hh, a, f);
      h = hh;
   }
   else
      MulByXModAux(h, a, f);
}


void random(zz_pX& x, long n)
{
   x.rep.SetLength(n);
   VectorRandom(n, x.rep.elts());
   x.normalize();
}





void fftRep::DoSetSize(long NewK, long NewNumPrimes)
{
   if (NewK < -1) LogicError("bad arg to fftRep::SetSize()");
   
   if (NewK >= NTL_BITS_PER_LONG-1)
      ResourceError("bad arg to fftRep::SetSize()");

   if (NewK == -1) {
      k = -1;
      return;
   }

   if (NewNumPrimes == 0) 
      NewNumPrimes = zz_pInfo->NumPrimes;

   if (MaxK >= 0 && NumPrimes != NewNumPrimes)
      LogicError("fftRep: inconsistent use");

   if (NewK <= MaxK) {
      k = NewK;
      return;
   }

   UniqueArray<long> new_tbl[4];
   long i;

   for (i = 0; i < NewNumPrimes; i++) 
      new_tbl[i].SetLength(1L << NewK);

   for (i = 0; i < NewNumPrimes; i++) 
      tbl[i].move(new_tbl[i]);

   NumPrimes = NewNumPrimes;
   k = MaxK = NewK;
}

void fftRep::SetSize(long NewK)
{
   DoSetSize(NewK, 0);
}


fftRep& fftRep::operator=(const fftRep& R)
{
   if (this == &R) return *this;

   if (MaxK >= 0 && R.MaxK >= 0 && NumPrimes != R.NumPrimes)
      LogicError("fftRep: inconsistent use");

   if (R.k < 0) {
      k = -1;
      len = 0;
      return *this;
   }

   DoSetSize(R.k, R.NumPrimes);
   len = R.len;

   long i, j;

   for (i = 0; i < NumPrimes; i++)
      for (j = 0; j < len; j++)
         tbl[i][j] = R.tbl[i][j];

   return *this;
}



static inline
void FromModularRep(zz_p& res, long *a, zz_pInfoT* info)
{
   long n = info->NumPrimes;
   long p = info->p;
   mulmod_t pinv = info->pinv;
   long *CoeffModP = info->CoeffModP.elts();
   double *x = info->x.elts();
   long *u = info->u.elts();
   mulmod_precon_t *uqinv = info->uqinv.elts();
   long MinusMModP = info->MinusMModP;
   mulmod_precon_t MinusMModPpinv = info->MinusMModPpinv;
   mulmod_precon_t *CoeffModPpinv = info->CoeffModPpinv.elts();

   long q, s, t;
   long i;
   double y;

   y = double(0L);
   t = 0;

   for (i = 0; i < n; i++) {
      s = MulModPrecon(a[i], u[i], GetFFTPrime(i), uqinv[i]);
      y = y + double(s)*GetFFTPrimeRecip(i);


      // DIRT: uses undocumented MulMod feature (see sp_arith.h)
      // input s is not reduced mod p
      s = MulModPrecon(s, CoeffModP[i], p, CoeffModPpinv[i]);

      t = AddMod(t, s, p);
   }

   q = (long) (y + 0.5);

   // DIRT: uses undocumented MulMod feature (see sp_arith.h)
   // input q may not be reduced mod p
   s = MulModPrecon(q, MinusMModP, p, MinusMModPpinv);

   t = AddMod(t, s, p);
   res.LoopHole() = t;

}


#if 0
// converts entries lo..lo+cnt-1 in R and stores results into res
static 
void FromModularRep(zz_p* res, const fftRep& R, long lo, long cnt, 
                    zz_pInfoT* info)
{
   if (cnt <= 0) return;

   long nprimes = info->NumPrimes;
   long p = info->p;
   mulmod_t pinv = info->pinv;
   long *CoeffModP = info->CoeffModP.elts();
   double *x = info->x.elts();
   long *u = info->u.elts();
   mulmod_precon_t *uqinv = info->uqinv.elts();
   long MinusMModP = info->MinusMModP;
   mulmod_precon_t MinusMModPpinv = info->MinusMModPpinv;
   mulmod_precon_t *CoeffModPpinv = info->CoeffModPpinv.elts();

   long primes[4];
   double prime_recip[4];
   long *tbl[4];

   long q, s, t;
   long i, j;
   double y;

   for (i = 0; i < nprimes; i++) {
      primes[i] = GetFFTPrime(i);
      prime_recip[i] = GetFFTPrimeRecip(i);
      tbl[i] = R.tbl[i].get();
   }

   for (j = 0; j < cnt; j++) {
      y = double(0L);
      t = 0;

      for (i = 0; i < nprimes; i++) {
         s = MulModPrecon(tbl[i][j+lo], u[i], primes[i], uqinv[i]);
         y = y + double(s)*prime_recip[i];


         // DIRT: uses undocumented MulMod feature (see sp_arith.h)
         // input s is not reduced mod p
         s = MulModPrecon(s, CoeffModP[i], p, CoeffModPpinv[i]);

         t = AddMod(t, s, p);
      }

      q = (long) (y + 0.5);

      // DIRT: uses undocumented MulMod feature (see sp_arith.h)
      // input q may not be reduced mod p
      s = MulModPrecon(q, MinusMModP, p, MinusMModPpinv);

      t = AddMod(t, s, p);
      res[j].LoopHole() = t;
   }

}
#else

#define NTL_FMR_LOOP_BODY(i) \
         s = MulModPrecon(tbl[i][j+lo], u[i], primes[i], uqinv[i]);\
         y = y + double(s)*prime_recip[i];\
\
\
         /* DIRT: uses undocumented MulMod feature (see sp_arith.h) */\
         /* input s is not reduced mod p */\
         s = MulModPrecon(s, CoeffModP[i], p, CoeffModPpinv[i]);\
\
         t = AddMod(t, s, p);\


#define NTL_FMP_OUTER_LOOP(XXX) \
   for (j = 0; j < cnt; j++) {\
      y = double(0L);\
      t = 0;\
      XXX \
      q = (long) (y + 0.5);\
      /* DIRT: uses undocumented MulMod feature (see sp_arith.h) */\
      /* input q may not be reduced mod p */\
      s = MulModPrecon(q, MinusMModP, p, MinusMModPpinv);\
      t = AddMod(t, s, p);\
      res[j].LoopHole() = t;\
   }\



// converts entries lo..lo+cnt-1 in R and stores results into res
static 
void FromModularRep(zz_p* res, const fftRep& R, long lo, long cnt, 
                    zz_pInfoT* info)
{
   if (cnt <= 0) return;

   long nprimes = info->NumPrimes;
   long p = info->p;
   mulmod_t pinv = info->pinv;
   long *CoeffModP = info->CoeffModP.elts();
   double *x = info->x.elts();
   long *u = info->u.elts();
   mulmod_precon_t *uqinv = info->uqinv.elts();
   long MinusMModP = info->MinusMModP;
   mulmod_precon_t MinusMModPpinv = info->MinusMModPpinv;
   mulmod_precon_t *CoeffModPpinv = info->CoeffModPpinv.elts();

   long primes[4];
   double prime_recip[4];
   long *tbl[4];

   long q, s, t;
   long i, j;
   double y;

   for (i = 0; i < nprimes; i++) {
      primes[i] = GetFFTPrime(i);
      prime_recip[i] = GetFFTPrimeRecip(i);
      tbl[i] = R.tbl[i].get();
   }

   if (nprimes == 1) {
      long *tbl_0 = tbl[0];
      mulmod_precon_t CoeffModPpinv_0 = CoeffModPpinv[0];
      long primes_0 = primes[0];
      long hp0 = primes_0 >> 1;
      
      for (j = 0; j < cnt; j++) {
         s = tbl_0[j+lo];

         // DIRT: uses undocumented MulMod feature (see sp_arith.h)
         // input s is not reduced mod p
         t = MulModPrecon(s, 1, p, CoeffModPpinv_0);

         res[j].LoopHole() = AddMod(t, sp_SignMask(hp0-s) & MinusMModP, p);
      }
   }
   else if (nprimes == 2) {
      NTL_FMP_OUTER_LOOP( NTL_FMR_LOOP_BODY(0) NTL_FMR_LOOP_BODY(1) )
   }
   else if (nprimes == 3) {
      NTL_FMP_OUTER_LOOP( NTL_FMR_LOOP_BODY(0) NTL_FMR_LOOP_BODY(1) NTL_FMR_LOOP_BODY(2) )
   }
   else { // nprimes == 4
      NTL_FMP_OUTER_LOOP( NTL_FMR_LOOP_BODY(0) NTL_FMR_LOOP_BODY(1) NTL_FMR_LOOP_BODY(2)  NTL_FMR_LOOP_BODY(3) )
   }
}




#endif




void TofftRep_trunc(fftRep& y, const zz_pX& x, long k, 
                    long len, long lo, long hi)
// computes an n = 2^k point convolution.
// if deg(x) >= 2^k, then x is first reduced modulo X^n-1.
{
   zz_pInfoT *info = zz_pInfo;
   long p = info->p;

   long n, i, j, m, j1;
   long accum;
   long nprimes = info->NumPrimes;


   if (k > info->MaxRoot) 
      ResourceError("Polynomial too big for FFT");

   if (lo < 0)
      LogicError("bad arg to TofftRep");

   hi = min(hi, deg(x));

   y.SetSize(k);
   n = 1L << k;

   y.len = len = FFTRoundUp(len, k);

   m = max(hi-lo + 1, 0);
   long ilen = FFTRoundUp(m, k);

   const zz_p *xx = x.rep.elts();

   FFTPrimeInfo *p_info = info->p_info;

   if (p_info) {
      if (n >= m) {
         long *yp = &y.tbl[0][0];
         for (j = 0; j < m; j++) {
            yp[j] = rep(xx[j+lo]);
         }
         for (j = m; j < ilen; j++) {
            yp[j] = 0;
         }
      }
      else {
         for (j = 0; j < n; j++) {
            accum = rep(xx[j+lo]);
            for (j1 = j + n; j1 < m; j1 += n)
               accum = AddMod(accum, rep(xx[j1+lo]), p);
            y.tbl[0][j] = accum;
         }
      }
   }
   else {
      if (n >= m) {
         for (i = 0; i < nprimes; i++) {
            long q = GetFFTPrime(i);
            long *yp = &y.tbl[i][0];
            for (j = 0; j < m; j++) {
               long t = rep(xx[j+lo]);
               t = sp_CorrectExcess(t, q);
               yp[j] = t;
            }
            for (j = m; j < ilen; j++) {
               yp[j] = 0;
            }
         }
      }
      else {
         for (j = 0; j < n; j++) {
            accum = rep(xx[j+lo]);
            for (j1 = j + n; j1 < m; j1 += n)
               accum = AddMod(accum, rep(xx[j1+lo]), p);
            for (i = 0; i < nprimes; i++) {
               long q = GetFFTPrime(i);
               long t = accum;
               t = sp_CorrectExcess(t, q);
               y.tbl[i][j] = t;
            }
         }
      }
   }
   

   if (p_info) {
      long *yp = &y.tbl[0][0];
      FFTFwd_trunc(yp, yp, k, *p_info, len, ilen);
   } 
   else {
      for (i = 0; i < nprimes; i++) {
         long *yp = &y.tbl[i][0];
         FFTFwd_trunc(yp, yp, k, i, len, ilen);
      }
   }
}



void RevTofftRep(fftRep& y, const vec_zz_p& x, 
                 long k, long lo, long hi, long offset)
// computes an n = 2^k point convolution of X^offset*x[lo..hi] mod X^n-1
// using "inverted" evaluation points.

{
   zz_pInfoT *info = zz_pInfo;
   long p = info->p;

   long n, i, j, m, j1;
   long accum;
   long NumPrimes = info->NumPrimes;

   if (k > info->MaxRoot) 
      ResourceError("Polynomial too big for FFT");

   if (lo < 0)
      LogicError("bad arg to TofftRep");

   hi = min(hi, x.length()-1);

   y.SetSize(k);

   n = 1L << k;
   y.len = n;

   m = max(hi-lo + 1, 0);

   const zz_p *xx = x.elts();

   FFTPrimeInfo *p_info = info->p_info;

   offset = offset & (n-1);

   if (p_info) {
      for (j = 0; j < n; j++) {
         if (j >= m) {
            y.tbl[0][offset] = 0;
         }
         else {
            accum = rep(xx[j+lo]);
            for (j1 = j + n; j1 < m; j1 += n)
               accum = AddMod(accum, rep(xx[j1+lo]), p);
               y.tbl[0][offset] = accum;
         }
         offset = (offset + 1) & (n-1);
      }
   }
   else {
      for (j = 0; j < n; j++) {
         if (j >= m) {
            for (i = 0; i < NumPrimes; i++)
               y.tbl[i][offset] = 0;
         }
         else {
            accum = rep(xx[j+lo]);
            for (j1 = j + n; j1 < m; j1 += n)
               accum = AddMod(accum, rep(xx[j1+lo]), p);
            for (i = 0; i < NumPrimes; i++) {
               long q = GetFFTPrime(i);
               long t = accum;
               t = sp_CorrectExcess(t, q);
               y.tbl[i][offset] = t;
            }
         }
         offset = (offset + 1) & (n-1);
      }
   }


   if (p_info) {
      long *yp = &y.tbl[0][0];
      FFTRev1_trans(yp, yp, k, *p_info);
   }
   else {
      for (i = 0; i < info->NumPrimes; i++) {
         long *yp = &y.tbl[i][0];
         FFTRev1_trans(yp, yp, k, i);
      }
   }
}

void FromfftRep(zz_pX& x, fftRep& y, long lo, long hi)

   // converts from FFT-representation to coefficient representation
   // only the coefficients lo..hi are computed
   

{
   zz_pInfoT *info = zz_pInfo;

   long k, n, i, j, l;
   long NumPrimes = info->NumPrimes;


   k = y.k;
   n = (1L << k);

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);

   long len = y.len;
   if (len <= hi) LogicError("FromfftRep: bad len"); 

   FFTPrimeInfo *p_info = info->p_info;

   if (p_info) {
      long *yp = &y.tbl[0][0];
      FFTRev1_trunc(yp, yp, k, *p_info, len);
   }
   else {
      for (i = 0; i < NumPrimes; i++) {
         long *yp = &y.tbl[i][0];
         FFTRev1_trunc(yp, yp, k, i, len);
      }
   }

   x.rep.SetLength(l);

   if (p_info) {
      zz_p *xp = x.rep.elts();
      long *yp = &y.tbl[0][0];
      for (j = 0; j < l; j++) 
         xp[j].LoopHole() = yp[j+lo];
   }
   else {
      FromModularRep(x.rep.elts(), y, lo, l, info);
   }

   x.normalize();
}

void RevFromfftRep(vec_zz_p& x, fftRep& y, long lo, long hi)

   // converts from FFT-representation to coefficient representation
   // using "inverted" evaluation points.
   // only the coefficients lo..hi are computed
   

{
   zz_pInfoT *info = zz_pInfo;

   long k, n, i, j, l;
   long NumPrimes = info->NumPrimes;


   k = y.k;
   n = (1L << k);

   if (y.len != n) LogicError("RevFromfftRep: bad len");

   FFTPrimeInfo *p_info = info->p_info;

   if (p_info) {
      long *yp = &y.tbl[0][0];
      FFTFwd_trans(yp, yp, k, *p_info);
   }
   else {
      for (i = 0; i < NumPrimes; i++) {
         long *yp = &y.tbl[i][0];
         FFTFwd_trans(yp, yp, k, i);
      }
   }

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);
   x.SetLength(l);

   if (p_info) {
      zz_p *xp = x.elts();
      long *yp = &y.tbl[0][0];
      for (j = 0; j < l; j++) 
         xp[j].LoopHole() = yp[j+lo];
   }
   else {
      FromModularRep(x.elts(), y, lo, l, info);
   }
}

void NDFromfftRep(zz_pX& x, const fftRep& y, long lo, long hi, fftRep& z)
{
   zz_pInfoT *info = zz_pInfo;
   
   long k, n, i, j, l;
   long NumPrimes = info->NumPrimes;


   k = y.k;
   n = (1L << k);

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);

   long len = y.len;
   if (len <= hi) LogicError("FromfftRep: bad len");

   z.SetSize(k);

   FFTPrimeInfo *p_info = info->p_info;

   if (p_info) {
      long *zp = &z.tbl[0][0];
      const long *yp = &y.tbl[0][0];
      FFTRev1_trunc(zp, yp, k, *p_info, len);
   }
   else {
      for (i = 0; i < NumPrimes; i++) {
         long *zp = &z.tbl[i][0];
         const long *yp = &y.tbl[i][0];
         FFTRev1_trunc(zp, yp, k, i, len);
      }
   }

   x.rep.SetLength(l);

   if (p_info) {
      zz_p *xp = x.rep.elts();
      long *zp = &z.tbl[0][0];
      for (j = 0; j < l; j++) 
         xp[j].LoopHole() = zp[j+lo];
   }
   else {
      FromModularRep(x.rep.elts(), z, lo, l, info);
   }

   x.normalize();
}

void NDFromfftRep(zz_pX& x, fftRep& y, long lo, long hi)
{
   fftRep z;
   NDFromfftRep(x, y, lo, hi, z);
}

void FromfftRep(zz_p* x, fftRep& y, long lo, long hi)

   // converts from FFT-representation to coefficient representation
   // only the coefficients lo..hi are computed
   

{
   zz_pInfoT *info = zz_pInfo;

   long k, n, i, j;
   long NumPrimes = info->NumPrimes;


   k = y.k;
   n = (1L << k);


   //if (y.len <= min(hi, n-1)) LogicError("FromfftRep: bad len");
   if (y.len != n) LogicError("FromfftRep: bad len");

   FFTPrimeInfo *p_info = info->p_info;

   if (p_info) {
      long *yp = &y.tbl[0][0];
      FFTRev1(yp, yp, k, *p_info);

      for (j = lo; j <= hi; j++) {
         if (j >= n)
            clear(x[j-lo]);
         else {
            x[j-lo].LoopHole() = y.tbl[0][j];
         }
      }
   }
   else {
      for (i = 0; i < NumPrimes; i++) {
         long *yp = &y.tbl[i][0];
         FFTRev1(yp, yp, k, i);
      }

      // take coefficients lo..min(hi, n-1) from y
      // zero out coefficients max(n, lo)..hi
   
      long l = min(hi, n-1) - lo + 1;
      l = max(l, 0);
      FromModularRep(x, y, lo, l, info); 
      for (j = max(n, lo); j <= hi; j++) clear(x[j-lo]);
   }
}


void mul(fftRep& z, const fftRep& x, const fftRep& y)
{
   zz_pInfoT *info = zz_pInfo;

   long k, n, i, j;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;
   n = 1L << k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   FFTPrimeInfo *p_info = info->p_info;

   if (p_info) {
      long *zp = &z.tbl[0][0];
      const long *xp = &x.tbl[0][0];
      const long *yp = &y.tbl[0][0];
      long q = p_info->q;
      mulmod_t qinv = p_info->qinv;

      if (NormalizedModulus(qinv)) {
         for (j = 0; j < len; j++)
            zp[j] = NormalizedMulMod(xp[j], yp[j], q, qinv);
      }
      else {
         for (j = 0; j < len; j++)
            zp[j] = MulMod(xp[j], yp[j], q, qinv);
      }
   }
   else {
      for (i = 0; i < info->NumPrimes; i++) {
         long *zp = &z.tbl[i][0];
         const long *xp = &x.tbl[i][0];
         const long *yp = &y.tbl[i][0];
         long q = GetFFTPrime(i);
         mulmod_t qinv = GetFFTPrimeInv(i);
   
         for (j = 0; j < len; j++)
            zp[j] = NormalizedMulMod(xp[j], yp[j], q, qinv);
      }
   }
}

void sub(fftRep& z, const fftRep& x, const fftRep& y)
{
   zz_pInfoT *info = zz_pInfo;

   long k, n, i, j;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;
   n = 1L << k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   FFTPrimeInfo *p_info = info->p_info;

   if (p_info) {
      long *zp = &z.tbl[0][0];
      const long *xp = &x.tbl[0][0];
      const long *yp = &y.tbl[0][0];
      long q = p_info->q;

      for (j = 0; j < len; j++)
         zp[j] = SubMod(xp[j], yp[j], q);
   }
   else {
      for (i = 0; i < info->NumPrimes; i++) {
         long *zp = &z.tbl[i][0];
         const long *xp = &x.tbl[i][0];
         const long *yp = &y.tbl[i][0];
         long q = GetFFTPrime(i);
   
         for (j = 0; j < len; j++)
            zp[j] = SubMod(xp[j], yp[j], q);
      }
   }
}

void add(fftRep& z, const fftRep& x, const fftRep& y)
{
   zz_pInfoT *info = zz_pInfo;

   long k, n, i, j;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;
   n = 1L << k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   FFTPrimeInfo *p_info = info->p_info;

   if (p_info) {
      long *zp = &z.tbl[0][0];
      const long *xp = &x.tbl[0][0];
      const long *yp = &y.tbl[0][0];
      long q = p_info->q;

      for (j = 0; j < len; j++)
         zp[j] = AddMod(xp[j], yp[j], q);
   }
   else {
      for (i = 0; i < info->NumPrimes; i++) {
         long *zp = &z.tbl[i][0];
         const long *xp = &x.tbl[i][0];
         const long *yp = &y.tbl[i][0];
         long q = GetFFTPrime(i);
   
         for (j = 0; j < len; j++)
            zp[j] = AddMod(xp[j], yp[j], q);
      }
   }
}


void reduce(fftRep& x, const fftRep& a, long k)
  // reduces a 2^l point FFT-rep to a 2^k point FFT-rep
  // input may alias output
{
   zz_pInfoT *info = zz_pInfo;

   long i, j, l, n;
   long* xp;
   const long* ap;

   l = a.k;
   n = 1L << k;

   if (l < k) LogicError("reduce: bad operands");
   if (a.len < n) LogicError("reduce: bad len");

   x.SetSize(k);
   x.len = n;

   if (&x == &a) return;

   for (i = 0; i < info->NumPrimes; i++) {
      ap = &a.tbl[i][0];   
      xp = &x.tbl[i][0];
      for (j = 0; j < n; j++) 
         xp[j] = ap[j];
   }
}


void AddExpand(fftRep& x, const fftRep& a)
//  x = x + (an "expanded" version of a)
{
   zz_pInfoT *info = zz_pInfo;

   long i, j, l, k, n;

   l = x.k;
   k = a.k;
   n = 1L << k;

   if (l < k) LogicError("AddExpand: bad args");
   if (x.len < n) LogicError("AddExpand: bad len");

   FFTPrimeInfo *p_info = info->p_info;
   
   if (p_info) {
      long q = p_info->q;
      const long *ap = &a.tbl[0][0];
      long *xp = &x.tbl[0][0];
      for (j = 0; j < n; j++) {
         xp[j] = AddMod(xp[j], ap[j], q);
      }
   }
   else {
      for (i = 0; i < info->NumPrimes; i++) {
         long q = GetFFTPrime(i);
         const long *ap = &a.tbl[i][0];
         long *xp = &x.tbl[i][0];
         for (j = 0; j < n; j++) {
            xp[j] = AddMod(xp[j], ap[j], q);
         }
      }
   }
}


void FFTMul(zz_pX& x, const zz_pX& a, const zz_pX& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(x);
      return;
   }

   long da = deg(a);
   long db = deg(b);
   long d = da+db;
   long k = NextPowerOfTwo(d+1);

   fftRep R1(INIT_SIZE, k), R2(INIT_SIZE, k);

   TofftRep_trunc(R1, a, k, d+1);
   TofftRep_trunc(R2, b, k, d+1);
   mul(R1, R1, R2);
   FromfftRep(x, R1, 0, d);
}

void FFTSqr(zz_pX& x, const zz_pX& a)
{
   if (IsZero(a)) {
      clear(x);
      return;
   }

   long da = deg(a);
   long d = 2*da;
   long k = NextPowerOfTwo(d+1);

   fftRep R1(INIT_SIZE, k);

   TofftRep_trunc(R1, a, k, d+1);
   mul(R1, R1, R1);
   FromfftRep(x, R1, 0, d);
}


void CopyReverse(zz_pX& x, const zz_pX& a, long lo, long hi)

   // x[0..hi-lo] = reverse(a[lo..hi]), with zero fill
   // input may not alias output

{
   long i, j, n, m;

   n = hi-lo+1;
   m = a.rep.length();

   x.rep.SetLength(n);

   const zz_p* ap = a.rep.elts();
   zz_p* xp = x.rep.elts();

   for (i = 0; i < n; i++) {
      j = hi-i;
      if (j < 0 || j >= m)
         clear(xp[i]);
      else
         xp[i] = ap[j];
   }

   x.normalize();
} 

void copy(zz_pX& x, const zz_pX& a, long lo, long hi)

   // x[0..hi-lo] = a[lo..hi], with zero fill
   // input may not alias output

{
   long i, j, n, m;

   n = hi-lo+1;
   m = a.rep.length();

   x.rep.SetLength(n);

   const zz_p* ap = a.rep.elts();
   zz_p* xp = x.rep.elts();

   for (i = 0; i < n; i++) {
      j = lo + i;
      if (j < 0 || j >= m)
         clear(xp[i]);
      else
         xp[i] = ap[j];
   }

   x.normalize();
} 


void rem21(zz_pX& x, const zz_pX& a, const zz_pXModulus& F)
{
   long i, da, ds, n, kk;

   da = deg(a);
   n = F.n;

   if (da > 2*n-2)
      LogicError("bad args to rem(zz_pX,zz_pX,zz_pXModulus)");


   if (da < n) {
      x = a;
      return;
   }

   if (!F.UseFFT || da - n <= NTL_zz_pX_MOD_CROSSOVER) {
      PlainRem(x, a, F.f);
      return;
   }

   fftRep R1(INIT_SIZE, F.l);
   zz_pX P1(INIT_SIZE, n);

   TofftRep_trunc(R1, a, F.l, 2*n-3, n, 2*(n-1));
   mul(R1, R1, F.HRep);
   FromfftRep(P1, R1, n-2, 2*n-4);

   TofftRep(R1, P1, F.k);
   mul(R1, R1, F.FRep);
   FromfftRep(P1, R1, 0, n-1);

   ds = deg(P1);

   kk = 1L << F.k;

   x.rep.SetLength(n);
   const zz_p* aa = a.rep.elts();
   const zz_p* ss = P1.rep.elts();
   zz_p* xx = x.rep.elts();

   for (i = 0; i < n; i++) {
      if (i <= ds)
         sub(xx[i], aa[i], ss[i]);
      else
         xx[i] = aa[i];

      if (i + kk <= da)
         add(xx[i], xx[i], aa[i+kk]);
   }

   x.normalize();
}


void DivRem21(zz_pX& q, zz_pX& x, const zz_pX& a, const zz_pXModulus& F)
{
   long i, da, ds, n, kk;

   da = deg(a);
   n = F.n;

   if (da > 2*n-2)
      LogicError("bad args to rem(zz_pX,zz_pX,zz_pXModulus)");


   if (da < n) {
      x = a;
      clear(q);
      return;
   }

   if (!F.UseFFT || da - n <= NTL_zz_pX_MOD_CROSSOVER) {
      PlainDivRem(q, x, a, F.f);
      return;
   }

   fftRep R1(INIT_SIZE, F.l);
   zz_pX P1(INIT_SIZE, n), qq;

   TofftRep_trunc(R1, a, F.l, 2*n-3, n, 2*(n-1));
   mul(R1, R1, F.HRep);
   FromfftRep(P1, R1, n-2, 2*n-4);
   qq = P1;

   TofftRep(R1, P1, F.k);
   mul(R1, R1, F.FRep);
   FromfftRep(P1, R1, 0, n-1);

   ds = deg(P1);

   kk = 1L << F.k;

   x.rep.SetLength(n);
   const zz_p* aa = a.rep.elts();
   const zz_p* ss = P1.rep.elts();
   zz_p* xx = x.rep.elts();

   for (i = 0; i < n; i++) {
      if (i <= ds)
         sub(xx[i], aa[i], ss[i]);
      else
         xx[i] = aa[i];

      if (i + kk <= da)
         add(xx[i], xx[i], aa[i+kk]);
   }

   x.normalize();
   q = qq;
}

void div21(zz_pX& x, const zz_pX& a, const zz_pXModulus& F)
{
   long da, n;

   da = deg(a);
   n = F.n;

   if (da > 2*n-2)
      LogicError("bad args to rem(zz_pX,zz_pX,zz_pXModulus)");


   if (da < n) {
      clear(x);
      return;
   }

   if (!F.UseFFT || da - n <= NTL_zz_pX_MOD_CROSSOVER) {
      PlainDiv(x, a, F.f);
      return;
   }

   fftRep R1(INIT_SIZE, F.l);
   zz_pX P1(INIT_SIZE, n);

   TofftRep_trunc(R1, a, F.l, 2*n-3, n, 2*(n-1));
   mul(R1, R1, F.HRep);
   FromfftRep(x, R1, n-2, 2*n-4);
}


void rem(zz_pX& x, const zz_pX& a, const zz_pXModulus& F)
{
   long da = deg(a);
   long n = F.n;

   if (n < 0) LogicError("rem: uninitialized modulus");

   if (da <= 2*n-2) {
      rem21(x, a, F);
      return;
   }
   else if (!F.UseFFT || da-n <= NTL_zz_pX_MOD_CROSSOVER) {
      PlainRem(x, a, F.f);
      return;
   }

   zz_pX buf(INIT_SIZE, 2*n-1);

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

      rem21(buf, buf, F);

      a_len -= amt;
   }

   x = buf;
}

void DivRem(zz_pX& q, zz_pX& r, const zz_pX& a, const zz_pXModulus& F)
{
   long da = deg(a);
   long n = F.n;

   if (n < 0) LogicError("DivRem: uninitialized modulus");

   if (da <= 2*n-2) {
      DivRem21(q, r, a, F);
      return;
   }
   else if (!F.UseFFT || da-n <= NTL_zz_pX_MOD_CROSSOVER) {
      PlainDivRem(q, r, a, F.f);
      return;
   }

   zz_pX buf(INIT_SIZE, 2*n-1);
   zz_pX qbuf(INIT_SIZE, n-1);

   zz_pX qq;
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

      DivRem21(qbuf, buf, buf, F);
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

void div(zz_pX& q, const zz_pX& a, const zz_pXModulus& F)
{
   long da = deg(a);
   long n = F.n;

   if (n < 0) LogicError("div: uninitialized modulus");

   if (da <= 2*n-2) {
      div21(q, a, F);
      return;
   }
   else if (!F.UseFFT || da-n <= NTL_zz_pX_MOD_CROSSOVER) {
      PlainDiv(q, a, F.f);
      return;
   }

   zz_pX buf(INIT_SIZE, 2*n-1);
   zz_pX qbuf(INIT_SIZE, n-1);

   zz_pX qq;
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
         DivRem21(qbuf, buf, buf, F);
      else
         div21(qbuf, buf, F);

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


void MulMod(zz_pX& x, const zz_pX& a, const zz_pX& b, const zz_pXModulus& F)
{
   long  da, db, d, n, k;

   da = deg(a);
   db = deg(b);
   n = F.n;

   if (n < 0) LogicError("MulMod: uninitialized modulus");

   if (da >= n || db >= n)
      LogicError("bad args to MulMod(zz_pX,zz_pX,zz_pX,zz_pXModulus)");

   if (da < 0 || db < 0) {
      clear(x);
      return;
   }

   if (!F.UseFFT || da <= NTL_zz_pX_MUL_CROSSOVER || db <= NTL_zz_pX_MUL_CROSSOVER) {
      zz_pX P1;
      mul(P1, a, b);
      rem(x, P1, F);
      return;
   }

   d = da + db + 1;

   k = NextPowerOfTwo(d);
   k = max(k, F.k);

   fftRep R1(INIT_SIZE, k), R2(INIT_SIZE, F.l);
   zz_pX P1(INIT_SIZE, n);

   long len;
   if (zz_p::IsFFTPrime()) 
      len = n;
   else
      len = 1L << F.k;

   TofftRep_trunc(R1, a, k, max(1L << F.k, d));
   TofftRep_trunc(R2, b, k, max(1L << F.k, d));
   mul(R1, R1, R2);
   NDFromfftRep(P1, R1, n, d-1, R2); // save R1 for future use

   TofftRep_trunc(R2, P1, F.l, 2*n-3);
   mul(R2, R2, F.HRep);
   FromfftRep(P1, R2, n-2, 2*n-4);

   TofftRep_trunc(R2, P1, F.k, len);
   mul(R2, R2, F.FRep);
   reduce(R1, R1, F.k);
   sub(R1, R1, R2);
   FromfftRep(x, R1, 0, n-1);
}

void SqrMod(zz_pX& x, const zz_pX& a, const zz_pXModulus& F)
{
   long  da, d, n, k;

   da = deg(a);
   n = F.n;

   if (n < 0) LogicError("SqrMod: uninitialized modulus");

   if (da >= n) 
      LogicError("bad args to SqrMod(zz_pX,zz_pX,zz_pXModulus)");

   if (!F.UseFFT || da <= NTL_zz_pX_MUL_CROSSOVER) {
      zz_pX P1;
      sqr(P1, a);
      rem(x, P1, F);
      return;
   }


   d = 2*da + 1;

   k = NextPowerOfTwo(d);
   k = max(k, F.k);

   fftRep R1(INIT_SIZE, k), R2(INIT_SIZE, F.l);
   zz_pX P1(INIT_SIZE, n);

   long len;
   if (zz_p::IsFFTPrime()) 
      len = n;
   else
      len = 1L << F.k;

   TofftRep_trunc(R1, a, k, max(1L << F.k, d));
   mul(R1, R1, R1);
   NDFromfftRep(P1, R1, n, d-1, R2); // save R1 for future use

   TofftRep_trunc(R2, P1, F.l, 2*n-3);
   mul(R2, R2, F.HRep);
   FromfftRep(P1, R2, n-2, 2*n-4);

   TofftRep_trunc(R2, P1, F.k, len);
   mul(R2, R2, F.FRep);
   reduce(R1, R1, F.k);
   sub(R1, R1, R2);
   FromfftRep(x, R1, 0, n-1);
}

void PlainInvTrunc(zz_pX& x, const zz_pX& a, long m)

   /* x = (1/a) % X^m, input not output, constant term a is nonzero */

{
   long i, k, n, lb;
   zz_p v, t;
   zz_p s;
   const zz_p* ap;
   zz_p* xp;
   

   n = deg(a);

   if (n < 0) ArithmeticError("division by zero");

   inv(s, ConstTerm(a));

   if (n == 0) {
      conv(x, s);
      return;
   }

   ap = a.rep.elts();
   x.rep.SetLength(m);
   xp = x.rep.elts();

   xp[0] = s;

   long is_one = IsOne(s);

   for (k = 1; k < m; k++) {
      clear(v);
      lb = max(k-n, 0);
      for (i = lb; i <= k-1; i++) {
         mul(t, xp[i], ap[k-i]);
         add(v, v, t);
      }
      xp[k] = v;
      negate(xp[k], xp[k]);
      if (!is_one) mul(xp[k], xp[k], s);
   }

   x.normalize();
}


void trunc(zz_pX& x, const zz_pX& a, long m)

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
      zz_p* xp;
      const zz_p* ap;

      n = min(a.rep.length(), m);
      x.rep.SetLength(n);

      xp = x.rep.elts();
      ap = a.rep.elts();

      for (i = 0; i < n; i++) xp[i] = ap[i];

      x.normalize();
   }
}

void CyclicReduce(zz_pX& x, const zz_pX& a, long m)

// computes x = a mod X^m-1

{
   long n = deg(a);
   long i, j;
   long accum;
   long p = zz_p::modulus();

   if (n < m) {
      x = a;
      return;
   }

   if (&x != &a)
      x.rep.SetLength(m);

   for (i = 0; i < m; i++) {
      accum = rep(a.rep[i]);
      for (j = i + m; j <= n; j += m)
         accum = AddMod(accum, rep(a.rep[j]), p);
      x.rep[i].LoopHole() = accum;
   }

   if (&x == &a)
      x.rep.SetLength(m);

   x.normalize();
}



void InvTrunc(zz_pX& x, const zz_pX& a, long m)
{
   if (m < 0) LogicError("InvTrunc: bad args");
   if (m == 0) {
      clear(x);
      return;
   }

   if (NTL_OVERFLOW(m, 1, 0))
      ResourceError("overflow in InvTrunc");

   if (&x == &a) {
      zz_pX la;
      la = a;
      if (m > NTL_zz_pX_NEWTON_CROSSOVER && deg(a) > 0)
         NewtonInvTrunc(x, la, m);
      else
         PlainInvTrunc(x, la, m);
   }
   else {
      if (m > NTL_zz_pX_NEWTON_CROSSOVER && deg(a) > 0)
         NewtonInvTrunc(x, a, m);
      else
         PlainInvTrunc(x, a, m);
   }
}
   


void build(zz_pXModulus& x, const zz_pX& f)
{
   x.f = f;
   x.n = deg(f);

   x.tracevec.make();

   if (x.n <= 0)
      LogicError("build: deg(f) must be at least 1");

   if (x.n <= NTL_zz_pX_MOD_CROSSOVER + 1) {
      x.UseFFT = 0;
      return;
   }

   x.UseFFT = 1;

   x.k = NextPowerOfTwo(x.n);
   x.l = NextPowerOfTwo(2*x.n - 3);
   TofftRep(x.FRep, f, x.k);

   zz_pX P1(INIT_SIZE, x.n+1), P2(INIT_SIZE, x.n);

   CopyReverse(P1, f, 0, x.n);
   InvTrunc(P2, P1, x.n-1);

   CopyReverse(P1, P2, 0, x.n-2);
   TofftRep(x.HRep, P1, x.l);
}

zz_pXModulus::zz_pXModulus(const zz_pX& ff)
{
   build(*this, ff);
}

zz_pXMultiplier::zz_pXMultiplier(const zz_pX& b, const zz_pXModulus& F)
{
   build(*this, b, F);
}



void build(zz_pXMultiplier& x, const zz_pX& b, 
                         const zz_pXModulus& F)
{
   long db;
   long n = F.n;

   if (n < 0) LogicError("build zz_pXMultiplier: uninitialized modulus"); 

   x.b = b;
   db = deg(b);

   if (db >= n) LogicError("build zz_pXMultiplier: deg(b) >= deg(f)");

   if (!F.UseFFT || db <= NTL_zz_pX_MOD_CROSSOVER) {
      x.UseFFT = 0;
      return;
   }

   x.UseFFT = 1;

   fftRep R1(INIT_SIZE, F.l);
   zz_pX P1(INIT_SIZE, n);
   

   TofftRep_trunc(R1, b, F.l, 2*n-2);
   reduce(x.B2, R1, F.k);
   mul(R1, R1, F.HRep);
   FromfftRep(P1, R1, n-1, 2*n-3); 

   TofftRep(x.B1, P1, F.l);
   // could be truncated to length max(1L << F.k, 2*n-2), except
   // for the usage in UpdateMap, where we would have to investigate
   // further

}


void MulMod(zz_pX& x, const zz_pX& a, const zz_pXMultiplier& B,
                                      const zz_pXModulus& F)
{

   long n = F.n;
   long da;

   da = deg(a);

   if (da >= n)
      LogicError(" bad args to MulMod(zz_pX,zz_pX,zz_pXMultiplier,zz_pXModulus)");

   if (da < 0) {
      clear(x);
      return;
   }

   if (!B.UseFFT || !F.UseFFT || da <= NTL_zz_pX_MOD_CROSSOVER) {
      zz_pX P1;
      mul(P1, a, B.b);
      rem(x, P1, F);
      return;
   }

   zz_pX P1(INIT_SIZE, n), P2(INIT_SIZE, n);
   fftRep R1(INIT_SIZE, F.l), R2(INIT_SIZE, F.l);

   long len;
   if (zz_p::IsFFTPrime()) 
      len = n;
   else
      len = 1L << F.k;

   TofftRep_trunc(R1, a, F.l, max(1L << F.k, 2*n-2));
   mul(R2, R1, B.B1);
   FromfftRep(P1, R2, n-1, 2*n-3);

   reduce(R1, R1, F.k);
   mul(R1, R1, B.B2);
   TofftRep_trunc(R2, P1, F.k, len);
   mul(R2, R2, F.FRep);
   sub(R1, R1, R2);

   FromfftRep(x, R1, 0, n-1);
}
   

void PowerXMod(zz_pX& hh, const ZZ& e, const zz_pXModulus& F)
{
   if (F.n < 0) LogicError("PowerXMod: uninitialized modulus");

   if (IsZero(e)) {
      set(hh);
      return;
   }

   long n = NumBits(e);
   long i;

   zz_pX h;

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



void PowerXPlusAMod(zz_pX& hh, zz_p a, const ZZ& e, const zz_pXModulus& F)
{
   if (F.n < 0) LogicError("PowerXPlusAMod: uninitialized modulus");

   if (IsZero(e)) {
      set(hh);
      return;
   }

   zz_pX t1(INIT_SIZE, F.n), t2(INIT_SIZE, F.n);
   long n = NumBits(e);
   long i;

   zz_pX h;

   h.SetMaxLength(F.n);
   set(h);

   for (i = n - 1; i >= 0; i--) {
      SqrMod(h, h, F);
      if (bit(e, i)) {
         MulByXMod(t1, h, F.f);
         mul(t2, h, a);
         add(h, t1, t2);
      }
   }

   if (e < 0) InvMod(h, h, F);

   hh = h;
}



void PowerMod(zz_pX& h, const zz_pX& g, const ZZ& e, const zz_pXModulus& F)
{
   if (deg(g) >= F.n) LogicError("PowerMod: bad args");

   if (IsZero(e)) {
      set(h);
      return;
   }

   zz_pXMultiplier G;

   zz_pX res;

   long n = NumBits(e);
   long i;

   build(G, g, F);

   res.SetMaxLength(F.n);
   set(res);

   for (i = n - 1; i >= 0; i--) {
      SqrMod(res, res, F);
      if (bit(e, i))
         MulMod(res, res, G, F);
   }

   if (e < 0) InvMod(res, res, F);

   h = res;
}


void NewtonInvTrunc(zz_pX& x, const zz_pX& a, long m)
{
   x.SetMaxLength(m);

   long i;
   long t;


   t = NextPowerOfTwo(2*m-1);

   fftRep R1(INIT_SIZE, t), R2(INIT_SIZE, t);
   zz_pX P1(INIT_SIZE, m);

   long log2_newton = NextPowerOfTwo(NTL_zz_pX_NEWTON_CROSSOVER)-1;

   PlainInvTrunc(x, a, 1L << log2_newton);
   long k = 1L << log2_newton;
   long a_len = min(m, a.rep.length());

   while (k < m) {
      long l = min(2*k, m);

      t = NextPowerOfTwo(2*k);
      TofftRep(R1, x, t);
      mul(R1, R1, R1);
      FromfftRep(P1, R1, 0, l-1);

      t = NextPowerOfTwo(deg(P1) + min(l, a_len));
      TofftRep(R1, P1, t);
      TofftRep(R2, a, t, 0, min(l, a_len)-1);
      mul(R1, R1, R2);
      FromfftRep(P1, R1, k, l-1);
      
      x.rep.SetLength(l);
      long y_len = P1.rep.length();
      for (i = k; i < l; i++) {
         if (i-k >= y_len)
            clear(x.rep[i]);
         else
            negate(x.rep[i], P1.rep[i-k]);
      }
      x.normalize();

      k = l;
   }
}




void FFTDivRem(zz_pX& q, zz_pX& r, const zz_pX& a, const zz_pX& b)
{
   long n = deg(b);
   long m = deg(a);
   long k, l;

   if (m < n) {
      clear(q);
      r = a;
      return;
   }

   if (m >= 3*n) {
      zz_pXModulus B;
      build(B, b);
      DivRem(q, r, a, B);
      return;
   }

   zz_pX P1, P2, P3;

   CopyReverse(P3, b, 0, n);
   InvTrunc(P2, P3, m-n+1);
   CopyReverse(P1, P2, 0, m-n);

   k = NextPowerOfTwo(2*(m-n)+1);
   long k1 = NextPowerOfTwo(n);
   long mx = max(k1, k);

   fftRep R1(INIT_SIZE, mx), R2(INIT_SIZE, mx);

   TofftRep(R1, P1, k);
   TofftRep(R2, a, k, n, m);
   mul(R1, R1, R2);
   FromfftRep(P3, R1, m-n, 2*(m-n));
   
   l = 1L << k1;

   
   TofftRep(R1, b, k1);
   TofftRep(R2, P3, k1);
   mul(R1, R1, R2);
   FromfftRep(P1, R1, 0, n-1);
   CyclicReduce(P2, a, l);
   trunc(r, P2, n);
   sub(r, r, P1);
   q = P3;
}




void FFTDiv(zz_pX& q, const zz_pX& a, const zz_pX& b)
{

   long n = deg(b);
   long m = deg(a);
   long k;

   if (m < n) {
      clear(q);
      return;
   }

   if (m >= 3*n) {
      zz_pXModulus B;
      build(B, b);
      div(q, a, B);
      return;
   }

   zz_pX P1, P2, P3;

   CopyReverse(P3, b, 0, n);
   InvTrunc(P2, P3, m-n+1);
   CopyReverse(P1, P2, 0, m-n);

   k = NextPowerOfTwo(2*(m-n)+1);

   fftRep R1(INIT_SIZE, k), R2(INIT_SIZE, k);

   TofftRep(R1, P1, k);
   TofftRep(R2, a, k, n, m);
   mul(R1, R1, R2);
   FromfftRep(q, R1, m-n, 2*(m-n));
}



void FFTRem(zz_pX& r, const zz_pX& a, const zz_pX& b)
{
   long n = deg(b);
   long m = deg(a);
   long k, l;

   if (m < n) {
      r = a;
      return;
   }

   if (m >= 3*n) {
      zz_pXModulus B;
      build(B, b);
      rem(r, a, B);
      return;
   }

   zz_pX P1, P2, P3;

   CopyReverse(P3, b, 0, n);
   InvTrunc(P2, P3, m-n+1);
   CopyReverse(P1, P2, 0, m-n);

   k = NextPowerOfTwo(2*(m-n)+1);
   long k1 = NextPowerOfTwo(n);
   long mx = max(k, k1);

   fftRep R1(INIT_SIZE, mx), R2(INIT_SIZE, mx);

   TofftRep(R1, P1, k);
   TofftRep(R2, a, k, n, m);
   mul(R1, R1, R2);
   FromfftRep(P3, R1, m-n, 2*(m-n));
   
   l = 1L << k1;

   
   TofftRep(R1, b, k1);
   TofftRep(R2, P3, k1);
   mul(R1, R1, R2);
   FromfftRep(P3, R1, 0, n-1);
   CyclicReduce(P2, a, l);
   trunc(r, P2, n);
   sub(r, r, P3);
}


void DivRem(zz_pX& q, zz_pX& r, const zz_pX& a, const zz_pX& b)
{
   if (deg(b) > NTL_zz_pX_DIV_CROSSOVER && deg(a) - deg(b) > NTL_zz_pX_DIV_CROSSOVER)
      FFTDivRem(q, r, a, b);
   else
      PlainDivRem(q, r, a, b);
}

void div(zz_pX& q, const zz_pX& a, const zz_pX& b)
{
   if (deg(b) > NTL_zz_pX_DIV_CROSSOVER && deg(a) - deg(b) > NTL_zz_pX_DIV_CROSSOVER)
      FFTDiv(q, a, b);
   else
      PlainDiv(q, a, b);
}

void div(zz_pX& q, const zz_pX& a, zz_p b)
{
   zz_p t;
   inv(t, b);
   mul(q, a, t);
}


void rem(zz_pX& r, const zz_pX& a, const zz_pX& b)
{
   if (deg(b) > NTL_zz_pX_DIV_CROSSOVER && deg(a) - deg(b) > NTL_zz_pX_DIV_CROSSOVER)
      FFTRem(r, a, b);
   else
      PlainRem(r, a, b);
}



long operator==(const zz_pX& a, long b)
{
   if (b == 0)
      return IsZero(a);

   if (b == 1)
      return IsOne(a);

   long da = deg(a);

   if (da > 0)
      return 0;

   zz_p bb;
   bb = b;

   if (da < 0)
      return IsZero(bb);

   return a.rep[0] == bb;
}

long operator==(const zz_pX& a, zz_p b)
{
   if (IsZero(b))
      return IsZero(a);

   long da = deg(a);

   if (da != 0)
      return 0;

   return a.rep[0] == b;
}

void power(zz_pX& x, const zz_pX& a, long e)
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

   zz_pX res;
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

void reverse(zz_pX& x, const zz_pX& a, long hi)
{
   if (hi < 0) { clear(x); return; }
   if (NTL_OVERFLOW(hi, 1, 0))
      ResourceError("overflow in reverse");

   if (&x == &a) {
      zz_pX tmp;
      CopyReverse(tmp, a, 0, hi);
      x = tmp;
   }
   else
      CopyReverse(x, a, 0, hi);
}

NTL_END_IMPL

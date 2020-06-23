#include <NTL/ZZ_pX.h>
#include <NTL/BasicThreadPool.h>
#include <NTL/FFT_impl.h>


// The mul & sqr routines use routines from ZZX, 
// which is faster for small degree polynomials.
// Define this macro to revert to old strategy.


#ifndef NTL_WIZARD_HACK

#include <NTL/ZZX.h>

#endif

NTL_START_IMPL


#if (defined(NTL_GMP_LIP))
#define KARX 200
#else
#define KARX 80
#endif

#define PAR_THRESH (4000.0)

#define PAR_THRESH1 (20000.0)
// Higher threshold for cheaper operations

static inline bool BelowThresh(long n)
{
   return double(n)*double(ZZ_p::ModulusSize()) < PAR_THRESH;
}

static inline bool BelowThresh1(long n)
{
   return double(n)*double(ZZ_p::ModulusSize()) < PAR_THRESH1;
}



const ZZ_pX& ZZ_pX::zero()
{
   static const ZZ_pX z; // GLOBAL (relies on C++11 thread-safe init)
   return z;
}


ZZ_pX& ZZ_pX::operator=(long a)
{
   conv(*this, a);
   return *this;
}


ZZ_pX& ZZ_pX::operator=(const ZZ_p& a)
{
   conv(*this, a);
   return *this;
}


istream& operator>>(istream& s, ZZ_pX& x)
{
   NTL_INPUT_CHECK_RET(s, s >> x.rep);
   x.normalize();
   return s;
}

ostream& operator<<(ostream& s, const ZZ_pX& a)
{
   return s << a.rep;
}


void ZZ_pX::normalize()
{
   long n;
   const ZZ_p* p;

   n = rep.length();
   if (n == 0) return;
   p = rep.elts() + n;
   while (n > 0 && IsZero(*--p)) {
      n--;
   }
   rep.SetLength(n);
}


long IsZero(const ZZ_pX& a)
{
   return a.rep.length() == 0;
}


long IsOne(const ZZ_pX& a)
{
    return a.rep.length() == 1 && IsOne(a.rep[0]);
}

void GetCoeff(ZZ_p& x, const ZZ_pX& a, long i)
{
   if (i < 0 || i > deg(a))
      clear(x);
   else
      x = a.rep[i];
}

void SetCoeff(ZZ_pX& x, long i, const ZZ_p& a)
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
         NTL_ZZ_pRegister(aa);
         aa = a;
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

void SetCoeff(ZZ_pX& x, long i, long a)
{
   if (a == 1) 
      SetCoeff(x, i);
   else {
      NTL_ZZ_pRegister(T);
      conv(T, a);
      SetCoeff(x, i, T);
   }
}

void SetCoeff(ZZ_pX& x, long i)
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


void SetX(ZZ_pX& x)
{
   clear(x);
   SetCoeff(x, 1);
}


long IsX(const ZZ_pX& a)
{
   return deg(a) == 1 && IsOne(LeadCoeff(a)) && IsZero(ConstTerm(a));
}
      
      

const ZZ_p& coeff(const ZZ_pX& a, long i)
{
   if (i < 0 || i > deg(a))
      return ZZ_p::zero();
   else
      return a.rep[i];
}


const ZZ_p& LeadCoeff(const ZZ_pX& a)
{
   if (IsZero(a))
      return ZZ_p::zero();
   else
      return a.rep[deg(a)];
}

const ZZ_p& ConstTerm(const ZZ_pX& a)
{
   if (IsZero(a))
      return ZZ_p::zero();
   else
      return a.rep[0];
}



void conv(ZZ_pX& x, const ZZ_p& a)
{
   if (IsZero(a))
      x.rep.SetLength(0);
   else {
      x.rep.SetLength(1);
      x.rep[0] = a;

      // note: if a aliases x.rep[i], i > 0, this code
      //       will still work, since is is assumed that
      //       SetLength(1) will not relocate or destroy x.rep[i]
   }
}

void conv(ZZ_pX& x, long a)
{
   if (a == 0)
      clear(x);
   else if (a == 1)
      set(x);
   else {
      NTL_ZZ_pRegister(T);
      conv(T, a);
      conv(x, T);
   }
}

void conv(ZZ_pX& x, const ZZ& a)
{
   if (IsZero(a))
      clear(x);
   else {
      NTL_ZZ_pRegister(T);
      conv(T, a);
      conv(x, T);
   }
}

void conv(ZZ_pX& x, const vec_ZZ_p& a)
{
   x.rep = a;
   x.normalize();
}


void add(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const ZZ_p *ap, *bp; 
   ZZ_p* xp;

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


void add(ZZ_pX& x, const ZZ_pX& a, const ZZ_p& b)
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

      ZZ_p *xp = x.rep.elts();
      add(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ_p *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}

void add(ZZ_pX& x, const ZZ_pX& a, long b)
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

void sub(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b)
{
   long da = deg(a);
   long db = deg(b);
   long minab = min(da, db);
   long maxab = max(da, db);
   x.rep.SetLength(maxab+1);

   long i;
   const ZZ_p *ap, *bp; 
   ZZ_p* xp;

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

void sub(ZZ_pX& x, const ZZ_pX& a, const ZZ_p& b)
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

      ZZ_p *xp = x.rep.elts();
      sub(xp[0], a.rep[0], b);
      x.rep.SetLength(n);
      xp = x.rep.elts();
      const ZZ_p *ap = a.rep.elts();
      long i;
      for (i = 1; i < n; i++)
         xp[i] = ap[i];
      x.normalize();
   }
}

void sub(ZZ_pX& x, const ZZ_pX& a, long b)
{
   if (b == 0) {
      x = a;
      return;
   }

   if (a.rep.length() == 0) {
      x.rep.SetLength(1);
      x.rep[0] = b;
      negate(x.rep[0], x.rep[0]);
   }
   else {
      if (&x != &a) x = a;
      sub(x.rep[0], x.rep[0], b);
   }
   x.normalize();
}

void sub(ZZ_pX& x, const ZZ_p& a, const ZZ_pX& b)
{
   NTL_ZZ_pRegister(T);
   T = a;

   negate(x, b);
   add(x, x, T);
}

void sub(ZZ_pX& x, long a, const ZZ_pX& b)
{
   NTL_ZZ_pRegister(T);
   T = a;

   negate(x, b);
   add(x, x, T);
}

void negate(ZZ_pX& x, const ZZ_pX& a)
{
   long n = a.rep.length();
   x.rep.SetLength(n);

   const ZZ_p* ap = a.rep.elts();
   ZZ_p* xp = x.rep.elts();
   long i;

   for (i = n; i; i--, ap++, xp++)
      negate((*xp), (*ap));

}


#if (!defined(NTL_WIZARD_HACK) && defined(NTL_GMP_LIP))
// This only seems to help if we have GMP


void mul(ZZ_pX& c, const ZZ_pX& a, const ZZ_pX& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(c);
      return;
   }

   if (&a == &b) {
      sqr(c, a);
      return;
   }

   long k = ZZ_p::ModulusSize();
   long s = min(deg(a), deg(b)) + 1;

   if (s == 1 || (k == 1 && s < 40) || (k == 2 && s < 20) ||
                 (k == 3 && s < 12) || (k <= 5 && s < 8) ||
                 (k <= 12 && s < 4) )  {
      PlainMul(c, a, b);
   }
   else if (s < KARX) {
      ZZX A, B, C;
      conv(A, a);
      conv(B, b);
      KarMul(C, A, B);
      conv(c, C);
   }

   else {
      long mbits;
      mbits = NumBits(ZZ_p::modulus());

      // long nt = AvailableThreads();
      // SSMul is now fully thread boosted
      
      double rat = SSRatio(deg(a), mbits, deg(b), mbits);

      if ( (k >= 106 && rat < 1.50) || 
           (k >= 212 && rat < 1.75) ) {

         SSMul(c, a, b);
      }
      else {
         FFTMul(c, a, b);
      }
   }
}

void sqr(ZZ_pX& c, const ZZ_pX& a)
{
   if (IsZero(a)) {
      clear(c);
      return;
   }

   long k = ZZ_p::ModulusSize();
   long s = deg(a) + 1;

   if (s == 1 || (k == 1 && s < 50) || (k == 2 && s < 25) ||
                 (k == 3 && s < 25) || (k <= 6 && s < 12) ||
                 (k <= 8 && s < 8)  || (k == 9 && s < 6)  ||
                 (k <= 30 && s < 4) ) {

      PlainSqr(c, a);
   }
   else if (s < 80) {
      ZZX C, A;
      conv(A, a);
      KarSqr(C, A);
      conv(c, C);
   }
   else {
      long mbits;
      mbits = NumBits(ZZ_p::modulus());


      // long nt = AvailableThreads();
      // SSMul is now fully thread boosted

      double rat = SSRatio(deg(a), mbits, deg(a), mbits);

      if ( (k >= 53  && rat < 1.20) || (k >= 106 && rat < 1.30) || 
           (k >= 212 && rat < 1.75) ) {

         SSSqr(c, a);
      }
      else {
         FFTSqr(c, a);
      }
   }
}

#else

void mul(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b)
{
   if (&a == &b) {
      sqr(x, a);
      return;
   }

   if (deg(a) > NTL_ZZ_pX_FFT_CROSSOVER && deg(b) > NTL_ZZ_pX_FFT_CROSSOVER)
      FFTMul(x, a, b);
   else
      PlainMul(x, a, b);
}

void sqr(ZZ_pX& x, const ZZ_pX& a)
{
   if (deg(a) > NTL_ZZ_pX_FFT_CROSSOVER)
      FFTSqr(x, a);
   else
      PlainSqr(x, a);
}


#endif


void PlainMul(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b)
{
   long da = deg(a);
   long db = deg(b);

   if (da < 0 || db < 0) {
      clear(x);
      return;
   }

   if (da == 0) {
      mul(x, b, a.rep[0]);
      return;
   }

   if (db == 0) {
      mul(x, a, b.rep[0]);
      return;
   }

   long d = da+db;



   const ZZ_p *ap, *bp;
   ZZ_p *xp;
   
   ZZ_pX la, lb;

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
   NTL_ZZRegister(t);
   NTL_ZZRegister(accum);

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

void PlainSqr(ZZ_pX& x, const ZZ_pX& a)
{
   long da = deg(a);

   if (da < 0) {
      clear(x);
      return;
   }

   long d = 2*da;

   const ZZ_p *ap;
   ZZ_p *xp;

   ZZ_pX la;

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
         mul(t, rep(ap[j]), rep(ap[i-j]));
         add(accum, accum, t);
      }
      add(accum, accum, accum);
      if (m & 1) {
         sqr(t, rep(ap[jmax + 1]));
         add(accum, accum, t);
      }

      conv(xp[i], accum);
   }

   x.normalize();
}

void PlainDivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_p *bp;
   ZZ_p *qp;
   ZZ *xp;


   ZZ_p LCInv, t;
   NTL_ZZRegister(s);

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pX: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   ZZ_pX lb;

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

   ZZVec x(da + 1, ZZ_p::ExtendedModulusSize());

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


void PlainRem(ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b, ZZVec& x)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_p *bp;
   ZZ *xp;


   ZZ_p LCInv, t;
   NTL_ZZRegister(s);

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pX: division by zero");

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


void PlainDivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b, ZZVec& x)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_p *bp;
   ZZ_p *qp;
   ZZ *xp;


   ZZ_p LCInv, t;
   NTL_ZZRegister(s);

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pX: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   ZZ_pX lb;

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


void PlainDiv(ZZ_pX& q, const ZZ_pX& a, const ZZ_pX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_p *bp;
   ZZ_p *qp;
   ZZ *xp;


   ZZ_p LCInv, t;
   NTL_ZZRegister(s);

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pX: division by zero");

   if (da < db) {
      clear(q);
      return;
   }

   ZZ_pX lb;

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

   ZZVec x(da + 1 - db, ZZ_p::ExtendedModulusSize());

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

void PlainRem(ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b)
{
   long da, db, dq, i, j, LCIsOne;
   const ZZ_p *bp;
   ZZ *xp;


   ZZ_p LCInv, t;
   NTL_ZZRegister(s);

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("ZZ_pX: division by zero");

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

   ZZVec x(da + 1, ZZ_p::ExtendedModulusSize());

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



NTL_TBDECL_static(MulAux)(ZZ_p* xp, const ZZ_p* ap, const ZZ_p& t, long n)
{
   for (long i = 0; i < n; i++) 
      mul(xp[i], ap[i], t);
}

#ifdef NTL_THREAD_BOOST
static void MulAux(ZZ_p* xp, const ZZ_p* ap, const ZZ_p& t, long n)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(n)) {
      basic_MulAux(xp, ap, t, n);
      return;
   }

   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(n,
   [xp, ap, &t, &local_context](long first, long last) {
      local_context.restore();
      for (long i = first; i < last; i++) 
         mul(xp[i], ap[i], t);
   } );
}
#endif



void mul(ZZ_pX& x, const ZZ_pX& a, const ZZ_p& b)
{
   if (IsZero(b)) {
      clear(x);
      return;
   }

   if (IsOne(b)) {
      x = a;
      return;
   }

   NTL_ZZ_pRegister(t);

   long da;

   const ZZ_p *ap;
   ZZ_p* xp;

   t = b;

   da = deg(a);
   x.rep.SetLength(da+1);
   ap = a.rep.elts();
   xp = x.rep.elts();

   MulAux(xp, ap, t, da+1);

   x.normalize();
}

void mul(ZZ_pX& x, const ZZ_pX& a, long b)
{
   NTL_ZZ_pRegister(T);
   conv(T, b);
   mul(x, a, T);
}


void PlainGCD(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b)
{
   ZZ_p t;

   if (IsZero(b))
      x = a;
   else if (IsZero(a))
      x = b;
   else {
      long n = max(deg(a),deg(b)) + 1;
      ZZ_pX u(INIT_SIZE, n), v(INIT_SIZE, n);
      ZZVec tmp(n, ZZ_p::ExtendedModulusSize());

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



         

void PlainXGCD(ZZ_pX& d, ZZ_pX& s, ZZ_pX& t, const ZZ_pX& a, const ZZ_pX& b)
{
   ZZ_p z;


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

      ZZ_pX temp(INIT_SIZE, e), u(INIT_SIZE, e), v(INIT_SIZE, e), 
            u0(INIT_SIZE, e), v0(INIT_SIZE, e), 
            u1(INIT_SIZE, e), v1(INIT_SIZE, e), 
            u2(INIT_SIZE, e), v2(INIT_SIZE, e), q(INIT_SIZE, e);


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


void MulMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, const ZZ_pX& f)
{
   if (deg(a) >= deg(f) || deg(b) >= deg(f) || deg(f) == 0) 
      LogicError("MulMod: bad args");

   ZZ_pX t;

   mul(t, a, b);
   rem(x, t, f);
}

void SqrMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("SqrMod: bad args");

   ZZ_pX t;

   sqr(t, a);
   rem(x, t, f);
}


void InvMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvMod: bad args");

   ZZ_pX d, t;

   XGCD(d, x, t, a, f);
   if (!IsOne(d))
      InvModError("ZZ_pX InvMod: can't compute multiplicative inverse");
}

long InvModStatus(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvModStatus: bad args");
   ZZ_pX d, t;

   XGCD(d, x, t, a, f);
   if (!IsOne(d)) {
      x = d;
      return 1;
   }
   else
      return 0;
}


NTL_TBDECL_static(MulByXModAux1)(long n, ZZ_p *hh, const ZZ_p* aa, const ZZ_p *ff, const ZZ_p& z)
{
   NTL_ZZ_pRegister(t);

   for (long i = n-1; i >= 1; i--) {
      // hh[i] = aa[i-1] + z*ff[i] 
      mul(t, z, ff[i]);
      add(hh[i], aa[i-1], t);
   }
}

#ifdef NTL_THREAD_BOOST

static void MulByXModAux1(long n, ZZ_p *hh, const ZZ_p* aa, const ZZ_p *ff, const ZZ_p& z)
{

   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || hh == aa || BelowThresh(n)) {
      // Careful! can't parallelize if hh == aa
      basic_MulByXModAux1(n, hh, aa, ff, z);
      return;
   }

   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(n-1,
   [n, hh, aa, ff, &z, &local_context]
   (long first, long last) {
      local_context.restore();
      NTL_ZZ_pRegister(t);

      for (long idx = first; idx < last; idx++) {
         long i = n-1-idx;
         // hh[i] = aa[i-1] + z*ff[i] 
         mul(t, z, ff[i]);
         add(hh[i], aa[i-1], t);
      }
   } );
}


#endif


static
void MulByXModAux(ZZ_pX& h, const ZZ_pX& a, const ZZ_pX& f)
{
   long i, n, m;
   ZZ_p* hh;
   const ZZ_p *aa, *ff;

   NTL_ZZ_pRegister(z);

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

      MulByXModAux1(n, hh, aa, ff, z); 
      
      mul(hh[0], z, ff[0]);
      h.normalize();
   }
}


void MulByXMod(ZZ_pX& h, const ZZ_pX& a, const ZZ_pX& f)
{
   if (&h == &f) {
      ZZ_pX hh;
      MulByXModAux(hh, a, f);
      h = hh;
   }
   else
      MulByXModAux(h, a, f);
}



void random(ZZ_pX& x, long n)
{
   long i;

   x.rep.SetLength(n);

   for (i = 0; i < n; i++)
      random(x.rep[i]); 

   x.normalize();
}


void FFTRep::DoSetSize(long NewK, long NewNumPrimes)
{

   if (NewK < -1) LogicError("bad arg to FFTRep::SetSize()");
   
   if (NewK >= NTL_BITS_PER_LONG-1)
      ResourceError("bad arg to FFTRep::SetSize()");

   if (NewK == -1) {
      k = -1;
      return;
   }

   if (NewNumPrimes == 0) {
      const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
      NewNumPrimes = FFTInfo->NumPrimes;
   }

   if (MaxK >= 0 && NumPrimes != NewNumPrimes)
      LogicError("FFTRep: inconsistent use");

   if (NewK <= MaxK) {
      k = NewK;
      return;
   }

   tbl.SetDims(NewNumPrimes, 1L << NewK);
   NumPrimes = NewNumPrimes;
   k = MaxK = NewK;
}

void FFTRep::SetSize(long NewK)
{
   DoSetSize(NewK, 0);
}


FFTRep& FFTRep::operator=(const FFTRep& R)
{
   if (this == &R) return *this;

   if (MaxK >= 0 && R.MaxK >= 0 && NumPrimes != R.NumPrimes)
      LogicError("FFTRep: inconsistent use");

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



void ZZ_pXModRep::SetSize(long NewN)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();


   if (NewN < 0)
      LogicError("bad arg to ZZ_pXModRep::SetSize()");

   if (NewN <= MaxN) {
      n = NewN;
      return;
   }

   tbl.SetDims(FFTInfo->NumPrimes, NewN);
   n = MaxN = NewN;
   NumPrimes = FFTInfo->NumPrimes;
}



// FIXME: maybe I could put this is scratch space associated
// with the current modulus
static inline
vec_long& ModularRepBuf()
{
   NTL_TLS_LOCAL(vec_long, t);
   return t;
}


void ToModularRep(vec_long& x, const ZZ_p& a, const ZZ_pFFTInfoT *FFTInfo,
                  ZZ_pTmpSpaceT *TmpSpace)
{
   FFTInfo->rem_struct.eval(&x[0], rep(a),  TmpSpace->rem_tmp_vec);
}


void FromModularRep(ZZ_p& x, vec_long& avec, const ZZ_pFFTInfoT *FFTInfo,
                    ZZ_pTmpSpaceT *TmpSpace)
// NOTE: a gets destroyed

{
   NTL_ZZRegister(t);
   long * NTL_RESTRICT a = avec.elts();

   if (FFTInfo->crt_struct.special()) {
       FFTInfo->crt_struct.eval(t, a, TmpSpace->crt_tmp_vec);
      x.LoopHole() = t;
      return;
   }

   long nprimes = FFTInfo->NumPrimes;
   const long *u = FFTInfo->u.elts();
   const long *prime = FFTInfo->prime.elts();
   const mulmod_precon_t  *uqinv = FFTInfo->uqinv.elts();
   const double *prime_recip = FFTInfo->prime_recip.elts();
      
   double y = 0.0;

   for (long i = 0; i < nprimes; i++) {
      long r = MulModPrecon(a[i], u[i], prime[i], uqinv[i]);
      a[i] = r;
      y += double(r)*prime_recip[i];
   }

   long q = long(y + 0.5);

   FFTInfo->crt_struct.eval(t, a, TmpSpace->crt_tmp_vec);

   MulAddTo(t, FFTInfo->MinusMModP, q);
   // TODO: this MulAddTo could be folded into the above
   // crt_struct.eval as just another product to accumulate...
   // but, savings would be marginal and a number of interfaces
   // would have to be modified...

   // montgomery
   FFTInfo->reduce_struct.eval(x.LoopHole(), t);
}





NTL_TBDECL(ToFFTRep_trunc)(FFTRep& y, const ZZ_pX& x, long k, long len, long lo, long hi)
// computes an n = 2^k point convolution.
// if deg(x) >= 2^k, then x is first reduced modulo X^n-1.
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
   

   long n, i, j, m, j1;
   vec_long& t = ModularRepBuf();


   if (k > FFTInfo->MaxRoot) 
      ResourceError("Polynomial too big for FFT");

   if (lo < 0)
      LogicError("bad arg to ToFFTRep");

   long nprimes = FFTInfo->NumPrimes;
   t.SetLength(nprimes);

   hi = min(hi, deg(x));

   y.SetSize(k);
   n = 1L << k;

   y.len = len = FFTRoundUp(len, k);

   m = max(hi-lo + 1, 0);
   long ilen = FFTRoundUp(m, k);

   const ZZ_p *xx = x.rep.elts();

   if (n >= m) {
      for (j = 0; j < m; j++) {
         ToModularRep(t, xx[j+lo], FFTInfo, TmpSpace);
         for (i = 0; i < nprimes; i++) {
            y.tbl[i][j] = t[i];
         }
      }

      if (ilen > m) {
         for (i = 0; i < nprimes; i++) {
            long *yp = &y.tbl[i][0];
            for (j = m; j < ilen; j++) {
               yp[j] = 0;
            }
         }
      }
   }
   else {
      NTL_ZZ_pRegister(accum);
      for (j = 0; j < n; j++) {
         accum = xx[j+lo];
         for (j1 = j + n; j1 < m; j1 += n)
            add(accum, accum, xx[j1+lo]);
         ToModularRep(t, accum, FFTInfo, TmpSpace);
         for (i = 0; i < nprimes; i++) {
            y.tbl[i][j] = t[i];
         }
      }
   }

   // FIXME: something to think about...part of the above logic
   // is essentially a matrix transpose, which could lead to bad
   // cache performance.  I don't really know if that is an issue.

   for (i = 0; i < nprimes; i++) {
      long *yp = &y.tbl[i][0];
      FFTFwd_trunc(yp, yp, k, i, len, ilen);
   }
}


#ifdef NTL_THREAD_BOOST

void ToFFTRep_trunc(FFTRep& y, const ZZ_pX& x, long k, long len, long lo, long hi)
// computes an n = 2^k point convolution.
// if deg(x) >= 2^k, then x is first reduced modulo X^n-1.
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(1L << k)) {
      basic_ToFFTRep_trunc(y, x, k, len, lo, hi);
      return;
   }



   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long n, m;


   if (k > FFTInfo->MaxRoot) 
      ResourceError("Polynomial too big for FFT");

   if (lo < 0)
      LogicError("bad arg to ToFFTRep");

   long nprimes = FFTInfo->NumPrimes;

   hi = min(hi, deg(x));

   y.SetSize(k);
   n = 1L << k;

   y.len = len = FFTRoundUp(len, k);

   m = max(hi-lo + 1, 0);
   long ilen = FFTRoundUp(m, k);

   const ZZ_p *xx = x.rep.elts();


   ZZ_pContext local_context;
   local_context.save();

   if (n >= m) {
      pool->exec_range(m, 
      [lo, xx, &y, nprimes, &local_context, FFTInfo]
      (long first, long last) {

        local_context.restore();
        ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
        // TmpSpace is thread local!

        vec_long& t = ModularRepBuf();
        t.SetLength(nprimes);
      
        for (long j = first; j < last; j++) {
           ToModularRep(t, xx[j+lo], FFTInfo, TmpSpace);
           for (long i = 0; i < nprimes; i++) {
              y.tbl[i][j] = t[i];
           }
        }
      } );
   }
   else {
      pool->exec_range(n, 
      [lo, m, n, xx, &y, nprimes, &local_context, FFTInfo]
      (long first, long last) {
         local_context.restore();
         ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
         // TmpSpace is thread local!
 
         vec_long& t = ModularRepBuf();
         t.SetLength(nprimes);
   
         NTL_ZZ_pRegister(accum);
         for (long j = first; j < last; j++) {
            accum = xx[j+lo];
            for (long j1 = j + n; j1 < m; j1 += n)
               add(accum, accum, xx[j1+lo]);
            ToModularRep(t, accum, FFTInfo, TmpSpace);
            for (long i = 0; i < nprimes; i++) {
               y.tbl[i][j] = t[i];
            }
         }
      } );
   }

   // FIXME: something to think about...part of the above logic
   // is essentially a matrix transpose, which could lead to bad
   // cache performance.  I don't really know if that is an issue.

   pool->exec_range(nprimes, 
   [&y, m, n, k, len, ilen](long first, long last) {
     for (long i = first; i < last; i++) {
        long *yp = &y.tbl[i][0];
        for (long j = m; j < ilen; j++) yp[j] = 0;
        FFTFwd_trunc(yp, yp, k, i, len, ilen);
     }
   } );
}

#endif



NTL_TBDECL(RevToFFTRep)(FFTRep& y, const vec_ZZ_p& x, 
                 long k, long lo, long hi, long offset)
// computes an n = 2^k point convolution of X^offset*x[lo..hi] mod X^n-1
// using "inverted" evaluation points.

{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();


   long n, i, j, m, j1;
   vec_long& t = ModularRepBuf();
   NTL_ZZ_pRegister(accum);

   if (k > FFTInfo->MaxRoot) 
      ResourceError("Polynomial too big for FFT");

   if (lo < 0)
      LogicError("bad arg to ToFFTRep");

   long nprimes = FFTInfo->NumPrimes;
   t.SetLength(nprimes);

   hi = min(hi, x.length()-1);

   y.SetSize(k);

   n = 1L << k;
   y.len = n;

   m = max(hi-lo + 1, 0);

   const ZZ_p *xx = x.elts();

   offset = offset & (n-1);

   for (j = 0; j < n; j++) {
      if (j >= m) {
         for (i = 0; i < nprimes; i++)
            y.tbl[i][offset] = 0;
      }
      else {
         accum = xx[j+lo];
         for (j1 = j + n; j1 < m; j1 += n)
            add(accum, accum, xx[j1+lo]);
         ToModularRep(t, accum, FFTInfo, TmpSpace);
         for (i = 0; i < nprimes; i++) {
            y.tbl[i][offset] = t[i];

         }
      }

      offset = (offset + 1) & (n-1);
   }


   for (i = 0; i < nprimes; i++) {
      long *yp = &y.tbl[i][0];
      FFTRev1_trans(yp, yp, k, i);
   }

}



#ifdef NTL_THREAD_BOOST

void RevToFFTRep(FFTRep& y, const vec_ZZ_p& x, 
                 long k, long lo, long hi, long offset)
// computes an n = 2^k point convolution of X^offset*x[lo..hi] mod X^n-1
// using "inverted" evaluation points.

{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(1L << k)) {
      basic_RevToFFTRep(y, x, k, lo, hi, offset);
      return;
   }

   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long n, m;

   if (k > FFTInfo->MaxRoot) 
      ResourceError("Polynomial too big for FFT");

   if (lo < 0)
      LogicError("bad arg to ToFFTRep");

   long nprimes = FFTInfo->NumPrimes;

   hi = min(hi, x.length()-1);

   y.SetSize(k);

   n = 1L << k;
   y.len = n;

   m = max(hi-lo + 1, 0);

   const ZZ_p *xx = x.elts();

   offset = offset & (n-1);

   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(n,
   [lo, m, n, offset, xx, &y, nprimes, &local_context, FFTInfo]
   (long first, long last) {

      local_context.restore();
      ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
      // TmpSpace is thread local!
 
      vec_long& t = ModularRepBuf();
      t.SetLength(nprimes);

      long local_offset = (offset + first) & (n-1);

      NTL_ZZ_pRegister(accum);

      for (long j = first; j < last; j++) {
         if (j >= m) {
            for (long i = 0; i < nprimes; i++)
               y.tbl[i][local_offset] = 0;
         }
         else {
            accum = xx[j+lo];
            for (long j1 = j + n; j1 < m; j1 += n)
               add(accum, accum, xx[j1+lo]);
            ToModularRep(t, accum, FFTInfo, TmpSpace);
            for (long i = 0; i < nprimes; i++) {
               y.tbl[i][local_offset] = t[i];
   
            }
         }
   
         local_offset = (local_offset + 1) & (n-1);
      }
   } );

   pool->exec_range(nprimes, 
   [&y, k](long first, long last) {
     for (long i = first; i < last; i++) {
        long *yp = &y.tbl[i][0];
        FFTRev1_trans(yp, yp, k, i);
     }
   } );

}


#endif






NTL_TBDECL(FromFFTRep)(ZZ_pX& x, FFTRep& y, long lo, long hi)

   // converts from FFT-representation to coefficient representation
   // only the coefficients lo..hi are computed
   

{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();

   long k, n, i, j, l;

   vec_long& t = ModularRepBuf();

   long nprimes = FFTInfo->NumPrimes;
   t.SetLength(nprimes);

   k = y.k;
   n = (1L << k);

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);

   long len = y.len;
   if (len <= hi) LogicError("FromFFTRep: bad len 1"); 


   for (i = 0; i < nprimes; i++) {
      long *yp = &y.tbl[i][0];
      FFTRev1_trunc(yp, yp, k, i, len);
   }

   x.rep.SetLength(l);


   for (j = 0; j < l; j++) {
      for (i = 0; i < nprimes; i++) 
         t[i] = y.tbl[i][j+lo]; 

      FromModularRep(x.rep[j], t, FFTInfo, TmpSpace);
   }

   x.normalize();
}

#ifdef NTL_THREAD_BOOST

void FromFFTRep(ZZ_pX& x, FFTRep& y, long lo, long hi)

   // converts from FFT-representation to coefficient representation
   // only the coefficients lo..hi are computed
   

{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(1L << y.k)) {
      basic_FromFFTRep(x, y, lo, hi);
      return;
   }
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k, n, l;

   long nprimes = FFTInfo->NumPrimes;

   k = y.k;
   n = (1L << k);

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);

   long len = y.len;
   if (len <= hi) LogicError("FromFFTRep: bad len 2");

   pool->exec_range(nprimes,
   [&y, k, len](long first, long last) {
      for (long i = first; i < last; i++) {
         long *yp = &y.tbl[i][0];
         FFTRev1_trunc(yp, yp, k, i, len);
      }
   } );


   x.rep.SetLength(l);
   ZZ_p *xx = x.rep.elts();

   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(l,
   [lo, xx, &y, nprimes, &local_context, FFTInfo]
   (long first, long last) {

      local_context.restore();
      ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
      // TmpSpace is thread local!

      vec_long& t = ModularRepBuf();
      t.SetLength(nprimes);

      for (long j = first; j < last; j++) {
         for (long i = 0; i < nprimes; i++) 
            t[i] = y.tbl[i][j+lo]; 
   
         FromModularRep(xx[j], t, FFTInfo, TmpSpace);
      }
   } );

   x.normalize();
}



#endif






NTL_TBDECL(RevFromFFTRep)(vec_ZZ_p& x, FFTRep& y, long lo, long hi)

   // converts from FFT-representation to coefficient representation
   // using "inverted" evaluation points.
   // only the coefficients lo..hi are computed
   

{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();


   long k, n, i, j, l;

   vec_long& t = ModularRepBuf();

   k = y.k;
   n = (1L << k);

   if (y.len != n) LogicError("RevFromFFTRep: bad len");


   long nprimes = FFTInfo->NumPrimes;
   t.SetLength(nprimes);

   for (i = 0; i < nprimes; i++) {
      long *yp = &y.tbl[i][0];
      FFTFwd_trans(yp, yp, k, i);
   }

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);
   x.SetLength(l);

   for (j = 0; j < l; j++) {
      for (i = 0; i < nprimes; i++) 
         t[i] = y.tbl[i][j+lo]; 

      FromModularRep(x[j], t, FFTInfo, TmpSpace);
   }
}


#ifdef NTL_THREAD_BOOST

void RevFromFFTRep(vec_ZZ_p& x, FFTRep& y, long lo, long hi)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(1L << y.k)) {
      basic_RevFromFFTRep(x, y, lo, hi);
      return;
   }
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k, n, l;

   long nprimes = FFTInfo->NumPrimes;

   k = y.k;
   n = (1L << k);

   if (y.len != n) LogicError("RevFromFFTRep: bad len");


   pool->exec_range(nprimes,
   [&y, k](long first, long last) {
      for (long i = first; i < last; i++) {
         long *yp = &y.tbl[i][0];
         FFTFwd_trans(yp, yp, k, i);
      }
   } );

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);
   x.SetLength(l);
   ZZ_p *xx = x.elts();

   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(l,
   [lo, xx, &y, nprimes, &local_context, FFTInfo]
   (long first, long last) {

      local_context.restore();
      ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
      // TmpSpace is thread local!

      vec_long& t = ModularRepBuf();
      t.SetLength(nprimes);

      for (long j = first; j < last; j++) {
         for (long i = 0; i < nprimes; i++) 
            t[i] = y.tbl[i][j+lo]; 
   
         FromModularRep(xx[j], t, FFTInfo, TmpSpace);
      }
   } );

}




#endif







NTL_TBDECL(NDFromFFTRep)(ZZ_pX& x, const FFTRep& y, long lo, long hi, FFTRep& z)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();


   long k, n, i, j, l;

   vec_long& t = ModularRepBuf();

   long nprimes = FFTInfo->NumPrimes;
   t.SetLength(nprimes);
   k = y.k;
   n = (1L << k);

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);

   long len = y.len;
   if (len <= hi) LogicError("FromFFTRep: bad len 3");

   z.SetSize(k);

   for (i = 0; i < nprimes; i++) {
      long *zp = &z.tbl[i][0];
      const long *yp = &y.tbl[i][0];

      FFTRev1_trunc(zp, yp, k, i, len);
   }

   x.rep.SetLength(l);

   for (j = 0; j < l; j++) {
      for (i = 0; i < nprimes; i++) 
         t[i] = z.tbl[i][j+lo]; 

      FromModularRep(x.rep[j], t, FFTInfo, TmpSpace);
   }

   x.normalize();
}

#ifdef NTL_THREAD_BOOST

void NDFromFFTRep(ZZ_pX& x, const FFTRep& y, long lo, long hi, FFTRep& z)

   // converts from FFT-representation to coefficient representation
   // only the coefficients lo..hi are computed
   

{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(1L << y.k)) {
      basic_NDFromFFTRep(x, y, lo, hi, z);
      return;
   }
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k, n, l;

   long nprimes = FFTInfo->NumPrimes;

   k = y.k;
   n = (1L << k);

   hi = min(hi, n-1);
   l = hi-lo+1;
   l = max(l, 0);

   long len = y.len;
   if (len <= hi) LogicError("FromFFTRep: bad len 4");

   z.SetSize(k);

   pool->exec_range(nprimes,
   [&y, &z, k, len](long first, long last) {
      for (long i = first; i < last; i++) {
         long *zp = &z.tbl[i][0];
         const long *yp = &y.tbl[i][0];
         FFTRev1_trunc(zp, yp, k, i, len);
      }
   } );

   x.rep.SetLength(l);
   ZZ_p *xx = x.rep.elts();

   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(l,
   [lo, xx, &z, nprimes, &local_context, FFTInfo]
   (long first, long last) {

      local_context.restore();
      ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
      // TmpSpace is thread local!

      vec_long& t = ModularRepBuf();
      t.SetLength(nprimes);

      for (long j = first; j < last; j++) {
         for (long i = 0; i < nprimes; i++) 
            t[i] = z.tbl[i][j+lo]; 
   
         FromModularRep(xx[j], t, FFTInfo, TmpSpace);
      }
   } );

   x.normalize();
}



#endif

void NDFromFFTRep(ZZ_pX& x, FFTRep& y, long lo, long hi)
{
   FFTRep z;
   NDFromFFTRep(x, y, lo, hi, z);
}



NTL_TBDECL(FromFFTRep)(ZZ_p* x, FFTRep& y, long lo, long hi)

   // converts from FFT-representation to coefficient representation
   // only the coefficients lo..hi are computed
   

{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();


   long k, n, i, j;

   vec_long& t = ModularRepBuf();

   k = y.k;
   n = (1L << k);

   //if (y.len <= min(hi, n-1)) LogicError("FromFFTRep: bad len");
   if (y.len != n) LogicError("FromFFTRep: bad len 5");

   long nprimes = FFTInfo->NumPrimes;
   t.SetLength(nprimes);

   for (i = 0; i < nprimes; i++) {
      long *yp = &y.tbl[i][0];
      FFTRev1(yp, yp, k, i);
   }

   for (j = lo; j <= hi; j++) {
      if (j >= n)
         clear(x[j-lo]);
      else {
         for (i = 0; i < nprimes; i++) 
            t[i] = y.tbl[i][j]; 

         FromModularRep(x[j-lo], t, FFTInfo, TmpSpace);
      }
   }
}


#ifdef NTL_THREAD_BOOST

void FromFFTRep(ZZ_p* x, FFTRep& y, long lo, long hi)

   // converts from FFT-representation to coefficient representation
   // only the coefficients lo..hi are computed
   

{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(1L << y.k)) {
      basic_FromFFTRep(x, y, lo, hi);
      return;
   }

   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();


   long k, n, l;

   k = y.k;
   n = (1L << k);

   //if (y.len <= min(hi, n-1)) LogicError("FromFFTRep: bad len");
   if (y.len != n) LogicError("FromFFTRep: bad len 6");

   long nprimes = FFTInfo->NumPrimes;


   pool->exec_range(nprimes,
   [&y, k](long first, long last) {
      for (long i = first; i < last; i++) {
         long *yp = &y.tbl[i][0];
         FFTRev1(yp, yp, k, i);
      }
   } );


   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(hi-lo+1,
   [n, lo, x, &y, nprimes, &local_context, FFTInfo]
   (long first, long last) {

      local_context.restore();
      ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
      // TmpSpace is thread local!

      vec_long& t = ModularRepBuf();
      t.SetLength(nprimes);

      for (long idx = first; idx < last; idx++) {
         long j = lo + idx;

         if (j >= n)
            clear(x[j-lo]);
         else {
            for (long i = 0; i < nprimes; i++) 
               t[i] = y.tbl[i][j]; 
   
            FromModularRep(x[j-lo], t, FFTInfo, TmpSpace);
         }
      }
   } );
}

#endif



NTL_TBDECL(mul)(FFTRep& z, const FFTRep& x, const FFTRep& y)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k, i, j;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   long nprimes = FFTInfo->NumPrimes;

   for (i = 0; i < nprimes; i++) {
      long *zp = &z.tbl[i][0];
      const long *xp = &x.tbl[i][0];
      const long *yp = &y.tbl[i][0];
      long q = GetFFTPrime(i);
      mulmod_t qinv = GetFFTPrimeInv(i);

      for (j = 0; j < len; j++)
         zp[j] = NormalizedMulMod(xp[j], yp[j], q, qinv);
   }

}


#ifdef NTL_THREAD_BOOST

void mul(FFTRep& z, const FFTRep& x, const FFTRep& y)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh1(1L << x.k)) {
      basic_mul(z, x, y);
      return;
   }
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   long nprimes = FFTInfo->NumPrimes;

   pool->exec_range(nprimes,
   [&x, &y, &z, len](long first, long last) {
      for (long i  = first; i < last; i++) {
         long *zp = &z.tbl[i][0];
         const long *xp = &x.tbl[i][0];
         const long *yp = &y.tbl[i][0];
         long q = GetFFTPrime(i);
         mulmod_t qinv = GetFFTPrimeInv(i);
   
         for (long j = 0; j < len; j++)
            zp[j] = NormalizedMulMod(xp[j], yp[j], q, qinv);
      }
   } );

}

#endif



NTL_TBDECL(sub)(FFTRep& z, const FFTRep& x, const FFTRep& y)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k, i, j;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   long nprimes = FFTInfo->NumPrimes;

   for (i = 0; i < nprimes; i++) {
      long *zp = &z.tbl[i][0];
      const long *xp = &x.tbl[i][0];
      const long *yp = &y.tbl[i][0];
      long q = GetFFTPrime(i);

      for (j = 0; j < len; j++)
         zp[j] = SubMod(xp[j], yp[j], q);
   }

}


#ifdef NTL_THREAD_BOOST

void sub(FFTRep& z, const FFTRep& x, const FFTRep& y)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh1(1L << x.k)) {
      basic_sub(z, x, y);
      return;
   }
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   long nprimes = FFTInfo->NumPrimes;

   pool->exec_range(nprimes,
   [&x, &y, &z, len](long first, long last) {
      for (long i  = first; i < last; i++) {
         long *zp = &z.tbl[i][0];
         const long *xp = &x.tbl[i][0];
         const long *yp = &y.tbl[i][0];
         long q = GetFFTPrime(i);
   
         for (long j = 0; j < len; j++)
            zp[j] = SubMod(xp[j], yp[j], q);
      }
   } );

}

#endif



NTL_TBDECL(add)(FFTRep& z, const FFTRep& x, const FFTRep& y)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k, i, j;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   long nprimes = FFTInfo->NumPrimes;

   for (i = 0; i < nprimes; i++) {
      long *zp = &z.tbl[i][0];
      const long *xp = &x.tbl[i][0];
      const long *yp = &y.tbl[i][0];
      long q = GetFFTPrime(i);

      for (j = 0; j < len; j++)
         zp[j] = AddMod(xp[j], yp[j], q);
   }

}


#ifdef NTL_THREAD_BOOST

void add(FFTRep& z, const FFTRep& x, const FFTRep& y)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh1(1L << x.k)) {
      basic_add(z, x, y);
      return;
   }
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long k;

   if (x.k != y.k) LogicError("FFT rep mismatch");

   k = x.k;

   z.SetSize(k);

   long len = z.len = min(x.len, y.len);

   long nprimes = FFTInfo->NumPrimes;

   pool->exec_range(nprimes,
   [&x, &y, &z, len](long first, long last) {
      for (long i  = first; i < last; i++) {
         long *zp = &z.tbl[i][0];
         const long *xp = &x.tbl[i][0];
         const long *yp = &y.tbl[i][0];
         long q = GetFFTPrime(i);
   
         for (long j = 0; j < len; j++)
            zp[j] = AddMod(xp[j], yp[j], q);
      }
   } );

}

#endif





NTL_TBDECL(reduce)(FFTRep& x, const FFTRep& a, long k)
  // reduces a 2^l point FFT-rep to a 2^k point FFT-rep
  // input may alias output
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

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

   long nprimes = FFTInfo->NumPrimes;

   for (i = 0; i < nprimes; i++) {
      ap = &a.tbl[i][0];   
      xp = &x.tbl[i][0];
      for (j = 0; j < n; j++) 
         xp[j] = ap[j];
   }
}


#ifdef NTL_THREAD_BOOST

void reduce(FFTRep& x, const FFTRep& a, long k)
  // reduces a 2^l point FFT-rep to a 2^k point FFT-rep
  // input may alias output
{
   BasicThreadPool *pool = GetThreadPool();

   if (&x == &a || !pool || pool->active() || pool->NumThreads() == 1 || BelowThresh1(1L << k)) {
      basic_reduce(x, a, k);
      return;
   }

   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long l, n;

   l = a.k;
   n = 1L << k;

   if (l < k) LogicError("reduce: bad operands");
   if (a.len < n) LogicError("reduce: bad len");

   x.SetSize(k);
   x.len = n;


   long nprimes = FFTInfo->NumPrimes;

   pool->exec_range(nprimes,
   [&x, &a, n, l, k](long first, long last) {
      for (long i = first; i < last; i++) {
         const long *ap = &a.tbl[i][0];   
         long *xp = &x.tbl[i][0];
         for (long j = 0; j < n; j++) 
            xp[j] = ap[j];
      }
   } );
}


#endif





NTL_TBDECL(AddExpand)(FFTRep& x, const FFTRep& a)
//  x = x + (an "expanded" version of a)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long i, j, l, k, n;

   l = x.k;
   k = a.k;
   n = 1L << k;

   if (l < k) LogicError("AddExpand: bad args");

   if (a.len != n) LogicError("AddExpand: bad len");
   if (x.len < n) LogicError("AddExpand: bad len");


   long nprimes = FFTInfo->NumPrimes;

   for (i = 0; i < nprimes; i++) {
      long q = GetFFTPrime(i);
      const long *ap = &a.tbl[i][0];
      long *xp = &x.tbl[i][0];
      for (j = 0; j < n; j++) {
         xp[j] = AddMod(xp[j], ap[j], q);
      }
   }
}

#ifdef NTL_THREAD_BOOST

void AddExpand(FFTRep& x, const FFTRep& a)
//  x = x + (an "expanded" version of a)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh1(1L << a.k)) {
      basic_AddExpand(x, a);
      return;
   }
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long l, k, n;

   l = x.k;
   k = a.k;
   n = 1L << k;

   if (l < k) LogicError("AddExpand: bad args");

   if (a.len != n) LogicError("AddExpand: bad len");
   if (x.len < n) LogicError("AddExpand: bad len");


   long nprimes = FFTInfo->NumPrimes;

   pool->exec_range(nprimes,
   [&x, &a, n, l, k](long first, long last) {
      for (long i = first; i < last; i++) {
         long q = GetFFTPrime(i);
         const long *ap = &a.tbl[i][0];
         long *xp = &x.tbl[i][0];
         for (long j = 0; j < n; j++) {
            xp[j] = AddMod(xp[j], ap[j], q);
         }
      }
   } );
}

#endif








NTL_TBDECL(ToZZ_pXModRep)(ZZ_pXModRep& y, const ZZ_pX& x, long lo, long hi)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();


   long n, i, j;
   vec_long& t = ModularRepBuf();


   long nprimes = FFTInfo->NumPrimes;
   t.SetLength(FFTInfo->NumPrimes);

   if (lo < 0)
      LogicError("bad arg to ToZZ_pXModRep");
   hi = min(hi, deg(x));
   n = max(hi-lo+1, 0);

   y.SetSize(n);

   const ZZ_p *xx = x.rep.elts();

   for (j = 0; j < n; j++) {
      ToModularRep(t, xx[j+lo], FFTInfo, TmpSpace);
      for (i = 0; i < nprimes; i++) 
         y.tbl[i][j] = t[i];
   }
}

#ifdef NTL_THREAD_BOOST
void ToZZ_pXModRep(ZZ_pXModRep& y, const ZZ_pX& x, long lo, long hi)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(max(hi-lo+1,0))) {
      basic_ToZZ_pXModRep(y, x, lo, hi);
      return;
   }

   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();


   long n;

   long nprimes = FFTInfo->NumPrimes;

   if (lo < 0)
      LogicError("bad arg to ToZZ_pXModRep");

   hi = min(hi, deg(x));
   n = max(hi-lo+1, 0);

   y.SetSize(n);

   const ZZ_p *xx = x.rep.elts();

   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(n,
   [lo, xx, &y, nprimes, &local_context, FFTInfo](long first, long last) {

      local_context.restore();
      ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();
      // TmpSpace is thread local!

      vec_long& t = ModularRepBuf();
      t.SetLength(nprimes);

      for (long j = first; j < last; j++) {
         ToModularRep(t, xx[j+lo], FFTInfo, TmpSpace);
         for (long i = 0; i < nprimes; i++) 
            y.tbl[i][j] = t[i];
      }
   } );
}
#endif









NTL_TBDECL(ToFFTRep)(FFTRep& x, const ZZ_pXModRep& a, long k, long lo, long hi)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long n, m, i, j;

   if (k < 0 || lo < 0)
      LogicError("bad args to ToFFTRep");

   if (hi > a.n-1) hi = a.n-1;

   n = 1L << k;
   m = max(hi-lo+1, 0);

   if (m > n)
      LogicError("bad args to ToFFTRep");


   x.SetSize(k);
   x.len = n;

   long nprimes = FFTInfo->NumPrimes;

   if (m == 0) {
      for (i = 0; i < nprimes; i++) {
         long *xp = &x.tbl[i][0];
         for (j = m; j < n; j++)
            xp[j] = 0;
      }
   }
   else {
      for (i = 0; i < nprimes; i++) {
         long *xp = &x.tbl[i][0];
         long *ap = &a.tbl[i][0];
         for (j = 0; j < m; j++)
            xp[j] = ap[lo+j];
         for (j = m; j < n; j++)
            xp[j] = 0;
         
         FFTFwd(xp, xp, k, i);
      }
   }
}

#ifdef NTL_THREAD_BOOST
void ToFFTRep(FFTRep& x, const ZZ_pXModRep& a, long k, long lo, long hi)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1 || BelowThresh(1L << k)) {
      basic_ToFFTRep(x, a, k, lo, hi);
      return;
   }

   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();

   long n, m;

   if (k < 0 || lo < 0)
      LogicError("bad args to ToFFTRep");

   if (hi > a.n-1) hi = a.n-1;

   n = 1L << k;
   m = max(hi-lo+1, 0);

   if (m > n)
      LogicError("bad args to ToFFTRep");


   x.SetSize(k);
   x.len = n;

   long nprimes = FFTInfo->NumPrimes;

   if (m == 0) {
      for (long i = 0; i < nprimes; i++) {
         long *xp = &x.tbl[i][0];
         for (long j = m; j < n; j++)
            xp[j] = 0;
      }
   }
   else {

      pool->exec_range(nprimes,
      [&x, &a, lo, m, n, k](long first, long last) { 

         for (long i = first; i < last; i++) {
            long *xp = &x.tbl[i][0];
            long *ap = &a.tbl[i][0];
            for (long j = 0; j < m; j++)
               xp[j] = ap[lo+j];
            for (long j = m; j < n; j++)
               xp[j] = 0;
            
            FFTFwd(xp, xp, k, i);
         }
      } );

   }
}
#endif



void FromFFTRep(ZZ_pXModRep& x, const FFTRep& a)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   long nprimes = FFTInfo->NumPrimes;
   long k = a.k;
   long n = 1L << k;

   if (a.len != n) LogicError("FromFFTRep: bad len 7");

   x.SetSize(n);
   for (long i = 0; i < nprimes; i++) {
      long *xp = &x.tbl[i][0];
      long *ap = &a.tbl[i][0];
      FFTRev1(xp, ap, k, i);
   }
}

void FromZZ_pXModRep(ZZ_pX& x, const ZZ_pXModRep& a, long lo, long hi)
{
   const ZZ_pFFTInfoT *FFTInfo = ZZ_p::GetFFTInfo();
   ZZ_pTmpSpaceT *TmpSpace = ZZ_p::GetTmpSpace();

   long n = a.n;
   long nprimes = FFTInfo->NumPrimes;

   vec_long& t = ModularRepBuf();
   t.SetLength(nprimes);

   hi = min(hi, n-1);
   long l = hi-lo+1;
   l = max(l, 0);
   x.rep.SetLength(l);

   for (long j = 0; j < l; j++) {
      for (long i = 0; i < nprimes; i++)
         t[i] = a.tbl[i][j+lo];

      FromModularRep(x.rep[j], t, FFTInfo, TmpSpace);
   }

   x.normalize();
}



void FFTMul(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b)
{
   if (IsZero(a) || IsZero(b)) {
      clear(x);
      return;
   }

   long da = deg(a);
   long db = deg(b);
   long d = da+db;
   long k = NextPowerOfTwo(d+1);

   FFTRep R1(INIT_SIZE, k), R2(INIT_SIZE, k);

   ToFFTRep_trunc(R1, a, k, d+1);
   ToFFTRep_trunc(R2, b, k, d+1);
   mul(R1, R1, R2);
   FromFFTRep(x, R1, 0, d);
}

void FFTSqr(ZZ_pX& x, const ZZ_pX& a)
{
   if (IsZero(a)) {
      clear(x);
      return;
   }

   long da = deg(a);
   long d = 2*da;
   long k = NextPowerOfTwo(d+1);

   FFTRep R1(INIT_SIZE, k);

   ToFFTRep_trunc(R1, a, k, d+1);
   mul(R1, R1, R1);
   FromFFTRep(x, R1, 0, d);
}



void CopyReverse(ZZ_pX& x, const ZZ_pX& a, long lo, long hi)

   // x[0..hi-lo] = reverse(a[lo..hi]), with zero fill
   // input may not alias output

{
   long i, j, n, m;

   n = hi-lo+1;
   m = a.rep.length();

   x.rep.SetLength(n);

   const ZZ_p* ap = a.rep.elts();
   ZZ_p* xp = x.rep.elts();

   for (i = 0; i < n; i++) {
      j = hi-i;
      if (j < 0 || j >= m)
         clear(xp[i]);
      else
         xp[i] = ap[j];
   }

   x.normalize();
} 

void copy(ZZ_pX& x, const ZZ_pX& a, long lo, long hi)

   // x[0..hi-lo] = a[lo..hi], with zero fill
   // input may not alias output

{
   long i, j, n, m;

   n = hi-lo+1;
   m = a.rep.length();

   x.rep.SetLength(n);

   const ZZ_p* ap = a.rep.elts();
   ZZ_p* xp = x.rep.elts();

   for (i = 0; i < n; i++) {
      j = lo + i;
      if (j < 0 || j >= m)
         clear(xp[i]);
      else
         xp[i] = ap[j];
   }

   x.normalize();
} 


void rem21(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXModulus& F)
{
   long i, da, ds, n, kk;

   da = deg(a);
   n = F.n;

   if (da > 2*n-2)
      LogicError("bad args to rem(ZZ_pX,ZZ_pX,ZZ_pXModulus)");


   if (da < n) {
      x = a;
      return;
   }

   if (!F.UseFFT || da - n <= NTL_ZZ_pX_FFT_CROSSOVER) {
      PlainRem(x, a, F.f);
      return;
   }

   FFTRep R1(INIT_SIZE, F.l);
   ZZ_pX P1(INIT_SIZE, n);

   ToFFTRep_trunc(R1, a, F.l, 2*n-3, n, 2*(n-1));
   mul(R1, R1, F.HRep);
   FromFFTRep(P1, R1, n-2, 2*n-4);

   ToFFTRep(R1, P1, F.k);
   mul(R1, R1, F.FRep);
   FromFFTRep(P1, R1, 0, n-1);

   ds = deg(P1);

   kk = 1L << F.k;

   x.rep.SetLength(n);
   const ZZ_p* aa = a.rep.elts();
   const ZZ_p* ss = P1.rep.elts();
   ZZ_p* xx = x.rep.elts();

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

void DivRem21(ZZ_pX& q, ZZ_pX& x, const ZZ_pX& a, const ZZ_pXModulus& F)
{
   long i, da, ds, n, kk;

   da = deg(a);
   n = F.n;

   if (da > 2*n-2)
      LogicError("bad args to rem(ZZ_pX,ZZ_pX,ZZ_pXModulus)");


   if (da < n) {
      x = a;
      clear(q);
      return;
   }

   if (!F.UseFFT || da - n <= NTL_ZZ_pX_FFT_CROSSOVER) {
      PlainDivRem(q, x, a, F.f);
      return;
   }

   FFTRep R1(INIT_SIZE, F.l);
   ZZ_pX P1(INIT_SIZE, n), qq;

   ToFFTRep_trunc(R1, a, F.l, 2*n-3, n, 2*(n-1));
   mul(R1, R1, F.HRep);
   FromFFTRep(P1, R1, n-2, 2*n-4);
   qq = P1;

   ToFFTRep(R1, P1, F.k);
   mul(R1, R1, F.FRep);
   FromFFTRep(P1, R1, 0, n-1);

   ds = deg(P1);

   kk = 1L << F.k;

   x.rep.SetLength(n);
   const ZZ_p* aa = a.rep.elts();
   const ZZ_p* ss = P1.rep.elts();
   ZZ_p* xx = x.rep.elts();

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

void div21(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXModulus& F)
{
   long da, n;

   da = deg(a);
   n = F.n;

   if (da > 2*n-2)
      LogicError("bad args to rem(ZZ_pX,ZZ_pX,ZZ_pXModulus)");


   if (da < n) {
      clear(x);
      return;
   }

   if (!F.UseFFT || da - n <= NTL_ZZ_pX_FFT_CROSSOVER) {
      PlainDiv(x, a, F.f);
      return;
   }

   FFTRep R1(INIT_SIZE, F.l);
   ZZ_pX P1(INIT_SIZE, n);

   ToFFTRep_trunc(R1, a, F.l, 2*n-3, n, 2*(n-1));
   mul(R1, R1, F.HRep);
   FromFFTRep(x, R1, n-2, 2*n-4);
}


void rem(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXModulus& F)
{
   long da = deg(a);
   long n = F.n;

   if (n < 0) LogicError("rem: unitialized modulus");

   if (da <= 2*n-2) {
      rem21(x, a, F);
      return;
   }
   else if (!F.UseFFT || da - n <= NTL_ZZ_pX_FFT_CROSSOVER) {
      PlainRem(x, a, F.f);
      return;
   }

   ZZ_pX buf(INIT_SIZE, 2*n-1);

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

void DivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pXModulus& F)
{
   long da = deg(a);
   long n = F.n;

   if (n < 0) LogicError("uninitialized modulus");

   if (da <= 2*n-2) {
      DivRem21(q, r, a, F);
      return;
   }
   else if (!F.UseFFT || da - n <= NTL_ZZ_pX_FFT_CROSSOVER) {
      PlainDivRem(q, r, a, F.f);
      return;
   }

   ZZ_pX buf(INIT_SIZE, 2*n-1);
   ZZ_pX qbuf(INIT_SIZE, n-1);

   ZZ_pX qq;
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

void div(ZZ_pX& q, const ZZ_pX& a, const ZZ_pXModulus& F)
{
   long da = deg(a);
   long n = F.n;

   if (n < 0) LogicError("uninitialized modulus");

   if (da <= 2*n-2) {
      div21(q, a, F);
      return;
   }
   else if (!F.UseFFT || da - n <= NTL_ZZ_pX_FFT_CROSSOVER) {
      PlainDiv(q, a, F.f);
      return;
   }

   ZZ_pX buf(INIT_SIZE, 2*n-1);
   ZZ_pX qbuf(INIT_SIZE, n-1);

   ZZ_pX qq;
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



void MulMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, const ZZ_pXModulus& F)
{
   long  da, db, d, n, k;

   da = deg(a);
   db = deg(b);
   n = F.n;

   if (n < 0) LogicError("MulMod: uninitialized modulus");

   if (da >= n || db >= n)
      LogicError("bad args to MulMod(ZZ_pX,ZZ_pX,ZZ_pX,ZZ_pXModulus)");

   if (da < 0 || db < 0) {
      clear(x);
      return;
   }

   if (!F.UseFFT || da <= NTL_ZZ_pX_FFT_CROSSOVER || db <= NTL_ZZ_pX_FFT_CROSSOVER) {
      ZZ_pX P1;
      mul(P1, a, b);
      rem(x, P1, F);
      return;
   }

   d = da + db + 1;

   k = NextPowerOfTwo(d);
   k = max(k, F.k);

   FFTRep R1(INIT_SIZE, k), R2(INIT_SIZE, F.l);
   ZZ_pX P1(INIT_SIZE, n);

   ToFFTRep_trunc(R1, a, k, max(1L << F.k, d));
   ToFFTRep_trunc(R2, b, k, max(1L << F.k, d));
   mul(R1, R1, R2);
   NDFromFFTRep(P1, R1, n, d-1, R2); // save R1 for future use
   
   ToFFTRep_trunc(R2, P1, F.l, 2*n-3);
   mul(R2, R2, F.HRep);
   FromFFTRep(P1, R2, n-2, 2*n-4);

   ToFFTRep(R2, P1, F.k);
   mul(R2, R2, F.FRep);
   reduce(R1, R1, F.k);
   sub(R1, R1, R2);
   FromFFTRep(x, R1, 0, n-1);
}

void SqrMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXModulus& F)
{
   long  da, d, n, k;

   da = deg(a);
   n = F.n;

   if (n < 0) LogicError("SqrMod: uninitailized modulus");

   if (da >= n) 
      LogicError("bad args to SqrMod(ZZ_pX,ZZ_pX,ZZ_pXModulus)");

   if (!F.UseFFT || da <= NTL_ZZ_pX_FFT_CROSSOVER) {
      ZZ_pX P1;
      sqr(P1, a);
      rem(x, P1, F);
      return;
   }


   d = 2*da + 1;

   k = NextPowerOfTwo(d);
   k = max(k, F.k);

   FFTRep R1(INIT_SIZE, k), R2(INIT_SIZE, F.l);
   ZZ_pX P1(INIT_SIZE, n);

   ToFFTRep_trunc(R1, a, k, max(1L << F.k, d));
   mul(R1, R1, R1);
   NDFromFFTRep(P1, R1, n, d-1, R2);  // save R1 for future use
   
   ToFFTRep_trunc(R2, P1, F.l, 2*n-3);
   mul(R2, R2, F.HRep);
   FromFFTRep(P1, R2, n-2, 2*n-4);

   ToFFTRep(R2, P1, F.k);
   mul(R2, R2, F.FRep);
   reduce(R1, R1, F.k);
   sub(R1, R1, R2);
   FromFFTRep(x, R1, 0, n-1);
}

void PlainInvTrunc(ZZ_pX& x, const ZZ_pX& a, long m)

   /* x = (1/a) % X^m, input not output, constant term a is nonzero */

{
   long i, k, n, lb;
   NTL_ZZRegister(v);
   NTL_ZZRegister(t);
   ZZ_p s;
   const ZZ_p* ap;
   ZZ_p* xp;
   

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
         mul(t, rep(xp[i]), rep(ap[k-i]));
         add(v, v, t);
      }
      conv(xp[k], v);
      negate(xp[k], xp[k]);
      if (!is_one) mul(xp[k], xp[k], s);
   }

   x.normalize();
}


void trunc(ZZ_pX& x, const ZZ_pX& a, long m)

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
      ZZ_p* xp;
      const ZZ_p* ap;

      n = min(a.rep.length(), m);
      x.rep.SetLength(n);

      xp = x.rep.elts();
      ap = a.rep.elts();

      for (i = 0; i < n; i++) xp[i] = ap[i];

      x.normalize();
   }
}

void CyclicReduce(ZZ_pX& x, const ZZ_pX& a, long m)

// computes x = a mod X^m-1

{
   long n = deg(a);
   long i, j;
   ZZ_p accum;

   if (n < m) {
      x = a;
      return;
   }

   if (&x != &a)
      x.rep.SetLength(m);

   for (i = 0; i < m; i++) {
      accum = a.rep[i];
      for (j = i + m; j <= n; j += m)
         add(accum, accum, a.rep[j]);
      x.rep[i] = accum;
   }

   if (&x == &a)
      x.rep.SetLength(m);

   x.normalize();
}



void InvTrunc(ZZ_pX& x, const ZZ_pX& a, long m)
{
   if (m < 0) LogicError("InvTrunc: bad args");

   if (m == 0) {
      clear(x);
      return;
   }

   if (NTL_OVERFLOW(m, 1, 0))
      ResourceError("overflow in InvTrunc");

   if (&x == &a) {
      ZZ_pX la;
      la = a;
      if (m > NTL_ZZ_pX_NEWTON_CROSSOVER && deg(a) > 0)
         NewtonInvTrunc(x, la, m);
      else
         PlainInvTrunc(x, la, m);
   }
   else {
      if (m > NTL_ZZ_pX_NEWTON_CROSSOVER && deg(a) > 0)
         NewtonInvTrunc(x, a, m);
      else
         PlainInvTrunc(x, a, m);
   }
}
   


void build(ZZ_pXModulus& x, const ZZ_pX& f)
{
   x.f = f;
   x.n = deg(f);

   x.tracevec.make();

   if (x.n <= 0)
      LogicError("build: deg(f) must be at least 1");

   if (x.n <= NTL_ZZ_pX_FFT_CROSSOVER + 1) {
      x.UseFFT = 0;
      return;
   }

   x.UseFFT = 1;

   x.k = NextPowerOfTwo(x.n);
   x.l = NextPowerOfTwo(2*x.n - 3);
   ToFFTRep(x.FRep, f, x.k);

   ZZ_pX P1(INIT_SIZE, x.n+1), P2(INIT_SIZE, x.n);

   CopyReverse(P1, f, 0, x.n);
   InvTrunc(P2, P1, x.n-1);

   CopyReverse(P1, P2, 0, x.n-2);
   ToFFTRep(x.HRep, P1, x.l);
}

ZZ_pXModulus::ZZ_pXModulus(const ZZ_pX& ff)
{
   build(*this, ff);
}

ZZ_pXMultiplier::ZZ_pXMultiplier(const ZZ_pX& b, const ZZ_pXModulus& F)
{
   build(*this, b, F); 
}

void build(ZZ_pXMultiplier& x, const ZZ_pX& b, 
                         const ZZ_pXModulus& F)
{
   long db;
   long n = F.n;

   if (n < 0) LogicError("build ZZ_pXMultiplier: uninitialized modulus");

   x.b = b;
   db = deg(b);

   if (db >= n) LogicError("build ZZ_pXMultiplier: deg(b) >= deg(f)");

   if (!F.UseFFT || db <= NTL_ZZ_pX_FFT_CROSSOVER) {
      x.UseFFT = 0;
      return;
   }

   x.UseFFT = 1;

   FFTRep R1(INIT_SIZE, F.l);
   ZZ_pX P1(INIT_SIZE, n);
   

   ToFFTRep_trunc(R1, b, F.l, 2*n-2);
   reduce(x.B2, R1, F.k);
   mul(R1, R1, F.HRep);
   FromFFTRep(P1, R1, n-1, 2*n-3); 

   ToFFTRep(x.B1, P1, F.l);
   // could be truncated to length max(1L << F.k, 2*n-2), except
   // for the usage in UpdateMap, where we would have to investigate
   // further
}


void MulMod(ZZ_pX& x, const ZZ_pX& a, const ZZ_pXMultiplier& B,
                                      const ZZ_pXModulus& F)
{

   long n = F.n;
   long da;

   da = deg(a);

   if (da >= n)
      LogicError(" bad args to MulMod(ZZ_pX,ZZ_pX,ZZ_pXMultiplier,ZZ_pXModulus)");

   if (da < 0) {
      clear(x);
      return;
   }

   if (!B.UseFFT || !F.UseFFT || da <= NTL_ZZ_pX_FFT_CROSSOVER) {
      ZZ_pX P1;
      mul(P1, a, B.b);
      rem(x, P1, F);
      return;
   }

   ZZ_pX P1(INIT_SIZE, n), P2(INIT_SIZE, n);
   FFTRep R1(INIT_SIZE, F.l), R2(INIT_SIZE, F.l);

   ToFFTRep_trunc(R1, a, F.l, max(1L << F.k, 2*n-2));
   mul(R2, R1, B.B1);
   FromFFTRep(P1, R2, n-1, 2*n-3);

   reduce(R1, R1, F.k);
   mul(R1, R1, B.B2);
   ToFFTRep(R2, P1, F.k);
   mul(R2, R2, F.FRep);
   sub(R1, R1, R2);

   FromFFTRep(x, R1, 0, n-1);
}
   

void PowerXMod(ZZ_pX& hh, const ZZ& e, const ZZ_pXModulus& F)
{
   if (F.n < 0) LogicError("PowerXMod: uninitialized modulus");

   if (IsZero(e)) {
      set(hh);
      return;
   }

   long n = NumBits(e);
   long i;

   ZZ_pX h, h1;

   h.SetMaxLength(F.n);
   set(h);

   for (i = n - 1; i >= 0; i--) {
      if (bit(e, i)) {
         SqrMod(h1, h, F);
         MulByXMod(h, h1, F);
         // NOTE: MulByXMod gives much faster multicore performance
         // when output does not alias input
      }
      else
         SqrMod(h, h, F);
   }

   if (e < 0) InvMod(h, h, F);

   hh = h;
}


void PowerXPlusAMod(ZZ_pX& hh, const ZZ_p& a, const ZZ& e, const ZZ_pXModulus& F)
{
   if (F.n < 0) LogicError("PowerXPlusAMod: uninitialized modulus");

   if (IsZero(e)) {
      set(hh);
      return;
   }

   ZZ_pX t1(INIT_SIZE, F.n), t2(INIT_SIZE, F.n);
   long n = NumBits(e);
   long i;

   ZZ_pX h;

   h.SetMaxLength(F.n);
   set(h);

   for (i = n - 1; i >= 0; i--) {
      SqrMod(h, h, F);
      if (bit(e, i)) {
         MulByXMod(t1, h, F);
         mul(t2, h, a);
         add(h, t1, t2);
      }
   }

   if (e < 0) InvMod(h, h, F);

   hh = h;
}


void PowerMod(ZZ_pX& h, const ZZ_pX& g, const ZZ& e, const ZZ_pXModulus& F)
{
   if (deg(g) >= F.n)
      LogicError("PowerMod: bad args");

   if (IsZero(e)) {
      set(h);
      return;
   }

   ZZ_pXMultiplier G;

   ZZ_pX res;

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


void NewtonInvTrunc(ZZ_pX& x, const ZZ_pX& a, long m)
{
   x.SetMaxLength(m);
   long i, t, k;

   long log2_newton = NextPowerOfTwo(NTL_ZZ_pX_NEWTON_CROSSOVER)-1;
   PlainInvTrunc(x, a, 1L << log2_newton);

   t = NextPowerOfTwo(m);

   FFTRep R1(INIT_SIZE, t), R2(INIT_SIZE, t);
   ZZ_pX P1(INIT_SIZE, m/2);

   long a_len = min(m, a.rep.length());

   ZZ_pXModRep a_rep;
   ToZZ_pXModRep(a_rep, a, 0, a_len-1);

   k = 1L << log2_newton; 
   t = log2_newton;

   while (k < m) {
      long l = min(2*k, m);

      ToFFTRep(R1, x, t+1);
      ToFFTRep(R2, a_rep, t+1, 0, l-1); 
      mul(R2, R2, R1);
      FromFFTRep(P1, R2, k, l-1);
      
      ToFFTRep(R2, P1, t+1);
      mul(R2, R2, R1);
      FromFFTRep(P1, R2, 0, l-k-1);

      x.rep.SetLength(l);
      long y_len = P1.rep.length();
      for (i = k; i < l; i++) {
         if (i-k >= y_len)
            clear(x.rep[i]);
         else
            negate(x.rep[i], P1.rep[i-k]);
      }
      x.normalize();

      t++;
      k = l;
   }
}



void FFTDivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b)
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
      ZZ_pXModulus B;
      build(B, b);
      DivRem(q, r, a, B);
      return;
   }

   ZZ_pX P1, P2, P3;

   CopyReverse(P3, b, 0, n);
   InvTrunc(P2, P3, m-n+1);
   CopyReverse(P1, P2, 0, m-n);

   k = NextPowerOfTwo(2*(m-n)+1);
   long k1 = NextPowerOfTwo(n);
   long mx = max(k1, k);

   FFTRep R1(INIT_SIZE, mx), R2(INIT_SIZE, mx);

   ToFFTRep(R1, P1, k);
   ToFFTRep(R2, a, k, n, m);
   mul(R1, R1, R2);
   FromFFTRep(P3, R1, m-n, 2*(m-n));
   
   l = 1L << k1;

   
   ToFFTRep(R1, b, k1);
   ToFFTRep(R2, P3, k1);
   mul(R1, R1, R2);
   FromFFTRep(P1, R1, 0, n-1);
   CyclicReduce(P2, a, l);
   trunc(r, P2, n);
   sub(r, r, P1);
   q = P3;
}




void FFTDiv(ZZ_pX& q, const ZZ_pX& a, const ZZ_pX& b)
{

   long n = deg(b);
   long m = deg(a);
   long k;

   if (m < n) {
      clear(q);
      return;
   }

   if (m >= 3*n) {
      ZZ_pXModulus B;
      build(B, b);
      div(q, a, B);
      return;
   }

   ZZ_pX P1, P2, P3;

   CopyReverse(P3, b, 0, n);
   InvTrunc(P2, P3, m-n+1);
   CopyReverse(P1, P2, 0, m-n);

   k = NextPowerOfTwo(2*(m-n)+1);

   FFTRep R1(INIT_SIZE, k), R2(INIT_SIZE, k);

   ToFFTRep(R1, P1, k);
   ToFFTRep(R2, a, k, n, m);
   mul(R1, R1, R2);
   FromFFTRep(q, R1, m-n, 2*(m-n));
}



void FFTRem(ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b)
{
   long n = deg(b);
   long m = deg(a);
   long k, l;

   if (m < n) {
      r = a;
      return;
   }

   if (m >= 3*n) {
      ZZ_pXModulus B;
      build(B, b);
      rem(r, a, B);
      return;
   }

   ZZ_pX P1, P2, P3;

   CopyReverse(P3, b, 0, n);
   InvTrunc(P2, P3, m-n+1);
   CopyReverse(P1, P2, 0, m-n);

   k = NextPowerOfTwo(2*(m-n)+1);
   long k1 = NextPowerOfTwo(n);
   long mx = max(k, k1);

   FFTRep R1(INIT_SIZE, mx), R2(INIT_SIZE, mx);

   ToFFTRep(R1, P1, k);
   ToFFTRep(R2, a, k, n, m);
   mul(R1, R1, R2);
   FromFFTRep(P3, R1, m-n, 2*(m-n));
   
   l = 1L << k1;
   
   ToFFTRep(R1, b, k1);
   ToFFTRep(R2, P3, k1);
   mul(R1, R1, R2);
   FromFFTRep(P3, R1, 0, n-1);
   CyclicReduce(P2, a, l);
   trunc(r, P2, n);
   sub(r, r, P3);
}


void DivRem(ZZ_pX& q, ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b)
{
   if (deg(b) > NTL_ZZ_pX_DIV_CROSSOVER && deg(a) - deg(b) > NTL_ZZ_pX_DIV_CROSSOVER)
      FFTDivRem(q, r, a, b);
   else
      PlainDivRem(q, r, a, b);
}

void div(ZZ_pX& q, const ZZ_pX& a, const ZZ_pX& b)
{
   if (deg(b) > NTL_ZZ_pX_DIV_CROSSOVER && deg(a) - deg(b) > NTL_ZZ_pX_DIV_CROSSOVER)
      FFTDiv(q, a, b);
   else
      PlainDiv(q, a, b);
}

void div(ZZ_pX& q, const ZZ_pX& a, const ZZ_p& b)
{
   NTL_ZZ_pRegister(T);

   inv(T, b);
   mul(q, a, T);
}

void div(ZZ_pX& q, const ZZ_pX& a, long b)
{
   NTL_ZZ_pRegister(T);

   T = b;
   inv(T, T);
   mul(q, a, T);
}



void rem(ZZ_pX& r, const ZZ_pX& a, const ZZ_pX& b)
{
   if (deg(b) > NTL_ZZ_pX_DIV_CROSSOVER && deg(a) - deg(b) > NTL_ZZ_pX_DIV_CROSSOVER)
      FFTRem(r, a, b);
   else
      PlainRem(r, a, b);
}


long operator==(const ZZ_pX& a, long b)
{
   if (b == 0)
      return IsZero(a);

   if (b == 1)
      return IsOne(a);

   long da = deg(a);

   if (da > 0)
      return 0;

   NTL_ZZ_pRegister(bb);
   bb = b;

   if (da < 0)
      return IsZero(bb);

   return a.rep[0] == bb;
}

long operator==(const ZZ_pX& a, const ZZ_p& b)
{
   if (IsZero(b))
      return IsZero(a);

   long da = deg(a);

   if (da != 0)
      return 0;

   return a.rep[0] == b;
}

void power(ZZ_pX& x, const ZZ_pX& a, long e)
{
   if (e < 0) {
      LogicError("power: negative exponent");
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

   ZZ_pX res;
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

void reverse(ZZ_pX& x, const ZZ_pX& a, long hi)
{
   if (hi < 0) { clear(x); return; }
   if (NTL_OVERFLOW(hi, 1, 0))
      ResourceError("overflow in reverse");

   if (&x == &a) {
      ZZ_pX tmp;
      CopyReverse(tmp, a, 0, hi);
      x = tmp;
   }
   else
      CopyReverse(x, a, 0, hi);
}

NTL_END_IMPL

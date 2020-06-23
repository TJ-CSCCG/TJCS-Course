
#include <NTL/ZZ_pEXFactoring.h>
#include <NTL/FacVec.h>
#include <NTL/fileio.h>


NTL_START_IMPL



static
void IterPower(ZZ_pE& c, const ZZ_pE& a, long n)
{
   ZZ_pE res;

   long i;

   res = a;

   for (i = 0; i < n; i++)
      power(res, res, ZZ_p::modulus());

   c = res;
}
   


void SquareFreeDecomp(vec_pair_ZZ_pEX_long& u, const ZZ_pEX& ff)
{
   ZZ_pEX f = ff;

   if (!IsOne(LeadCoeff(f)))
      LogicError("SquareFreeDecomp: bad args");

   ZZ_pEX r, t, v, tmp1;
   long m, j, finished, done;

   u.SetLength(0);

   if (deg(f) == 0)
      return;

   m = 1;
   finished = 0;

   do {
      j = 1;
      diff(tmp1, f);
      GCD(r, f, tmp1);
      div(t, f, r);

      if (deg(t) > 0) {
         done = 0;
         do {
            GCD(v, r, t);
            div(tmp1, t, v);
            if (deg(tmp1) > 0) append(u, cons(tmp1, j*m));
            if (deg(v) > 0) {
               div(r, r, v);
               t = v;
               j++;
            }
            else
               done = 1;
         } while (!done);
         if (deg(r) == 0) finished = 1;
      }

      if (!finished) {
         /* r is a p-th power */

         long k, d;
         long p = to_long(ZZ_p::modulus()); 

         d = deg(r)/p;
         f.rep.SetLength(d+1);
         for (k = 0; k <= d; k++) 
            IterPower(f.rep[k], r.rep[k*p], ZZ_pE::degree()-1);
         m = m*p;
      }
   } while (!finished);
}
         


static
void AbsTraceMap(ZZ_pEX& h, const ZZ_pEX& a, const ZZ_pEXModulus& F)
{
   ZZ_pEX res, tmp;

   long k = NumBits(ZZ_pE::cardinality())-1;

   res = a;
   tmp = a;

   long i;
   for (i = 0; i < k-1; i++) {
      SqrMod(tmp, tmp, F);
      add(res, res, tmp);
   }

   h = res;
}

void FrobeniusMap(ZZ_pEX& h, const ZZ_pEXModulus& F)
{
   PowerXMod(h, ZZ_pE::cardinality(), F);
}


static
void RecFindRoots(vec_ZZ_pE& x, const ZZ_pEX& f)
{
   if (deg(f) == 0) return;

   if (deg(f) == 1) {
      long k = x.length();
      x.SetLength(k+1);
      negate(x[k], ConstTerm(f));
      return;
   }
      
   ZZ_pEX h;

   ZZ_pEX r;

   
   {
      ZZ_pEXModulus F;
      build(F, f);

      do {
         random(r, deg(F));
         if (IsOdd(ZZ_pE::cardinality())) {
            PowerMod(h, r, RightShift(ZZ_pE::cardinality(), 1), F);
            sub(h, h, 1);
         }
         else {
            AbsTraceMap(h, r, F);
         }
         GCD(h, h, f);
      } while (deg(h) <= 0 || deg(h) == deg(f));
   }

   RecFindRoots(x, h);
   div(h, f, h); 
   RecFindRoots(x, h);
}

void FindRoots(vec_ZZ_pE& x, const ZZ_pEX& ff)
{
   ZZ_pEX f = ff;

   if (!IsOne(LeadCoeff(f)))
      LogicError("FindRoots: bad args");

   x.SetMaxLength(deg(f));
   x.SetLength(0);
   RecFindRoots(x, f);
}

void split(ZZ_pEX& f1, ZZ_pEX& g1, ZZ_pEX& f2, ZZ_pEX& g2,
           const ZZ_pEX& f, const ZZ_pEX& g, 
           const vec_ZZ_pE& roots, long lo, long mid)
{
   long r = mid-lo+1;

   ZZ_pEXModulus F;
   build(F, f);

   vec_ZZ_pE lroots(INIT_SIZE, r);
   long i;

   for (i = 0; i < r; i++)
      lroots[i] = roots[lo+i];


   ZZ_pEX h, a, d;
   BuildFromRoots(h, lroots);
   CompMod(a, h, g, F);


   GCD(f1, a, f);
   
   div(f2, f, f1);

   rem(g1, g, f1);
   rem(g2, g, f2);
}

void RecFindFactors(vec_ZZ_pEX& factors, const ZZ_pEX& f, const ZZ_pEX& g,
                    const vec_ZZ_pE& roots, long lo, long hi)
{
   long r = hi-lo+1;

   if (r == 0) return;

   if (r == 1) {
      append(factors, f);
      return;
   }

   ZZ_pEX f1, g1, f2, g2;

   long mid = (lo+hi)/2;

   split(f1, g1, f2, g2, f, g, roots, lo, mid);

   RecFindFactors(factors, f1, g1, roots, lo, mid);
   RecFindFactors(factors, f2, g2, roots, mid+1, hi);
}


void FindFactors(vec_ZZ_pEX& factors, const ZZ_pEX& f, const ZZ_pEX& g,
                 const vec_ZZ_pE& roots)
{
   long r = roots.length();

   factors.SetMaxLength(r);
   factors.SetLength(0);

   RecFindFactors(factors, f, g, roots, 0, r-1);
}

void IterFindFactors(vec_ZZ_pEX& factors, const ZZ_pEX& f,
                     const ZZ_pEX& g, const vec_ZZ_pE& roots)
{
   long r = roots.length();
   long i;
   ZZ_pEX h;

   factors.SetLength(r);

   for (i = 0; i < r; i++) {
      sub(h, g, roots[i]);
      GCD(factors[i], f, h);
   }
}


void TraceMap(ZZ_pEX& w, const ZZ_pEX& a, long d, const ZZ_pEXModulus& F, 
              const ZZ_pEX& b)

{
   if (d < 0) LogicError("TraceMap: bad args");

   ZZ_pEX y, z, t;

   z = b;
   y = a;
   clear(w);

   while (d) {
      if (d == 1) {
         if (IsZero(w)) 
            w = y;
         else {
            CompMod(w, w, z, F);
            add(w, w, y);
         }
      }
      else if ((d & 1) == 0) {
         Comp2Mod(z, t, z, y, z, F);
         add(y, t, y);
      }
      else if (IsZero(w)) {
         w = y;
         Comp2Mod(z, t, z, y, z, F);
         add(y, t, y);
      }
      else {
         Comp3Mod(z, t, w, z, y, w, z, F);
         add(w, w, y);
         add(y, t, y);
      }

      d = d >> 1;
   }
}


void PowerCompose(ZZ_pEX& y, const ZZ_pEX& h, long q, const ZZ_pEXModulus& F)
{
   if (q < 0) LogicError("PowerCompose: bad args");

   ZZ_pEX z(INIT_SIZE, F.n);
   long sw;

   z = h;
   SetX(y);

   while (q) {
      sw = 0;

      if (q > 1) sw = 2;
      if (q & 1) {
         if (IsX(y))
            y = z;
         else
            sw = sw | 1;
      }

      switch (sw) {
      case 0:
         break;

      case 1:
         CompMod(y, y, z, F);
         break;

      case 2:
         CompMod(z, z, z, F);
         break;

      case 3:
         Comp2Mod(y, z, y, z, z, F);
         break;
      }

      q = q >> 1;
   }
}


long ProbIrredTest(const ZZ_pEX& f, long iter)
{
   long n = deg(f);

   if (n <= 0) return 0;
   if (n == 1) return 1;

   ZZ_pEXModulus F;

   build(F, f);

   ZZ_pEX b, r, s;

   FrobeniusMap(b, F);

   long all_zero = 1;

   long i;

   for (i = 0; i < iter; i++) {
      random(r, n);
      TraceMap(s, r, n, F, b);

      all_zero = all_zero && IsZero(s);

      if (deg(s) > 0) return 0;
   }

   if (!all_zero || (n & 1)) return 1;

   PowerCompose(s, b, n/2, F);
   return !IsX(s);
}


NTL_CHEAP_THREAD_LOCAL long ZZ_pEX_BlockingFactor = 10;




void RootEDF(vec_ZZ_pEX& factors, const ZZ_pEX& f, long verbose)
{
   vec_ZZ_pE roots;
   double t;

   if (verbose) { cerr << "finding roots..."; t = GetTime(); }
   FindRoots(roots, f);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   long r = roots.length();
   factors.SetLength(r);
   for (long j = 0; j < r; j++) {
      SetX(factors[j]);
      sub(factors[j], factors[j], roots[j]);
   }
}

void EDFSplit(vec_ZZ_pEX& v, const ZZ_pEX& f, const ZZ_pEX& b, long d)
{
   ZZ_pEX a, g, h;
   ZZ_pEXModulus F;
   vec_ZZ_pE roots;
   
   build(F, f);
   long n = F.n;
   long r = n/d;
   random(a, n);
   TraceMap(g, a, d, F, b);
   MinPolyMod(h, g, F, r);
   FindRoots(roots, h);
   FindFactors(v, f, g, roots);
}

void RecEDF(vec_ZZ_pEX& factors, const ZZ_pEX& f, const ZZ_pEX& b, long d,
            long verbose)
{
   vec_ZZ_pEX v;
   long i;
   ZZ_pEX bb;

   if (verbose) cerr << "+";

   EDFSplit(v, f, b, d);
   for (i = 0; i < v.length(); i++) {
      if (deg(v[i]) == d) {
         append(factors, v[i]);
      }
      else {
         ZZ_pEX bb;
         rem(bb, b, v[i]);
         RecEDF(factors, v[i], bb, d, verbose);
      }
   }
}
         

void EDF(vec_ZZ_pEX& factors, const ZZ_pEX& ff, const ZZ_pEX& bb,
         long d, long verbose)

{
   ZZ_pEX f = ff;
   ZZ_pEX b = bb;

   if (!IsOne(LeadCoeff(f)))
      LogicError("EDF: bad args");

   long n = deg(f);
   long r = n/d;

   if (r == 0) {
      factors.SetLength(0);
      return;
   }

   if (r == 1) {
      factors.SetLength(1);
      factors[0] = f;
      return;
   }

   if (d == 1) {
      RootEDF(factors, f, verbose);
      return;
   }

   
   double t;
   if (verbose) { 
      cerr << "computing EDF(" << d << "," << r << ")..."; 
      t = GetTime(); 
   }

   factors.SetLength(0);

   RecEDF(factors, f, b, d, verbose);

   if (verbose) cerr << (GetTime()-t) << "\n";
}


void SFCanZass(vec_ZZ_pEX& factors, const ZZ_pEX& ff, long verbose)
{
   ZZ_pEX f = ff;

   if (!IsOne(LeadCoeff(f)))
      LogicError("SFCanZass: bad args");

   if (deg(f) == 0) {
      factors.SetLength(0);
      return;
   }

   if (deg(f) == 1) {
      factors.SetLength(1);
      factors[0] = f;
      return;
   }

   factors.SetLength(0);

   double t;

   
   ZZ_pEXModulus F;
   build(F, f);

   ZZ_pEX h;

   if (verbose) { cerr << "computing X^p..."; t = GetTime(); }
   FrobeniusMap(h, F);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   vec_pair_ZZ_pEX_long u;
   if (verbose) { cerr << "computing DDF..."; t = GetTime(); }
   NewDDF(u, f, h, verbose);
   if (verbose) { 
      t = GetTime()-t; 
      cerr << "DDF time: " << t << "\n";
   }

   ZZ_pEX hh;
   vec_ZZ_pEX v;

   long i;
   for (i = 0; i < u.length(); i++) {
      const ZZ_pEX& g = u[i].a;
      long d = u[i].b;
      long r = deg(g)/d;

      if (r == 1) {
         // g is already irreducible

         append(factors, g);
      }
      else {
         // must perform EDF

         if (d == 1) {
            // root finding
            RootEDF(v, g, verbose);
            append(factors, v);
         }
         else {
            // general case
            rem(hh, h, g);
            EDF(v, g, hh, d, verbose);
            append(factors, v);
         }
      }
   }
}
   
void CanZass(vec_pair_ZZ_pEX_long& factors, const ZZ_pEX& f, long verbose)
{
   if (!IsOne(LeadCoeff(f)))
      LogicError("CanZass: bad args");

   double t;
   vec_pair_ZZ_pEX_long sfd;
   vec_ZZ_pEX x;

   
   if (verbose) { cerr << "square-free decomposition..."; t = GetTime(); }
   SquareFreeDecomp(sfd, f);
   if (verbose) cerr << (GetTime()-t) << "\n";

   factors.SetLength(0);

   long i, j;

   for (i = 0; i < sfd.length(); i++) {
      if (verbose) {
         cerr << "factoring multiplicity " << sfd[i].b 
              << ", deg = " << deg(sfd[i].a) << "\n";
      }

      SFCanZass(x, sfd[i].a, verbose);

      for (j = 0; j < x.length(); j++)
         append(factors, cons(x[j], sfd[i].b));
   }
}

void mul(ZZ_pEX& f, const vec_pair_ZZ_pEX_long& v)
{
   long i, j, n;

   n = 0;
   for (i = 0; i < v.length(); i++)
      n += v[i].b*deg(v[i].a);

   ZZ_pEX g(INIT_SIZE, n+1);

   set(g);
   for (i = 0; i < v.length(); i++)
      for (j = 0; j < v[i].b; j++) {
         mul(g, g, v[i].a);
      }

   f = g;
}


long BaseCase(const ZZ_pEX& h, long q, long a, const ZZ_pEXModulus& F)
{
   long b, e;
   ZZ_pEX lh(INIT_SIZE, F.n);

   lh = h;
   b = 1;
   e = 0;
   while (e < a-1 && !IsX(lh)) {
      e++;
      b *= q;
      PowerCompose(lh, lh, q, F);
   }

   if (!IsX(lh)) b *= q;

   return b;
}



void TandemPowerCompose(ZZ_pEX& y1, ZZ_pEX& y2, const ZZ_pEX& h, 
                        long q1, long q2, const ZZ_pEXModulus& F)
{
   ZZ_pEX z(INIT_SIZE, F.n);
   long sw;

   z = h;
   SetX(y1);
   SetX(y2);

   while (q1 || q2) {
      sw = 0;

      if (q1 > 1 || q2 > 1) sw = 4;

      if (q1 & 1) {
         if (IsX(y1))
            y1 = z;
         else
            sw = sw | 2;
      }

      if (q2 & 1) {
         if (IsX(y2))
            y2 = z;
         else
            sw = sw | 1;
      }

      switch (sw) {
      case 0:
         break;

      case 1:
         CompMod(y2, y2, z, F);
         break;

      case 2:
         CompMod(y1, y1, z, F);
         break;

      case 3:
         Comp2Mod(y1, y2, y1, y2, z, F);
         break;

      case 4:
         CompMod(z, z, z, F);
         break;

      case 5:
         Comp2Mod(z, y2, z, y2, z, F);
         break;

      case 6:
         Comp2Mod(z, y1, z, y1, z, F);
         break;

      case 7:
         Comp3Mod(z, y1, y2, z, y1, y2, z, F);
         break;
      }

      q1 = q1 >> 1;
      q2 = q2 >> 1;
   }
}


long RecComputeDegree(long u, const ZZ_pEX& h, const ZZ_pEXModulus& F,
                      FacVec& fvec)
{
   if (IsX(h)) return 1;

   if (fvec[u].link == -1) return BaseCase(h, fvec[u].q, fvec[u].a, F);

   ZZ_pEX h1, h2;
   long q1, q2, r1, r2;

   q1 = fvec[fvec[u].link].val; 
   q2 = fvec[fvec[u].link+1].val;

   TandemPowerCompose(h1, h2, h, q1, q2, F);
   r1 = RecComputeDegree(fvec[u].link, h2, F, fvec);
   r2 = RecComputeDegree(fvec[u].link+1, h1, F, fvec);
   return r1*r2;
}

   


long RecComputeDegree(const ZZ_pEX& h, const ZZ_pEXModulus& F)
   // f = F.f is assumed to be an "equal degree" polynomial
   // h = X^p mod f
   // the common degree of the irreducible factors of f is computed
{
   if (F.n == 1 || IsX(h)) 
      return 1;

   FacVec fvec;

   FactorInt(fvec, F.n);

   return RecComputeDegree(fvec.length()-1, h, F, fvec);
}


void FindRoot(ZZ_pE& root, const ZZ_pEX& ff)
// finds a root of ff.
// assumes that ff is monic and splits into distinct linear factors

{
   ZZ_pEXModulus F;
   ZZ_pEX h, h1, f;
   ZZ_pEX r;

   f = ff;
   
   if (!IsOne(LeadCoeff(f)))
      LogicError("FindRoot: bad args");

   if (deg(f) == 0)
      LogicError("FindRoot: bad args");


   while (deg(f) > 1) {
      build(F, f);
      random(r, deg(F));
      if (IsOdd(ZZ_pE::cardinality())) {
         PowerMod(h, r, RightShift(ZZ_pE::cardinality(), 1), F);
         sub(h, h, 1);
      }
      else {
         AbsTraceMap(h, r, F);
      }
      GCD(h, h, f);
      if (deg(h) > 0 && deg(h) < deg(f)) {
         if (deg(h) > deg(f)/2)
            div(f, f, h);
         else
            f = h;
      }
   }
 
   negate(root, ConstTerm(f));
}


static
long power(long a, long e)
{
   long i, res;

   res = 1;
   for (i = 1; i <= e; i++)
      res = res * a;

   return res;
}


static
long IrredBaseCase(const ZZ_pEX& h, long q, long a, const ZZ_pEXModulus& F)
{
   long e;
   ZZ_pEX X, s, d;

   e = power(q, a-1);
   PowerCompose(s, h, e, F);
   SetX(X);
   sub(s, s, X);
   GCD(d, F.f, s);
   return IsOne(d);
}


static
long RecIrredTest(long u, const ZZ_pEX& h, const ZZ_pEXModulus& F,
                 const FacVec& fvec)
{
   long  q1, q2;
   ZZ_pEX h1, h2;

   if (IsX(h)) return 0;

   if (fvec[u].link == -1) {
      return IrredBaseCase(h, fvec[u].q, fvec[u].a, F);
   }


   q1 = fvec[fvec[u].link].val; 
   q2 = fvec[fvec[u].link+1].val;

   TandemPowerCompose(h1, h2, h, q1, q2, F);
   return RecIrredTest(fvec[u].link, h2, F, fvec) 
          && RecIrredTest(fvec[u].link+1, h1, F, fvec);
}

long DetIrredTest(const ZZ_pEX& f)
{
   if (deg(f) <= 0) return 0;
   if (deg(f) == 1) return 1;

   ZZ_pEXModulus F;

   build(F, f);
   
   ZZ_pEX h;

   FrobeniusMap(h, F);

   ZZ_pEX s;
   PowerCompose(s, h, F.n, F);
   if (!IsX(s)) return 0;

   FacVec fvec;

   FactorInt(fvec, F.n);

   return RecIrredTest(fvec.length()-1, h, F, fvec);
}



long IterIrredTest(const ZZ_pEX& f)
{
   if (deg(f) <= 0) return 0;
   if (deg(f) == 1) return 1;

   ZZ_pEXModulus F;

   build(F, f);
   
   ZZ_pEX h;

   FrobeniusMap(h, F);

   long CompTableSize = 2*SqrRoot(deg(f));

   ZZ_pEXArgument H;

   build(H, h, F, CompTableSize);

   long i, d, limit, limit_sqr;
   ZZ_pEX g, X, t, prod;


   SetX(X);

   i = 0;
   g = h;
   d = 1;
   limit = 2;
   limit_sqr = limit*limit;

   set(prod);


   while (2*d <= deg(f)) {
      sub(t, g, X);
      MulMod(prod, prod, t, F);
      i++;
      if (i == limit_sqr) {
         GCD(t, f, prod);
         if (!IsOne(t)) return 0;

         set(prod);
         limit++;
         limit_sqr = limit*limit;
         i = 0;
      }

      d = d + 1;
      if (2*d <= deg(f)) {
         CompMod(g, g, H, F);
      }
   }

   if (i > 0) {
      GCD(t, f, prod);
      if (!IsOne(t)) return 0;
   }

   return 1;
}

static
void MulByXPlusY(vec_ZZ_pEX& h, const ZZ_pEX& f, const ZZ_pEX& g)
// h represents the bivariate polynomial h[0] + h[1]*Y + ... + h[n-1]*Y^k,
// where the h[i]'s are polynomials in X, each of degree < deg(f),
// and k < deg(g).
// h is replaced by the bivariate polynomial h*(X+Y) (mod f(X), g(Y)).

{
   long n = deg(g);
   long k = h.length()-1;

   if (k < 0) return;

   if (k < n-1) {
      h.SetLength(k+2);
      h[k+1] = h[k];
      for (long i = k; i >= 1; i--) {
         MulByXMod(h[i], h[i], f);
         add(h[i], h[i], h[i-1]);
      }
      MulByXMod(h[0], h[0], f);
   }
   else {
      ZZ_pEX b, t;

      b = h[n-1];
      for (long i = n-1; i >= 1; i--) {
         mul(t, b, g.rep[i]);
         MulByXMod(h[i], h[i], f);
         add(h[i], h[i], h[i-1]);
         sub(h[i], h[i], t);
      }
      mul(t, b, g.rep[0]);
      MulByXMod(h[0], h[0], f);
      sub(h[0], h[0], t);
   }

   // normalize

   k = h.length()-1;
   while (k >= 0 && IsZero(h[k])) k--;
   h.SetLength(k+1);
}


static
void IrredCombine(ZZ_pEX& x, const ZZ_pEX& f, const ZZ_pEX& g)
{
   if (deg(f) < deg(g)) {
      IrredCombine(x, g, f);
      return;
   }

   // deg(f) >= deg(g)...not necessary, but maybe a little more
   //                    time & space efficient

   long df = deg(f);
   long dg = deg(g);
   long m = df*dg;

   vec_ZZ_pEX h(INIT_SIZE, dg);

   long i;
   for (i = 0; i < dg; i++) h[i].SetMaxLength(df);

   h.SetLength(1);
   set(h[0]);

   vec_ZZ_pE a;

   a.SetLength(2*m);

   for (i = 0; i < 2*m; i++) {
      a[i] = ConstTerm(h[0]);
      if (i < 2*m-1)
         MulByXPlusY(h, f, g);
   }

   MinPolySeq(x, a, m);
}


static
void BuildPrimePowerIrred(ZZ_pEX& f, long q, long e)
{
   long n = power(q, e);

   do {
      random(f, n);
      SetCoeff(f, n);
   } while (!IterIrredTest(f));
}

static
void RecBuildIrred(ZZ_pEX& f, long u, const FacVec& fvec)
{
   if (fvec[u].link == -1)
      BuildPrimePowerIrred(f, fvec[u].q, fvec[u].a);
   else {
      ZZ_pEX g, h;
      RecBuildIrred(g, fvec[u].link, fvec);
      RecBuildIrred(h, fvec[u].link+1, fvec);
      IrredCombine(f, g, h);
   }
}


void BuildIrred(ZZ_pEX& f, long n)
{
   if (n <= 0)
      LogicError("BuildIrred: n must be positive");

   if (NTL_OVERFLOW(n, 1, 0)) ResourceError("overflow in BuildIrred");

   if (n == 1) {
      SetX(f);
      return;
   }

   FacVec fvec;

   FactorInt(fvec, n);

   RecBuildIrred(f, fvec.length()-1, fvec);
}



#if 0
void BuildIrred(ZZ_pEX& f, long n)
{
   if (n <= 0)
      LogicError("BuildIrred: n must be positive");

   if (n == 1) {
      SetX(f);
      return;
   }

   ZZ_pEX g;

   do {
      random(g, n);
      SetCoeff(g, n);
   } while (!IterIrredTest(g));

   f = g;

}
#endif



void BuildRandomIrred(ZZ_pEX& f, const ZZ_pEX& g)
{
   ZZ_pEXModulus G;
   ZZ_pEX h, ff;

   build(G, g);
   do {
      random(h, deg(g));
      IrredPolyMod(ff, h, G);
   } while (deg(ff) < deg(g));

   f = ff;
}


/************* NEW DDF ****************/

NTL_CHEAP_THREAD_LOCAL long ZZ_pEX_GCDTableSize = 4;
NTL_CHEAP_THREAD_LOCAL double ZZ_pEXFileThresh = NTL_FILE_THRESH;
static NTL_CHEAP_THREAD_LOCAL vec_ZZ_pEX *BabyStepFile=0;
static NTL_CHEAP_THREAD_LOCAL vec_ZZ_pEX *GiantStepFile=0;
static NTL_CHEAP_THREAD_LOCAL long use_files;


static
double CalcTableSize(long n, long k)
{
   double sz = ZZ_p::storage();
   sz = sz*ZZ_pE::degree();
   sz = sz + NTL_VECTOR_HEADER_SIZE + sizeof(vec_ZZ_p);
   sz = sz*n;
   sz = sz + NTL_VECTOR_HEADER_SIZE + sizeof(vec_ZZ_pE);
   sz = sz * k;
   sz = sz/1024;
   return sz;
}


static
void GenerateBabySteps(ZZ_pEX& h1, const ZZ_pEX& f, const ZZ_pEX& h, long k,
                       FileList& flist, long verbose)

{
   double t;

   if (verbose) { cerr << "generating baby steps..."; t = GetTime(); }

   ZZ_pEXModulus F;
   build(F, f);

   ZZ_pEXArgument H;

#if 0
   double n2 = sqrt(double(F.n));
   double n4 = sqrt(n2);
   double n34 = n2*n4;
   long sz = long(ceil(n34/sqrt(sqrt(2.0))));
#else
   long sz = 2*SqrRoot(F.n);
#endif

   build(H, h, F, sz);


   h1 = h;

   long i;

   if (!use_files) {
      (*BabyStepFile).SetLength(k-1);
   }

   for (i = 1; i <= k-1; i++) {
      if (use_files) {
         ofstream s;
         OpenWrite(s, FileName("baby", i), flist);
         s << h1 << "\n";
         CloseWrite(s);
      }
      else
         (*BabyStepFile)(i) = h1;

      CompMod(h1, h1, H, F);
      if (verbose) cerr << "+";
   }

   if (verbose)
      cerr << (GetTime()-t) << "\n";

}


static
void GenerateGiantSteps(const ZZ_pEX& f, const ZZ_pEX& h, long l, 
                        FileList& flist, long verbose)
{

   double t;

   if (verbose) { cerr << "generating giant steps..."; t = GetTime(); }

   ZZ_pEXModulus F;
   build(F, f);

   ZZ_pEXArgument H;

#if 0
   double n2 = sqrt(double(F.n));
   double n4 = sqrt(n2);
   double n34 = n2*n4;
   long sz = long(ceil(n34/sqrt(sqrt(2.0))));
#else
   long sz = 2*SqrRoot(F.n);
#endif

   build(H, h, F, sz);

   ZZ_pEX h1;

   h1 = h;

   long i;

   if (!use_files) {
      (*GiantStepFile).SetLength(l);
   }

   for (i = 1; i <= l-1; i++) {
      if (use_files) {
         ofstream s;
         OpenWrite(s, FileName("giant", i), flist);
         s << h1 << "\n";
         CloseWrite(s);
      }
      else
        (*GiantStepFile)(i) = h1;

      CompMod(h1, h1, H, F);
      if (verbose) cerr << "+";
   }

   if (use_files) {
      ofstream s;
      OpenWrite(s, FileName("giant", i), flist);
      s << h1 << "\n";
      CloseWrite(s);
   }
   else
      (*GiantStepFile)(i) = h1;

   if (verbose)
      cerr << (GetTime()-t) << "\n";

}


static
void NewAddFactor(vec_pair_ZZ_pEX_long& u, const ZZ_pEX& g, long m, long verbose)
{
   long len = u.length();

   u.SetLength(len+1);
   u[len].a = g;
   u[len].b = m;

   if (verbose) {
      cerr << "split " << m << " " << deg(g) << "\n";
   }
}

   


static
void NewProcessTable(vec_pair_ZZ_pEX_long& u, ZZ_pEX& f, const ZZ_pEXModulus& F,
                     vec_ZZ_pEX& buf, long size, long StartInterval,
                     long IntervalLength, long verbose)

{
   if (size == 0) return;

   ZZ_pEX& g = buf[size-1];

   long i;

   for (i = 0; i < size-1; i++)
      MulMod(g, g, buf[i], F);

   GCD(g, f, g);

   if (deg(g) == 0) return;

   div(f, f, g);

   long d = (StartInterval-1)*IntervalLength + 1;
   i = 0;
   long interval = StartInterval;

   while (i < size-1 && 2*d <= deg(g)) {
      GCD(buf[i], buf[i], g);
      if (deg(buf[i]) > 0) {
         NewAddFactor(u, buf[i], interval, verbose);
         div(g, g, buf[i]);
      }

      i++;
      interval++;
      d += IntervalLength;
   }

   if (deg(g) > 0) {
      if (i == size-1)
         NewAddFactor(u, g, interval, verbose);
      else
         NewAddFactor(u, g, (deg(g)+IntervalLength-1)/IntervalLength, verbose);
   }
}


static
void FetchGiantStep(ZZ_pEX& g, long gs, const ZZ_pEXModulus& F)
{
   if (use_files) {
      ifstream s;
      OpenRead(s, FileName("giant", gs));
      NTL_INPUT_CHECK_ERR(s >> g);
   }
   else
      g = (*GiantStepFile)(gs);


   rem(g, g, F);
}


static
void FetchBabySteps(vec_ZZ_pEX& v, long k)
{
   v.SetLength(k);

   SetX(v[0]);

   long i;
   for (i = 1; i <= k-1; i++) {
      if (use_files) {
         ifstream s;
         OpenRead(s, FileName("baby", i));
         NTL_INPUT_CHECK_ERR(s >> v[i]);
      }
      else
         v[i] = (*BabyStepFile)(i);
   }
}
      


static
void GiantRefine(vec_pair_ZZ_pEX_long& u, const ZZ_pEX& ff, long k, long l,
                 long verbose)

{
   double t;

   if (verbose) {
      cerr << "giant refine...";
      t = GetTime();
   }

   u.SetLength(0);

   vec_ZZ_pEX BabyStep;

   FetchBabySteps(BabyStep, k);

   vec_ZZ_pEX buf(INIT_SIZE, ZZ_pEX_GCDTableSize);

   ZZ_pEX f;
   f = ff;

   ZZ_pEXModulus F;
   build(F, f);

   ZZ_pEX g;
   ZZ_pEX h;

   long size = 0;

   long first_gs;

   long d = 1;

   while (2*d <= deg(f)) {

      long old_n = deg(f);

      long gs = (d+k-1)/k;
      long bs = gs*k - d;

      if (bs == k-1) {
         size++;
         if (size == 1) first_gs = gs;
         FetchGiantStep(g, gs, F);
         sub(buf[size-1], g, BabyStep[bs]);
      }
      else {
         sub(h, g, BabyStep[bs]);
         MulMod(buf[size-1], buf[size-1], h, F);
      }

      if (verbose && bs == 0) cerr << "+";

      if (size == ZZ_pEX_GCDTableSize && bs == 0) {
         NewProcessTable(u, f, F, buf, size, first_gs, k, verbose);
         if (verbose) cerr << "*";
         size = 0;
      }

      d++;

      if (2*d <= deg(f) && deg(f) < old_n) {
         build(F, f);

         long i;
         for (i = 1; i <= k-1; i++) 
            rem(BabyStep[i], BabyStep[i], F);
      }
   }

   if (size > 0) {
      NewProcessTable(u, f, F, buf, size, first_gs, k, verbose);
      if (verbose) cerr << "*";
   }

   if (deg(f) > 0) 
      NewAddFactor(u, f, 0, verbose);

   if (verbose) {
      t = GetTime()-t;
      cerr << "giant refine time: " << t << "\n";
   }
}


static
void IntervalRefine(vec_pair_ZZ_pEX_long& factors, const ZZ_pEX& ff,
                    long k, long gs, const vec_ZZ_pEX& BabyStep, long verbose)

{
   vec_ZZ_pEX buf(INIT_SIZE, ZZ_pEX_GCDTableSize);

   ZZ_pEX f;
   f = ff;

   ZZ_pEXModulus F;
   build(F, f);

   ZZ_pEX g;

   FetchGiantStep(g, gs, F);

   long size = 0;

   long first_d;

   long d = (gs-1)*k + 1;
   long bs = k-1;

   while (bs >= 0 && 2*d <= deg(f)) {

      long old_n = deg(f);

      if (size == 0) first_d = d;
      rem(buf[size], BabyStep[bs], F);
      sub(buf[size], buf[size], g);
      size++;

      if (size == ZZ_pEX_GCDTableSize) {
         NewProcessTable(factors, f, F, buf, size, first_d, 1, verbose);
         size = 0;
      }

      d++;
      bs--;

      if (bs >= 0 && 2*d <= deg(f) && deg(f) < old_n) {
         build(F, f);
         rem(g, g, F);
      }
   }

   NewProcessTable(factors, f, F, buf, size, first_d, 1, verbose);

   if (deg(f) > 0) 
      NewAddFactor(factors, f, deg(f), verbose);
}
   



static
void BabyRefine(vec_pair_ZZ_pEX_long& factors, const vec_pair_ZZ_pEX_long& u,
                long k, long l, long verbose)

{
   double t;

   if (verbose) {
      cerr << "baby refine...";
      t = GetTime();
   }

   factors.SetLength(0);

   vec_ZZ_pEX BabyStep;

   long i;
   for (i = 0; i < u.length(); i++) {
      const ZZ_pEX& g = u[i].a;
      long gs = u[i].b;

      if (gs == 0 || 2*((gs-1)*k+1) > deg(g))
         NewAddFactor(factors, g, deg(g), verbose);
      else {
         if (BabyStep.length() == 0)
            FetchBabySteps(BabyStep, k);
         IntervalRefine(factors, g, k, gs, BabyStep, verbose);
      }
   }

   if (verbose) {
      t = GetTime()-t;
      cerr << "baby refine time: " << t << "\n";
   }
}

      
      

      

void NewDDF(vec_pair_ZZ_pEX_long& factors,
            const ZZ_pEX& f,
            const ZZ_pEX& h,
            long verbose)

{
   if (!IsOne(LeadCoeff(f)))
      LogicError("NewDDF: bad args");

   if (deg(f) == 0) {
      factors.SetLength(0);
      return;
   }

   if (deg(f) == 1) {
      factors.SetLength(0);
      append(factors, cons(f, 1L));
      return;
   }

   long B = deg(f)/2;
   long k = SqrRoot(B);
   long l = (B+k-1)/k;

   ZZ_pEX h1;

   if (CalcTableSize(deg(f), k + l - 1) > ZZ_pEXFileThresh)
      use_files = 1;
   else
      use_files = 0;


   FileList flist;

   vec_ZZ_pEX local_BabyStepFile;
   vec_ZZ_pEX local_GiantStepFile;

   BabyStepFile = &local_BabyStepFile;
   GiantStepFile = &local_GiantStepFile;


   GenerateBabySteps(h1, f, h, k, flist, verbose);

   GenerateGiantSteps(f, h1, l, flist, verbose);

   vec_pair_ZZ_pEX_long u;
   GiantRefine(u, f, k, l, verbose);
   BabyRefine(factors, u, k, l, verbose);
}

long IterComputeDegree(const ZZ_pEX& h, const ZZ_pEXModulus& F)
{
   long n = deg(F);

   if (n == 1 || IsX(h)) return 1;

   long B = n/2;
   long k = SqrRoot(B);
   long l = (B+k-1)/k;


   ZZ_pEXArgument H;

#if 0
   double n2 = sqrt(double(n));
   double n4 = sqrt(n2);
   double n34 = n2*n4;
   long sz = long(ceil(n34/sqrt(sqrt(2.0))));
#else
   long sz = 2*SqrRoot(F.n);
#endif

   build(H, h, F, sz);

   ZZ_pEX h1;
   h1 = h;

   vec_ZZ_pEX baby;
   baby.SetLength(k);

   SetX(baby[0]);

   long i;

   for (i = 1; i <= k-1; i++) {
      baby[i] = h1;
      CompMod(h1, h1, H, F);
      if (IsX(h1)) return i+1;
   }

   build(H, h1, F, sz);

   long j;

   for (j = 2; j <= l; j++) {
      CompMod(h1, h1, H, F);

      for (i = k-1; i >= 0; i--) {
         if (h1 == baby[i])
            return j*k-i;
      }
   }

   return n;
}

NTL_END_IMPL

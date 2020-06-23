
#include <NTL/ZZ_pXFactoring.h>
#include <NTL/vec_ZZVec.h>
#include <NTL/fileio.h>
#include <NTL/FacVec.h>


NTL_START_IMPL





void SquareFreeDecomp(vec_pair_ZZ_pX_long& u, const ZZ_pX& ff)
{
   ZZ_pX f = ff;

   if (!IsOne(LeadCoeff(f)))
      LogicError("SquareFreeDecomp: bad args");

   ZZ_pX r, t, v, tmp1;
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
         long p, k, d;
         conv(p, ZZ_p::modulus());
         d = deg(r)/p;
         f.rep.SetLength(d+1);
         for (k = 0; k <= d; k++) 
            f.rep[k] = r.rep[k*p];
         m = m*p;
      }
   } while (!finished);
}
         


static
void NullSpace(long& r, vec_long& D, vec_ZZVec& M, long verbose)
{
   long k, l, n;
   long i, j;
   long pos;
   ZZ t1, t2;
   ZZ *x, *y;

   const ZZ& p = ZZ_p::modulus();

   n = M.length();

   D.SetLength(n);
   for (j = 0; j < n; j++) D[j] = -1;

   r = 0;

   l = 0;
   for (k = 0; k < n; k++) {

      if (verbose && k % 10 == 0) cerr << "+";

      pos = -1;
      for (i = l; i < n; i++) {
         rem(t1, M[i][k], p);
         M[i][k] = t1;
         if (pos == -1 && !IsZero(t1))
            pos = i;
      }

      if (pos != -1) {
         swap(M[pos], M[l]);

         // make M[l, k] == -1 mod p, and make row l reduced

         InvMod(t1, M[l][k], p);
         NegateMod(t1, t1, p);
         for (j = k+1; j < n; j++) {
            rem(t2, M[l][j], p);
            MulMod(M[l][j], t2, t1, p);
         }

         for (i = l+1; i < n; i++) {
            // M[i] = M[i] + M[l]*M[i,k]

            t1 = M[i][k];   // this is already reduced

            x = M[i].elts() + (k+1);
            y = M[l].elts() + (k+1);

            for (j = k+1; j < n; j++, x++, y++) {
               // *x = *x + (*y)*t1

               mul(t2, *y, t1);
               add(*x, *x, t2);
            }
         }

         D[k] = l;   // variable k is defined by row l
         l++;

      }
      else {
         r++;
      }
   }
}



static
void BuildMatrix(vec_ZZVec& M, long n, const ZZ_pX& g, const ZZ_pXModulus& F,
                 long verbose)
{
   long i, j, m;
   ZZ_pXMultiplier G;
   ZZ_pX h;

   ZZ t;
   sqr(t, ZZ_p::modulus());
   mul(t, t, n);

   long size = t.size();

   M.SetLength(n);
   for (i = 0; i < n; i++)
      M[i].SetSize(n, size);

   build(G, g, F);

   set(h);
   for (j = 0; j < n; j++) {
      if (verbose && j % 10 == 0) cerr << "+";

      m = deg(h);
      for (i = 0; i < n; i++) {
         if (i <= m)
            M[i][j] = rep(h.rep[i]);
         else
            clear(M[i][j]);
      }

      if (j < n-1)
         MulMod(h, h, G, F);
   }

   for (i = 0; i < n; i++)
      AddMod(M[i][i], M[i][i], -1, ZZ_p::modulus());

}



static
void RecFindRoots(vec_ZZ_p& x, const ZZ_pX& f)
{
   if (deg(f) == 0) return;

   if (deg(f) == 1) {
      long k = x.length();
      x.SetLength(k+1);
      negate(x[k], ConstTerm(f));
      return;
   }
      
   ZZ_pX h;

   ZZ_p r;
   ZZ p1;


   RightShift(p1, ZZ_p::modulus(), 1);
   
   {
      ZZ_pXModulus F;
      build(F, f);

      do {
         random(r);
         PowerXPlusAMod(h, r, p1, F);
         add(h, h, -1);
         GCD(h, h, f);
      } while (deg(h) <= 0 || deg(h) == deg(f));
   }

   RecFindRoots(x, h);
   div(h, f, h); 
   RecFindRoots(x, h);
}

void FindRoots(vec_ZZ_p& x, const ZZ_pX& ff)
{
   ZZ_pX f = ff;

   if (!IsOne(LeadCoeff(f)))
      LogicError("FindRoots: bad args");

   x.SetMaxLength(deg(f));
   x.SetLength(0);
   RecFindRoots(x, f);
}


static
void RandomBasisElt(ZZ_pX& g, const vec_long& D, const vec_ZZVec& M)
{
   ZZ t1, t2;

   long n = D.length();

   long i, j, s;

   g.rep.SetLength(n);

   vec_ZZ_p& v = g.rep;

   for (j = n-1; j >= 0; j--) {
      if (D[j] == -1)
         random(v[j]);
      else {
         i = D[j];

         // v[j] = sum_{s=j+1}^{n-1} v[s]*M[i,s]

         clear(t1);

         for (s = j+1; s < n; s++) {
            mul(t2, rep(v[s]), M[i][s]);
            add(t1, t1, t2);
         }

         conv(v[j], t1);
      }
   }

   g.normalize();
}



static
void split(ZZ_pX& f1, ZZ_pX& g1, ZZ_pX& f2, ZZ_pX& g2,
           const ZZ_pX& f, const ZZ_pX& g, 
           const vec_ZZ_p& roots, long lo, long mid)
{
   long r = mid-lo+1;

   ZZ_pXModulus F;
   build(F, f);

   vec_ZZ_p lroots(INIT_SIZE, r);
   long i;

   for (i = 0; i < r; i++)
      lroots[i] = roots[lo+i];


   ZZ_pX h, a, d;
   BuildFromRoots(h, lroots);
   CompMod(a, h, g, F);


   GCD(f1, a, f);
   
   div(f2, f, f1);

   rem(g1, g, f1);
   rem(g2, g, f2);
}

static
void RecFindFactors(vec_ZZ_pX& factors, const ZZ_pX& f, const ZZ_pX& g,
                    const vec_ZZ_p& roots, long lo, long hi)
{
   long r = hi-lo+1;

   if (r == 0) return;

   if (r == 1) {
      append(factors, f);
      return;
   }

   ZZ_pX f1, g1, f2, g2;

   long mid = (lo+hi)/2;

   split(f1, g1, f2, g2, f, g, roots, lo, mid);

   RecFindFactors(factors, f1, g1, roots, lo, mid);
   RecFindFactors(factors, f2, g2, roots, mid+1, hi);
}


static
void FindFactors(vec_ZZ_pX& factors, const ZZ_pX& f, const ZZ_pX& g,
                 const vec_ZZ_p& roots)
{
   long r = roots.length();

   factors.SetMaxLength(r);
   factors.SetLength(0);

   RecFindFactors(factors, f, g, roots, 0, r-1);
}

#if 0

static
void IterFindFactors(vec_ZZ_pX& factors, const ZZ_pX& f,
                     const ZZ_pX& g, const vec_ZZ_p& roots)
{
   long r = roots.length();
   long i;
   ZZ_pX h;

   factors.SetLength(r);

   for (i = 0; i < r; i++) {
      sub(h, g, roots[i]);
      GCD(factors[i], f, h);
   }
}

#endif


   

void SFBerlekamp(vec_ZZ_pX& factors, const ZZ_pX& ff, long verbose)
{
   ZZ_pX f = ff;

   if (!IsOne(LeadCoeff(f)))
      LogicError("SFBerlekamp: bad args");

   if (deg(f) == 0) {
      factors.SetLength(0);
      return;
   }

   if (deg(f) == 1) {
      factors.SetLength(1);
      factors[0] = f;
      return;
   }

   double t;

   const ZZ& p = ZZ_p::modulus();

   long n = deg(f);

   ZZ_pXModulus F;

   build(F, f);

   ZZ_pX g, h;

   if (verbose) { cerr << "computing X^p..."; t = GetTime(); }
   PowerXMod(g, p, F);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   vec_long D;
   long r;

   vec_ZZVec M;

   if (verbose) { cerr << "building matrix..."; t = GetTime(); }
   BuildMatrix(M, n, g, F, verbose);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   if (verbose) { cerr << "diagonalizing..."; t = GetTime(); }
   NullSpace(r, D, M, verbose);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }


   if (verbose) cerr << "number of factors = " << r << "\n";

   if (r == 1) {
      factors.SetLength(1);
      factors[0] = f;
      return;
   }

   if (verbose) { cerr << "factor extraction..."; t = GetTime(); }

   vec_ZZ_p roots;

   RandomBasisElt(g, D, M);
   MinPolyMod(h, g, F, r);
   if (deg(h) == r) M.kill();
   FindRoots(roots, h);
   FindFactors(factors, f, g, roots);

   ZZ_pX g1;
   vec_ZZ_pX S, S1;
   long i;

   while (factors.length() < r) {
      if (verbose) cerr << "+";
      RandomBasisElt(g, D, M);
      S.kill();
      for (i = 0; i < factors.length(); i++) {
         const ZZ_pX& f = factors[i];
         if (deg(f) == 1) {
            append(S, f);
            continue;
         }
         build(F, f);
         rem(g1, g, F);
         if (deg(g1) <= 0) {
            append(S, f);
            continue;
         }
         MinPolyMod(h, g1, F, min(deg(f), r-factors.length()+1));
         FindRoots(roots, h);
         S1.kill();
         FindFactors(S1, f, g1, roots);
         append(S, S1);
      }
      swap(factors, S);
   }

   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   if (verbose) {
      cerr << "degrees:";
      long i;
      for (i = 0; i < factors.length(); i++)
         cerr << " " << deg(factors[i]);
      cerr << "\n";
   }
}


void berlekamp(vec_pair_ZZ_pX_long& factors, const ZZ_pX& f, long verbose)
{
   double t;
   vec_pair_ZZ_pX_long sfd;
   vec_ZZ_pX x;

   if (!IsOne(LeadCoeff(f)))
      LogicError("berlekamp: bad args");

   
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

      SFBerlekamp(x, sfd[i].a, verbose);

      for (j = 0; j < x.length(); j++)
         append(factors, cons(x[j], sfd[i].b));
   }
}



static
void AddFactor(vec_pair_ZZ_pX_long& factors, const ZZ_pX& g, long d, long verbose)
{
   if (verbose)
      cerr << "degree=" << d << ", number=" << deg(g)/d << "\n";
   append(factors, cons(g, d));
}

static
void ProcessTable(ZZ_pX& f, vec_pair_ZZ_pX_long& factors, 
                  const ZZ_pXModulus& F, long limit, const vec_ZZ_pX& tbl,
                  long d, long verbose)

{
   if (limit == 0) return;

   if (verbose) cerr << "+";

   ZZ_pX t1;

   if (limit == 1) {
      GCD(t1, f, tbl[0]);
      if (deg(t1) > 0) {
         AddFactor(factors, t1, d, verbose);
         div(f, f, t1);
      }

      return;
   }

   long i;

   t1 = tbl[0];
   for (i = 1; i < limit; i++)
      MulMod(t1, t1, tbl[i], F);

   GCD(t1, f, t1);

   if (deg(t1) == 0) return;

   div(f, f, t1);

   ZZ_pX t2;

   i = 0;
   d = d - limit + 1;

   while (2*d <= deg(t1)) {
      GCD(t2, tbl[i], t1); 
      if (deg(t2) > 0) {
         AddFactor(factors, t2, d, verbose);
         div(t1, t1, t2);
      }

      i++;
      d++;
   }

   if (deg(t1) > 0)
      AddFactor(factors, t1, deg(t1), verbose);
}


void TraceMap(ZZ_pX& w, const ZZ_pX& a, long d, const ZZ_pXModulus& F, 
              const ZZ_pX& b)

{
   if (d < 0) LogicError("TraceMap: bad args");

   ZZ_pX y, z, t;

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


void PowerCompose(ZZ_pX& y, const ZZ_pX& h, long q, const ZZ_pXModulus& F)
{
   if (q < 0) LogicError("PowerCompose: bad args");

   ZZ_pX z(INIT_SIZE, F.n);
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


long ProbIrredTest(const ZZ_pX& f, long iter)
{
   long n = deg(f);

   if (n <= 0) return 0;
   if (n == 1) return 1;

   const ZZ& p = ZZ_p::modulus();

   ZZ_pXModulus F;

   build(F, f);

   ZZ_pX b, r, s;

   PowerXMod(b, p, F);

   long i;

   for (i = 0; i < iter; i++) {
      random(r, n);
      TraceMap(s, r, n, F, b);

      if (deg(s) > 0) return 0;
   }

   if (p >= n) return 1;

   long pp;

   conv(pp, p);
   
   if (n % pp != 0) return 1;

   PowerCompose(s, b, n/pp, F);
   return !IsX(s);
}

NTL_CHEAP_THREAD_LOCAL long ZZ_pX_BlockingFactor = 10;

void DDF(vec_pair_ZZ_pX_long& factors, const ZZ_pX& ff, const ZZ_pX& hh, 
         long verbose)
{
   ZZ_pX f = ff;
   ZZ_pX h = hh;

   if (!IsOne(LeadCoeff(f)))
      LogicError("DDF: bad args");

   factors.SetLength(0);

   if (deg(f) == 0)
      return;

   if (deg(f) == 1) {
      AddFactor(factors, f, 1, verbose);
      return;
   }

   long CompTableSize = 2*SqrRoot(deg(f)); 

   long GCDTableSize = ZZ_pX_BlockingFactor;

   ZZ_pXModulus F;
   build(F, f);

   ZZ_pXNewArgument H;

   build(H, h, F, min(CompTableSize, deg(f)));

   long i, d, limit, old_n;
   ZZ_pX g, X;


   vec_ZZ_pX tbl(INIT_SIZE, GCDTableSize);

   SetX(X);

   i = 0;
   g = h;
   d = 1;
   limit = GCDTableSize;


   while (2*d <= deg(f)) {

      old_n = deg(f);
      sub(tbl[i], g, X);
      i++;
      if (i == limit) {
         ProcessTable(f, factors, F, i, tbl, d, verbose);
         i = 0;
      }

      d = d + 1;
      if (2*d <= deg(f)) {
         // we need to go further

         if (deg(f) < old_n) {
            // f has changed 

            build(F, f);
            rem(h, h, f);
            rem(g, g, f);
            build(H, h, F, min(CompTableSize, deg(f)));
         }

         CompMod(g, g, H, F);
      }
   }

   ProcessTable(f, factors, F, i, tbl, d-1, verbose);

   if (!IsOne(f)) AddFactor(factors, f, deg(f), verbose);
}



void RootEDF(vec_ZZ_pX& factors, const ZZ_pX& f, long verbose)
{
   vec_ZZ_p roots;
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

static
void EDFSplit(vec_ZZ_pX& v, const ZZ_pX& f, const ZZ_pX& b, long d)
{
   ZZ_pX a, g, h;
   ZZ_pXModulus F;
   vec_ZZ_p roots;
   
   build(F, f);
   long n = F.n;
   long r = n/d;
   random(a, n);
   TraceMap(g, a, d, F, b);
   MinPolyMod(h, g, F, r);
   FindRoots(roots, h);
   FindFactors(v, f, g, roots);
}

static
void RecEDF(vec_ZZ_pX& factors, const ZZ_pX& f, const ZZ_pX& b, long d,
            long verbose)
{
   vec_ZZ_pX v;
   long i;
   ZZ_pX bb;

   if (verbose) cerr << "+";

   EDFSplit(v, f, b, d);
   for (i = 0; i < v.length(); i++) {
      if (deg(v[i]) == d) {
         append(factors, v[i]);
      }
      else {
         ZZ_pX bb;
         rem(bb, b, v[i]);
         RecEDF(factors, v[i], bb, d, verbose);
      }
   }
}
         

void EDF(vec_ZZ_pX& factors, const ZZ_pX& ff, const ZZ_pX& bb,
         long d, long verbose)

{
   ZZ_pX f = ff;
   ZZ_pX b = bb;

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


void SFCanZass(vec_ZZ_pX& factors, const ZZ_pX& ff, long verbose)
{
   ZZ_pX f = ff;

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

   const ZZ& p = ZZ_p::modulus();

   
   ZZ_pXModulus F;
   build(F, f);

   ZZ_pX h;

   if (verbose) { cerr << "computing X^p..."; t = GetTime(); }
   PowerXMod(h, p, F);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   vec_pair_ZZ_pX_long u;
   if (verbose) { cerr << "computing DDF..."; t = GetTime(); }
   NewDDF(u, f, h, verbose);
   if (verbose) { 
      t = GetTime()-t; 
      cerr << "DDF time: " << t << "\n";
   }

   ZZ_pX hh;
   vec_ZZ_pX v;

   long i;
   for (i = 0; i < u.length(); i++) {
      const ZZ_pX& g = u[i].a;
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
   
void CanZass(vec_pair_ZZ_pX_long& factors, const ZZ_pX& f, long verbose)
{
   if (!IsOne(LeadCoeff(f)))
      LogicError("CanZass: bad args");

   double t;
   vec_pair_ZZ_pX_long sfd;
   vec_ZZ_pX x;

   
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

void mul(ZZ_pX& f, const vec_pair_ZZ_pX_long& v)
{
   long i, j, n;

   n = 0;
   for (i = 0; i < v.length(); i++)
      n += v[i].b*deg(v[i].a);

   ZZ_pX g(INIT_SIZE, n+1);

   set(g);
   for (i = 0; i < v.length(); i++)
      for (j = 0; j < v[i].b; j++) {
         mul(g, g, v[i].a);
      }

   f = g;
}




static
long BaseCase(const ZZ_pX& h, long q, long a, const ZZ_pXModulus& F)
{
   long b, e;
   ZZ_pX lh(INIT_SIZE, F.n);

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



static
void TandemPowerCompose(ZZ_pX& y1, ZZ_pX& y2, const ZZ_pX& h, 
                        long q1, long q2, const ZZ_pXModulus& F)
{
   ZZ_pX z(INIT_SIZE, F.n);
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



static
long RecComputeDegree(long u, const ZZ_pX& h, const ZZ_pXModulus& F,
                      FacVec& fvec)
{
   if (IsX(h)) return 1;

   if (fvec[u].link == -1) return BaseCase(h, fvec[u].q, fvec[u].a, F);

   ZZ_pX h1, h2;
   long q1, q2, r1, r2;

   q1 = fvec[fvec[u].link].val; 
   q2 = fvec[fvec[u].link+1].val;

   TandemPowerCompose(h1, h2, h, q1, q2, F);
   r1 = RecComputeDegree(fvec[u].link, h2, F, fvec);
   r2 = RecComputeDegree(fvec[u].link+1, h1, F, fvec);
   return r1*r2;
}

   


long ComputeDegree(const ZZ_pX& h, const ZZ_pXModulus& F)
   // f = F.f is assumed to be an "equal degree" polynomial
   // h = X^p mod f
   // the common degree of the irreducible factors of f is computed
{
   if (F.n == 1 || IsX(h)) return 1;

   FacVec fvec;

   FactorInt(fvec, F.n);

   return RecComputeDegree(fvec.length()-1, h, F, fvec);
}

long ProbComputeDegree(const ZZ_pX& h, const ZZ_pXModulus& F)
{
   if (F.n == 1 || IsX(h))
      return 1;

   long n = F.n;

   ZZ_pX P1, P2, P3;

   random(P1, n);
   TraceMap(P2, P1, n, F, h);
   ProbMinPolyMod(P3, P2, F, n/2);

   long r = deg(P3);

   if (r <= 0 || n % r != 0)
      return 0;
   else
      return n/r;
}



void FindRoot(ZZ_p& root, const ZZ_pX& ff)
// finds a root of ff.
// assumes that ff is monic and splits into distinct linear factors

{
   ZZ_pXModulus F;
   ZZ_pX h, h1, f;
   ZZ_p r;
   ZZ p1;

   f = ff;
   
   if (!IsOne(LeadCoeff(f)))
      LogicError("FindRoot: bad args");

   if (deg(f) == 0)
      LogicError("FindRoot: bad args");

   RightShift(p1, ZZ_p::modulus(), 1);
   h1 = 1;

   while (deg(f) > 1) {
      build(F, f);
      random(r);
      PowerXPlusAMod(h, r, p1, F);
      sub(h, h, h1);
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
long IrredBaseCase(const ZZ_pX& h, long q, long a, const ZZ_pXModulus& F)
{
   long e;
   ZZ_pX X, s, d;

   e = power(q, a-1);
   PowerCompose(s, h, e, F);
   SetX(X);
   sub(s, s, X);
   GCD(d, F.f, s);
   return IsOne(d);
}


static
long RecIrredTest(long u, const ZZ_pX& h, const ZZ_pXModulus& F,
                 const FacVec& fvec)
{
   long  q1, q2;
   ZZ_pX h1, h2;

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

long DetIrredTest(const ZZ_pX& f)
{
   if (deg(f) <= 0) return 0;
   if (deg(f) == 1) return 1;

   ZZ_pXModulus F;

   build(F, f);
   
   ZZ_pX h;

   PowerXMod(h, ZZ_p::modulus(), F);

   ZZ_pX s;
   PowerCompose(s, h, F.n, F);
   if (!IsX(s)) return 0;

   FacVec fvec;

   FactorInt(fvec, F.n);

   return RecIrredTest(fvec.length()-1, h, F, fvec);
}



long IterIrredTest(const ZZ_pX& f)
{
   if (deg(f) <= 0) return 0;
   if (deg(f) == 1) return 1;

   ZZ_pXModulus F;

   build(F, f);
   
   ZZ_pX h;

   PowerXMod(h, ZZ_p::modulus(), F);

   long CompTableSize = 2*SqrRoot(deg(f));

   ZZ_pXNewArgument H;

   build(H, h, F, CompTableSize);

   long i, d, limit, limit_sqr;
   ZZ_pX g, X, t, prod;


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
void MulByXPlusY(vec_ZZ_pX& h, const ZZ_pX& f, const ZZ_pX& g)
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
      ZZ_pX b, t;

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
void IrredCombine(ZZ_pX& x, const ZZ_pX& f, const ZZ_pX& g)
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

   vec_ZZ_pX h(INIT_SIZE, dg);

   long i;
   for (i = 0; i < dg; i++) h[i].SetMaxLength(df);

   h.SetLength(1);
   set(h[0]);

   vec_ZZ_p a;

   a.SetLength(2*m);

   for (i = 0; i < 2*m; i++) {
      a[i] = ConstTerm(h[0]);
      if (i < 2*m-1)
         MulByXPlusY(h, f, g);
   }

   MinPolySeq(x, a, m);
}


static
void BuildPrimePowerIrred(ZZ_pX& f, long q, long e)
{
   long n = power(q, e);

   do {
      random(f, n);
      SetCoeff(f, n);
   } while (!IterIrredTest(f));
}

static
void RecBuildIrred(ZZ_pX& f, long u, const FacVec& fvec)
{
   if (fvec[u].link == -1)
      BuildPrimePowerIrred(f, fvec[u].q, fvec[u].a);
   else {
      ZZ_pX g, h;
      RecBuildIrred(g, fvec[u].link, fvec);
      RecBuildIrred(h, fvec[u].link+1, fvec);
      IrredCombine(f, g, h);
   }
}


void BuildIrred(ZZ_pX& f, long n)
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



void BuildRandomIrred(ZZ_pX& f, const ZZ_pX& g)
{
   ZZ_pXModulus G;
   ZZ_pX h, ff;

   build(G, g);
   do {
      random(h, deg(g));
      IrredPolyMod(ff, h, G);
   } while (deg(ff) < deg(g));

   f = ff;
}


/************* NEW DDF ****************/

NTL_CHEAP_THREAD_LOCAL long ZZ_pX_GCDTableSize = 4;
NTL_CHEAP_THREAD_LOCAL double ZZ_pXFileThresh = NTL_FILE_THRESH;
static NTL_CHEAP_THREAD_LOCAL vec_ZZ_pX *BabyStepFile = 0;
static NTL_CHEAP_THREAD_LOCAL vec_ZZ_pX *GiantStepFile = 0;
static NTL_CHEAP_THREAD_LOCAL long use_files;


static 
double CalcTableSize(long n, long k)
{
   double sz = ZZ_p::storage();
   sz = sz * n;
   sz = sz + NTL_VECTOR_HEADER_SIZE + sizeof(vec_ZZ_p);
   sz = sz * k;
   sz = sz/1024;
   return sz;
}


static
void GenerateBabySteps(ZZ_pX& h1, const ZZ_pX& f, const ZZ_pX& h, long k,
                       FileList& flist, long verbose)

{
   double t;

   if (verbose) { cerr << "generating baby steps..."; t = GetTime(); }

   ZZ_pXModulus F;
   build(F, f);

   ZZ_pXNewArgument H;
   build(H, h, F, 2*SqrRoot(F.n));


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
void GenerateGiantSteps(const ZZ_pX& f, const ZZ_pX& h, long l, 
                        FileList& flist, long verbose)
{

   double t;

   if (verbose) { cerr << "generating giant steps..."; t = GetTime(); }

   ZZ_pXModulus F;
   build(F, f);

   ZZ_pXNewArgument H;
   build(H, h, F, 2*SqrRoot(F.n));

   ZZ_pX h1;

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
void NewAddFactor(vec_pair_ZZ_pX_long& u, const ZZ_pX& g, long m, long verbose)
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
void NewProcessTable(vec_pair_ZZ_pX_long& u, ZZ_pX& f, const ZZ_pXModulus& F,
                     vec_ZZ_pX& buf, long size, long StartInterval,
                     long IntervalLength, long verbose)

{
   if (size == 0) return;

   ZZ_pX& g = buf[size-1];

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
void FetchGiantStep(ZZ_pX& g, long gs, const ZZ_pXModulus& F)
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
void FetchBabySteps(vec_ZZ_pX& v, long k)
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
void GiantRefine(vec_pair_ZZ_pX_long& u, const ZZ_pX& ff, long k, long l,
                 long verbose)

{
   double t;

   if (verbose) {
      cerr << "giant refine...";
      t = GetTime();
   }

   u.SetLength(0);

   vec_ZZ_pX BabyStep;

   FetchBabySteps(BabyStep, k);

   vec_ZZ_pX buf(INIT_SIZE, ZZ_pX_GCDTableSize);

   ZZ_pX f;
   f = ff;

   ZZ_pXModulus F;
   build(F, f);

   ZZ_pX g;
   ZZ_pX h;

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

      if (size == ZZ_pX_GCDTableSize && bs == 0) {
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
void IntervalRefine(vec_pair_ZZ_pX_long& factors, const ZZ_pX& ff,
                    long k, long gs, const vec_ZZ_pX& BabyStep, long verbose)

{
   vec_ZZ_pX buf(INIT_SIZE, ZZ_pX_GCDTableSize);

   ZZ_pX f;
   f = ff;

   ZZ_pXModulus F;
   build(F, f);

   ZZ_pX g;

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

      if (size == ZZ_pX_GCDTableSize) {
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
void BabyRefine(vec_pair_ZZ_pX_long& factors, const vec_pair_ZZ_pX_long& u,
                long k, long l, long verbose)

{
   double t;

   if (verbose) {
      cerr << "baby refine...";
      t = GetTime();
   }

   factors.SetLength(0);

   vec_ZZ_pX BabyStep;

   long i;
   for (i = 0; i < u.length(); i++) {
      const ZZ_pX& g = u[i].a;
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

      

void NewDDF(vec_pair_ZZ_pX_long& factors,
            const ZZ_pX& f,
            const ZZ_pX& h,
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

   ZZ_pX h1;

   if (CalcTableSize(deg(f), k + l - 1) > ZZ_pXFileThresh)
      use_files = 1;
   else
      use_files = 0;


   FileList flist;

   vec_ZZ_pX local_BabyStepFile;
   vec_ZZ_pX local_GiantStepFile;

   BabyStepFile = &local_BabyStepFile;
   GiantStepFile = &local_GiantStepFile;

   GenerateBabySteps(h1, f, h, k, flist, verbose);

   GenerateGiantSteps(f, h1, l, flist, verbose);


   vec_pair_ZZ_pX_long u;
   GiantRefine(u, f, k, l, verbose);
   BabyRefine(factors, u, k, l, verbose);
}

NTL_END_IMPL

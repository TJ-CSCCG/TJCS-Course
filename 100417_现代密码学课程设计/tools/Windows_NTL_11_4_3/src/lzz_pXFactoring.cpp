
#include <NTL/lzz_pXFactoring.h>
#include <NTL/mat_lzz_p.h>
#include <NTL/FacVec.h>


NTL_START_IMPL



void SquareFreeDecomp(vec_pair_zz_pX_long& u, const zz_pX& ff)
{
   zz_pX f = ff;

   if (!IsOne(LeadCoeff(f)))
      LogicError("SquareFreeDecomp: bad args");

   zz_pX r, t, v, tmp1;
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
         p = long(zz_p::modulus());
         d = deg(r)/p;
         f.rep.SetLength(d+1);
         for (k = 0; k <= d; k++) 
            f.rep[k] = r.rep[k*p];
         m = m*p;
      }
   } while (!finished);
}
         


static
void NullSpace(long& r, vec_long& D, vec_vec_zz_p& M, long verbose)
{
   long k, l, n;
   long i, j;
   long pos;
   zz_p t1, t2;
   zz_p *x, *y;

   n = M.length();

   D.SetLength(n);
   for (j = 0; j < n; j++) D[j] = -1;

   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();
   long T1, T2;
   mulmod_precon_t T1pinv;

   r = 0;

   l = 0;
   for (k = 0; k < n; k++) {

      if (verbose && k % 10 == 0) cerr << "+";

      pos = -1;
      for (i = l; i < n; i++) {
         if (!IsZero(M[i][k])) {
            pos = i;
            break;
         }
      }

      if (pos != -1) {
         swap(M[pos], M[l]);

         // make M[l, k] == -1 mod p

         inv(t1, M[l][k]);
         negate(t1, t1);
         for (j = k+1; j < n; j++) {
            mul(M[l][j], M[l][j], t1);
         }

         for (i = l+1; i < n; i++) {
            // M[i] = M[i] + M[l]*M[i,k]

            t1 = M[i][k];

            T1 = rep(t1);
            T1pinv = PrepMulModPrecon(T1, p, pinv); 

            x = M[i].elts() + (k+1);
            y = M[l].elts() + (k+1);

            for (j = k+1; j < n; j++, x++, y++) {
               // *x = *x + (*y)*t1

               T2 = MulModPrecon(rep(*y), T1, p, T1pinv);
               T2 = AddMod(T2, rep(*x), p);
               (*x).LoopHole() = T2;
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
void BuildMatrix(mat_zz_p& M, 
                 long n, const zz_pX& g, const zz_pXModulus& F, long verbose)
{
   zz_pXMultiplier G;
   zz_pX h;

   M.SetDims(n, n);

   build(G, g, F);

   set(h);
   for (long i = 0; i < n; i++) {
      if (verbose && i % 10 == 0) cerr << "+";

      VectorCopy(M[i], h, n);

      if (i < n-1)
         MulMod(h, h, G, F);
   }

   for (long i = 0; i < n; i++)
      add(M[i][i], M[i][i], -1);

}



static
void RecFindRoots(vec_zz_p& x, const zz_pX& f)
{
   if (deg(f) == 0) return;

   if (deg(f) == 1) {
      long k = x.length();
      x.SetLength(k+1);
      negate(x[k], ConstTerm(f));
      return;
   }
      
   zz_pX h;

   zz_p r;

   long p1 = zz_p::modulus() >> 1;

   {
      zz_pXModulus F;
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

void FindRoots(vec_zz_p& x, const zz_pX& ff)
{
   zz_pX f = ff;

   x.SetMaxLength(deg(f));
   x.SetLength(0);
   RecFindRoots(x, f);
}


static
void RandomBasisElt(zz_pX& g, mat_zz_p& ker)
{
   long r = ker.NumRows();
   long n = ker.NumCols();

   vec_zz_p v;
   v.SetLength(r);
   for (long i = 0; i < r; i++) random(v[i]);

   mul(g.rep, v, ker);
   g.normalize();
}



static
void split(zz_pX& f1, zz_pX& g1, zz_pX& f2, zz_pX& g2,
           const zz_pX& f, const zz_pX& g, 
           const vec_zz_p& roots, long lo, long mid)
{
   long r = mid-lo+1;

   zz_pXModulus F;
   build(F, f);

   vec_zz_p lroots(INIT_SIZE, r);
   long i;

   for (i = 0; i < r; i++)
      lroots[i] = roots[lo+i];


   zz_pX h, a, d;
   BuildFromRoots(h, lroots);
   CompMod(a, h, g, F);


   GCD(f1, a, f);
   
   div(f2, f, f1);

   rem(g1, g, f1);
   rem(g2, g, f2);
}

static
void RecFindFactors(vec_zz_pX& factors, const zz_pX& f, const zz_pX& g,
                    const vec_zz_p& roots, long lo, long hi)
{
   long r = hi-lo+1;

   if (r == 0) return;

   if (r == 1) {
      append(factors, f);
      return;
   }

   zz_pX f1, g1, f2, g2;

   long mid = (lo+hi)/2;

   split(f1, g1, f2, g2, f, g, roots, lo, mid);

   RecFindFactors(factors, f1, g1, roots, lo, mid);
   RecFindFactors(factors, f2, g2, roots, mid+1, hi);
}


static
void FindFactors(vec_zz_pX& factors, const zz_pX& f, const zz_pX& g,
                 const vec_zz_p& roots)
{
   long r = roots.length();

   factors.SetMaxLength(r);
   factors.SetLength(0);

   RecFindFactors(factors, f, g, roots, 0, r-1);
}

#if 0

static
void IterFindFactors(vec_zz_pX& factors, const zz_pX& f,
                     const zz_pX& g, const vec_zz_p& roots)
{
   long r = roots.length();
   long i;
   zz_pX h;

   factors.SetLength(r);

   for (i = 0; i < r; i++) {
      sub(h, g, roots[i]);
      GCD(factors[i], f, h);
   }
}

#endif

   

void SFBerlekamp(vec_zz_pX& factors, const zz_pX& ff, long verbose)
{
   zz_pX f = ff;

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

   long p;

   p = zz_p::modulus();

   long n = deg(f);

   zz_pXModulus F;

   build(F, f);

   zz_pX g, h;

   if (verbose) { cerr << "computing X^p..."; t = GetTime(); }
   PowerXMod(g, p, F);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   mat_zz_p M, ker;

   if (verbose) { cerr << "building matrix..."; t = GetTime(); }
   BuildMatrix(M, n, g, F, verbose);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   if (verbose) { cerr << "diagonalizing..."; t = GetTime(); }
   kernel(ker, M);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }


   M.kill();

   long r = ker.NumRows();

   if (verbose) cerr << "number of factors = " << r << "\n";

   if (r == 1) {
      factors.SetLength(1);
      factors[0] = f;
      return;
   }

   if (verbose) { cerr << "factor extraction..."; t = GetTime(); }

   vec_zz_p roots;

   RandomBasisElt(g, ker);
   MinPolyMod(h, g, F, r);
   FindRoots(roots, h);
   FindFactors(factors, f, g, roots);

   zz_pX g1;
   vec_zz_pX S, S1;
   long i;

   while (factors.length() < r) {
      if (verbose) cerr << "+";
      RandomBasisElt(g, ker);
      S.kill();
      for (i = 0; i < factors.length(); i++) {
         const zz_pX& f = factors[i];
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


void berlekamp(vec_pair_zz_pX_long& factors, const zz_pX& f, long verbose)
{
   double t;
   vec_pair_zz_pX_long sfd;
   vec_zz_pX x;

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
void AddFactor(vec_pair_zz_pX_long& factors, const zz_pX& g, long d, long verbose)
{
   if (verbose)
      cerr << "degree=" << d << ", number=" << deg(g)/d << "\n";
   append(factors, cons(g, d));
}

static
void ProcessTable(zz_pX& f, vec_pair_zz_pX_long& factors, 
                  const zz_pXModulus& F, long limit, const vec_zz_pX& tbl,
                  long d, long verbose)

{
   if (limit == 0) return;

   if (verbose) cerr << "+";

   zz_pX t1;

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

   zz_pX t2;

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


void TraceMap(zz_pX& w, const zz_pX& a, long d, const zz_pXModulus& F, 
              const zz_pX& b)

{
   if (d < 0) LogicError("TraceMap: bad args");

   zz_pX y, z, t;

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


void PowerCompose(zz_pX& y, const zz_pX& h, long q, const zz_pXModulus& F)
{
   if (q < 0) LogicError("PowerCompose: bad args");

   zz_pX z(INIT_SIZE, F.n);
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


long ProbIrredTest(const zz_pX& f, long iter)
{
   long n = deg(f);

   if (n <= 0) return 0;
   if (n == 1) return 1;

   long p;
   p = zz_p::modulus();

   zz_pXModulus F;

   build(F, f);

   zz_pX b, r, s;

   PowerXMod(b, p, F);

   long i;

   for (i = 0; i < iter; i++) {
      random(r, n);
      TraceMap(s, r, n, F, b);

      if (deg(s) > 0) return 0;
   }

   if (p >= n) return 1;

   if (n % p != 0) return 1;

   PowerCompose(s, b, n/p, F);
   return !IsX(s);
}

NTL_CHEAP_THREAD_LOCAL long zz_pX_BlockingFactor = 10;

void DDF(vec_pair_zz_pX_long& factors, const zz_pX& ff, const zz_pX& hh, 
         long verbose)
{
   zz_pX f = ff;
   zz_pX h = hh;

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

   long GCDTableSize = zz_pX_BlockingFactor;

   zz_pXModulus F;
   build(F, f);

   zz_pXNewArgument H;

   build(H, h, F, min(CompTableSize, deg(f)));

   long i, d, limit, old_n;
   zz_pX g, X;


   vec_zz_pX tbl(INIT_SIZE, GCDTableSize);

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



void RootEDF(vec_zz_pX& factors, const zz_pX& f, long verbose)
{
   vec_zz_p roots;
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
void EDFSplit(vec_zz_pX& v, const zz_pX& f, const zz_pX& b, long d)
{
   zz_pX a, g, h;
   zz_pXModulus F;
   vec_zz_p roots;
   
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
void RecEDF(vec_zz_pX& factors, const zz_pX& f, const zz_pX& b, long d,
            long verbose)
{
   vec_zz_pX v;
   long i;

   if (verbose) cerr << "+";

   EDFSplit(v, f, b, d);
   for (i = 0; i < v.length(); i++) {
      if (deg(v[i]) == d) {
         append(factors, v[i]);
      }
      else {
         zz_pX bb;
         rem(bb, b, v[i]);
         RecEDF(factors, v[i], bb, d, verbose);
      }
   }
}
         

void EDF(vec_zz_pX& factors, const zz_pX& ff, const zz_pX& bb,
         long d, long verbose)

{
   zz_pX f = ff;
   zz_pX b = bb;

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


void SFCanZass1(vec_pair_zz_pX_long& u, zz_pX& h, const zz_pX& f, long verbose)
{
   if (!IsOne(LeadCoeff(f)) || deg(f) == 0)
      LogicError("SFCanZass1: bad args");

   double t;

   long p = zz_p::modulus();

   
   zz_pXModulus F;
   build(F, f);


   if (verbose) { cerr << "computing X^p..."; t = GetTime(); }
   PowerXMod(h, p, F);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   if (verbose) { cerr << "computing DDF..."; t = GetTime(); }
   NewDDF(u, f, h, verbose);
   if (verbose) { 
      t = GetTime()-t; 
      cerr << "DDF time: " << t << "\n";
   }
}

void SFCanZass2(vec_zz_pX& factors, const vec_pair_zz_pX_long& u,
                const zz_pX& h, long verbose)
{
   zz_pX hh;
   vec_zz_pX v;

   factors.SetLength(0);

   long i;
   for (i = 0; i < u.length(); i++) {
      const zz_pX& g = u[i].a;
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


void SFCanZass(vec_zz_pX& factors, const zz_pX& ff, long verbose)
{
   zz_pX f = ff;

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

   long p = zz_p::modulus();

   
   zz_pXModulus F;
   build(F, f);

   zz_pX h;

   if (verbose) { cerr << "computing X^p..."; t = GetTime(); }
   PowerXMod(h, p, F);
   if (verbose) { cerr << (GetTime()-t) << "\n"; }

   vec_pair_zz_pX_long u;
   if (verbose) { cerr << "computing DDF..."; t = GetTime(); }
   NewDDF(u, f, h, verbose);
   if (verbose) { 
      t = GetTime()-t; 
      cerr << "DDF time: " << t << "\n";
   }

   zz_pX hh;
   vec_zz_pX v;

   long i;
   for (i = 0; i < u.length(); i++) {
      const zz_pX& g = u[i].a;
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
   
void CanZass(vec_pair_zz_pX_long& factors, const zz_pX& f, long verbose)
{
   if (!IsOne(LeadCoeff(f)))
      LogicError("CanZass: bad args");

   double t;
   vec_pair_zz_pX_long sfd;
   vec_zz_pX x;

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

void mul(zz_pX& f, const vec_pair_zz_pX_long& v)
{
   long i, j, n;

   n = 0;
   for (i = 0; i < v.length(); i++)
      n += v[i].b*deg(v[i].a);

   zz_pX g(INIT_SIZE, n+1);

   set(g);
   for (i = 0; i < v.length(); i++)
      for (j = 0; j < v[i].b; j++) {
         mul(g, g, v[i].a);
      }

   f = g;
}




static
long BaseCase(const zz_pX& h, long q, long a, const zz_pXModulus& F)
{
   long b, e;
   zz_pX lh(INIT_SIZE, F.n);

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



void TandemPowerCompose(zz_pX& y1, zz_pX& y2, const zz_pX& h, 
                        long q1, long q2, const zz_pXModulus& F)
{
   zz_pX z(INIT_SIZE, F.n);
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



long RecComputeDegree(long u, const zz_pX& h, const zz_pXModulus& F,
                      FacVec& fvec)
{
   if (IsX(h)) return 1;

   if (fvec[u].link == -1) return BaseCase(h, fvec[u].q, fvec[u].a, F);

   zz_pX h1, h2;
   long q1, q2, r1, r2;

   q1 = fvec[fvec[u].link].val; 
   q2 = fvec[fvec[u].link+1].val;

   TandemPowerCompose(h1, h2, h, q1, q2, F);
   r1 = RecComputeDegree(fvec[u].link, h2, F, fvec);
   r2 = RecComputeDegree(fvec[u].link+1, h1, F, fvec);
   return r1*r2;
}

   


long ComputeDegree(const zz_pX& h, const zz_pXModulus& F)
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

long ProbComputeDegree(const zz_pX& h, const zz_pXModulus& F)
{
   if (F.n == 1 || IsX(h))
      return 1;

   long n = F.n;

   zz_pX P1, P2, P3;

   random(P1, n);
   TraceMap(P2, P1, n, F, h);
   ProbMinPolyMod(P3, P2, F, n/2);

   long r = deg(P3);

   if (r <= 0 || n % r != 0)
      return 0;
   else
      return n/r;
}



void FindRoot(zz_p& root, const zz_pX& ff)
// finds a root of ff.
// assumes that ff is monic and splits into distinct linear factors

{
   zz_pXModulus F;
   zz_pX h, h1, f;
   zz_p r;
   long p1;


   f = ff;

   if (!IsOne(LeadCoeff(f)))
      LogicError("FindRoot: bad args");

   if (deg(f) == 0)
      LogicError("FindRoot: bad args");

   p1 = zz_p::modulus() >> 1;
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
long IrredBaseCase(const zz_pX& h, long q, long a, const zz_pXModulus& F)
{
   long e;
   zz_pX X, s, d;

   e = power(q, a-1);
   PowerCompose(s, h, e, F);
   SetX(X);
   sub(s, s, X);
   GCD(d, F.f, s);
   return IsOne(d);
}


static
long RecIrredTest(long u, const zz_pX& h, const zz_pXModulus& F,
                 const FacVec& fvec)
{
   long  q1, q2;
   zz_pX h1, h2;

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

long DetIrredTest(const zz_pX& f)
{
   if (deg(f) <= 0) return 0;
   if (deg(f) == 1) return 1;

   zz_pXModulus F;

   build(F, f);
   
   zz_pX h;

   PowerXMod(h, zz_p::modulus(), F);

   zz_pX s;
   PowerCompose(s, h, F.n, F);
   if (!IsX(s)) return 0;

   FacVec fvec;

   FactorInt(fvec, F.n);

   return RecIrredTest(fvec.length()-1, h, F, fvec);
}



long IterIrredTest(const zz_pX& f)
{
   if (deg(f) <= 0) return 0;
   if (deg(f) == 1) return 1;

   zz_pXModulus F;

   build(F, f);
   
   zz_pX h;

   PowerXMod(h, zz_p::modulus(), F);

   long rootn = SqrRoot(deg(f));

   long CompTableSize = 2*rootn;

   zz_pXNewArgument H;

   long UseModComp = 1;

   if (NumBits(zz_p::modulus()) < rootn/2)
      UseModComp = 0;

   if (UseModComp) build(H, h, F, CompTableSize);

   long i, d, limit, limit_sqr;
   zz_pX g, X, t, prod;


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
         if (UseModComp)
            CompMod(g, g, H, F);
         else
            PowerMod(g, g, zz_p::modulus(), F);
      }
   }

   if (i > 0) {
      GCD(t, f, prod);
      if (!IsOne(t)) return 0;
   }

   return 1;
}


static
void MulByXPlusY(vec_zz_pX& h, const zz_pX& f, const zz_pX& g)
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
      zz_pX b, t;

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
void IrredCombine(zz_pX& x, const zz_pX& f, const zz_pX& g)
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

   vec_zz_pX h(INIT_SIZE, dg);

   long i;
   for (i = 0; i < dg; i++) h[i].SetMaxLength(df);

   h.SetLength(1);
   set(h[0]);

   vec_zz_p a;

   a.SetLength(2*m);

   for (i = 0; i < 2*m; i++) {
      a[i] = ConstTerm(h[0]);
      if (i < 2*m-1)
         MulByXPlusY(h, f, g);
   }

   MinPolySeq(x, a, m);
}

static
void BuildPrimePowerIrred(zz_pX& f, long q, long e)
{
   long n = power(q, e);

   do {
      random(f, n);
      SetCoeff(f, n);
   } while (!IterIrredTest(f));
}

static
void RecBuildIrred(zz_pX& f, long u, const FacVec& fvec)
{
   if (fvec[u].link == -1)
      BuildPrimePowerIrred(f, fvec[u].q, fvec[u].a);
   else {
      zz_pX g, h;
      RecBuildIrred(g, fvec[u].link, fvec);
      RecBuildIrred(h, fvec[u].link+1, fvec);
      IrredCombine(f, g, h);
   }
}


void BuildIrred(zz_pX& f, long n)
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



void BuildRandomIrred(zz_pX& f, const zz_pX& g)
{
   zz_pXModulus G;
   zz_pX h, ff;

   build(G, g);
   do {
      random(h, deg(g));
      IrredPolyMod(ff, h, G);
   } while (deg(ff) < deg(g));

   f = ff;
}


/************* NEW DDF ****************/

NTL_CHEAP_THREAD_LOCAL long zz_pX_GCDTableSize = 4;
static NTL_CHEAP_THREAD_LOCAL vec_zz_pX *BabyStepFile = 0;
static NTL_CHEAP_THREAD_LOCAL vec_zz_pX *GiantStepFile = 0;
static NTL_CHEAP_THREAD_LOCAL zz_pXNewArgument *HHH = 0;
static NTL_CHEAP_THREAD_LOCAL long OldN = 0;


static
void GenerateBabySteps(zz_pX& h1, const zz_pX& f, const zz_pX& h, long k,
                       long verbose)
{
   double t;

   if (verbose) { cerr << "generating baby steps..."; t = GetTime(); }

   zz_pXModulus F;
   build(F, f);


   (*BabyStepFile).SetLength(k-1);

   h1 = h;

   long i;

   long rootn = SqrRoot(F.n);

   if (NumBits(zz_p::modulus()) < rootn/2) {
      for (i = 1; i <= k-1; i++) {
         (*BabyStepFile)(i) = h1;

         PowerMod(h1, h1, zz_p::modulus(), F);
         if (verbose) cerr << "+";
      }
   }
   else {
      zz_pXNewArgument H;
      build(H, h, F, 2*rootn);

      for (i = 1; i <= k-1; i++) {
         (*BabyStepFile)(i) = h1; 
   
         CompMod(h1, h1, H, F);
         if (verbose) cerr << ".";
      }
   }
   
   if (verbose)
      cerr << (GetTime()-t) << "\n";
}



static
void GenerateGiantSteps(const zz_pX& f, const zz_pX& h, long l, long verbose)
{
   zz_pXModulus F;

   build(F, f);

   build(*HHH, h, F, 2*SqrRoot(F.n));

   OldN = F.n;

   (*GiantStepFile).SetLength(1);
   (*GiantStepFile)(1) = h;
}


static
void NewAddFactor(vec_pair_zz_pX_long& u, const zz_pX& g, long m, long verbose)
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
void NewProcessTable(vec_pair_zz_pX_long& u, zz_pX& f, const zz_pXModulus& F,
                     vec_zz_pX& buf, long size, long StartInterval,
                     long IntervalLength, long verbose)

{
   if (size == 0) return;

   zz_pX& g = buf[size-1];

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
void FetchGiantStep(zz_pX& g, long gs, const zz_pXModulus& F)
{
   long l = (*GiantStepFile).length();
   zz_pX last;

   if (gs > l+1)
      LogicError("bad arg to FetchGiantStep");

   if (gs == l+1) {
      last = (*GiantStepFile)(l);
      if (F.n < OldN) {
         rem(last, last, F);
         reduce(*HHH, F);
         OldN = F.n;
      }

      (*GiantStepFile).SetLength(l+1);
      CompMod((*GiantStepFile)(l+1), last, *HHH, F);
      g = (*GiantStepFile)(l+1);
   }
   else if (deg((*GiantStepFile)(gs)) >= F.n)
      rem(g, (*GiantStepFile)(gs), F);
   else
      g = (*GiantStepFile)(gs);
}


static
void FetchBabySteps(vec_zz_pX& v, long k)
{
   v.SetLength(k);

   SetX(v[0]);

   long i;
   for (i = 1; i <= k-1; i++) {
      v[i] = (*BabyStepFile)(i);
   }
}
      


static
void GiantRefine(vec_pair_zz_pX_long& u, const zz_pX& ff, long k, long l,
                 long verbose)

{
   double t;

   if (verbose) {
      cerr << "giant refine...";
      t = GetTime();
   }

   u.SetLength(0);

   vec_zz_pX BabyStep;

   FetchBabySteps(BabyStep, k);

   vec_zz_pX buf(INIT_SIZE, zz_pX_GCDTableSize);

   zz_pX f;
   f = ff;

   zz_pXModulus F;
   build(F, f);

   zz_pX g;
   zz_pX h;

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

      if (size == zz_pX_GCDTableSize && bs == 0) {
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
void IntervalRefine(vec_pair_zz_pX_long& factors, const zz_pX& ff,
                    long k, long gs, const vec_zz_pX& BabyStep, long verbose)

{
   vec_zz_pX buf(INIT_SIZE, zz_pX_GCDTableSize);

   zz_pX f;
   f = ff;

   zz_pXModulus F;
   build(F, f);

   zz_pX g;

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

      if (size == zz_pX_GCDTableSize) {
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
void BabyRefine(vec_pair_zz_pX_long& factors, const vec_pair_zz_pX_long& u,
                long k, long l, long verbose)

{
   double t;

   if (verbose) {
      cerr << "baby refine...";
      t = GetTime();
   }

   factors.SetLength(0);

   vec_zz_pX BabyStep;

   long i;
   for (i = 0; i < u.length(); i++) {
      const zz_pX& g = u[i].a;
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

      

      

void NewDDF(vec_pair_zz_pX_long& factors,
            const zz_pX& f,
            const zz_pX& h,
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

   // we double the number of baby steps if it seems like
   // baby steps are significantly cheaper than giant steps.
   // The calculations below are closely tied to a test in GenerateBabySteps:
   // if nbm >= sdf/2, then scale should be 1 (baby steps and giant steps balanced)
   if (B >= 500) {
      long sdf = SqrRoot(deg(f));
      long nbm = NumBits(zz_p::modulus());
      double scale = 0.25*double(sdf)/double(nbm);
      if (scale < 1) scale = 1;
      if (scale > 2) scale = 2;
      k = long(scale*k);
   }

   long l = (B+k-1)/k;

   vec_zz_pX local_BabyStepFile;
   vec_zz_pX local_GiantStepFile;
   zz_pXNewArgument local_HHH;

   BabyStepFile = &local_BabyStepFile;
   GiantStepFile = &local_GiantStepFile;
   HHH = &local_HHH;
   
   zz_pX h1;
   GenerateBabySteps(h1, f, h, k, verbose);
   GenerateGiantSteps(f, h1, l, verbose);

   vec_pair_zz_pX_long u;
   GiantRefine(u, f, k, l, verbose);
   BabyRefine(factors, u, k, l, verbose);
}

NTL_END_IMPL

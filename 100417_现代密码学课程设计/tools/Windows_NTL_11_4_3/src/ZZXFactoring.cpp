
#include <NTL/ZZXFactoring.h>
#include <NTL/lzz_pXFactoring.h>
#include <NTL/vec_vec_long.h>
#include <NTL/vec_vec_ulong.h>
#include <NTL/vec_double.h>

#include <NTL/LLL.h>


NTL_START_IMPL

NTL_CHEAP_THREAD_LOCAL long ZZXFac_van_Hoeij = 1;

static NTL_CHEAP_THREAD_LOCAL long ok_to_abandon = 0;

struct LocalInfoT {
   long n;
   long NumPrimes;
   long NumFactors;
   vec_long p;
   vec_vec_long pattern;
   ZZ PossibleDegrees;
   PrimeSeq s;
};



static
void mul(ZZ_pX& x, vec_ZZ_pX& a)
// this performs multiplications in close-to-optimal order,
// and kills a in the process
{
   long n = a.length();

   // first, deal with some trivial cases

   if (n == 0) {
      set(x);
      a.kill();
      return;
   }
   else if (n == 1) {
      x = a[0];
      a.kill();
      return;
   }

   long i, j;

   // assume n > 1 and all a[i]'s are nonzero

   // sort into non-increasing degrees

   for (i = 1; i <= n - 1; i++)
      for (j = 0; j <= n - i - 1; j++)
         if (deg(a[j]) < deg(a[j+1]))
            swap(a[j], a[j+1]);

   ZZ_pX g;

   while (n > 1) {
      // replace smallest two poly's by their product
      mul(g, a[n-2], a[n-1]);
      a[n-2].kill();
      a[n-1].kill();
      swap(g, a[n-2]);
      n--;

      // re-establish order

      i = n-1;
      while (i > 0 && deg(a[i-1]) < deg(a[i])) {
         swap(a[i-1], a[i]);
         i--;
      }
   }

   x = a[0];

   a[0].kill();
   a.SetLength(0);
}


void mul(ZZX& x, const vec_pair_ZZX_long& a)
{
   long l = a.length();
   ZZX res;
   long i, j;

   set(res);
   for (i = 0; i < l; i++)
      for (j = 0; j < a[i].b; j++)
         mul(res, res, a[i].a);

   x = res;
}


void SquareFreeDecomp(vec_pair_ZZX_long& u, const ZZX& ff)
// input is primitive 
{
   ZZX f = ff;

   ZZX d, v, w, s, t1;
   long i;

   u.SetLength(0);

   if (deg(f) <= 0)
      return;

   diff(t1, f);
   GCD(d, f, t1);

   if (deg(d) == 0) {
      append(u, cons(f, 1L));
      return;
   }

   divide(v, f, d); 
   divide(w, t1, d);
   i = 0;

   for (;;) {
      i = i + 1;

      diff(t1, v);
      sub(s, w, t1);

      if (IsZero(s)) {
         if (deg(v) != 0) append(u, cons(v, i));
         return;
      }

      GCD(d, v, s);
      divide(v, v, d);
      divide(w, s, d);

      if (deg(d) != 0) append(u, cons(d, i));
   }
}




static
void HenselLift(ZZX& Gout, ZZX& Hout, ZZX& Aout, ZZX& Bout,
                const ZZX& f, const ZZX& g, const ZZX& h,
                const ZZX& a, const ZZX& b, const ZZ& p) 
{
   ZZX c, g1, h1, G, H, A, B;

   mul(c, g, h);
   sub(c, f, c);

   if (!divide(c, c, p))
      LogicError("inexact division");

   ZZ_pX cc, gg, hh, aa, bb, tt, gg1, hh1;

   conv(cc, c);
   conv(gg, g);
   conv(hh, h);
   conv(aa, a);
   conv(bb, b);

   ZZ_pXModulus GG;
   ZZ_pXModulus HH;

   build(GG, gg);
   build(HH, hh);

   ZZ_pXMultiplier AA;
   ZZ_pXMultiplier BB;

   build(AA, aa, HH);
   build(BB, bb, GG);

   rem(gg1, cc, GG);
   MulMod(gg1, gg1, BB, GG);
   
   rem(hh1, cc, HH);
   MulMod(hh1, hh1, AA, HH);

   conv(g1, gg1);
   mul(g1, g1, p);
   add(G, g, g1);

   conv(h1, hh1);
   mul(h1, h1, p);
   add(H, h, h1);

   /* lift inverses */

   ZZX t1, t2, r;

   mul(t1, a, G);
   mul(t2, b, H);
   add(t1, t1, t2);
   add(t1, t1, -1);
   negate(t1, t1);

   if (!divide(r, t1, p))
      LogicError("inexact division");

   ZZ_pX rr, aa1, bb1;

   conv(rr, r);
   
   rem(aa1, rr, HH);
   MulMod(aa1, aa1, AA, HH);
   rem(bb1, rr, GG);
   MulMod(bb1, bb1, BB, GG);

   ZZX a1, b1;

   conv(a1, aa1);
   mul(a1, a1, p);
   add(A, a, a1);

   conv(b1, bb1);
   mul(b1, b1, p);
   add(B, b, b1);

   Gout = G;
   Hout = H;
   Aout = A;
   Bout = B;
}

static
void HenselLift1(ZZX& Gout, ZZX& Hout, 
                const ZZX& f, const ZZX& g, const ZZX& h,
                const ZZX& a, const ZZX& b, const ZZ& p) 
{
   ZZX c, g1, h1, G, H;

   mul(c, g, h);
   sub(c, f, c);

   if (!divide(c, c, p))
      LogicError("inexact division");

   ZZ_pX cc, gg, hh, aa, bb, tt, gg1, hh1;

   conv(cc, c);
   conv(gg, g);
   conv(hh, h);
   conv(aa, a);
   conv(bb, b);

   ZZ_pXModulus GG;
   ZZ_pXModulus HH;

   build(GG, gg);
   build(HH, hh);

   rem(gg1, cc, GG);
   MulMod(gg1, gg1, bb, GG);
   
   rem(hh1, cc, HH);
   MulMod(hh1, hh1, aa, HH);

   conv(g1, gg1);
   mul(g1, g1, p);
   add(G, g, g1);

   conv(h1, hh1);
   mul(h1, h1, p);
   add(H, h, h1);

   Gout = G;
   Hout = H;
}

static
void BuildTree(vec_long& link, vec_ZZX& v, vec_ZZX& w,
               const vec_zz_pX& a)
{
   long k = a.length();

   if (k < 2) LogicError("bad arguments to BuildTree");

   vec_zz_pX V, W;

   V.SetLength(2*k-2);
   W.SetLength(2*k-2);
   link.SetLength(2*k-2);

   long i, j, s;
   long minp, mind;

   for (i = 0; i < k; i++) {
      V[i] = a[i];
      link[i] = -(i+1);
   }

   for (j = 0; j < 2*k-4; j += 2) {
      minp = j;
      mind = deg(V[j]);

      for (s = j+1; s < i; s++)
         if (deg(V[s]) < mind) {
            minp = s;
            mind = deg(V[s]);
         }

      swap(V[j], V[minp]);
      swap(link[j], link[minp]);

      minp = j+1;
      mind = deg(V[j+1]);

      for (s = j+2; s < i; s++)
         if (deg(V[s]) < mind) {
            minp = s;
            mind = deg(V[s]);
         }

      swap(V[j+1], V[minp]);
      swap(link[j+1], link[minp]);

      mul(V[i], V[j], V[j+1]);
      link[i] = j;
      i++;
   }

   zz_pX d;

   for (j = 0; j < 2*k-2; j += 2) {
      XGCD(d, W[j], W[j+1], V[j], V[j+1]);
      if (!IsOne(d))
         LogicError("relatively prime polynomials expected");
   }

   v.SetLength(2*k-2);
   for (j = 0; j < 2*k-2; j++)
      conv(v[j], V[j]);

   w.SetLength(2*k-2);
   for (j = 0; j < 2*k-2; j++)
      conv(w[j], W[j]);
}

static
void RecTreeLift(const vec_long& link, vec_ZZX& v, vec_ZZX& w,
                 const ZZ& p, const ZZX& f, long j, long inv)
{
   if (j < 0) return;

   if (inv)
      HenselLift(v[j], v[j+1], w[j], w[j+1],
                 f, v[j], v[j+1], w[j], w[j+1], p);
   else
      HenselLift1(v[j], v[j+1], f, v[j], v[j+1], w[j], w[j+1], p);

   RecTreeLift(link, v, w, p, v[j], link[j], inv);
   RecTreeLift(link, v, w, p, v[j+1], link[j+1], inv);
}

static
void TreeLift(const vec_long& link, vec_ZZX& v, vec_ZZX& w, 
              long e0, long e1, const ZZX& f, long inv)

// lift from p^{e0} to p^{e1}

{
   ZZ p0, p1;

   power(p0, zz_p::modulus(), e0);
   power(p1, zz_p::modulus(), e1-e0);

   ZZ_pBak bak;
   bak.save();
   ZZ_p::init(p1);

   RecTreeLift(link, v, w, p0, f, v.length()-2, inv);

   bak.restore();
} 

void MultiLift(vec_ZZX& A, const vec_zz_pX& a, const ZZX& f, long e,
               long verbose)

{
   long k = a.length();
   long i;

   if (k < 2 || e < 1 || NTL_OVERFLOW(e, 1, 0)) LogicError("MultiLift: bad args");

   if (!IsOne(LeadCoeff(f)))
      LogicError("MultiLift: bad args");

   for (i = 0; i < a.length(); i++)
      if (!IsOne(LeadCoeff(a[i])))
         LogicError("MultiLift: bad args");

   if (e == 1) {
      A.SetLength(k);
      for (i = 0; i < k; i++)
         conv(A[i], a[i]);
      return;
   }

   vec_long E;
   append(E, e);
   while (e > 1) {
      e = (e+1)/2;
      append(E, e);
   }
   long l = E.length();

   vec_ZZX v, w;
   vec_long link;

   double t;

   if (verbose) {
      cerr << "building tree...";
      t = GetTime();
   }

   BuildTree(link, v, w, a);

   if (verbose) cerr << (GetTime()-t) << "\n";


   for (i = l-1; i > 0; i--) {
      if (verbose) {
         cerr << "lifting to " << E[i-1] << "...";
         t = GetTime();
      }
      
      TreeLift(link, v, w, E[i], E[i-1], f, i != 1);

      if (verbose) cerr << (GetTime()-t) << "\n";
   }

   A.SetLength(k);
   for (i = 0; i < 2*k-2; i++) {
      long t = link[i];
      if (t < 0)
         A[-(t+1)] = v[i];
   }
}

static
void inplace_rev(ZZX& f)
{
   long n = deg(f);
   long i, j;

   i = 0;
   j = n;
   while (i < j) {
      swap(f.rep[i], f.rep[j]);
      i++;
      j--;
   }

   f.normalize();
}

NTL_CHEAP_THREAD_LOCAL long ZZXFac_InitNumPrimes = 7;
NTL_CHEAP_THREAD_LOCAL long ZZXFac_MaxNumPrimes = 50;

static 
void RecordPattern(vec_long& pat, vec_pair_zz_pX_long& fac)
{
   long n = pat.length()-1;
   long i;

   for (i = 0; i <= n; i++)
      pat[i] = 0;

   long k = fac.length();

   for (i = 0; i < k; i++) {
      long d = fac[i].b;
      long m = deg(fac[i].a)/d;

      pat[d] = m;
   }
}

static
long NumFactors(const vec_long& pat)
{
   long n = pat.length()-1;

   long i;
   long res = 0;

   for (i = 0; i <= n; i++) 
      res += pat[i];

   return res;
}

static
void CalcPossibleDegrees(ZZ& pd, const vec_long& pat)
{
   long n = pat.length()-1;
   set(pd);

   long d, j;
   ZZ t1;

   for (d = 1; d <= n; d++) 
      for (j = 0; j < pat[d]; j++) {
         LeftShift(t1, pd, d);
         bit_or(pd, pd, t1);
      }
}

static 
void CalcPossibleDegrees(vec_ZZ& S, const vec_ZZ_pX& fac, long k)

// S[i] = possible degrees of the product of any subset of size k
//        among fac[i...], encoded as a bit vector.      

{
   long r = fac.length();

   S.SetLength(r);

   if (r == 0)
      return;

   if (k < 1 || k > r)
      LogicError("CalcPossibleDegrees: bad args");

   long i, l;
   ZZ old, t1;

   set(S[r-1]);
   LeftShift(S[r-1], S[r-1], deg(fac[r-1]));

   for (i = r-2; i >= 0; i--) {
      set(t1);
      LeftShift(t1, t1, deg(fac[i]));
      bit_or(S[i], t1, S[i+1]);
   }

   for (l = 2; l <= k; l++) {
      old = S[r-l];
      LeftShift(S[r-l], S[r-l+1], deg(fac[r-l]));

      for (i = r-l-1; i >= 0; i--) {
         LeftShift(t1, old, deg(fac[i]));
         old = S[i];
         bit_or(S[i], S[i+1], t1);
      }
   }
}



static
vec_zz_pX *
SmallPrimeFactorization(LocalInfoT& LocalInfo, const ZZX& f,
                            long verbose)

{
   long n = deg(f);
   long i;
   double t;

   LocalInfo.n = n;
   long& NumPrimes = LocalInfo.NumPrimes;
   NumPrimes = 0;

   LocalInfo.NumFactors = 0;

   // some sanity checking...

   if (ZZXFac_InitNumPrimes < 1 || ZZXFac_InitNumPrimes > 10000)
      LogicError("bad ZZXFac_InitNumPrimes");

   if (ZZXFac_MaxNumPrimes < ZZXFac_InitNumPrimes || ZZXFac_MaxNumPrimes > 10000)
      LogicError("bad ZZXFac_MaxNumPrimes");

   LocalInfo.p.SetLength(ZZXFac_InitNumPrimes);
   LocalInfo.pattern.SetLength(ZZXFac_InitNumPrimes);
  
   // set bits 0..n of LocalInfo.PossibleDegrees 
   SetBit(LocalInfo.PossibleDegrees, n+1);
   add(LocalInfo.PossibleDegrees, LocalInfo.PossibleDegrees, -1);

   long minr = n+1;
   long irred = 0;

   UniquePtr<vec_pair_zz_pX_long> bestfac;
   UniquePtr<zz_pX> besth;
   UniquePtr<vec_zz_pX> spfactors;
   zz_pContext bestp;
   long bestp_index;

   long maxroot = NextPowerOfTwo(deg(f))+1;

   for (; NumPrimes < ZZXFac_InitNumPrimes;) {
      long p = LocalInfo.s.next();
      if (!p) ResourceError("out of small primes");
      if (divide(LeadCoeff(f), p)) {
         if (verbose) cerr << "skipping " << p << "\n";
         continue;
      }
      zz_p::init(p, maxroot);

      zz_pX ff, ffp, d;

      conv(ff, f);
      MakeMonic(ff);
      diff(ffp, ff);

      GCD(d, ffp, ff);
      if (!IsOne(d)) {
         if (verbose)  cerr << "skipping " << p << "\n";
         continue;
      }


      if (verbose) {
         cerr << "factoring mod " << p << "...";
         t = GetTime();
      }

      vec_pair_zz_pX_long thisfac;
      zz_pX thish; 

      SFCanZass1(thisfac, thish, ff, 0);

      LocalInfo.p[NumPrimes] = p;

      vec_long& pattern = LocalInfo.pattern[NumPrimes];
      pattern.SetLength(n+1);

      RecordPattern(pattern, thisfac);
      long r = NumFactors(pattern);
      
      if (verbose) {
         cerr << (GetTime()-t) << "\n";
         cerr << "degree sequence: ";
         for (i = 0; i <= n; i++)
            if (pattern[i]) {
               cerr << pattern[i] << "*" << i << " ";
            }
         cerr << "\n";
      }

      if (r == 1) {
         irred = 1;
         break;
      }

      // update admissibility info

      ZZ pd;

      CalcPossibleDegrees(pd, pattern);
      bit_and(LocalInfo.PossibleDegrees, LocalInfo.PossibleDegrees, pd);

      if (weight(LocalInfo.PossibleDegrees) == 2) {
         irred = 1;
         break;
      }


      if (r < minr) {
         minr = r;
         bestfac.make(thisfac);
         besth.make(thish);
         bestp.save();
         bestp_index = NumPrimes;
      }

      NumPrimes++;
   }

   if (!irred) {
      // remove best prime from LocalInfo
      swap(LocalInfo.pattern[bestp_index], LocalInfo.pattern[NumPrimes-1]);
      LocalInfo.p[bestp_index] = LocalInfo.p[NumPrimes-1];
      NumPrimes--;

      bestp.restore();

      spfactors.make();

      if (verbose) {
         cerr << "p = " << zz_p::modulus() << ", completing factorization...";
         t = GetTime();
      }
      SFCanZass2(*spfactors, *bestfac, *besth, 0);
      if (verbose) {
         cerr << (GetTime()-t) << "\n";
      }
   }

   return spfactors.release();
}


static
long ConstTermTest(const vec_ZZ_pX& W, 
                  const vec_long& I,
                  const ZZ& ct,
                  const ZZ_p& lc,
                  vec_ZZ_p& prod,
                  long& ProdLen) 
{
   long k = I.length();
   ZZ_p t;
   ZZ t1, t2;
   long i;

   if (ProdLen == 0) {
      mul(prod[0], lc, ConstTerm(W[I[0]]));
      ProdLen++;
   }

   for (i = ProdLen; i < k; i++)
      mul(prod[i], prod[i-1], ConstTerm(W[I[i]]));

   ProdLen = k-1;

   // should make this a routine in ZZ_p
   t1 = rep(prod[k-1]);
   RightShift(t2, ZZ_p::modulus(), 1);
   if (t1 > t2)
      sub(t1, t1, ZZ_p::modulus());

   return divide(ct, t1);
}

static
void BalCopy(ZZX& g, const ZZ_pX& G)
{
   const ZZ& p = ZZ_p::modulus();
   ZZ p2, t;
   RightShift(p2, p, 1);

   long n = G.rep.length();
   long i;

   g.rep.SetLength(n);
   for (i = 0; i < n; i++) {
      t = rep(G.rep[i]);
      if (t > p2) sub(t, t, p);
      g.rep[i] = t;
   }
}




static
void mul(ZZ_pX& g, const vec_ZZ_pX& W, const vec_long& I)
{
   vec_ZZ_pX w;
   long k = I.length();
   w.SetLength(k);
   long i;

   for (i = 0; i < k; i++)
      w[i] = W[I[i]];

   mul(g, w);
}




static
void InvMul(ZZ_pX& g, const vec_ZZ_pX& W, const vec_long& I)
{
   vec_ZZ_pX w;
   long k = I.length();
   long r = W.length();
   w.SetLength(r-k);
   long i, j;

   i = 0;
   for (j = 0; j < r; j++) {
      if (i < k && j == I[i])
         i++;
      else
         w[j-i] = W[j];
   } 

   mul(g, w);
}




static
void RemoveFactors(vec_ZZ_pX& W, const vec_long& I)
{
   long k = I.length();
   long r = W.length();
   long i, j;

   i = 0;
   for (j = 0; j < r; j++) {
      if (i < k && j == I[i])
         i++;
      else
         swap(W[j-i], W[j]); 
   }

   W.SetLength(r-k);
}

static
void unpack(vec_long& x, const ZZ& a, long n)
{
   x.SetLength(n+1);
   long i;

   for (i = 0; i <= n; i++)
      x[i] = bit(a, i);
}

static
void SubPattern(vec_long& p1, const vec_long& p2)
{
   long l = p1.length();

   if (p2.length() != l)
      LogicError("SubPattern: bad args");

   long i;

   for (i = 0; i < l; i++) {
      p1[i] -= p2[i];
      if (p1[i] < 0)
         LogicError("SubPattern: internal error");
   }
}

static
void UpdateLocalInfo(LocalInfoT& LocalInfo, vec_ZZ& pdeg,
                     const vec_ZZ_pX& W, const vec_ZZX& factors,
                     const ZZX& f, long k, long verbose)
{
   static NTL_CHEAP_THREAD_LOCAL long cnt = 0;

   if (verbose) {
      cnt = (cnt + 1) % 100;
      if (!cnt) cerr << "#";
   }

   double t;
   long i, j;

   if (LocalInfo.NumFactors < factors.length()) {
      zz_pBak bak;
      bak.save();

      vec_long pattern;
      pattern.SetLength(LocalInfo.n+1);

      ZZ pd;

      if (verbose) {
         cerr << "updating local info...";
         t = GetTime();
      }

      for (i = 0; i < LocalInfo.NumPrimes; i++) {
         zz_p::init(LocalInfo.p[i], NextPowerOfTwo(LocalInfo.n)+1);

         for (j = LocalInfo.NumFactors; j < factors.length(); j++) {
            vec_pair_zz_pX_long thisfac;
            zz_pX thish; 

            zz_pX ff;
            conv(ff, factors[j]);
            MakeMonic(ff);

            SFCanZass1(thisfac, thish, ff, 0);
            RecordPattern(pattern, thisfac);
            SubPattern(LocalInfo.pattern[i], pattern);
         }

         CalcPossibleDegrees(pd, LocalInfo.pattern[i]);
         bit_and(LocalInfo.PossibleDegrees, LocalInfo.PossibleDegrees, pd);

      }

      bak.restore();
      LocalInfo.NumFactors = factors.length();

      CalcPossibleDegrees(pdeg, W, k);

      if (verbose) cerr << (GetTime()-t) << "\n";
   }

   if (!ZZXFac_van_Hoeij && LocalInfo.NumPrimes + 1 < ZZXFac_MaxNumPrimes) {
      if (verbose)
         cerr << "adding a prime\n";

      zz_pBak bak;
      bak.save();

      for (;;) {
         long p = LocalInfo.s.next();
         if (!p)
            ResourceError("UpdateLocalInfo: out of primes");

         if (divide(LeadCoeff(f), p)) {
            if (verbose) cerr << "skipping " << p << "\n";
            continue;
         }

         zz_p::init(p, NextPowerOfTwo(deg(f))+1);

         zz_pX ff, ffp, d;
   
         conv(ff, f);
         MakeMonic(ff);
         diff(ffp, ff);
   
         GCD(d, ffp, ff);
         if (!IsOne(d)) {
            if (verbose)  cerr << "skipping " << p << "\n";
            continue;
         }

         vec_pair_zz_pX_long thisfac;
         zz_pX thish;

         if (verbose) {
            cerr << "factoring mod " << p << "...";
            t = GetTime();
         }

         SFCanZass1(thisfac, thish, ff, 0);

         LocalInfo.p.SetLength(LocalInfo.NumPrimes+1);
         LocalInfo.pattern.SetLength(LocalInfo.NumPrimes+1);

         LocalInfo.p[LocalInfo.NumPrimes] = p;
         vec_long& pattern = LocalInfo.pattern[LocalInfo.NumPrimes];

         pattern.SetLength(LocalInfo.n+1);
         RecordPattern(pattern, thisfac);

         if (verbose) {
            cerr << (GetTime()-t) << "\n";
            cerr << "degree sequence: ";
            for (i = 0; i <= LocalInfo.n; i++)
               if (pattern[i]) {
                  cerr << pattern[i] << "*" << i << " ";
               }
            cerr << "\n";
         }

         ZZ pd;
         CalcPossibleDegrees(pd, pattern);
         bit_and(LocalInfo.PossibleDegrees, LocalInfo.PossibleDegrees, pd);

         LocalInfo.NumPrimes++;

         break;
      }

      bak.restore();
   }
}



const int ZZX_OVERLIFT = NTL_BITS_PER_LONG;
  // number of bits by which we "overlift"....this enables, in particular,
  // the "n-1" test.  
  // Must lie in the range 4..NTL_BITS_PER_LONG.


#define EXTRA_BITS (1)
// Any small number, like 1, 2 or 3, should be OK.


static
void CardinalitySearch(vec_ZZX& factors, ZZX& f, 
                       vec_ZZ_pX& W, 
                       LocalInfoT& LocalInfo, 
                       long k,
                       long bnd,
                       long verbose)
{
   double start_time, end_time;

   if (verbose) {
      start_time = GetTime();
      cerr << "\n************ ";
      cerr << "start cardinality " << k << "\n";
   }

   vec_long I, D;
   I.SetLength(k);
   D.SetLength(k);

   long r = W.length();

   vec_ZZ_p prod;
   prod.SetLength(k);
   long ProdLen;

   vec_ZZ pdeg;
   CalcPossibleDegrees(pdeg, W, k);

   ZZ pd;
   vec_long upd;

   long i, state;

   long cnt = 0;

   ZZ ct;
   mul(ct, ConstTerm(f), LeadCoeff(f));

   ZZ_p lc;
   conv(lc, LeadCoeff(f));

   ZZ_pX gg;
   ZZX g, h;

   I[0] = 0;  

   while (I[0] <= r-k) {
      bit_and(pd, pdeg[I[0]], LocalInfo.PossibleDegrees);

      if (IsZero(pd)) {
         if (verbose) cerr << "skipping\n";
         goto done;
      }

      unpack(upd, pd, LocalInfo.n);

      D[0] = deg(W[I[0]]);
      i = 1;
      state = 0;
      ProdLen = 0;

      for (;;) {
         if (i < ProdLen)
            ProdLen = i;

         if (i == k) {
            // process indices I[0], ..., I[k-1]

            if (cnt > 2000000) { 
               cnt = 0;
               UpdateLocalInfo(LocalInfo, pdeg, W, factors, f, k, verbose);
               bit_and(pd, pdeg[I[0]], LocalInfo.PossibleDegrees);
               if (IsZero(pd)) {
                  if (verbose) cerr << "skipping\n";
                  goto done;
               }
               unpack(upd, pd, LocalInfo.n);
            }

            state = 1;  // default continuation state


            if (!upd[D[k-1]]) {
               i--;
               cnt++;
               continue;
            }

            if (!ConstTermTest(W, I, ct, lc, prod, ProdLen)) {
               i--;
               cnt += 100;
               continue;
            }

            if (verbose) {
               cerr << "+";
            }

            cnt += 1000;

            if (2*D[k-1] <= deg(f)) {
               mul(gg, W, I);
               mul(gg, gg, lc);
               BalCopy(g, gg);
               if(MaxBits(g) > bnd) {
                  i--;
                  continue;
               }
               if (verbose) {
                  cerr << "*";
               }
               PrimitivePart(g, g);
               if (!divide(h, f, g)) {
                  i--;
                  continue;
               }
               
               // factor found!
               append(factors, g);
               if (verbose) {
                 cerr << "degree " << deg(g) << " factor found\n";
               }
               f = h;
               mul(ct, ConstTerm(f), LeadCoeff(f));
               conv(lc, LeadCoeff(f));
            }
            else {
               InvMul(gg, W, I);
               mul(gg, gg, lc);
               BalCopy(g, gg);
               if(MaxBits(g) > bnd) {
                  i--;
                  continue;
               }
               if (verbose) {
                  cerr << "*";
               }
               PrimitivePart(g, g);
               if (!divide(h, f, g)) {
                  i--;
                  continue;
               }

               // factor found!
               append(factors, h);
               if (verbose) {
                 cerr << "degree " << deg(h) << " factor found\n";
               }
               f = g;
               mul(ct, ConstTerm(f), LeadCoeff(f));
               conv(lc, LeadCoeff(f));
            }

            RemoveFactors(W, I);
            r = W.length();
            cnt = 0;

            if (2*k > r) 
               goto done;
            else 
               break;
         }
         else if (state == 0) {
            I[i] = I[i-1] + 1;
            D[i] = D[i-1] + deg(W[I[i]]);
            i++;
         }
         else { // state == 1
            I[i]++;
            if (i == 0) break;

            if (I[i] > r-k+i)
               i--;
            else {
               D[i] = D[i-1] + deg(W[I[i]]);
               i++;
               state = 0;
            }
         }
      }
   }


   done: 


   if (verbose) {
      end_time = GetTime();
      cerr << "\n************ ";
      cerr << "end cardinality " << k << "\n";
      cerr << "time: " << (end_time-start_time) << "\n";
   }
}



typedef unsigned long TBL_T;

#if (NTL_BITS_PER_LONG >= 64)

// for 64-bit machines

#define TBL_MSK (63)
#define TBL_SHAMT (6)

#else

// for 32-bit machines

#define TBL_MSK (31)
#define TBL_SHAMT (5)

#endif


#if 0

// recursive version

static
void RecInitTab(TBL_T ***lookup_tab, long i, const vec_ulong& ratio, 
             long r, long k, unsigned long thresh1, long **shamt_tab,
             unsigned long sum, long card, long j)
{
   if (j >= i || card >= k-1) {
      if (card > 1) {
         long shamt = shamt_tab[i][card];
         unsigned long index1 = ((-sum) >> shamt);
         lookup_tab[i][card][index1 >> TBL_SHAMT] |= (1UL << (index1 & TBL_MSK));
         unsigned long index2 = ((-sum+thresh1) >> shamt);
         if (index1 != index2)
            lookup_tab[i][card][index2 >> TBL_SHAMT] |= (1UL << (index2 & TBL_MSK));

      }

      return;
   }


   RecInitTab(lookup_tab, i, ratio, r, k, thresh1, shamt_tab, sum, card, j+1);
   RecInitTab(lookup_tab, i, ratio, r, k, thresh1, shamt_tab, 
              sum+ratio[r-1-j], card+1, j+1);
}


static
void DoInitTab(TBL_T ***lookup_tab, long i, const vec_ulong& ratio, 
               long r, long k, unsigned long thresh1, long **shamt_tab)
{
   RecInitTab(lookup_tab, i, ratio, r, k, thresh1, shamt_tab, 0, 0, 0);
}

#else

// iterative version



typedef Vec< Vec< Vec< TBL_T > > > lookup_tab_t;
typedef Vec< Vec<long> > shamt_tab_t;


static
void DoInitTab(lookup_tab_t& lookup_tab, long i, const vec_ulong& ratio,
               long r, long k, unsigned long thresh1, shamt_tab_t& shamt_tab)
{
   vec_long sum_vec, card_vec, location_vec;
   sum_vec.SetLength(i+1);
   card_vec.SetLength(i+1);
   location_vec.SetLength(i+1);

   long j = 0;
   sum_vec[0] = 0;
   card_vec[0] = 0;

   unsigned long sum;
   long  card, location;

   location = 0;

   while (j >= 0) {
      sum = sum_vec[j];
      card = card_vec[j];

      switch (location) {

      case 0:

         if (j >= i || card >= k-1) {
            if (card > 1) {
               long shamt = shamt_tab[i][card];
               unsigned long index1 = ((-sum) >> shamt);
               lookup_tab[i][card][index1 >> TBL_SHAMT] |= (1UL << (index1 & TBL_MSK));
               unsigned long index2 = ((-sum+thresh1) >> shamt);
               if (index1 != index2)
                  lookup_tab[i][card][index2 >> TBL_SHAMT] |= (1UL << (index2 & TBL_MSK));
      
            }
      
            location = location_vec[j];
            j--;
            continue;
         }


         sum_vec[j+1] = sum;
         card_vec[j+1] = card;
         location_vec[j+1] = 1;
         j++;
         location = 0;
         continue;

      case 1:

         sum_vec[j+1] = sum+ratio[r-1-j];
         card_vec[j+1] = card+1;
         location_vec[j+1] = 2;
         j++;
         location = 0;
         continue;  

      case 2:

         location = location_vec[j];
         j--;
         continue;
      }
   }
}
         
#endif
   
   

static
void InitTab(lookup_tab_t& lookup_tab, const vec_ulong& ratio, long r, long k,
             unsigned long thresh1, shamt_tab_t& shamt_tab, long pruning)
{
   long i, j, t;

   if (pruning) {
      for (i = 2; i <= pruning; i++) {
         long len = min(k-1, i);
         for (j = 2; j <= len; j++) {
            long ub = (((1L << (NTL_BITS_PER_LONG-shamt_tab[i][j])) 
                      + TBL_MSK) >> TBL_SHAMT); 
            for (t = 0; t < ub; t++)
               lookup_tab[i][j][t] = 0;
         }
   
         DoInitTab(lookup_tab, i, ratio, r, k, thresh1, shamt_tab);
      }
   }
}


static
void RatioInit1(vec_ulong& ratio, const vec_ZZ_pX& W, const ZZ_p& lc,
                long pruning, lookup_tab_t& lookup_tab, 
                vec_vec_ulong& pair_ratio, long k, unsigned long thresh1, 
                shamt_tab_t& shamt_tab)
{
   long r = W.length();
   long i, j;

   ZZ_p a;

   ZZ p;
   p = ZZ_p::modulus();

   ZZ aa;

   for (i = 0; i < r; i++) {
      long m = deg(W[i]);
      mul(a, W[i].rep[m-1], lc);
      LeftShift(aa, rep(a), NTL_BITS_PER_LONG);
      div(aa, aa, p);
      ratio[i] = to_ulong(aa);
   }

   InitTab(lookup_tab, ratio, r, k, thresh1, shamt_tab, pruning);

   for (i = 0; i < r; i++)
      for (j = 0; j < i; j++) {
         mul(a, W[i].rep[deg(W[i])-1], W[j].rep[deg(W[j])-1]);
         mul(a, a, lc);
         LeftShift(aa, rep(a), NTL_BITS_PER_LONG);
         div(aa, aa, p);
         pair_ratio[i][j] = to_ulong(aa);
      }

   for (i = 0; i < r; i++) {
      long m = deg(W[i]);
      if (m >= 2) {
         mul(a, W[i].rep[m-2], lc);
         LeftShift(aa, rep(a), NTL_BITS_PER_LONG);
         div(aa, aa, p);
         pair_ratio[i][i] = to_ulong(aa);
      }
      else
         pair_ratio[i][i] = 0;
   }
}

static 
long SecondOrderTest(const vec_long& I_vec, const vec_vec_ulong& pair_ratio_vec,
                     vec_ulong& sum_stack_vec, long& SumLen)
{
   long k = I_vec.length();
   const long *I = I_vec.elts();
   unsigned long *sum_stack = sum_stack_vec.elts();

   unsigned long sum, thresh1;

   if (SumLen == 0) {
      unsigned long epsilon = (1UL << (NTL_BITS_PER_LONG-ZZX_OVERLIFT));
      unsigned long delta = (unsigned long) ((k*(k+1)) >> 1);
      unsigned long thresh = epsilon + delta;
      thresh1 = (epsilon << 1) + delta;

      sum = thresh;
      sum_stack[k] = thresh1;
   }
   else {
      sum = sum_stack[SumLen-1];
      thresh1 = sum_stack[k];
   }

   long i, j;

   for (i = SumLen; i < k; i++) {
      const unsigned long *p = pair_ratio_vec[I[i]].elts();
      for (j = 0; j <= i; j++) {
         sum += p[I[j]];
      }

      sum_stack[i] = sum;
   }

   SumLen = k-1;

   return (sum <= thresh1);
}


static
ZZ choose_fn(long r, long k)
{
   ZZ a, b;

   a = 1; 
   b = 1;

   long i;
   for (i = 0; i < k; i++) {
      a *= r-i;
      b *= k-i;
   }

   return a/b;
}

static
void PrintInfo(const char *s, const ZZ& a, const ZZ& b)
{
   cerr << s << a << " / " << b << " = ";
   
   double x = to_double(a)/to_double(b);

   if (x == 0) 
      cerr << "0"; 
   else {
      int n;
      double f;

      f = frexp(x, &n);
      cerr << f << "*2^" << n;
   }

   cerr << "\n";
}

static
void RemoveFactors1(vec_long& W, const vec_long& I, long r)
{
   long k = I.length();
   long i, j;

   i = 0;
   for (j = 0; j < r; j++) {
      if (i < k && j == I[i])
         i++;
      else
         swap(W[j-i], W[j]); 
   }
}

static
void RemoveFactors1(vec_vec_long& W, const vec_long& I, long r)
{
   long k = I.length();
   long i, j;

   i = 0;
   for (j = 0; j < r; j++) {
      if (i < k && j == I[i])
         i++;
      else
         swap(W[j-i], W[j]); 
   }

   for (i = 0; i < r-k; i++)
      RemoveFactors1(W[i], I, r);
}


static
void RemoveFactors1(vec_ulong& W, const vec_long& I, long r)
{
   long k = I.length();
   long i, j;

   i = 0;
   for (j = 0; j < r; j++) {
      if (i < k && j == I[i])
         i++;
      else
         _ntl_swap(W[j-i], W[j]); 
   }
}

static
void RemoveFactors1(vec_vec_ulong& W, const vec_long& I, long r)
{
   long k = I.length();
   long i, j;

   i = 0;
   for (j = 0; j < r; j++) {
      if (i < k && j == I[i])
         i++;
      else
         swap(W[j-i], W[j]); 
   }

   for (i = 0; i < r-k; i++)
      RemoveFactors1(W[i], I, r);
}


static
void RemoveFactors1(vec_ZZ_p& W, const vec_long& I, long r)
{
   long k = I.length();
   long i, j;

   i = 0;
   for (j = 0; j < r; j++) {
      if (i < k && j == I[i])
         i++;
      else
         swap(W[j-i], W[j]);
   }
}

static
void SumCoeffs(ZZ& sum, const ZZX& a)
{
   ZZ res;
   res = 0;
   long i;
   long n = a.rep.length();
   for (i = 0; i < n; i++)
      res += a.rep[i];

   sum = res;
}

static
void SumCoeffs(ZZ_p& sum, const ZZ_pX& a)
{
   ZZ_p res;
   res = 0;
   long i;
   long n = a.rep.length();
   for (i = 0; i < n; i++)
      res += a.rep[i];

   sum = res;
}


static
long ConstTermTest(const vec_ZZ_p& W, 
                  const vec_long& I,
                  const ZZ& ct,
                  const ZZ_p& lc,
                  vec_ZZ_p& prod,
                  long& ProdLen) 
{
   long k = I.length();
   ZZ_p t;
   ZZ t1, t2;
   long i;

   if (ProdLen == 0) {
      mul(prod[0], lc, W[I[0]]);
      ProdLen++;
   }

   for (i = ProdLen; i < k; i++)
      mul(prod[i], prod[i-1], W[I[i]]);

   ProdLen = k-1;

   // should make this a routine in ZZ_p
   t1 = rep(prod[k-1]);
   RightShift(t2, ZZ_p::modulus(), 1);
   if (t1 > t2)
      sub(t1, t1, ZZ_p::modulus());

   return divide(ct, t1);
}


NTL_CHEAP_THREAD_LOCAL long ZZXFac_MaxPrune = 10;



static
long pruning_bnd(long r, long k)
{
   double x = 0; 

   long i;
   for (i = 0; i < k; i++) {
      x += log(double(r-i)/double(k-i));
   }

   return long((x/log(2.0)) * 0.75);
}

static
long shamt_tab_init(long pos, long card, long pruning, long thresh1_len)
{
   double x = 1;
   long i;

   for (i = 0; i < card; i++) {
      x *= double(pos-i)/double(card-i);
   }

   x *= pruning;  // this can be adjusted to control the density
   if (pos <= 6) x *= 2;  // a little boost that costs very little
      

   long t = long(ceil(log(x)/log(2.0)));

   t = max(t, TBL_SHAMT); 

   t = min(t, NTL_BITS_PER_LONG-thresh1_len);


   return NTL_BITS_PER_LONG-t;
}

// The following routine should only be called for k > 1,
// and is only worth calling for k > 2.


static
void CardinalitySearch1(vec_ZZX& factors, ZZX& f, 
                       vec_ZZ_pX& W, 
                       LocalInfoT& LocalInfo, 
                       long k,
                       long bnd,
                       long verbose)
{
   double start_time, end_time;

   if (verbose) {
      start_time = GetTime();
      cerr << "\n************ ";
      cerr << "start cardinality " << k << "\n";
   }

   if (k <= 1) LogicError("internal error: call CardinalitySearch");

   // This test is needed to ensure correcntes of "n-2" test
   if (NumBits(k) > NTL_BITS_PER_LONG/2-2)
      ResourceError("Cardinality Search: k too large...");

   vec_ZZ pdeg;
   CalcPossibleDegrees(pdeg, W, k);
   ZZ pd;

   bit_and(pd, pdeg[0], LocalInfo.PossibleDegrees);
   if (pd == 0) {
      if (verbose) cerr << "skipping\n";
      return;
   }

   vec_long I, D;
   I.SetLength(k);
   D.SetLength(k);

   long r = W.length();

   long initial_r = r;

   vec_ulong ratio, ratio_sum;
   ratio.SetLength(r);
   ratio_sum.SetLength(k);

   unsigned long epsilon = (1UL << (NTL_BITS_PER_LONG-ZZX_OVERLIFT));
   unsigned long delta = (unsigned long) k;
   unsigned long thresh = epsilon + delta;
   unsigned long thresh1 = (epsilon << 1) + delta;

   long thresh1_len = NumBits(long(thresh1)); 

   long pruning;

   pruning = min(r/2, ZZXFac_MaxPrune);
   pruning = min(pruning, pruning_bnd(r, k));
   pruning = min(pruning, NTL_BITS_PER_LONG-EXTRA_BITS-thresh1_len);

   if (pruning <= 4) pruning = 0;

   long init_pruning = pruning;

   lookup_tab_t lookup_tab;

   shamt_tab_t shamt_tab;

   if (pruning) {

      long i, j;

      shamt_tab.SetLength(pruning+1);

      for (i = 2; i <= pruning; i++) {
         long len = min(k-1, i);
         shamt_tab[i].SetLength(len+1);
         shamt_tab[i][0] = shamt_tab[i][1] = 0;

         for (j = 2; j <= len; j++)
            shamt_tab[i][j] = shamt_tab_init(i, j, pruning, thresh1_len);
      }

      lookup_tab.SetLength(pruning+1);

      for (i = 2; i <= pruning; i++) {
         long len = min(k-1, i);
         lookup_tab[i].SetLength(len+1);

         for (j = 2; j <= len; j++) {
            lookup_tab[i][j].SetLength(((1L << (NTL_BITS_PER_LONG-shamt_tab[i][j]))+TBL_MSK) >> TBL_SHAMT);
         }
      }
   }

   if (verbose) {
      cerr << "pruning = " << pruning << "\n";
   }

   vec_ZZ_p prod;
   prod.SetLength(k);
   long ProdLen;

   vec_ZZ_p prod1;
   prod1.SetLength(k);
   long ProdLen1;

   vec_ulong sum_stack;
   sum_stack.SetLength(k+1);
   long SumLen;

   vec_long upd;

   long i, state;

   long cnt = 0;

   ZZ ct;
   mul(ct, ConstTerm(f), LeadCoeff(f));

   ZZ_p lc;
   conv(lc, LeadCoeff(f));

   vec_vec_ulong pair_ratio;
   pair_ratio.SetLength(r);
   for (i = 0; i < r; i++)
      pair_ratio[i].SetLength(r);

   RatioInit1(ratio, W, lc, pruning, lookup_tab, pair_ratio, k, thresh1, shamt_tab);

   ZZ c1;
   SumCoeffs(c1, f);
   mul(c1, c1, LeadCoeff(f));

   vec_ZZ_p sum_coeffs;
   sum_coeffs.SetLength(r);
   for (i = 0; i < r; i++)
      SumCoeffs(sum_coeffs[i], W[i]);

   vec_long degv;
   degv.SetLength(r);

   for (i = 0; i < r; i++)
      degv[i] = deg(W[i]);

   ZZ_pX gg;
   ZZX g, h;

   I[0] = 0;  

   long loop_cnt = 0, degree_cnt = 0, n2_cnt = 0, sl_cnt = 0, ct_cnt = 0, 
        pl_cnt = 0, c1_cnt = 0, pl1_cnt = 0, td_cnt = 0;

   ZZ loop_total, degree_total, n2_total, sl_total, ct_total, 
      pl_total, c1_total, pl1_total, td_total;

   while (I[0] <= r-k) {
      bit_and(pd, pdeg[I[0]], LocalInfo.PossibleDegrees);

      if (IsZero(pd)) {
         if (verbose) cerr << "skipping\n";
         goto done;
      }

      unpack(upd, pd, LocalInfo.n);

      D[0] = degv[I[0]];
      ratio_sum[0] = ratio[I[0]] + thresh;
      i = 1;
      state = 0;
      ProdLen = 0;
      ProdLen1 = 0;
      SumLen = 0;

      for (;;) {
         cnt++;

         if (cnt > 2000000) { 
            if (verbose) {
               loop_total += loop_cnt;  loop_cnt = 0;
               degree_total += degree_cnt;  degree_cnt = 0;
               n2_total += n2_cnt;  n2_cnt = 0;
               sl_total += sl_cnt;  sl_cnt = 0;
               ct_total += ct_cnt;  ct_cnt = 0;
               pl_total += pl_cnt;  pl_cnt = 0;
               c1_total += c1_cnt;  c1_cnt = 0;
               pl1_total += pl1_cnt;  pl1_cnt = 0;
               td_total += td_cnt;  td_cnt = 0;
            }

            cnt = 0;
            UpdateLocalInfo(LocalInfo, pdeg, W, factors, f, k, verbose);
            bit_and(pd, pdeg[I[0]], LocalInfo.PossibleDegrees);
            if (IsZero(pd)) {
               if (verbose) cerr << "skipping\n";
               goto done;
            }
            unpack(upd, pd, LocalInfo.n);
         }

         if (i == k-1) {

            unsigned long ratio_sum_last = ratio_sum[k-2];
            long I_last = I[k-2];


            {
               long D_last = D[k-2];
   
               unsigned long rs;
               long I_this;
               long D_this;
   
               for (I_this = I_last+1; I_this < r; I_this++) {
                  loop_cnt++;
   
                  rs = ratio_sum_last + ratio[I_this];
                  if (rs > thresh1) {
                     cnt++;
                     continue;
                  }

                  degree_cnt++;
   
                  D_this = D_last + degv[I_this];
   
                  if (!upd[D_this]) {
                     cnt++;
                     continue;
                  }
   
                  n2_cnt++;
                  sl_cnt += (k-SumLen);

                  I[k-1] = I_this;

                  if (!SecondOrderTest(I, pair_ratio, sum_stack, SumLen)) {
                     cnt += 2;
                     continue;
                  }

                  c1_cnt++;
                  pl1_cnt += (k-ProdLen1);

                  if (!ConstTermTest(sum_coeffs, I, c1, lc, prod1, ProdLen1)) {
                     cnt += 100;
                     continue;
                  }

                  ct_cnt++;
                  pl_cnt += (k-ProdLen);

                  D[k-1] = D_this;

                  if (!ConstTermTest(W, I, ct, lc, prod, ProdLen)) {
                     cnt += 100;
                     continue;
                  }

                  td_cnt++;
   
                  if (verbose) {
                     cerr << "+";
                  }
   
                  cnt += 1000;
   
                  if (2*D[k-1] <= deg(f)) {
                     mul(gg, W, I);
                     mul(gg, gg, lc);
                     BalCopy(g, gg);
                     if(MaxBits(g) > bnd) {
                        continue;
                     }
                     if (verbose) {
                        cerr << "*";
                     }
                     PrimitivePart(g, g);
                     if (!divide(h, f, g)) {
                        continue;
                     }
                  
                     // factor found!
                     append(factors, g);
                     if (verbose) {
                       cerr << "degree " << deg(g) << " factor found\n";
                     }
                     f = h;
                     mul(ct, ConstTerm(f), LeadCoeff(f));
                     conv(lc, LeadCoeff(f));
                  }
                  else {
                     InvMul(gg, W, I);
                     mul(gg, gg, lc);
                     BalCopy(g, gg);
                     if(MaxBits(g) > bnd) {
                        continue;
                     }
                     if (verbose) {
                        cerr << "*";
                     }
                     PrimitivePart(g, g);
                     if (!divide(h, f, g)) {
                        continue;
                     }
      
                     // factor found!
                     append(factors, h);
                     if (verbose) {
                       cerr << "degree " << deg(h) << " factor found\n";
                     }
                     f = g;
                     mul(ct, ConstTerm(f), LeadCoeff(f));
                     conv(lc, LeadCoeff(f));
                  }
      
                  RemoveFactors(W, I);
                  RemoveFactors1(degv, I, r);
                  RemoveFactors1(sum_coeffs, I, r);
                  RemoveFactors1(ratio, I, r);
                  RemoveFactors1(pair_ratio, I, r);

                  r = W.length();
                  cnt = 0;

                  pruning = min(pruning, r/2);
                  if (pruning <= 4) pruning = 0;

                  InitTab(lookup_tab, ratio, r, k, thresh1, shamt_tab, pruning);

                  if (2*k > r) 
                     goto done;
                  else 
                     goto restart;
               } /* end of inner for loop */ 

            }

            i--;
            state = 1;  
         }
         else {
            if (state == 0) {
               long I_i = I[i-1] + 1;
               I[i] = I_i;

               long pruned;

               if (pruning && r-I_i <= pruning) {
                  long pos = r-I_i;
                  unsigned long rs = ratio_sum[i-1];
                  unsigned long index1 = (rs >> shamt_tab[pos][k-i]);
                  if (lookup_tab[pos][k-i][index1 >> TBL_SHAMT] & (1UL << (index1&TBL_MSK)))
                     pruned = 0;
                  else
                     pruned = 1;
               }
               else
                  pruned = 0; 

               if (pruned) {
                  i--;
                  state = 1;
               }
               else {
                  D[i] = D[i-1] + degv[I_i];
                  ratio_sum[i] = ratio_sum[i-1] + ratio[I_i];
                  i++;
               }
            }
            else { // state == 1
      
               loop_cnt++;
      
               if (i < ProdLen)
                  ProdLen = i;
      
               if (i < ProdLen1)
                  ProdLen1 = i;
      
               if (i < SumLen)
                  SumLen = i;

               long I_i = (++I[i]);

               if (i == 0) break;
   
               if (I_i > r-k+i) {
                  i--;
               }
               else {

                  long pruned;

                  if (pruning && r-I_i <= pruning) {
                     long pos = r-I_i;
                     unsigned long rs = ratio_sum[i-1];
                     unsigned long index1 = (rs >> shamt_tab[pos][k-i]);
                     if (lookup_tab[pos][k-i][index1 >> TBL_SHAMT] & (1UL << (index1&TBL_MSK)))
                        pruned = 0;
                     else
                        pruned = 1;
                  }
                  else
                     pruned = 0; 
   

                  if (pruned) {
                     i--;
                  }
                  else {
                     D[i] = D[i-1] + degv[I_i];
                     ratio_sum[i] = ratio_sum[i-1] + ratio[I_i];
                     i++;
                     state = 0;
                  }
               }
            }
         }
      }

      restart: ;
   }

   done:

   if (verbose) { 
      end_time = GetTime();
      cerr << "\n************ ";
      cerr << "end cardinality " << k << "\n";
      cerr << "time: " << (end_time-start_time) << "\n";
      ZZ loops_max = choose_fn(initial_r+1, k);
      ZZ tuples_max = choose_fn(initial_r, k);

      loop_total += loop_cnt;
      degree_total += degree_cnt;
      n2_total += n2_cnt;
      sl_total += sl_cnt;
      ct_total += ct_cnt;
      pl_total += pl_cnt;
      c1_total += c1_cnt;
      pl1_total += pl1_cnt;
      td_total += td_cnt;

      cerr << "\n";
      PrintInfo("loops: ", loop_total, loops_max);
      PrintInfo("degree tests: ", degree_total, tuples_max);

      PrintInfo("n-2 tests: ", n2_total, tuples_max);

      cerr << "ave sum len: ";
      if (n2_total == 0) 
         cerr << "--";
      else
         cerr << (to_double(sl_total)/to_double(n2_total));
      cerr << "\n";

      PrintInfo("f(1) tests: ", c1_total, tuples_max);

      cerr << "ave prod len: ";
      if (c1_total == 0) 
         cerr << "--";
      else
         cerr << (to_double(pl1_total)/to_double(c1_total));
      cerr << "\n";

      PrintInfo("f(0) tests: ", ct_total, tuples_max);

      cerr << "ave prod len: ";
      if (ct_total == 0) 
         cerr << "--";
      else
         cerr << (to_double(pl_total)/to_double(ct_total));
      cerr << "\n";

      PrintInfo("trial divs: ", td_total, tuples_max);
   }
}



static
void FindTrueFactors(vec_ZZX& factors, const ZZX& ff, 
                     const vec_ZZX& w, const ZZ& P, 
                     LocalInfoT& LocalInfo,
                     long verbose,
                     long bnd)
{
   ZZ_pBak bak;
   bak.save();
   ZZ_p::init(P);

   long r = w.length();

   vec_ZZ_pX W;
   W.SetLength(r);

   long i;
   for (i = 0; i < r; i++)
      conv(W[i], w[i]);


   ZZX f;

   f = ff;

   long k;

   k = 1;
   factors.SetLength(0);
   while (2*k <= W.length()) {
      if (k <= 1)
         CardinalitySearch(factors, f, W, LocalInfo, k, bnd, verbose);
      else
         CardinalitySearch1(factors, f, W, LocalInfo, k, bnd, verbose);
      k++;
   }

   append(factors, f);

   bak.restore();
}





/**********************************************************************\

                        van Hoeij's algorithm 

\**********************************************************************/



const long van_hoeij_size_thresh = 12; 
// Use van Hoeij's algorithm if number of modular factors exceeds this bound.
// Must be >= 1.

const long van_hoeij_card_thresh = 3;
// Switch to knapsack method if cardinality of candidate factors
// exceeds this bound.
// Must be >= 1.




// This routine assumes that the input f is a non-zero polynomial
// of degree n, and returns the value f(a).

static 
ZZ PolyEval(const ZZX& f, const ZZ& a)
{
   if (f == 0) LogicError("PolyEval: internal error");

   long n = deg(f);

   ZZ acc, t1, t2;
   long i;

   acc = f.rep[n];

   for (i = n-1; i >= 0; i--) {
      mul(t1, acc, a);
      add(acc, t1, f.rep[i]);
   }

   return acc;
}


// This routine assumes that the input f is a polynomial with non-zero constant
// term, of degree n, and with leading coefficient c; it returns 
// an upper bound on the absolute value of the roots of the
// monic, integer polynomial g(X) =  c^{n-1} f(X/c).

static 
ZZ RootBound(const ZZX& f)
{
   if (ConstTerm(f) == 0) LogicError("RootBound: internal error");

   long n = deg(f);

   ZZX g;
   long i;

   g = f;

   if (g.rep[n] < 0) negate(g.rep[n], g.rep[n]);
   for (i = 0; i < n; i++) {
      if (g.rep[i] > 0) negate(g.rep[i], g.rep[i]);
   }

   ZZ lb, ub, mb;


   lb = 0;

   ub = 1;
   while (PolyEval(g, ub) < 0) {
      ub = 2*ub;
   }

   // lb < root <= ub

   while (ub - lb > 1) {
      ZZ mb = (ub + lb)/2;

      if (PolyEval(g, mb) < 0) 
         lb = mb;
      else 
         ub = mb;
   }

   return ub*g.rep[n];
}


// This routine takes as input an n x m integer matrix M, where the rows of M 
// are assumed to be linearly independent.
// It is also required that both n and m are non-zero.
// It computes an integer d, along with an n x m matrix R, such that
// R*d^{-1} is the reduced row echelon form of M.
// The routine is probabilistic: the output is always correct, but the
// routine may abort the program with negligible probability
// (specifically, if GenPrime returns a composite, and the modular
// gauss routine can't invert a non-zero element).

static
void gauss(ZZ& d_out, mat_ZZ& R_out, const mat_ZZ& M)
{
   long n = M.NumRows();
   long m = M.NumCols();

   if (n == 0 || m == 0) LogicError("gauss: internal error");

   zz_pBak bak;
   bak.save();

   for (;;) {
      long p = GenPrime_long(NTL_SP_NBITS);
      zz_p::init(p);

      mat_zz_p MM;
      conv(MM, M);

      long r = gauss(MM);
      if (r < n) continue;

      // compute pos(1..n), so that pos(i) is the index 
      // of the i-th pivot column

      vec_long pos;
      pos.SetLength(n);

      long i, j;
      for (i = j = 1; i <= n; i++) {
         while (MM(i, j) == 0) j++;
         pos(i) = j;
         j++;
      } 

      // compute the n x n sub-matrix consisting of the
      // pivot columns of M

      mat_ZZ S;
      S.SetDims(n, n);

      for (i = 1; i <= n; i++)
         for (j = 1; j <= n; j++)
            S(i, j) = M(i, pos(j));

      mat_ZZ S_inv;
      ZZ d;

      inv(d, S_inv, S);
      if (d == 0) continue;

      mat_ZZ R;
      mul(R, S_inv, M);

      // now check that R is of the right form, which it will be
      // if we were not unlucky

      long OK = 1;

      for (i = 1; i <= n && OK; i++) {
         for (j = 1; j < pos(i) && OK; j++)
            if (R(i, j) != 0) OK = 0;

         if (R(i, pos(i)) != d) OK = 0;

         for (j = 1; j < i && OK; j++)
            if (R(j, pos(i)) != 0) OK = 0;
      }

      if (!OK) continue;

      d_out = d;
      R_out = R;
      break;
   }
}


// The input polynomial f should be monic, and deg(f) > 0.
// The input P should be > 1.
// Tr.length() >= d, and Tr(i), for i = 1..d-1, should be the
// Tr_i(f) mod P (in van Hoeij's notation).
// The quantity Tr_d(f) mod P is computed, and stored in Tr(d).


void ComputeTrace(vec_ZZ& Tr, const ZZX& f, long d, const ZZ& P)
{
   long n = deg(f);

   // check arguments

   if (n <= 0 || LeadCoeff(f) != 1) 
      LogicError("ComputeTrace: internal error (1)");

   if (d <= 0)
      LogicError("ComputeTrace: internal error (2)");

   if (Tr.length() < d)
      LogicError("ComputeTrace: internal error (3)");

   if (P <= 1)
      LogicError("ComputeTrace: internal error (4)");

   // treat d > deg(f) separately

   if (d > n) {
      ZZ t1, t2;
      long i;

      t1 = 0;

      for (i = 1; i <= n; i++) {
         mul(t2, Tr(i + d - n - 1), f.rep[i-1]); 
         add(t1, t1, t2);
      }

      rem(t1, t1, P);
      NegateMod(t1, t1, P);
      Tr(d) = t1;
   }
   else {
      ZZ t1, t2;
      long i;

      mul(t1, f.rep[n-d], d);

      for (i = 1; i < d; i++) {
         mul(t2, Tr(i), f.rep[n-d+i]);
         add(t1, t1, t2);
      }

      rem(t1, t1, P);
      NegateMod(t1, t1, P);
      Tr(d) = t1;
   }
}

// Tr(1..d) are traces as computed above.
// C and pb have length at least d.
// For i = 1..d, pb(i) = p^{a_i} for a_i > 0.
// pdelta = p^delta for delta > 0.
// P = p^a for some a >= max{ a_i : i=1..d }.

// This routine computes C(1..d), where 
// C(i) = C_{a_i}^{a_i + delta}( Tr(i)*lc^i ) for i = 1..d.


void ChopTraces(vec_ZZ& C, const vec_ZZ& Tr, long d,
                const vec_ZZ& pb, const ZZ& pdelta, const ZZ& P, const ZZ& lc)
{
   if (d <= 0) LogicError("ChopTraces: internal error (1)");
   if (C.length() < d) LogicError("ChopTraces: internal error (2)");
   if (Tr.length() < d) LogicError("ChopTraces: internal error (3)");
   if (pb.length() < d) LogicError("ChopTraces: internal error (4)");
   if (P <= 1) LogicError("ChopTraces: internal error (5)");

   ZZ lcpow, lcred;
   lcpow = 1;
   rem(lcred, lc, P);

   ZZ pdelta_2;
   RightShift(pdelta_2, pdelta, 1);

   ZZ t1, t2;

   long i;
   for (i = 1; i <= d; i++) {
      MulMod(lcpow, lcpow, lcred, P);
      MulMod(t1, lcpow, Tr(i), P);

      RightShift(t2, pb(i), 1);
      add(t1, t1, t2);
      div(t1, t1, pb(i));
      rem(t1, t1, pdelta);
      if (t1 > pdelta_2)
         sub(t1, t1, pdelta);

      C(i) = t1;
   }
}


// Similar to above, but computes a linear combination of traces.


static
void DenseChopTraces(vec_ZZ& C, const vec_ZZ& Tr, long d, long d1, 
                     const ZZ& pb_eff, const ZZ& pdelta, const ZZ& P, 
                     const ZZ& lc, const mat_ZZ& A)
{

   ZZ pdelta_2;
   RightShift(pdelta_2, pdelta, 1);

   ZZ pb_eff_2;
   RightShift(pb_eff_2, pb_eff, 1);

   ZZ acc, t1, t2;

   long i, j;

   ZZ lcpow, lcred;
   rem(lcred, lc, P);

   for (i = 1; i <= d1; i++) {
      lcpow = 1;
      acc = 0;

      for (j = 1; j <= d; j++) {
         MulMod(lcpow, lcpow, lcred, P);
         MulMod(t1, lcpow, Tr(j), P);
         rem(t2, A(i, j), P);
         MulMod(t1, t1, t2, P);
         AddMod(acc, acc, t1, P);
      }

      t1 = acc;
      add(t1, t1, pb_eff_2);
      div(t1, t1, pb_eff);
      rem(t1, t1, pdelta);
      if (t1 > pdelta_2)
         sub(t1, t1, pdelta);

      C(i) = t1;
   }
}


static
void Compute_pb(vec_long& b,vec_ZZ& pb, long p, long d, 
                const ZZ& root_bound, long n)
{
   ZZ t1, t2;
   long i;

   t1 = 2*power(root_bound, d)*n;

   if (d == 1) {
      i = 0;
      t2 = 1;
   }
   else {
      i = b(d-1);
      t2 = pb(d-1);
   }

   while (t2 <= t1) {
      i++;
      t2 *= p;
   }

   b.SetLength(d);
   b(d) = i;

   pb.SetLength(d);
   pb(d) = t2;
}

static
void Compute_pdelta(long& delta, ZZ& pdelta, long p, long bit_delta)
{
   ZZ t1;
   long i;

   i = delta;
   t1 = pdelta;

   while (NumBits(t1) <= bit_delta) {
      i++;
      t1 *= p;
   }

   delta = i;
   pdelta = t1;
}

static
void BuildReductionMatrix(mat_ZZ& M, long& C, long r, long d, const ZZ& pdelta,
                          const vec_vec_ZZ& chop_vec, 
                          const mat_ZZ& B_L, long verbose)
{
   long s = B_L.NumRows();

   C = long( sqrt(double(d) * double(r)) / 2.0 ) + 1;

   M.SetDims(s+d, r+d);
   clear(M);


   long i, j, k;
   ZZ t1, t2;

   for (i = 1; i <= s; i++)
      for (j = 1; j <= r; j++)
         mul(M(i, j), B_L(i, j), C);

   ZZ pdelta_2;

   RightShift(pdelta_2, pdelta, 1);

   long maxbits = 0;

   for (i = 1; i <= s; i++)
      for (j = 1; j <= d; j++) {
         t1 = 0;
         for (k = 1; k <= r; k++) {
            mul(t2, B_L(i, k), chop_vec(k)(j));
            add(t1, t1, t2);
         }

         rem(t1, t1, pdelta);
         if (t1 > pdelta_2)
            sub(t1, t1, pdelta);

         maxbits = max(maxbits, NumBits(t1));

         M(i, j+r) = t1;
      }
  

   for (i = 1; i <= d; i++)
      M(i+s, i+r) = pdelta;

   if (verbose) 
      cerr << "ratio = " << double(maxbits)/double(NumBits(pdelta))
           << "; ";
}


static
void CutAway(mat_ZZ& B1, vec_ZZ& D, mat_ZZ& M, 
             long C, long r, long d)
{
   long k = M.NumRows();
   ZZ bnd = 4*to_ZZ(C)*to_ZZ(C)*to_ZZ(r) + to_ZZ(d)*to_ZZ(r)*to_ZZ(r);

   while (k >= 1 && 4*D[k] > bnd*D[k-1]) k--;

   mat_ZZ B2;

   B2.SetDims(k, r);
   long i, j;

   for (i = 1; i <= k; i++)
      for (j = 1; j <= r; j++)
         div(B2(i, j), M(i, j), C);

   M.kill(); // save space
   D.kill();

   ZZ det2;
   long rnk;

   rnk = image(det2, B2);

   B1.SetDims(rnk, r);
   for (i = 1; i <= rnk; i++)
      for (j = 1; j <= r; j++)
         B1(i, j) = B2(i + k - rnk, j);
}




static
long GotThem(vec_ZZX& factors, 
             const mat_ZZ& B_L,
             const vec_ZZ_pX& W, 
             const ZZX& f, 
             long bnd,
             long verbose)
{
   double tt0, tt1;
   ZZ det;
   mat_ZZ R;
   long s, r;
   long i, j, cnt;

   if (verbose) {
      cerr << "   checking A (s = " << B_L.NumRows() 
           << "): gauss...";
   }

   tt0 = GetTime();

   gauss(det, R, B_L);

   tt1 = GetTime();

   if (verbose) cerr << (tt1-tt0) << "; ";

   // check if condition A holds

   s = B_L.NumRows();
   r = B_L.NumCols();

   for (j = 0; j < r; j++) {
      cnt = 0;
      for (i = 0; i < s; i++) {
         if (R[i][j] == 0) continue;
         if (R[i][j] != det) {
            if (verbose) cerr << "failed.\n";
            return 0;
         }
         cnt++;
      }

      if (cnt != 1) {
         if (verbose) cerr << "failed.\n";
         return 0;
      }
   }

   if (verbose) {
      cerr << "passed.\n";
      cerr << "   checking B...";
   }

   // extract relevant information from R

   vec_vec_long I_vec;
   I_vec.SetLength(s);

   vec_long deg_vec;
   deg_vec.SetLength(s);

   for (i = 0; i < s; i++) {
      long dg = 0;

      for (j = 0; j < r; j++) {
         if (R[i][j] != 0) append(I_vec[i], j);
         dg += deg(W[j]);
      }

      deg_vec[i] = dg;
   }

   R.kill(); // save space


   // check if any candidate factor is the product of too few
   // modular factors

   for (i = 0; i < s; i++)
      if (I_vec[i].length() <= van_hoeij_card_thresh) {
         if (verbose) cerr << "X\n";
         return 0;
      }

   if (verbose) cerr << "1";


   // sort deg_vec, I_vec in order of increasing degree

   for (i = 0; i < s-1; i++)
      for (j = 0; j < s-1-i; j++)
         if (deg_vec[j] > deg_vec[j+1]) {
            _ntl_swap(deg_vec[j], deg_vec[j+1]);
            swap(I_vec[j], I_vec[j+1]);
         }


   // perform constant term tests

   ZZ ct;
   mul(ct, LeadCoeff(f), ConstTerm(f));

   ZZ half_P;
   RightShift(half_P, ZZ_p::modulus(), 1);

   ZZ_p lc, prod;
   conv(lc, LeadCoeff(f));

   ZZ t1;

   for (i = 0; i < s; i++) {
      vec_long& I = I_vec[i];
      prod = lc;
      for (j = 0; j < I.length(); j++)
         mul(prod, prod, ConstTerm(W[I[j]]));

      t1 = rep(prod);
      if (t1 > half_P)
         sub(t1, t1, ZZ_p::modulus());

      if (!divide(ct, t1)) {
          if (verbose) cerr << "X\n";
          return 0;
      }
   }

   if (verbose) cerr << "2";


   // multiply out polynomials and perform size tests

   vec_ZZX fac;
   ZZ_pX gg;
   ZZX g;

   for (i = 0; i < s-1; i++) {
      vec_long& I = I_vec[i];
      mul(gg, W, I);
      mul(gg, gg, lc);
      BalCopy(g, gg);
      if (MaxBits(g) > bnd) {
         if (verbose) cerr << "X\n";
         return 0;
      }
      PrimitivePart(g, g);
      append(fac, g);
   }

   if (verbose) cerr << "3";


   // finally...trial division

   ZZX f1 = f;
   ZZX h;

   for (i = 0; i < s-1; i++) {
      if (!divide(h, f1, fac[i])) {
         cerr << "X\n";
         return 0;
      }

      f1 = h;
   }

   // got them!

   if (verbose) cerr << "$\n";

   append(factors, fac);
   append(factors, f1);

   return 1;
}


void AdditionalLifting(ZZ& P1, 
                       long& e1, 
                       vec_ZZX& w1, 
                       long p, 
                       long new_bound,
                       const ZZX& f, 
                       long doubling,
                       long verbose)
{
   long new_e1;

   if (doubling)
      new_e1 = max(2*e1, new_bound); // at least double e1
   else
      new_e1 = new_bound;

   if (verbose) {
      cerr << ">>> additional hensel lifting to " << new_e1 << "...\n";
   }

   ZZ new_P1;

   power(new_P1, p, new_e1);

   ZZX f1;
   ZZ t1, t2;
   long i;
   long n = deg(f);

   if (LeadCoeff(f) == 1)
      f1 = f;
   else if (LeadCoeff(f) == -1)
      negate(f1, f);
   else {
      rem(t1, LeadCoeff(f), new_P1);
      InvMod(t1, t1, new_P1);
      f1.rep.SetLength(n+1); 
      for (i = 0; i <= n; i++) {
         mul(t2, f.rep[i], t1);
         rem(f1.rep[i], t2, new_P1);
      }
   }

   zz_pBak bak;
   bak.save();

   zz_p::init(p, NextPowerOfTwo(n)+1);

   long r = w1.length();

   vec_zz_pX ww1;
   ww1.SetLength(r);
   for (i = 0; i < r; i++)
      conv(ww1[i], w1[i]);

   w1.kill();

   double tt0, tt1;

   tt0 = GetTime();

   MultiLift(w1, ww1, f1, new_e1, verbose);

   tt1 = GetTime();

   if (verbose) {
      cerr << "lifting time: " << (tt1-tt0) << "\n\n";
   }

   P1 = new_P1;
   e1 = new_e1;

   bak.restore();
}

static
void Compute_pb_eff(long& b_eff, ZZ& pb_eff, long p, long d, 
                    const ZZ& root_bound,  
                    long n, long ran_bits)
{
   ZZ t1, t2;
   long i;

   if (root_bound == 1)
      t1 = (to_ZZ(d)*to_ZZ(n)) << (ran_bits + 1);
   else
      t1 = (power(root_bound, d)*n) << (ran_bits + 2);

   i = 0;
   t2 = 1;

   while (t2 <= t1) {
      i++;
      t2 *= p;
   }

   b_eff = i;
   pb_eff = t2;
}



static
long d1_val(long bit_delta, long r, long s)
{
   return long( 0.30*double(r)*double(s)/double(bit_delta) ) + 1;
}




// Next comes van Hoeij's algorithm itself.
// Some notation that differs from van Hoeij's paper:
//   n = deg(f)
//   r = # modular factors
//   s = dim(B_L)  (gets smaller over time)
//   d = # traces used
//   d1 = number of "compressed" traces
//
// The algorithm starts with a "sparse" version of van Hoeij, so that
// at first the traces d = 1, 2, ... are used in conjunction with
// a d x d identity matrix for van Hoeij's matrix A.
// The number of "excess" bits used for each trace, bit_delta, is initially
// 2*r.
// 
// When d*bit_delta exceeds 0.25*r*s, we switch to 
// a "dense" mode, where we use only about 0.25*r*s "compressed" traces.
// These bounds follow from van Hoeij's heuristic estimates.
//
// In sparse mode, d and bit_delta increase exponentially (but gently).
// In dense mode, but d increases somewhat more aggressively,
// and bit_delta is increased more gently.


static
void FindTrueFactors_vH(vec_ZZX& factors, const ZZX& ff, 
                        const vec_ZZX& w, const ZZ& P, 
                        long p, long e,
                        LocalInfoT& LocalInfo,
                        long verbose,
                        long bnd)
{
   const long SkipSparse = 0;

   ZZ_pBak bak;
   bak.save();
   ZZ_p::init(P);

   long r = w.length();

   vec_ZZ_pX W;
   W.SetLength(r);

   long i, j;

   for (i = 0; i < r; i++)
      conv(W[i], w[i]);


   ZZX f;

   f = ff;

   long k;

   k = 1;
   factors.SetLength(0);
   while (2*k <= W.length() && 
      (k <= van_hoeij_card_thresh || W.length() <= van_hoeij_size_thresh)) {

      if (k <= 1)
         CardinalitySearch(factors, f, W, LocalInfo, k, bnd, verbose);
      else
         CardinalitySearch1(factors, f, W, LocalInfo, k, bnd, verbose);
      k++;
   }

   if (2*k > W.length()) {
      // rest is irreducible, so we're done

      append(factors, f);
   }
   else {

      // now we apply van Hoeij's algorithm proper to f
   
      double time_start, time_stop, lll_time, tt0, tt1;

      time_start = GetTime();
      lll_time = 0;
   
      if (verbose) {
         cerr << "\n\n*** starting knapsack procedure\n";
      }
   
      ZZ P1 = P;
      long e1 = e;    // invariant: P1 = p^{e1}
   
      r = W.length();
   
      vec_ZZX w1;
      w1.SetLength(r);
      for (i = 0; i < r; i++)
         conv(w1[i], W[i]);
   
      long n = deg(f);
   
      mat_ZZ B_L;            // van Hoeij's lattice
      ident(B_L, r);
   
      long d = 0;            // number of traces
      
      long bit_delta = 0;    // number of "excess" bits

      vec_long b;
      vec_ZZ pb;             // pb(i) = p^{b(i)}

      long delta = 0;
      ZZ pdelta = to_ZZ(1);  // pdelta = p^delta
      pdelta = 1;
   
      vec_vec_ZZ trace_vec;
      trace_vec.SetLength(r);
   
      vec_vec_ZZ chop_vec;
      chop_vec.SetLength(r);
   
      ZZ root_bound = RootBound(f);
   
      if (verbose) {
         cerr << "NumBits(root_bound) = " << NumBits(root_bound) << "\n";
      }

      long dense = 0;
      long ran_bits = 32;

      long loop_cnt = 0;


      long s = r;

      for (;;) {

         loop_cnt++;
   
         // if we are using the power hack, then we do not try too hard...
         // this is really a hack on a hack!

         if (ok_to_abandon && 
             ((d >= 2 && s > 128) || (d >= 3 && s > 32) || (d >= 4 && s > 8) ||
              d >= 5) ) {
            if (verbose) cerr << "   abandoning\n";
            append(factors, f);
            break;
         }

         long d_last, d_inc, d_index;

         d_last = d;

         // set d_inc: 

         if (!dense) {
            d_inc = 1 + d/8;
         }
         else {
            d_inc = 1 + d/4; 
         }

         d_inc = min(d_inc, n-1-d);
            
         d += d_inc;

         // set bit_delta:
   
         if (bit_delta == 0) {
            // set initial value...don't make it any smaller than 2*r

            bit_delta = 2*r; 
         }
         else {
            long extra_bits;

            if (!dense) {
               extra_bits = 1 + bit_delta/8;
            }
            else if (d_inc != 0) {
               if (d1_val(bit_delta, r, s) > 1)
                  extra_bits = 1 + bit_delta/16; 
               else
                  extra_bits = 0;
            }
            else
               extra_bits = 1 + bit_delta/8;

            bit_delta += extra_bits;
         }

         if (d > d1_val(bit_delta, r, s)) 
            dense = 1;
   
         Compute_pdelta(delta, pdelta, p, bit_delta);

         long d1;
         long b_eff;
         ZZ pb_eff;

         if (!dense) {
            for (d_index = d_last + 1; d_index <= d; d_index++)
               Compute_pb(b, pb, p, d_index, root_bound, n);

            d1 = d;
            b_eff = b(d);
            pb_eff = pb(d);
         }
         else {
            d1 = d1_val(bit_delta, r, s);
            Compute_pb_eff(b_eff, pb_eff, p, d, root_bound, n, ran_bits); 
         }

         if (verbose) {
            cerr << "*** d = " << d 
                 << "; s = " << s 
                 << "; delta = " << delta 
                 << "; b_eff = " << b_eff;

            if (dense) cerr << "; dense [" << d1 << "]";
            cerr << "\n";
         }
   
         if (b_eff + delta > e1) {
            long doubling;

            doubling = 1;

            AdditionalLifting(P1, e1, w1, p, b_eff + delta, f, 
                              doubling, verbose);

            if (verbose) {
               cerr << ">>> recomputing traces...";
            }

            tt0 = GetTime();

            trace_vec.kill();
            trace_vec.SetLength(r);

            for (i = 0; i < r; i++) {
               trace_vec[i].SetLength(d_last);

               for (d_index = 1; d_index <= d_last; d_index++) {
                  ComputeTrace(trace_vec[i], w1[i], d_index, P1);
               }
            }

            tt1 = GetTime();
            if (verbose) cerr << (tt1-tt0) << "\n";
         }
   
         if (verbose) cerr << "   trace..."; 
   
         tt0 = GetTime();

         mat_ZZ A;

         if (dense) {
            A.SetDims(d1, d);
            for (i = 1; i <= d1; i++)
               for (j = 1; j <= d; j++) {
                  RandomBits(A(i, j), ran_bits);
                  if (RandomBnd(2)) negate(A(i, j), A(i, j));
               }
         }
      
   
         for (i = 0; i < r; i++) {
            trace_vec[i].SetLength(d);
            for (d_index = d_last + 1; d_index <= d; d_index++)
               ComputeTrace(trace_vec[i], w1[i], d_index, P1);
   
            chop_vec[i].SetLength(d1);

            if (!dense)
               ChopTraces(chop_vec[i], trace_vec[i], d, pb, pdelta, 
                          P1, LeadCoeff(f));
            else
               DenseChopTraces(chop_vec[i], trace_vec[i], d, d1, pb_eff, 
                               pdelta, P1, LeadCoeff(f), A);
         }

         A.kill();
   
         tt1 = GetTime();
   
         if (verbose) cerr << (tt1-tt0) << "\n";
   
         mat_ZZ M;
         long C;
   
         if (verbose) cerr << "   building matrix...";
   
         tt0 = GetTime();
   
         BuildReductionMatrix(M, C, r, d1, pdelta, chop_vec, B_L, verbose);
   
         tt1 = GetTime();
   
         if (verbose) cerr << (tt1-tt0) << "\n";

         if (SkipSparse) {   
            if (!dense) {
               if (verbose) cerr << "skipping LLL\n";
               continue;
            }
         }

         if (verbose) cerr << "   LLL...";
   
         tt0 = GetTime();
   
         vec_ZZ D;
         long rnk = LLL_plus(D, M);
   
         tt1 = GetTime();

         lll_time += (tt1-tt0);
   
         if (verbose) cerr << (tt1-tt0) << "\n";
   
         if (rnk != s + d1) {
            LogicError("van Hoeij -- bad rank");
         }
   
         mat_ZZ B1;
   
         if (verbose) cerr << "   CutAway...";
   
         tt0 = GetTime();
   
         CutAway(B1, D, M, C, r, d1);
   
         tt1 = GetTime();
   
         if (verbose) cerr << (tt1-tt0) << "\n";
   
         if (B1.NumRows() >= s) continue;
         // no progress...try again

         // otherwise, update B_L and test if we are done
   
         swap(B1, B_L);
         B1.kill();
         s = B_L.NumRows();
   
         if (s == 0)
            LogicError("oops! s == 0 should not happen!");
   
         if (s == 1) {
            if (verbose) cerr << "   irreducible!\n";
            append(factors, f);
            break;
         }
   
         if (s > r / (van_hoeij_card_thresh + 1)) continue;
         // dimension too high...we can't be done
   
         if (GotThem(factors, B_L, W, f, bnd, verbose)) break;
      }

      time_stop = GetTime();

      if (verbose) {
         cerr << "*** knapsack finished: total time = " 
              << (time_stop - time_start) << "; LLL time = "
              << lll_time << "\n";
      }
   }

   bak.restore();
}


static
void ll_SFFactor(vec_ZZX& factors, const ZZX& ff, 
                 long verbose,
                 long bnd)

// input is primitive and square-free, with positive leading
// coefficient
{
   if (deg(ff) <= 1) {
      factors.SetLength(1);
      factors[0] = ff;
      if (verbose) {
         cerr << "*** SFFactor, trivial case 1.\n";
      }
      return;
   }

   // remove a factor of X, if necessary

   ZZX f;
   long xfac;
   long rev;

   double t;

   if (IsZero(ConstTerm(ff))) {
      RightShift(f, ff, 1);
      xfac = 1;
   }
   else {
      f = ff;
      xfac = 0;
   }

   // return a factor of X-1 if necessary

   long x1fac = 0;

   ZZ c1;
   SumCoeffs(c1, f);

   if (c1 == 0) {
      x1fac = 1;
      div(f, f, ZZX(1,1) - 1);
   }

   SumCoeffs(c1, f);

   if (deg(f) <= 1) {
      long r = 0;
      factors.SetLength(0);
      if (deg(f) > 0) {
         factors.SetLength(r+1);
         factors[r] = f;
         r++;
      }
      if (xfac) {
         factors.SetLength(r+1);
         SetX(factors[r]);
         r++;
      }

      if (x1fac) {
         factors.SetLength(r+1);
         factors[r] = ZZX(1,1) - 1;
         r++;
      }

      if (verbose) {
         cerr << "*** SFFactor: trivial case 2.\n";
      }

      return;
   }

   if (verbose) {
      cerr << "*** start SFFactor.\n";
   }

   // reverse f if this makes lead coefficient smaller

   ZZ t1, t2;

   abs(t1, LeadCoeff(f));
   abs(t2, ConstTerm(f));

   if (t1 > t2) {
      inplace_rev(f);
      rev = 1;
   }
   else 
      rev = 0;

   // obtain factorization modulo small primes

   if (verbose) {
      cerr << "factorization modulo small primes...\n";
      t = GetTime();
   }

   LocalInfoT LocalInfo;

   zz_pBak bak;
   bak.save();

   UniquePtr<vec_zz_pX> spfactors( SmallPrimeFactorization(LocalInfo, f, verbose) );

   if (!spfactors) {
      // f was found to be irreducible 

      bak.restore();

      if (verbose) {
         t = GetTime()-t;
         cerr << "small prime time: " << t << ", irreducible.\n";
      }

      if (rev)
         inplace_rev(f);

      long r = 0;

      factors.SetLength(r+1);
      factors[r] = f;
      r++;

      if (xfac) {
         factors.SetLength(r+1);
         SetX(factors[r]);
         r++;
      }

      if (x1fac) {
         factors.SetLength(r+1);
         factors[r] = ZZX(1,1) - 1;
         r++;
      }

      return;
   }

   if (verbose) {
      t = GetTime()-t;
      cerr << "small prime time: ";
      cerr << t << ", number of factors = " << spfactors->length() << "\n";
   }

   // prepare for Hensel lifting

   // first, calculate bit bound 

   long bnd1;
   long n = deg(f);
   long i;
   long e;
   ZZ P;
   long p;
   
   bnd1 = MaxBits(f) + (NumBits(n+1)+1)/2;

   if (!bnd || bnd1 < bnd)
      bnd = bnd1;

   i = n/2;
   while (!bit(LocalInfo.PossibleDegrees, i))
      i--;

   long lc_bnd = NumBits(LeadCoeff(f));

   long coeff_bnd = bnd + lc_bnd + i;

   long lift_bnd;

   lift_bnd = coeff_bnd + 15;  
   // +15 helps avoid trial divisions...can be any number >= 0

   lift_bnd = max(lift_bnd, bnd + lc_bnd + 2*NumBits(n) + ZZX_OVERLIFT);
   // facilitates "n-1" and "n-2" tests

   lift_bnd = max(lift_bnd, lc_bnd + NumBits(c1));
   // facilitates f(1) test

   lift_bnd += 2;
   // +2 needed to get inequalities right


   p = zz_p::modulus();

   e = long(double(lift_bnd)/(log(double(p))/log(double(2))));
   power(P, p, e);

   while (NumBits(P) <= lift_bnd) { 
      mul(P, P, p);
      e++;
   }

   if (verbose) {
      cerr << "lifting bound = " << lift_bnd << " bits.\n";
      cerr << "Hensel lifting to exponent " << e << "...\n";
      t = GetTime();
   }

   // third, compute f1 so that it is monic and equal to f mod P

   ZZX f1;

   if (LeadCoeff(f) == 1)
      f1 = f;
   else if (LeadCoeff(f) == -1)
      negate(f1, f);
   else {
      rem(t1, LeadCoeff(f), P);
      if (sign(P) < 0)
         LogicError("whoops!!!");
      InvMod(t1, t1, P);
      f1.rep.SetLength(n+1);
      for (i = 0; i <= n; i++) {
         mul(t2, f.rep[i], t1);
         rem(f1.rep[i], t2, P);
      }
   }


   // Do Hensel lift

   vec_ZZX w;

   MultiLift(w, *spfactors, f1, e, verbose);


   if (verbose) {
      t = GetTime()-t;
      cerr << "\nlifting time: ";
      cerr << t << "\n\n";
   }

   // We're done with zz_p...restore

   spfactors.reset();
   bak.restore();

   // search for true factors

   if (verbose) {
      cerr << "searching for true factors...\n";
      t = GetTime();
   }

   if (ZZXFac_van_Hoeij && w.length() > van_hoeij_size_thresh)
      FindTrueFactors_vH(factors, f, w, P, p, e, 
                         LocalInfo, verbose, coeff_bnd);
   else
      FindTrueFactors(factors, f, w, P, LocalInfo, verbose, coeff_bnd);

   if (verbose) {
      t = GetTime()-t;
      cerr << "factor search time " << t << "\n";
   }

   long r = factors.length();

   if (rev) {
      for (i = 0; i < r; i++) {
         inplace_rev(factors[i]);
         if (sign(LeadCoeff(factors[i])) < 0)
            negate(factors[i], factors[i]);
      }
   }

   if (xfac) {
      factors.SetLength(r+1);
      SetX(factors[r]);
      r++;
   }

   if (x1fac) {
      factors.SetLength(r+1);
      factors[r] = ZZX(1,1)-1;
      r++;
   }

   // that's it!!

   if (verbose) {
      cerr << "*** end SFFactor.  degree sequence:\n";
      for (i = 0; i < r; i++)
         cerr << deg(factors[i]) << " ";
      cerr << "\n";
   }
}



static 
long DeflationFactor(const ZZX& f)
{
   long n = deg(f);
   long m = 0;
   long i;

   for (i = 1; i <= n && m != 1; i++) {
      if (f.rep[i] != 0)
         m = GCD(m, i);
   }

   return m;
}

static
void inflate(ZZX& g, const ZZX& f, long m)
// input may not alias output
{
   long n = deg(f);
   long i;

   g = 0;
   for (i = n; i >= 0; i--) 
      SetCoeff(g, i*m, f.rep[i]);
}

static
void deflate(ZZX& g, const ZZX& f, long m)
// input may not alias output
{
   long n = deg(f);
   long i;

   g = 0;
   for (i = n; i >= 0; i -= m) 
      SetCoeff(g, i/m, f.rep[i]);
}

static
void MakeFacList(vec_long& v, long m)
{
   if (m <= 0) LogicError("internal error: MakeFacList");

   v.SetLength(0);

   long p = 2;
   while (m > 1) {
      while (m % p == 0)  {
         append(v, p);
         m = m / p;
      }

      p++;
   }
}

NTL_CHEAP_THREAD_LOCAL long ZZXFac_PowerHack = 1;

void SFFactor(vec_ZZX& factors, const ZZX& ff, 
              long verbose,
              long bnd)

// input is primitive and square-free, with positive leading
// coefficient

{
   if (ff == 0) 
      LogicError("SFFactor: bad args");

   if (deg(ff) <= 0) {
      factors.SetLength(0);
      return;
   }


   if (!ZZXFac_PowerHack) {
      ok_to_abandon = 0;
      ll_SFFactor(factors, ff, verbose, bnd);
      return;
   }

   long m = DeflationFactor(ff);

   if (m == 1) {
      if (verbose) {
         cerr << "SFFactor -- no deflation\n";
      }

      ok_to_abandon = 0;
      ll_SFFactor(factors, ff, verbose, bnd);
      return;
   }


   vec_long v;
   MakeFacList(v, m);
   long l = v.length();

   if (verbose) {
      cerr << "SFFactor -- deflation: " << v << "\n";
   }

   vec_ZZX res;
   res.SetLength(1);
   deflate(res[0], ff, m);

   long done;
   long j, k;

   done = 0;
   k = l-1;

   while (!done) {
      vec_ZZX res1;
      res1.SetLength(0);
      for (j = 0; j < res.length(); j++) {
         vec_ZZX res2;
         double t;
         if (verbose) {
            cerr << "begin - step " << k << ", " << j << "; deg = " 
                 << deg(res[j]) << "\n";
            t = GetTime();
         }

         if (k < 0)
            ok_to_abandon = 0;
         else
            ok_to_abandon = 1;

         ll_SFFactor(res2, res[j], verbose, k < 0 ? bnd : 0);

         if (verbose) {
            t = GetTime()-t;
            cerr << "end   - step " << k << ", " << j << "; time = "
                 << t << "\n\n";
         }

         append(res1, res2);
      }

      if (k < 0) {
         done = 1;
         swap(res, res1);
      }
      else {
         vec_ZZX res2;
         res2.SetLength(res1.length());
         for (j = 0; j < res1.length(); j++)
            inflate(res2[j], res1[j], v[k]);
         k--;
         swap(res, res2);
      }
   }

   factors = res;
}





void factor(ZZ& c,
            vec_pair_ZZX_long& factors,
            const ZZX& f,
            long verbose,
            long bnd)

{
   ZZX ff = f;

   if (deg(ff) <= 0) {
      c = ConstTerm(ff);
      factors.SetLength(0);
      return;
   }

   content(c, ff);
   divide(ff, ff, c);

   long bnd1 = MaxBits(ff) + (NumBits(deg(ff)+1)+1)/2;
   if (!bnd || bnd > bnd1)
      bnd = bnd1;

   vec_pair_ZZX_long sfd;

   double t;

   if (verbose) { cerr << "square-free decomposition..."; t = GetTime(); }
   SquareFreeDecomp(sfd, ff);
   if (verbose) cerr << (GetTime()-t) << "\n";

   factors.SetLength(0);

   vec_ZZX x;

   long i, j;

   for (i = 0; i < sfd.length(); i++) {
      if (verbose) {
         cerr << "factoring multiplicity " << sfd[i].b
              << ", deg = " << deg(sfd[i].a) << "\n";
         t = GetTime();
      }

      SFFactor(x, sfd[i].a, verbose, bnd);

      if (verbose) {
         t = GetTime()-t;
         cerr << "total time for multiplicity " 
              << sfd[i].b << ": " << t << "\n";
      }

      for (j = 0; j < x.length(); j++)
         append(factors, cons(x[j], sfd[i].b));
   }
}

NTL_END_IMPL

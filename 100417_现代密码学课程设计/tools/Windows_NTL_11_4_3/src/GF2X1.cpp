

#include <NTL/GF2X.h>
#include <NTL/vec_long.h>

#ifndef NTL_WIZARD_HACK

#include <NTL/ZZX.h>

#endif


#if (defined(NTL_WIZARD_HACK) && defined(NTL_GF2X_LIB))
#undef NTL_GF2X_LIB
#endif


// simple scaling factor for some crossover points:
// we use a lower crossover of the underlying multiplication
// is faster  
#if (defined(NTL_GF2X_LIB) || defined(NTL_HAVE_PCLMUL))
#define XOVER_SCALE (1L)
#else
#define XOVER_SCALE (2L)
#endif



#define NTL_GF2X_GCD_CROSSOVER (XOVER_SCALE*300L*NTL_BITS_PER_LONG) 

#define NTL_GF2X_BERMASS_CROSSOVER (XOVER_SCALE*200L*NTL_BITS_PER_LONG)

#define NTL_GF2X_HalfGCD_CROSSOVER (4L*NTL_BITS_PER_LONG)



NTL_START_IMPL



NTL_TLS_GLOBAL_DECL(vec_GF2X, stab)  
// used by PlainDivRem and PlainRem

NTL_TLS_GLOBAL_DECL(WordVector, GF2X_rembuf)


void PlainDivRem(GF2X& q, GF2X& r, const GF2X& a, const GF2X& b)
{
   NTL_TLS_GLOBAL_ACCESS(stab);

   long da, sa, posa, db, sb, posb, dq, sq, posq;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("GF2X: division by zero");

   if (da < db) {
      r = a;
      clear(q);
      return;
   }

   sa = a.xrep.length();
   posa = da - NTL_BITS_PER_LONG*(sa-1);
   sb = b.xrep.length();
   posb = db - NTL_BITS_PER_LONG*(sb-1);

   dq = da - db;
   sq = dq/NTL_BITS_PER_LONG + 1;
   posq = dq - NTL_BITS_PER_LONG*(sq-1);

   NTL_TLS_GLOBAL_ACCESS(GF2X_rembuf);
   WordVectorWatcher watch_GF2X_rembuf(GF2X_rembuf);

   _ntl_ulong *ap;
   if (&r == &a)
      ap = r.xrep.elts();
   else {
      GF2X_rembuf = a.xrep;
      ap = GF2X_rembuf.elts();
   }

   stab.SetLength(NTL_BITS_PER_LONG);
   long i;

   NTL_SCOPE(guard) {
      for (i = 0; i <= min(dq, NTL_BITS_PER_LONG-1); i++) {
         WordVector& st = stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG].xrep;
         st.KillBig();
      }
   };

   stab[posb] = b;
   for (i = 1; i <= min(dq, NTL_BITS_PER_LONG-1); i++) 
      MulByX(stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG], 
             stab[((_ntl_ulong)(posb+i-1))%NTL_BITS_PER_LONG]);

   _ntl_ulong *stab_ptr[NTL_BITS_PER_LONG];
   long stab_cnt[NTL_BITS_PER_LONG];

   for (i = 0; i <= min(dq, NTL_BITS_PER_LONG-1); i++) {
      WordVector& st = stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG].xrep;
      long k = st.length();
      stab_ptr[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG] = &st[k-1];
      stab_cnt[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG] = -k+1;
   }

   q.xrep.SetLength(sq);
   _ntl_ulong *qp = q.xrep.elts();
   for (i = 0; i < sq; i++)
      qp[i] = 0;

   _ntl_ulong *atop = &ap[sa-1];
   _ntl_ulong *qtop = &qp[sq-1];
   _ntl_ulong *stab_top;

   while (1) {
      if (atop[0] & (1UL << posa)) {
         qtop[0] |= (1UL << posq);
         stab_top = stab_ptr[posa];
         for (i = stab_cnt[posa]; i <= 0; i++)
            atop[i] ^= stab_top[i];
      }

      da--;
      if (da < db) break;

      posa--;
      if (posa < 0) {
         posa = NTL_BITS_PER_LONG-1;
         atop--;
      }

      posq--;
      if (posq < 0) {
         posq = NTL_BITS_PER_LONG-1;
         qtop--;
      }
   }

   if (posb == 0) sb--;

   r.xrep.SetLength(sb);
   if (&r != &a) {
      _ntl_ulong *rp = r.xrep.elts();
      for (i = 0; i < sb; i++)
         rp[i] = ap[i];
   }
   r.normalize();

   guard.relax();
   for (i = 0; i <= min(dq, NTL_BITS_PER_LONG-1); i++) {
      WordVector& st = stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG].xrep;
      st.KillBig();
   }
}



void PlainDiv(GF2X& q, const GF2X& a, const GF2X& b)
{
   NTL_GF2XRegister(r);
   PlainDivRem(q, r, a, b);
}


void PlainRem(GF2X& r, const GF2X& a, const GF2X& b)
{
   NTL_TLS_GLOBAL_ACCESS(stab);

   long da, sa, posa, db, sb, posb;

   da = deg(a);
   db = deg(b);

   if (db < 0) ArithmeticError("GF2X: division by zero");

   if (da < db) {
      r = a;
      return;
   }

   sa = a.xrep.length();
   posa = da - NTL_BITS_PER_LONG*(sa-1);
   sb = b.xrep.length();
   posb = db - NTL_BITS_PER_LONG*(sb-1);


   NTL_TLS_GLOBAL_ACCESS(GF2X_rembuf);
   WordVectorWatcher watch_GF2X_rembuf(GF2X_rembuf);

   _ntl_ulong *ap;
   if (&r == &a)
      ap = r.xrep.elts();
   else {
      GF2X_rembuf = a.xrep;
      ap = GF2X_rembuf.elts();
   }

   stab.SetLength(NTL_BITS_PER_LONG);
   long i;

   NTL_SCOPE(guard) {
      for (i = 0; i <= min(da-db, NTL_BITS_PER_LONG-1); i++) {
         WordVector& st = stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG].xrep;
         st.KillBig();
      }
   };

   stab[posb] = b;
   for (i = 1; i <= min(da-db, NTL_BITS_PER_LONG-1); i++) 
      MulByX(stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG], 
             stab[((_ntl_ulong)(posb+i-1))%NTL_BITS_PER_LONG]);

   _ntl_ulong *stab_ptr[NTL_BITS_PER_LONG];
   long stab_cnt[NTL_BITS_PER_LONG];

   for (i = 0; i <= min(da-db, NTL_BITS_PER_LONG-1); i++) {
      WordVector& st = stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG].xrep;
      long k = st.length();
      stab_ptr[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG] = &st[k-1];
      stab_cnt[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG] = -k+1;
   }


   _ntl_ulong *atop = &ap[sa-1];
   _ntl_ulong *stab_top;

   while (1) {
      if (atop[0] & (1UL << posa)) {
         stab_top = stab_ptr[posa];
         for (i = stab_cnt[posa]; i <= 0; i++)
            atop[i] ^= stab_top[i];
      }

      da--;
      if (da < db) break;

      posa--;
      if (posa < 0) {
         posa = NTL_BITS_PER_LONG-1;
         atop--;
      }
   }

   if (posb == 0) sb--;

   r.xrep.SetLength(sb);
   if (&r != &a) {
      _ntl_ulong *rp = r.xrep.elts();
      for (i = 0; i < sb; i++)
         rp[i] = ap[i];
   }
   r.normalize();

   guard.relax();
   for (i = 0; i <= min(da-db, NTL_BITS_PER_LONG-1); i++) {
      WordVector& st = stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG].xrep;
      st.KillBig();
   }
}

#define MASK8 ((1UL << 8)-1UL)

static const _ntl_ulong invtab[128] = {
1UL, 255UL, 85UL, 219UL, 73UL, 151UL, 157UL, 51UL, 17UL, 175UL,
69UL, 139UL, 89UL, 199UL, 141UL, 99UL, 33UL, 95UL, 117UL, 123UL,
105UL, 55UL, 189UL, 147UL, 49UL, 15UL, 101UL, 43UL, 121UL, 103UL,
173UL, 195UL, 65UL, 191UL, 21UL, 155UL, 9UL, 215UL, 221UL, 115UL,
81UL, 239UL, 5UL, 203UL, 25UL, 135UL, 205UL, 35UL, 97UL, 31UL,
53UL, 59UL, 41UL, 119UL, 253UL, 211UL, 113UL, 79UL, 37UL, 107UL,
57UL, 39UL, 237UL, 131UL, 129UL, 127UL, 213UL, 91UL, 201UL, 23UL,
29UL, 179UL, 145UL, 47UL, 197UL, 11UL, 217UL, 71UL, 13UL, 227UL,
161UL, 223UL, 245UL, 251UL, 233UL, 183UL, 61UL, 19UL, 177UL, 143UL,
229UL, 171UL, 249UL, 231UL, 45UL, 67UL, 193UL, 63UL, 149UL, 27UL,
137UL, 87UL, 93UL, 243UL, 209UL, 111UL, 133UL, 75UL, 153UL, 7UL,
77UL, 163UL, 225UL, 159UL, 181UL, 187UL, 169UL, 247UL, 125UL, 83UL,
241UL, 207UL, 165UL, 235UL, 185UL, 167UL, 109UL, 3UL };



void NewtonInvTrunc(GF2X& c, const GF2X& a, long e)
{
   if (e == 1) {
      set(c);
      return;
   }

   NTL_TLS_LOCAL(vec_long, E);
   E.SetLength(0);
   append(E, e);
   while (e > 8) {
      e = (e+1)/2;
      append(E, e);
   }

   long L = E.length();

   NTL_GF2XRegister(g);
   NTL_GF2XRegister(g0);
   NTL_GF2XRegister(g1);
   NTL_GF2XRegister(g2);

   g.xrep.SetMaxLength((E[0]+NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG + 1);
   g0.xrep.SetMaxLength((E[0]+NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG + 1);
   g1.xrep.SetMaxLength(((3*E[0]+1)/2+NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG+1);
   g2.xrep.SetMaxLength((E[0]+NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG + 1);

   g.xrep.SetLength(1);
   g.xrep[0] = invtab[(a.xrep[0] & MASK8) >> 1] & ((1UL<<e)-1UL);

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

void InvTrunc(GF2X& c, const GF2X& a, long e)
{
   if (ConstTerm(a) == 0 || e < 0)
      LogicError("inv: bad args");

   if (NTL_OVERFLOW(e, 1, 0))
      ResourceError("overflow in InvTrunc");

   if (e == 0) {
      clear(c);
      return;
   }

   NewtonInvTrunc(c, a, e);
}



static 
long weight1(_ntl_ulong a)
{
   long res = 0;
   while (a) {
      if (a & 1) res ++;
      a >>= 1;
   }
   return res;
}

long weight(const GF2X& a)
{
   long wlen = a.xrep.length();
   long res = 0;
   long i;
   for (i = 0; i < wlen; i++)
      res += weight1(a.xrep[i]);

   return res;
}



static
void SparsityCheck(const GF2X& f, long& k3, long& k2, long& k1)
{
   long w = weight(f);
   if (w != 3 && w != 5) {
      k3 = 0;
      return;
   }

   if (ConstTerm(f) != 1) {
      k3 = 0;
      return;
   }

   GF2X g = f;

   long n = deg(f);

   trunc(g, g, n);
   
   long t = deg(g);

   if (n-t < NTL_BITS_PER_LONG || t > (n+1)/2) {
      k3 = 0;
      return;
   }

   if (w == 3) {
      k3 = t;
      k2 = 0;
      return;
   }

   k3 = t;
   trunc(g, g, t);
   t = deg(g);
   k2 = t;
   trunc(g, g, t);
   t = deg(g);
   k1 = t;
}




const long GF2X_MOD_PLAIN = 0;
const long GF2X_MOD_MUL = 1;
const long GF2X_MOD_SPECIAL = 2;
const long GF2X_MOD_TRI = 3;
const long GF2X_MOD_PENT = 4;

void build(GF2XModulus& F, const GF2X& f)
{
   long n = deg(f);
   long i;

   if (n <= 0) LogicError("build(GF2XModulus,GF2X): deg(f) <= 0");

   F.tracevec.make();

   F.f = f;
   F.n = n;
   F.sn = f.xrep.length();

   long sb = F.sn;
   long posb = n - NTL_BITS_PER_LONG*(sb-1);

   F.posn = posb; 

   if (F.posn > 0) {
      F.size = F.sn;
      F.msk = (1UL << F.posn) - 1UL;
   }
   else {
      F.size = F.sn-1;
      F.msk = ~0UL;
   }

   SparsityCheck(f, F.k3, F.k2, F.k1);

   if (F.k3 != 0) {
      if (F.k2 == 0)
         F.method = GF2X_MOD_TRI;
      else
         F.method = GF2X_MOD_PENT;

      return;
   }


   GF2X f0;
   trunc(f0, f, n);

   if (F.n >= (NTL_BITS_PER_LONG/2)*XOVER_SCALE)
      F.method = GF2X_MOD_MUL;
   else 
      F.method = GF2X_MOD_PLAIN;


   // NOTE: I've run some tests which indicate that the GF2X_MOD_SPECIAL
   // method is not worth it.
   // FIXME: in a future version, I should eliminate all code
   // and data associated with GF2X_MOD_SPECIAL

   // NOTE: I've runs some tests which indicate that the crossover
   // for GF2X_MOD_MUL is extremely low, even without PCLMUL support.
      

   if (F.method == GF2X_MOD_SPECIAL) {
      if (!F.stab_cnt) F.stab_cnt.SetLength(NTL_BITS_PER_LONG);
      long *stab_cnt = F.stab_cnt.get();

      if (!F.stab1) F.stab1.SetLength(2*NTL_BITS_PER_LONG);
      _ntl_ulong *stab1 = F.stab1.get();

      stab1[posb<<1] = f.xrep[0];
      stab1[(posb<<1)+1] = 0;

      stab_cnt[posb] = -sb+1;

      for (i = 1; i < NTL_BITS_PER_LONG; i++) {
         long kk0 = ((_ntl_ulong)(posb+i-1))%NTL_BITS_PER_LONG;
         long kk1 = ((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG;

         stab1[kk1<<1] = stab1[kk0<<1] << 1;
         stab1[(kk1<<1)+1] = (stab1[(kk0<<1)+1] << 1) 
                          | (stab1[kk0<<1] >> (NTL_BITS_PER_LONG-1));

         if (kk1 < posb) 
            stab_cnt[kk1] = -sb;
         else
            stab_cnt[kk1] = -sb+1;
      }
   }
   else if (F.method == GF2X_MOD_PLAIN) {
      vec_GF2X& stab = F.stab;
      stab.SetLength(NTL_BITS_PER_LONG);


      if (!F.stab_ptr) F.stab_ptr.SetLength(NTL_BITS_PER_LONG);
      _ntl_ulong **stab_ptr = F.stab_ptr.get();

      if (!F.stab_cnt) F.stab_cnt.SetLength(NTL_BITS_PER_LONG);
      long *stab_cnt = F.stab_cnt.get();
      
   
      stab[posb] = f;
      for (i = 1; i < NTL_BITS_PER_LONG; i++) 
         MulByX(stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG], 
                stab[((_ntl_ulong)(posb+i-1))%NTL_BITS_PER_LONG]);
   
   
      for (i = 0; i < NTL_BITS_PER_LONG; i++) {
         WordVector& st = stab[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG].xrep;
         long k = st.length();
         stab_ptr[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG] = &st[k-1];
         stab_cnt[((_ntl_ulong)(posb+i))%NTL_BITS_PER_LONG] = -k+1;
      }
   }
   else if (F.method == GF2X_MOD_MUL) {
      GF2X P1, P2;

      CopyReverse(P1, f, n);
      InvTrunc(P2, P1, n-1);
      CopyReverse(P1, P2, n-2);
      trunc(F.h0, P1, n-2);
      F.f0 = f0;
   }
}

GF2XModulus::GF2XModulus()
{
   n = -1;
   method = GF2X_MOD_PLAIN;
}


// The following two routines are total spaghetti...unfortunately,
// cleaning them up would require too much re-coding in other
// places.

GF2XModulus::GF2XModulus(const GF2XModulus& F) :
   f(F.f), n(F.n), sn(F.sn), posn(F.posn), k3(F.k3), k2(F.k2), k1(F.k1),
   size(F.size), 
   msk(F.msk), method(F.method), stab(F.stab), h0(F.h0), f0(F.f0),
   tracevec(F.tracevec)
{
   if (method == GF2X_MOD_SPECIAL) {
      long i;
      stab1.SetLength(2*NTL_BITS_PER_LONG);
      for (i = 0; i < 2*NTL_BITS_PER_LONG; i++)
         stab1[i] = F.stab1[i];
      stab_cnt.SetLength(NTL_BITS_PER_LONG);
      for (i = 0; i < NTL_BITS_PER_LONG; i++)
         stab_cnt[i] = F.stab_cnt[i];
   }
   else if (method == GF2X_MOD_PLAIN) {
      long i;

      if (F.stab_cnt) {
         stab_cnt.SetLength(NTL_BITS_PER_LONG);
         for (i = 0; i < NTL_BITS_PER_LONG; i++)
            stab_cnt[i] = F.stab_cnt[i];
      }

      if (F.stab_ptr) {
         stab_ptr.SetLength(NTL_BITS_PER_LONG);
         for (i = 0; i < NTL_BITS_PER_LONG; i++) {
            WordVector& st = stab[((_ntl_ulong)(posn+i))%NTL_BITS_PER_LONG].xrep;
            long k = st.length();
            stab_ptr[((_ntl_ulong)(posn+i))%NTL_BITS_PER_LONG] = &st[k-1];
            stab_cnt[((_ntl_ulong)(posn+i))%NTL_BITS_PER_LONG] = -k+1;
         }
      }
   }
}

GF2XModulus& GF2XModulus::operator=(const GF2XModulus& F)
{
   if (this == &F) return *this;

   f=F.f; n=F.n; sn=F.sn; posn=F.posn; 
   k3=F.k3; k2=F.k2; k1=F.k1;
   size=F.size; 
   msk=F.msk; method=F.method; stab=F.stab; h0=F.h0; f0 = F.f0;
   tracevec=F.tracevec;

   if (method == GF2X_MOD_SPECIAL) {
      long i;
      if (!stab1) stab1.SetLength(2*NTL_BITS_PER_LONG);
      for (i = 0; i < 2*NTL_BITS_PER_LONG; i++)
         stab1[i] = F.stab1[i];
      if (!stab_cnt) stab_cnt.SetLength(NTL_BITS_PER_LONG);
      for (i = 0; i < NTL_BITS_PER_LONG; i++)
         stab_cnt[i] = F.stab_cnt[i];
   }
   else if (method == GF2X_MOD_PLAIN) {
      long i;

      if (F.stab_cnt) {
         if (!stab_cnt) stab_cnt.SetLength(NTL_BITS_PER_LONG);
         for (i = 0; i < NTL_BITS_PER_LONG; i++)
            stab_cnt[i] = F.stab_cnt[i];
      }

      if (F.stab_ptr) {
         if (!stab_ptr) stab_ptr.SetLength(NTL_BITS_PER_LONG);
      
         for (i = 0; i < NTL_BITS_PER_LONG; i++) {
            WordVector& st = stab[((_ntl_ulong)(posn+i))%NTL_BITS_PER_LONG].xrep;
            long k = st.length();
            stab_ptr[((_ntl_ulong)(posn+i))%NTL_BITS_PER_LONG] = &st[k-1];
            stab_cnt[((_ntl_ulong)(posn+i))%NTL_BITS_PER_LONG] = -k+1;
         }
      }
   }

   return *this;
}
   



GF2XModulus::GF2XModulus(const GF2X& ff)
{
   n = -1;
   method = GF2X_MOD_PLAIN;

   build(*this, ff);
}





void UseMulRem21(GF2X& r, const GF2X& a, const GF2XModulus& F)
{
   NTL_GF2XRegister(P1);
   NTL_GF2XRegister(P2);

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   add(P2, P2, P1);
   mul(P1, P2, F.f0);
   trunc(P1, P1, F.n);
   trunc(r, a, F.n);
   add(r, r, P1);
}

void UseMulDivRem21(GF2X& q, GF2X& r, const GF2X& a, const GF2XModulus& F)
{
   NTL_GF2XRegister(P1);
   NTL_GF2XRegister(P2);

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   add(P2, P2, P1);
   mul(P1, P2, F.f0);
   trunc(P1, P1, F.n);
   trunc(r, a, F.n);
   add(r, r, P1);
   q = P2;
}

void UseMulDiv21(GF2X& q, const GF2X& a, const GF2XModulus& F)
{
   NTL_GF2XRegister(P1);
   NTL_GF2XRegister(P2);

   RightShift(P1, a, F.n);
   mul(P2, P1, F.h0);
   RightShift(P2, P2, F.n-2);
   add(P2, P2, P1);
   q = P2;
}


void UseMulRemX1(GF2X& r, const GF2X& aa, const GF2XModulus& F)
{
   NTL_GF2XRegister(buf);
   NTL_GF2XRegister(tmp);
   NTL_GF2XRegister(a);

   clear(buf);
   a = aa;

   long n = F.n;
   long a_len = deg(a) + 1;

   while (a_len > 0) {
      long old_buf_len = deg(buf) + 1;
      long amt = min(2*n-1-old_buf_len, a_len);

      LeftShift(buf, buf, amt);
      RightShift(tmp, a, a_len-amt);
      add(buf, buf, tmp);
      trunc(a, a, a_len-amt);

      UseMulRem21(buf, buf, F);
      a_len -= amt;
   }

   r = buf;
}
   

void UseMulDivRemX1(GF2X& q, GF2X& r, const GF2X& aa, const GF2XModulus& F)
{
   NTL_GF2XRegister(buf);
   NTL_GF2XRegister(tmp);
   NTL_GF2XRegister(a);
   NTL_GF2XRegister(qq);
   NTL_GF2XRegister(qbuf);

   clear(buf);
   a = aa;
   clear(qq);

   long n = F.n;
   long a_len = deg(a) + 1;

   while (a_len > 0) {
      long old_buf_len = deg(buf) + 1;
      long amt = min(2*n-1-old_buf_len, a_len);

      LeftShift(buf, buf, amt);
      RightShift(tmp, a, a_len-amt);
      add(buf, buf, tmp);
      trunc(a, a, a_len-amt);

      UseMulDivRem21(qbuf, buf, buf, F);
      a_len -= amt;

      ShiftAdd(qq, qbuf, a_len);
   }

   r = buf;
   q = qq;
}


void UseMulDivX1(GF2X& q, const GF2X& aa, const GF2XModulus& F)
{
   NTL_GF2XRegister(buf);
   NTL_GF2XRegister(tmp);
   NTL_GF2XRegister(a);
   NTL_GF2XRegister(qq);
   NTL_GF2XRegister(qbuf);
   
   clear(buf);
   a = aa;
   clear(qq);

   long n = F.n;
   long a_len = deg(a) + 1;

   while (a_len > 0) {
      long old_buf_len = deg(buf) + 1;
      long amt = min(2*n-1-old_buf_len, a_len);

      LeftShift(buf, buf, amt);
      RightShift(tmp, a, a_len-amt);
      add(buf, buf, tmp);
      trunc(a, a, a_len-amt);

      UseMulDivRem21(qbuf, buf, buf, F);
      a_len -= amt;

      ShiftAdd(qq, qbuf, a_len);
   }

   q = qq;
}

static
void TrinomReduce(GF2X& x, const GF2X& a, long n, long k)
{
   long wn = n / NTL_BITS_PER_LONG;
   long bn = n - wn*NTL_BITS_PER_LONG;

   long wdiff = (n-k)/NTL_BITS_PER_LONG;
   long bdiff = (n-k) - wdiff*NTL_BITS_PER_LONG;

   long m = a.xrep.length()-1;

   if (wn > m) {
      x = a;
      return;
   }

   NTL_GF2XRegister(r);

   r = a;

   _ntl_ulong *p = r.xrep.elts();

   _ntl_ulong *pp;


   _ntl_ulong w;

   if (bn == 0) {
      if (bdiff == 0) {
         // bn == 0 && bdiff == 0

         while (m >= wn) {
            w = p[m];
            p[m-wdiff] ^= w;
            p[m-wn] ^= w;
            m--;
         }
      }
      else {
         // bn == 0 && bdiff != 0

         while (m >= wn) {
            w = p[m];
            pp = &p[m-wdiff];
            *pp ^= (w >> bdiff);
            *(pp-1) ^= (w << (NTL_BITS_PER_LONG-bdiff));
            p[m-wn] ^= w;
            m--;
         }
      }
   }
   else {
      if (bdiff == 0) {
         // bn != 0 && bdiff == 0

         while (m > wn) {
            w = p[m];
            p[m-wdiff] ^= w;
            pp = &p[m-wn];
            *pp ^= (w >> bn);
            *(pp-1) ^= (w << (NTL_BITS_PER_LONG-bn));
            m--;
         }

         w = (p[m] >> bn) << bn;;

         p[m-wdiff] ^= w;
         p[0] ^= (w >> bn);

         p[m] &= ((1UL<<bn)-1UL); 
      }
      else {
         // bn != 0 && bdiff != 0

         while (m > wn) {
            w = p[m];
            pp = &p[m-wdiff];
            *pp ^= (w >> bdiff);;
            *(pp-1) ^= (w << (NTL_BITS_PER_LONG-bdiff));
            pp = &p[m-wn];
            *pp ^= (w >> bn);
            *(pp-1) ^= (w << (NTL_BITS_PER_LONG-bn));
            m--;
         }

         w = (p[m] >> bn) << bn;;

         p[m-wdiff] ^= (w >> bdiff);
         if (m-wdiff-1 >= 0) p[m-wdiff-1] ^= (w << (NTL_BITS_PER_LONG-bdiff));
         p[0] ^= (w >> bn);
         p[m] &= ((1UL<<bn)-1UL); 
      }
   }

   if (bn == 0)
      wn--;

   while (wn >= 0 && p[wn] == 0)
      wn--;

   r.xrep.QuickSetLength(wn+1);

   x = r;
}

static
void PentReduce(GF2X& x, const GF2X& a, long n, long k3, long k2, long k1)
{
   long wn = n / NTL_BITS_PER_LONG;
   long bn = n - wn*NTL_BITS_PER_LONG;

   long m = a.xrep.length()-1;

   if (wn > m) {
      x = a;
      return;
   }

   long wdiff1 = (n-k1)/NTL_BITS_PER_LONG;
   long bdiff1 = (n-k1) - wdiff1*NTL_BITS_PER_LONG;

   long wdiff2 = (n-k2)/NTL_BITS_PER_LONG;
   long bdiff2 = (n-k2) - wdiff2*NTL_BITS_PER_LONG;

   long wdiff3 = (n-k3)/NTL_BITS_PER_LONG;
   long bdiff3 = (n-k3) - wdiff3*NTL_BITS_PER_LONG;

   NTL_GF2XRegister(r);
   r = a;

   _ntl_ulong *p = r.xrep.elts();

   _ntl_ulong *pp;

   _ntl_ulong w;

   while (m > wn) {
      w = p[m];

      if (bn == 0) 
         p[m-wn] ^= w;
      else {
         pp = &p[m-wn];
         *pp ^= (w >> bn);
         *(pp-1) ^= (w << (NTL_BITS_PER_LONG-bn));
      }

      if (bdiff1 == 0) 
         p[m-wdiff1] ^= w;
      else {
         pp = &p[m-wdiff1];
         *pp ^= (w >> bdiff1);
         *(pp-1) ^= (w << (NTL_BITS_PER_LONG-bdiff1));
      }

      if (bdiff2 == 0) 
         p[m-wdiff2] ^= w;
      else {
         pp = &p[m-wdiff2];
         *pp ^= (w >> bdiff2);
         *(pp-1) ^= (w << (NTL_BITS_PER_LONG-bdiff2));
      }

      if (bdiff3 == 0) 
         p[m-wdiff3] ^= w;
      else {
         pp = &p[m-wdiff3];
         *pp ^= (w >> bdiff3);
         *(pp-1) ^= (w << (NTL_BITS_PER_LONG-bdiff3));
      }

      m--;
   }

   w = (p[m] >> bn) << bn;

   p[0] ^= (w >> bn); 

   if (bdiff1 == 0)
      p[m-wdiff1] ^= w;
   else {
      p[m-wdiff1] ^= (w >> bdiff1);
      if (m-wdiff1-1 >= 0) p[m-wdiff1-1] ^= (w << (NTL_BITS_PER_LONG-bdiff1));
   }

   if (bdiff2 == 0)
      p[m-wdiff2] ^= w;
   else {
      p[m-wdiff2] ^= (w >> bdiff2);
      if (m-wdiff2-1 >= 0) p[m-wdiff2-1] ^= (w << (NTL_BITS_PER_LONG-bdiff2));
   }

   if (bdiff3 == 0)
      p[m-wdiff3] ^= w;
   else {
      p[m-wdiff3] ^= (w >> bdiff3);
      if (m-wdiff3-1 >= 0) p[m-wdiff3-1] ^= (w << (NTL_BITS_PER_LONG-bdiff3));
   }

   if (bn != 0)
      p[m] &= ((1UL<<bn)-1UL);

   
   if (bn == 0)
      wn--;

   while (wn >= 0 && p[wn] == 0)
      wn--;

   r.xrep.QuickSetLength(wn+1);

   x = r;
}




static
void RightShiftAdd(GF2X& c, const GF2X& a, long n)
{
   if (n < 0) {
      LogicError("RightShiftAdd: negative shamt");
   }

   if (n == 0) {
      add(c, c, a);
      return;
   }

   long sa = a.xrep.length();
   long wn = n/NTL_BITS_PER_LONG;
   long bn = n - wn*NTL_BITS_PER_LONG;

   if (wn >= sa) {
      return;
   }

   long sc = c.xrep.length();
   long i;

   if (sa-wn > sc)
      c.xrep.SetLength(sa-wn);

   _ntl_ulong *cp = c.xrep.elts();
   const _ntl_ulong *ap = a.xrep.elts();

   for (i = sc; i < sa-wn; i++)
      cp[i] = 0;


   if (bn == 0) {
      for (i = 0; i < sa-wn; i++)
         cp[i] ^= ap[i+wn];
   }
   else {
      for (i = 0; i < sa-wn-1; i++)
         cp[i] ^= (ap[i+wn] >> bn) | (ap[i+wn+1] << (NTL_BITS_PER_LONG - bn));

      cp[sa-wn-1] ^= ap[sa-1] >> bn;
   }

   c.normalize();
}


static
void TriDiv21(GF2X& q, const GF2X& a, long n, long k)
{
   NTL_GF2XRegister(P1);

   RightShift(P1, a, n);
   if (k != 1) 
      RightShiftAdd(P1, P1, n-k);

   q = P1;
}

static 
void TriDivRem21(GF2X& q, GF2X& r, const GF2X& a, long n, long k)
{
   NTL_GF2XRegister(Q);
   TriDiv21(Q, a, n, k);
   TrinomReduce(r, a, n, k);
   q = Q;
}


static
void PentDiv21(GF2X& q, const GF2X& a, long n, long k3, long k2, long k1)
{
   if (deg(a) < n) {
      clear(q);
      return;
   }

   NTL_GF2XRegister(P1);
   NTL_GF2XRegister(P2);

   RightShift(P1, a, n);
   
   RightShift(P2, P1, n-k3);
   RightShiftAdd(P2, P1, n-k2);
   if (k1 != 1) {
      RightShiftAdd(P2, P1, n-k1);
   }

   add(P2, P2, P1);

   q = P2;
}

static 
void PentDivRem21(GF2X& q, GF2X& r, const GF2X& a, long n, 
                  long k3, long k2, long k1)
{
   NTL_GF2XRegister(Q);
   PentDiv21(Q, a, n, k3, k2, k1);
   PentReduce(r, a, n, k3, k2, k1);
   q = Q;
}

static
void TriDivRemX1(GF2X& q, GF2X& r, const GF2X& aa, long n, long k)
{
   NTL_GF2XRegister(buf);
   NTL_GF2XRegister(tmp);
   NTL_GF2XRegister(a);
   NTL_GF2XRegister(qq);
   NTL_GF2XRegister(qbuf);

   clear(buf);
   a = aa;
   clear(qq);

   long a_len = deg(a) + 1;

   while (a_len > 0) {
      long old_buf_len = deg(buf) + 1;
      long amt = min(2*n-1-old_buf_len, a_len);

      LeftShift(buf, buf, amt);
      RightShift(tmp, a, a_len-amt);
      add(buf, buf, tmp);
      trunc(a, a, a_len-amt);

      TriDivRem21(qbuf, buf, buf, n, k);
      a_len -= amt;

      ShiftAdd(qq, qbuf, a_len);
   }

   r = buf;
   q = qq;
}


static
void TriDivX1(GF2X& q, const GF2X& aa, long n, long k)
{
   NTL_GF2XRegister(buf);
   NTL_GF2XRegister(tmp);
   NTL_GF2XRegister(a);
   NTL_GF2XRegister(qq);
   NTL_GF2XRegister(qbuf);
   
   clear(buf);
   a = aa;
   clear(qq);

   long a_len = deg(a) + 1;

   while (a_len > 0) {
      long old_buf_len = deg(buf) + 1;
      long amt = min(2*n-1-old_buf_len, a_len);

      LeftShift(buf, buf, amt);
      RightShift(tmp, a, a_len-amt);
      add(buf, buf, tmp);
      trunc(a, a, a_len-amt);

      TriDivRem21(qbuf, buf, buf, n, k);
      a_len -= amt;

      ShiftAdd(qq, qbuf, a_len);
   }

   q = qq;
}

static
void PentDivRemX1(GF2X& q, GF2X& r, const GF2X& aa, long n, 
                  long k3, long k2, long k1)
{
   NTL_GF2XRegister(buf);
   NTL_GF2XRegister(tmp);
   NTL_GF2XRegister(a);
   NTL_GF2XRegister(qq);
   NTL_GF2XRegister(qbuf);

   clear(buf);
   a = aa;
   clear(qq);

   long a_len = deg(a) + 1;

   while (a_len > 0) {
      long old_buf_len = deg(buf) + 1;
      long amt = min(2*n-1-old_buf_len, a_len);

      LeftShift(buf, buf, amt);
      RightShift(tmp, a, a_len-amt);
      add(buf, buf, tmp);
      trunc(a, a, a_len-amt);

      PentDivRem21(qbuf, buf, buf, n, k3, k2, k1);
      a_len -= amt;

      ShiftAdd(qq, qbuf, a_len);
   }

   r = buf;
   q = qq;
}


static
void PentDivX1(GF2X& q, const GF2X& aa, long n, long k3, long k2, long k1)
{
   NTL_GF2XRegister(buf);
   NTL_GF2XRegister(tmp);
   NTL_GF2XRegister(a);
   NTL_GF2XRegister(qq);
   NTL_GF2XRegister(qbuf);
   
   clear(buf);
   a = aa;
   clear(qq);

   long a_len = deg(a) + 1;

   while (a_len > 0) {
      long old_buf_len = deg(buf) + 1;
      long amt = min(2*n-1-old_buf_len, a_len);

      LeftShift(buf, buf, amt);
      RightShift(tmp, a, a_len-amt);
      add(buf, buf, tmp);
      trunc(a, a, a_len-amt);

      PentDivRem21(qbuf, buf, buf, n, k3, k2, k1);
      a_len -= amt;

      ShiftAdd(qq, qbuf, a_len);
   }

   q = qq;
}



void rem(GF2X& r, const GF2X& a, const GF2XModulus& F)
{
   long n = F.n;

   if (n < 0) LogicError("rem: uninitialized modulus");

   if (F.method == GF2X_MOD_TRI) {
      TrinomReduce(r, a, n, F.k3);
      return;
   }

   if (F.method == GF2X_MOD_PENT) {
      PentReduce(r, a, n, F.k3, F.k2, F.k1);
      return;
   }

   long da = deg(a);


   if (da < n) {
      r = a;
   }
   else if (F.method == GF2X_MOD_MUL) {
      if (da <= 2*(n-1)) 
         UseMulRem21(r, a, F);
      else
         UseMulRemX1(r, a, F);
   }
   else if (F.method == GF2X_MOD_SPECIAL) {
      NTL_TLS_GLOBAL_ACCESS(GF2X_rembuf);
      WordVectorWatcher watch_GF2X_rembuf(GF2X_rembuf);

      long sa = a.xrep.length();
      long posa = da - NTL_BITS_PER_LONG*(sa-1);
   
      _ntl_ulong *ap;
      if (&r == &a)
         ap = r.xrep.elts();
      else {
         GF2X_rembuf = a.xrep;
         ap = GF2X_rembuf.elts();
      }
   
      _ntl_ulong *atop = &ap[sa-1];
      _ntl_ulong *stab_top;
   
      long i;
   
      while (1) {
         if (atop[0] & (1UL << posa)) {
            stab_top = &F.stab1[posa << 1];
            i = F.stab_cnt[posa];
            atop[i] ^= stab_top[0];
            atop[i+1] ^= stab_top[1];
         }

         da--;
         if (da < n) break;

         posa--;
         if (posa < 0) {
            posa = NTL_BITS_PER_LONG-1;
            atop--;
         }
      }
   
      long sn = F.size;
      r.xrep.SetLength(sn);
      if (&r != &a) {
         _ntl_ulong *rp = r.xrep.elts();
         for (i = 0; i < sn; i++)
            rp[i] = ap[i];
      }
      r.xrep[sn-1] &= F.msk;
      r.normalize();
   }
   else {
      NTL_TLS_GLOBAL_ACCESS(GF2X_rembuf);
      WordVectorWatcher watch_GF2X_rembuf(GF2X_rembuf);

      long sa = a.xrep.length();
      long posa = da - NTL_BITS_PER_LONG*(sa-1);
   
      _ntl_ulong *ap;
      if (&r == &a)
         ap = r.xrep.elts();
      else {
         GF2X_rembuf = a.xrep;
         ap = GF2X_rembuf.elts();
      }
   
      _ntl_ulong *atop = &ap[sa-1];
      _ntl_ulong *stab_top;
   
      long i;
   
      while (1) {
         if (atop[0] & (1UL << posa)) {
            stab_top = F.stab_ptr[posa];
            for (i = F.stab_cnt[posa]; i <= 0; i++)
               atop[i] ^= stab_top[i];
         }
   
         da--;
         if (da < n) break;

         posa--;
         if (posa < 0) {
            posa = NTL_BITS_PER_LONG-1;
            atop--;
         }
      }
   
      long sn = F.size;
      r.xrep.SetLength(sn);
      if (&r != &a) {
         _ntl_ulong *rp = r.xrep.elts();
         for (i = 0; i < sn; i++)
            rp[i] = ap[i];
      }
      r.normalize();
   }

}

void DivRem(GF2X& q, GF2X& r, const GF2X& a, const GF2XModulus& F)
{
   long da = deg(a);
   long n = F.n;

   if (n < 0) LogicError("DivRem: uninitialized modulus");

   if (da < n) {
      r = a;
      clear(q);
   }
   else if (F.method == GF2X_MOD_TRI) {
      if (da <= 2*(n-1)) 
         TriDivRem21(q, r, a, F.n, F.k3);
      else
         TriDivRemX1(q, r, a, F.n, F.k3);
   }
   else if (F.method == GF2X_MOD_PENT) {
      if (da <= 2*(n-1)) 
         PentDivRem21(q, r, a, F.n, F.k3, F.k2, F.k1);
      else
         PentDivRemX1(q, r, a, F.n, F.k3, F.k2, F.k1);
   }
   else if (F.method == GF2X_MOD_MUL) {
      if (da <= 2*(n-1)) 
         UseMulDivRem21(q, r, a, F);
      else
         UseMulDivRemX1(q, r, a, F);
   }
   else if (F.method == GF2X_MOD_SPECIAL) {
      NTL_TLS_GLOBAL_ACCESS(GF2X_rembuf);
      WordVectorWatcher watch_GF2X_rembuf(GF2X_rembuf);

      long sa = a.xrep.length();
      long posa = da - NTL_BITS_PER_LONG*(sa-1);
   
      long dq = da - n;
      long sq = dq/NTL_BITS_PER_LONG + 1;
      long posq = dq - NTL_BITS_PER_LONG*(sq-1);
   
      _ntl_ulong *ap;
      if (&r == &a)
         ap = r.xrep.elts();
      else {
         GF2X_rembuf = a.xrep;
         ap = GF2X_rembuf.elts();
      }
   
      _ntl_ulong *atop = &ap[sa-1];
      _ntl_ulong *stab_top;

      long i;

      q.xrep.SetLength(sq);
      _ntl_ulong *qp = q.xrep.elts();
      for (i = 0; i < sq; i++)
         qp[i] = 0;

      _ntl_ulong *qtop = &qp[sq-1];

   
      while (1) {
         if (atop[0] & (1UL << posa)) {
            qtop[0] |= (1UL << posq);
            stab_top = &F.stab1[posa << 1];
            i = F.stab_cnt[posa];
            atop[i] ^= stab_top[0];
            atop[i+1] ^= stab_top[1];
         }
   
         da--;
         if (da < n) break;

         posa--;
         if (posa < 0) {
            posa = NTL_BITS_PER_LONG-1;
            atop--;
         }

         posq--;
         if (posq < 0) {
            posq = NTL_BITS_PER_LONG-1;
            qtop--;
         }
      }
   
      long sn = F.size;
      r.xrep.SetLength(sn);
      if (&r != &a) {
         _ntl_ulong *rp = r.xrep.elts();
         for (i = 0; i < sn; i++)
            rp[i] = ap[i];
      }
      r.xrep[sn-1] &= F.msk;
      r.normalize();
   }
   else {
      NTL_TLS_GLOBAL_ACCESS(GF2X_rembuf);
      WordVectorWatcher watch_GF2X_rembuf(GF2X_rembuf);

      long sa = a.xrep.length();
      long posa = da - NTL_BITS_PER_LONG*(sa-1);
   
      long dq = da - n;
      long sq = dq/NTL_BITS_PER_LONG + 1;
      long posq = dq - NTL_BITS_PER_LONG*(sq-1);
   
      _ntl_ulong *ap;
      if (&r == &a)
         ap = r.xrep.elts();
      else {
         GF2X_rembuf = a.xrep;
         ap = GF2X_rembuf.elts();
      }
   
      _ntl_ulong *atop = &ap[sa-1];
      _ntl_ulong *stab_top;
   
      long i;

      q.xrep.SetLength(sq);
      _ntl_ulong *qp = q.xrep.elts();
      for (i = 0; i < sq; i++)
         qp[i] = 0;

      _ntl_ulong *qtop = &qp[sq-1];
   
      while (1) {
         if (atop[0] & (1UL << posa)) {
            qtop[0] |= (1UL << posq);
            stab_top = F.stab_ptr[posa];
            for (i = F.stab_cnt[posa]; i <= 0; i++)
               atop[i] ^= stab_top[i];
         }
   
         da--;
         if (da < n) break;

         posa--;
         if (posa < 0) {
            posa = NTL_BITS_PER_LONG-1;
            atop--;
         }

         posq--;
         if (posq < 0) {
            posq = NTL_BITS_PER_LONG-1;
            qtop--;
         }
      }
   
      long sn = F.size;
      r.xrep.SetLength(sn);
      if (&r != &a) {
         _ntl_ulong *rp = r.xrep.elts();
         for (i = 0; i < sn; i++)
            rp[i] = ap[i];
      }
      r.normalize();
   }
}



void div(GF2X& q, const GF2X& a, const GF2XModulus& F)
{
   long da = deg(a);
   long n = F.n;

   if (n < 0) LogicError("div: uninitialized modulus");


   if (da < n) {
      clear(q);
   }
   else if (F.method == GF2X_MOD_TRI) {
      if (da <= 2*(n-1)) 
         TriDiv21(q, a, F.n, F.k3);
      else
         TriDivX1(q, a, F.n, F.k3);
   }
   else if (F.method == GF2X_MOD_PENT) {
      if (da <= 2*(n-1)) 
         PentDiv21(q, a, F.n, F.k3, F.k2, F.k1);
      else
         PentDivX1(q, a, F.n, F.k3, F.k2, F.k1);
   }
   else if (F.method == GF2X_MOD_MUL) {
      if (da <= 2*(n-1)) 
         UseMulDiv21(q, a, F);
      else
         UseMulDivX1(q, a, F);
   }
   else if (F.method == GF2X_MOD_SPECIAL) {
      NTL_TLS_GLOBAL_ACCESS(GF2X_rembuf);
      WordVectorWatcher watch_GF2X_rembuf(GF2X_rembuf);

      long sa = a.xrep.length();
      long posa = da - NTL_BITS_PER_LONG*(sa-1);
   
      long dq = da - n;
      long sq = dq/NTL_BITS_PER_LONG + 1;
      long posq = dq - NTL_BITS_PER_LONG*(sq-1);
   
      _ntl_ulong *ap;
      GF2X_rembuf = a.xrep;
      ap = GF2X_rembuf.elts();

   
      _ntl_ulong *atop = &ap[sa-1];
      _ntl_ulong *stab_top;

      long i;

      q.xrep.SetLength(sq);
      _ntl_ulong *qp = q.xrep.elts();
      for (i = 0; i < sq; i++)
         qp[i] = 0;

      _ntl_ulong *qtop = &qp[sq-1];
   
      while (1) {
         if (atop[0] & (1UL << posa)) {
            qtop[0] |= (1UL << posq);
            stab_top = &F.stab1[posa << 1];
            i = F.stab_cnt[posa];
            atop[i] ^= stab_top[0];
            atop[i+1] ^= stab_top[1];
         }
   
         da--;
         if (da < n) break;

         posa--;
         if (posa < 0) {
            posa = NTL_BITS_PER_LONG-1;
            atop--;
         }

         posq--;
         if (posq < 0) {
            posq = NTL_BITS_PER_LONG-1;
            qtop--;
         }
      }
   }
   else {
      NTL_TLS_GLOBAL_ACCESS(GF2X_rembuf);
      WordVectorWatcher watch_GF2X_rembuf(GF2X_rembuf);

      long sa = a.xrep.length();
      long posa = da - NTL_BITS_PER_LONG*(sa-1);
   
      long dq = da - n;
      long sq = dq/NTL_BITS_PER_LONG + 1;
      long posq = dq - NTL_BITS_PER_LONG*(sq-1);
   
      _ntl_ulong *ap;
      GF2X_rembuf = a.xrep;
      ap = GF2X_rembuf.elts();
   
      _ntl_ulong *atop = &ap[sa-1];
      _ntl_ulong *stab_top;
   
      long i;

      q.xrep.SetLength(sq);
      _ntl_ulong *qp = q.xrep.elts();
      for (i = 0; i < sq; i++)
         qp[i] = 0;

      _ntl_ulong *qtop = &qp[sq-1];
   
      while (1) {
         if (atop[0] & (1UL << posa)) {
            qtop[0] |= (1UL << posq);
            stab_top = F.stab_ptr[posa];
            for (i = F.stab_cnt[posa]; i <= 0; i++)
               atop[i] ^= stab_top[i];
         }
   
         da--;
         if (da < n) break;

         posa--;
         if (posa < 0) {
            posa = NTL_BITS_PER_LONG-1;
            atop--;
         }

         posq--;
         if (posq < 0) {
            posq = NTL_BITS_PER_LONG-1;
            qtop--;
         }
      }
   }
}


void MulMod(GF2X& c, const GF2X& a, const GF2X& b, const GF2XModulus& F)
{
   if (F.n < 0) LogicError("MulMod: uninitialized modulus");

   NTL_GF2XRegister(t);
   mul(t, a, b);
   rem(c, t, F);
}


void SqrMod(GF2X& c, const GF2X& a, const GF2XModulus& F)
{
   if (F.n < 0) LogicError("SqrMod: uninitialized modulus");

   NTL_GF2XRegister(t);
   sqr(t, a);
   rem(c, t, F);
}


// we need these two versions to prevent a GF2XModulus
// from being constructed.


void MulMod(GF2X& c, const GF2X& a, const GF2X& b, const GF2X& f)
{
   NTL_GF2XRegister(t);
   mul(t, a, b);
   rem(c, t, f);
}

void SqrMod(GF2X& c, const GF2X& a, const GF2X& f)
{
   NTL_GF2XRegister(t);
   sqr(t, a);
   rem(c, t, f);
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
      


void PowerMod(GF2X& h, const GF2X& g, const ZZ& e, const GF2XModulus& F)
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

   GF2X res;
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

   k = min(k, 9);

   vec_GF2X v;

   v.SetLength(1L << (k-1));

   v[0] = g;
 
   if (k > 1) {
      GF2X t;
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

   


void PowerXMod(GF2X& hh, const ZZ& e, const GF2XModulus& F)
{
   if (F.n < 0) LogicError("PowerXMod: uninitialized modulus");

   if (IsZero(e)) {
      set(hh);
      return;
   }

   long n = NumBits(e);
   long i;

   GF2X h;

   h.SetMaxLength(F.n+1);
   set(h);

   for (i = n - 1; i >= 0; i--) {
      SqrMod(h, h, F);
      if (bit(e, i)) {
         MulByX(h, h);
         if (coeff(h, F.n) != 0)
            add(h, h, F.f);
      }
   }

   if (e < 0) InvMod(h, h, F);

   hh = h;
}


      


void UseMulRem(GF2X& r, const GF2X& a, const GF2X& b)
{
   NTL_GF2XRegister(P1);
   NTL_GF2XRegister(P2);

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

void UseMulDivRem(GF2X& q, GF2X& r, const GF2X& a, const GF2X& b)
{
   NTL_GF2XRegister(P1);
   NTL_GF2XRegister(P2);

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

void UseMulDiv(GF2X& q, const GF2X& a, const GF2X& b)
{
   NTL_GF2XRegister(P1);
   NTL_GF2XRegister(P2);

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


const long GF2X_DIV_CROSS = (NTL_BITS_PER_LONG/2)*XOVER_SCALE; 

void DivRem(GF2X& q, GF2X& r, const GF2X& a, const GF2X& b)
{
   long da = deg(a);
   long db = deg(b);

   if (db < GF2X_DIV_CROSS || da-db < GF2X_DIV_CROSS)
      PlainDivRem(q, r, a, b);
   else if (da < 4*db)
      UseMulDivRem(q, r, a, b);
   else {
      GF2XModulus B;
      build(B, b);
      DivRem(q, r, a, B);
   }
}

void div(GF2X& q, const GF2X& a, const GF2X& b)
{
   long da = deg(a);
   long db = deg(b);

   if (db < GF2X_DIV_CROSS || da-db < GF2X_DIV_CROSS)
      PlainDiv(q, a, b);
   else if (da < 4*db)
      UseMulDiv(q, a, b);
   else {
      GF2XModulus B;
      build(B, b);
      div(q, a, B);
   }
}

void rem(GF2X& r, const GF2X& a, const GF2X& b)
{
   long da = deg(a);
   long db = deg(b);

   if (db < GF2X_DIV_CROSS || da-db < GF2X_DIV_CROSS)
      PlainRem(r, a, b);
   else if (da < 4*db)
      UseMulRem(r, a, b);
   else {
      GF2XModulus B;
      build(B, b);
      rem(r, a, B);
   }
}


static inline 
void swap(_ntl_ulong_ptr& a, _ntl_ulong_ptr& b)  
{  _ntl_ulong_ptr t;  t = a; a = b; b = t; }




static
void BaseGCD(GF2X& d, const GF2X& a_in, const GF2X& b_in)
{
   NTL_GF2XRegister(a);
   NTL_GF2XRegister(b);
   
   if (IsZero(a_in)) {
      d = b_in;
      return;
   }

   if (IsZero(b_in)) {
      d = a_in;
      return;
   }
      
   a.xrep.SetMaxLength(a_in.xrep.length()+1);
   b.xrep.SetMaxLength(b_in.xrep.length()+1);

   a = a_in;
   b = b_in;

   _ntl_ulong *ap = a.xrep.elts();
   _ntl_ulong *bp = b.xrep.elts();

   long da = deg(a);
   long wa = da/NTL_BITS_PER_LONG;
   long ba = da - wa*NTL_BITS_PER_LONG;

   long db = deg(b);
   long wb = db/NTL_BITS_PER_LONG;
   long bb = db - wb*NTL_BITS_PER_LONG;

   long parity = 0;

   for (;;) {
      if (da < db) {
         swap(ap, bp);
         swap(da, db);
         swap(wa, wb);
         swap(ba, bb);
         parity = 1 - parity;
      }

      // da >= db

      if (db == -1) break;

      ShiftAdd(ap, bp, wb+1, da-db);

      _ntl_ulong msk = 1UL << ba;
      _ntl_ulong aa = ap[wa];

      while ((aa & msk) == 0) {
         da--;
         msk = msk >> 1;
         ba--;
         if (!msk) {
            wa--;
            ba = NTL_BITS_PER_LONG-1;
            msk = 1UL << (NTL_BITS_PER_LONG-1);
            if (wa < 0) break;
            aa = ap[wa];
         }
      }
   }

   a.normalize();
   b.normalize();

   if (!parity) {
      d = a;
   }
   else {
      d = b;
   }
}


void OldGCD(GF2X& d, const GF2X& a, const GF2X& b)
{
   long sa = a.xrep.length();
   long sb = b.xrep.length();

   if (sb >= 10 && 2*sa > 3*sb) {
      NTL_GF2XRegister(r);

      rem(r, a, b);
      BaseGCD(d, b, r);
   }
   else if (sa >= 10 && 2*sb > 3*sa) {
      NTL_GF2XRegister(r);

      rem(r, b, a);
      BaseGCD(d, a, r);
   }
   else {
      BaseGCD(d, a, b);
   }
}





#define XX_STEP(ap,da,wa,ba,rp,sr,bp,db,wb,bb,sp,ss)  \
      long delta = da-db;  \
  \
      if (delta == 0) {  \
         long i;  \
         for (i = wb; i >= 0; i--) ap[i] ^= bp[i];  \
         for (i = ss-1; i >= 0; i--) rp[i] ^= sp[i];  \
         if (ss > sr) sr = ss; \
      }  \
      else if (delta == 1) {  \
         long i; \
         _ntl_ulong tt, tt1;  \
  \
         tt = bp[wb] >> (NTL_BITS_PER_LONG-1);  \
         if (tt) ap[wb+1] ^= tt;  \
         tt = bp[wb];  \
         for (i = wb; i >= 1; i--)  \
            tt1 = bp[i-1], ap[i] ^= (tt << 1) | (tt1 >> (NTL_BITS_PER_LONG-1)),  \
            tt = tt1; \
         ap[0] ^= tt << 1;  \
  \
         if (ss > 0) {  \
            long t = ss; \
            tt = sp[ss-1] >> (NTL_BITS_PER_LONG-1);  \
            if (tt) rp[ss] ^= tt, t++;  \
            tt = sp[ss-1]; \
            for (i = ss-1; i >= 1; i--)  \
               tt1=sp[i-1],  \
               rp[i] ^= (tt << 1) | (tt1 >> (NTL_BITS_PER_LONG-1)),  \
               tt = tt1; \
            rp[0] ^= tt << 1;  \
            if (t > sr) sr = t; \
         }  \
      }  \
      else if (delta < NTL_BITS_PER_LONG) {  \
         long i; \
         _ntl_ulong tt, tt1;  \
         long rdelta = NTL_BITS_PER_LONG-delta; \
  \
         tt = bp[wb] >> rdelta;  \
         if (tt) ap[wb+1] ^= tt;  \
         tt=bp[wb]; \
         for (i = wb; i >= 1; i--)  \
            tt1=bp[i-1], ap[i] ^= (tt << delta) | (tt1 >> rdelta),  \
            tt=tt1; \
         ap[0] ^= tt << delta;  \
  \
         if (ss > 0) {  \
            long t = ss; \
            tt = sp[ss-1] >> rdelta;  \
            if (tt) rp[ss] ^= tt, t++;  \
            tt=sp[ss-1]; \
            for (i = ss-1; i >= 1; i--)  \
               tt1=sp[i-1], rp[i] ^= (tt << delta) | (tt1 >> rdelta),  \
               tt=tt1; \
            rp[0] ^= tt << delta;  \
            if (t > sr) sr = t; \
         }  \
      }  \
      else {  \
         ShiftAdd(ap, bp, wb+1, da-db);  \
         ShiftAdd(rp, sp, ss, da-db);  \
         long t = ss + (da-db+NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG;  \
         if (t > sr) {  \
            while (t > 0 && rp[t-1] == 0) t--;   \
            sr = t;  \
         }  \
      } \
  \
      _ntl_ulong msk = 1UL << ba;  \
      _ntl_ulong aa = ap[wa];  \
  \
      while ((aa & msk) == 0) {  \
         da--;  \
         msk = msk >> 1;  \
         ba--;  \
         if (!msk) {  \
            wa--;  \
            ba = NTL_BITS_PER_LONG-1;  \
            msk = 1UL << (NTL_BITS_PER_LONG-1);  \
            if (wa < 0) break;  \
            aa = ap[wa];  \
         }  \
      }  \




static
void XXGCD(GF2X& d, GF2X& r_out, const GF2X& a_in, const GF2X& b_in)
{
   NTL_GF2XRegister(a);
   NTL_GF2XRegister(b);
   NTL_GF2XRegister(r);
   NTL_GF2XRegister(s);

   if (IsZero(b_in)) {
      d = a_in;
      set(r_out);
      return;
   }

   if (IsZero(a_in)) {
      d = b_in;
      clear(r_out);
      return;
   }
      
   a.xrep.SetMaxLength(a_in.xrep.length()+1);
   b.xrep.SetMaxLength(b_in.xrep.length()+1);

   long max_sz = max(a_in.xrep.length(), b_in.xrep.length());
   r.xrep.SetLength(max_sz+1);
   s.xrep.SetLength(max_sz+1);

   _ntl_ulong *rp = r.xrep.elts();
   _ntl_ulong *sp = s.xrep.elts();

   long i;
   for (i = 0; i <= max_sz; i++) {
      rp[i] = sp[i] = 0;
   }

   rp[0] = 1;

   long sr = 1;
   long ss = 0;

   a = a_in;
   b = b_in;

   _ntl_ulong *ap = a.xrep.elts();
   _ntl_ulong *bp = b.xrep.elts();

   long da = deg(a);
   long wa = da/NTL_BITS_PER_LONG;
   long ba = da - wa*NTL_BITS_PER_LONG;

   long db = deg(b);
   long wb = db/NTL_BITS_PER_LONG;
   long bb = db - wb*NTL_BITS_PER_LONG;

   long parity = 0;


   for (;;) {
      if (da == -1 || db == -1) break;

      if (da < db || (da == db && parity)) {
         if (da < db && !parity) parity = 1;
         XX_STEP(bp,db,wb,bb,sp,ss,ap,da,wa,ba,rp,sr)

      }
      else {
         parity = 0;
         XX_STEP(ap,da,wa,ba,rp,sr,bp,db,wb,bb,sp,ss)
      }
   }

   a.normalize();
   b.normalize();
   r.normalize();
   s.normalize();

   if (db == -1) {
      d = a;
      r_out = r;
   }
   else {
      d = b;
      r_out = s;
   }
}



static
void BaseXGCD(GF2X& d, GF2X& s, GF2X& t, const GF2X& a, const GF2X& b)
{
   if (IsZero(b)) {
      d = a;
      set(s);
      clear(t);
   }
   else {
      NTL_GF2XRegister(t1);
      NTL_GF2XRegister(b1);

      b1 = b;
      XXGCD(d, s, a, b);
      mul(t1, a, s);
      add(t1, t1, d);
      div(t, t1, b1);
   }
}




void OldXGCD(GF2X& d, GF2X& s, GF2X& t, const GF2X& a, const GF2X& b)
{
   long sa = a.xrep.length();
   long sb = b.xrep.length();


   if (sb >= 10 && 2*sa > 3*sb) {
      NTL_GF2XRegister(r);
      NTL_GF2XRegister(q);
      NTL_GF2XRegister(s1);
      NTL_GF2XRegister(t1);


      DivRem(q, r, a, b);
      BaseXGCD(d, s1, t1, b, r);

      
      mul(r, t1, q);
      add(r, r, s1);  // r = s1 - t1*q, but sign doesn't matter

      s = t1;
      t = r;   
   }
   else if (sa >= 10 && 2*sb > 3*sa) {
      NTL_GF2XRegister(r);
      NTL_GF2XRegister(q);
      NTL_GF2XRegister(s1);
      NTL_GF2XRegister(t1);


      DivRem(q, r, b, a);
      BaseXGCD(d, s1, t1, a, r);

      
      mul(r, t1, q);
      add(r, r, s1);  // r = s1 - t1*q, but sign doesn't matter

      t = t1;
      s = r;  
   }
   else {
      BaseXGCD(d, s, t, a, b);
   }

}




static
void BaseInvMod(GF2X& d, GF2X& s, const GF2X& a, const GF2X& f)
{
   if (deg(a) >= deg(f) || deg(f) == 0) LogicError("InvMod: bad args");

   long sa = a.xrep.length();
   long sf = f.xrep.length();

   if ((sa >= 10 && 2*sf > 3*sa) || 
       sf > NTL_GF2X_GCD_CROSSOVER/NTL_BITS_PER_LONG) {
      NTL_GF2XRegister(t);

      XGCD(d, s, t, a, f);
   }
   else {
      XXGCD(d, s, a, f);
   }

}



void InvMod(GF2X& c, const GF2X& a, const GF2X& f)
{ 
   NTL_GF2XRegister(d);
   NTL_GF2XRegister(s);
   BaseInvMod(d, s, a, f);

   if (!IsOne(d)) InvModError("InvMod: inverse undefined");

   c = s;
}



long InvModStatus(GF2X& c, const GF2X& a, const GF2X& f)
{ 
   NTL_GF2XRegister(d);
   NTL_GF2XRegister(s);
   BaseInvMod(d, s, a, f);

   if (!IsOne(d)) {
      c = d;
      return 1;
   }

   c = s;
   return 0;
}



   
void diff(GF2X& c, const GF2X& a)
{
   RightShift(c, a, 1);
   
   // clear odd coeffs

   long dc = deg(c);
   long i;
   for (i = 1; i <= dc; i += 2)
      SetCoeff(c, i, 0);
}

void conv(GF2X& c, long a)
{
   if (a & 1)
      set(c);
   else
      clear(c);
}

void conv(GF2X& c, GF2 a)
{
   if (a == 1)
      set(c);
   else
      clear(c);
}

void conv(GF2X& x, const vec_GF2& a)
{
   x.xrep = a.rep;
   x.normalize();
}

void conv(vec_GF2& x, const GF2X& a)
{
   VectorCopy(x, a, deg(a)+1);
}



/* additional legacy conversions for v6 conversion regime */

#ifndef NTL_WIZARD_HACK
void conv(GF2X& x, const ZZX& a)
{
   long n = deg(a) + 1;
   long i;

   x.SetLength(n);
   for (i = 0; i < n; i++)
      conv(x[i], a[i]);
   x.normalize();
}

void conv(ZZX& x, const GF2X& a)
{
   long n = deg(a) + 1;
   long i;

   x.rep.SetLength(n);
   for (i = 0; i < n; i++)
      x.rep[i] = rep(coeff(a, i));

   x.normalize();
}
#endif

/* ------------------------------------- */

void VectorCopy(vec_GF2& x, const GF2X& a, long n)
{
   if (n < 0) LogicError("VectorCopy: negative length"); 

   if (NTL_OVERFLOW(n, 1, 0))
      ResourceError("overflow in VectorCopy");

   long wa = a.xrep.length();
   long wx = (n + NTL_BITS_PER_LONG - 1)/NTL_BITS_PER_LONG;

   long wmin = min(wa, wx);

   x.SetLength(n);

   const _ntl_ulong *ap = a.xrep.elts();
   _ntl_ulong *xp = x.rep.elts();

   long i;
   for (i = 0; i < wmin; i++)
      xp[i] = ap[i];

   if (wa < wx) {
      for (i = wa; i < wx; i++)
         xp[i] = 0;
   }
   else {
      long p = n % NTL_BITS_PER_LONG;
      if (p != 0)
         xp[wx-1] &= (1UL << p) - 1UL;
   }
}


void add(GF2X& c, const GF2X& a, long b)
{
   c = a;
   if (b & 1) {
      long n = c.xrep.length();
      if (n == 0) 
         set(c);
      else {
         c.xrep[0] ^= 1;
         if (n == 1 && !c.xrep[0]) c.xrep.SetLength(0);
      }
   }
}

void add(GF2X& c, const GF2X& a, GF2 b)
{
   add(c, a, rep(b));
}


void MulTrunc(GF2X& c, const GF2X& a, const GF2X& b, long n)
{
   NTL_GF2XRegister(t);

   mul(t, a, b);
   trunc(c, t, n);
}

void SqrTrunc(GF2X& c, const GF2X& a, long n)
{
   NTL_GF2XRegister(t);

   sqr(t, a);
   trunc(c, t, n);
}


long divide(GF2X& q, const GF2X& a, const GF2X& b)
{
   if (IsZero(b)) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   NTL_GF2XRegister(lq);
   NTL_GF2XRegister(r);

   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0;
   q = lq;
   return 1;
}

long divide(const GF2X& a, const GF2X& b)
{
   if (IsZero(b)) return IsZero(a);
   NTL_GF2XRegister(r);
   rem(r, a, b);
   if (!IsZero(r)) return 0;
   return 1;
}



/*** modular composition routines and data structures ***/


void InnerProduct(GF2X& x, const GF2X& v, long dv, long low, long high, 
                   const vec_GF2X& H, long n, WordVector& t)
{
   long i, j;

   _ntl_ulong *tp = t.elts();

   for (i = 0; i < n; i++)
      tp[i] = 0;


   long w_low = low/NTL_BITS_PER_LONG;
   long b_low = low - w_low*NTL_BITS_PER_LONG;

   
   const _ntl_ulong *vp = &v.xrep[w_low];
   _ntl_ulong msk = 1UL << b_low;
   _ntl_ulong vv = *vp;

   high = min(high, dv);

   i = low;
   for (;;) {
      if (vv & msk) {
         const WordVector& h = H[i-low].xrep;
         long m = h.length();
         const _ntl_ulong *hp = h.elts();
         for (j = 0; j < m; j++)
            tp[j] ^= hp[j];
      }

      i++;
      if (i > high) break;

      msk = msk << 1;
      if (!msk) {
         msk = 1UL;
         vp++;
         vv = *vp;
      }
   }

   x.xrep = t;
   x.normalize();
}


void CompMod(GF2X& x, const GF2X& g, const GF2XArgument& A, const GF2XModulus& F)
{
   long dg = deg(g);
   if (dg <= 0) {
      x = g;
      return;
   }

   GF2X s, t;
   WordVector scratch(INIT_SIZE, F.size);

   long m = A.H.length() - 1;
   long l = (((dg+1)+m-1)/m) - 1;

   InnerProduct(t, g, dg, l*m, l*m + m - 1, A.H, F.size, scratch);
   for (long i = l-1; i >= 0; i--) {
      InnerProduct(s, g, dg, i*m, i*m + m - 1, A.H, F.size, scratch);
      MulMod(t, t, A.H[m], F);
      add(t, t, s);
   }

   x = t;
}

void build(GF2XArgument& A, const GF2X& h, const GF2XModulus& F, long m)
{
   if (m <= 0 || deg(h) >= F.n) LogicError("build GF2XArgument: bad args");

   if (m > F.n) m = F.n;

   long i;

   A.H.SetLength(m+1);

   set(A.H[0]);
   A.H[1] = h;
   for (i = 2; i <= m; i++) 
      MulMod(A.H[i], A.H[i-1], h, F);
}


void CompMod(GF2X& x, const GF2X& g, const GF2X& h, const GF2XModulus& F)
   // x = g(h) mod f
{
   long m = SqrRoot(deg(g)+1);

   if (m == 0) {
      clear(x);
      return;
   }

   GF2XArgument A;

   build(A, h, F, m);

   CompMod(x, g, A, F);
}




void Comp2Mod(GF2X& x1, GF2X& x2, const GF2X& g1, const GF2X& g2,
              const GF2X& h, const GF2XModulus& F)

{
   long m = SqrRoot(deg(g1) + deg(g2) + 2);

   if (m == 0) {
      clear(x1);
      clear(x2);
      return;
   }

   GF2XArgument A;

   build(A, h, F, m);

   GF2X xx1, xx2;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);

   x1 = xx1;
   x2 = xx2;
}

void Comp3Mod(GF2X& x1, GF2X& x2, GF2X& x3, 
              const GF2X& g1, const GF2X& g2, const GF2X& g3,
              const GF2X& h, const GF2XModulus& F)

{
   long m = SqrRoot(deg(g1) + deg(g2) + deg(g3) + 3);

   if (m == 0) {
      clear(x1);
      clear(x2);
      clear(x3);
      return;
   }

   GF2XArgument A;

   build(A, h, F, m);

   GF2X xx1, xx2, xx3;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);
   CompMod(xx3, g3, A, F);

   x1 = xx1;
   x2 = xx2;
   x3 = xx3;
}



void build(GF2XTransMultiplier& B, const GF2X& b, const GF2XModulus& F)
{
   long db = deg(b);

   if (db >= F.n) LogicError("build TransMultiplier: bad args");

   GF2X t;

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

   if (F.method != GF2X_MOD_TRI && F.method != GF2X_MOD_PENT) {
   
      // The following code optimizes the case when 
      // f = X^n + low degree poly
   
      trunc(t, F.f, F.n);
      d = deg(t);
      if (d < 0)
         B.shamt = 0;
      else
         B.shamt = d;

      CopyReverse(B.f0, t, d);
   }


   if (db < 0)
      B.shamt_b = 0;
   else
      B.shamt_b = db;

   CopyReverse(B.b, b, db);
}

void TransMulMod(GF2X& x, const GF2X& a, const GF2XTransMultiplier& B,
               const GF2XModulus& F)
{
   if (deg(a) >= F.n) LogicError("TransMulMod: bad args");

   NTL_GF2XRegister(t1);
   NTL_GF2XRegister(t2);
   NTL_GF2XRegister(t3);

   mul(t1, a, B.b);
   RightShift(t1, t1, B.shamt_b);

   if (F.method == GF2X_MOD_TRI) {
      RightShift(t2, a, F.k3);
      add(t2, t2, a);
   }
   else if (F.method == GF2X_MOD_PENT) {
      RightShift(t2, a, F.k3);
      RightShift(t3, a, F.k2);
      add(t2, t2, t3);
      RightShift(t3, a, F.k1);
      add(t2, t2, t3);
      add(t2, t2, a);
   }
   else {
      mul(t2, a, B.f0);
      RightShift(t2, t2, B.shamt);
   }

   trunc(t2, t2, F.n-1);

   mul(t2, t2, B.fbi);
   if (B.shamt_fbi > 0) LeftShift(t2, t2, B.shamt_fbi);
   trunc(t2, t2, F.n-1);
   MulByX(t2, t2);

   add(x, t1, t2);
}

void UpdateMap(vec_GF2& x, const vec_GF2& a, const GF2XTransMultiplier& B,
       const GF2XModulus& F)
{
   NTL_GF2XRegister(xx);
   NTL_GF2XRegister(aa);
   conv(aa, a);
   TransMulMod(xx, aa, B, F);
   conv(x, xx);
}
   

void ProjectPowers(GF2X& x, const GF2X& a, long k, const GF2XArgument& H,
                   const GF2XModulus& F)
{
   long n = F.n;

   if (deg(a) >= n || k < 0) 
      LogicError("ProjectPowers: bad args");

   if (NTL_OVERFLOW(k, 1, 0)) 
      ResourceError("ProjectPowers: excessive parameter");

   long m = H.H.length()-1;
   long l = (k+m-1)/m - 1;

   GF2XTransMultiplier M;
   build(M, H.H[m], F);

   GF2X s;
   s = a;

   x.SetMaxLength(k);
   clear(x);

   long i;

   for (i = 0; i <= l; i++) {
      long m1 = min(m, k-i*m);
      for (long j = 0; j < m1; j++)
         SetCoeff(x, i*m+j, InnerProduct(H.H[j].xrep, s.xrep));
      if (i < l)
         TransMulMod(s, s, M, F);
   }
}


void ProjectPowers(vec_GF2& x, const vec_GF2& a, long k, 
                   const GF2XArgument& H, const GF2XModulus& F)
{
   GF2X xx;
   ProjectPowers(xx, to_GF2X(a), k, H, F);
   VectorCopy(x, xx, k);
}


void ProjectPowers(GF2X& x, const GF2X& a, long k, const GF2X& h, 
                   const GF2XModulus& F)
{
   if (deg(a) >= F.n || k < 0) LogicError("ProjectPowers: bad args");

   if (k == 0) {
      clear(x);
      return;
   }

   long m = SqrRoot(k);

   GF2XArgument H;
   build(H, h, F, m);

   ProjectPowers(x, a, k, H, F);
}

void ProjectPowers(vec_GF2& x, const vec_GF2& a, long k, const GF2X& H,
                   const GF2XModulus& F)
{
   GF2X xx;
   ProjectPowers(xx, to_GF2X(a), k, H, F);
   VectorCopy(x, xx, k);
}


void OldMinPolyInternal(GF2X& h, const GF2X& x, long m)
{
   GF2X a, b, r, s;
   GF2X a_in, b_in;

   if (IsZero(x)) {
      set(h);
      return;
   }

   clear(a_in);
   SetCoeff(a_in, 2*m);

   CopyReverse(b_in, x, 2*m-1);
      
   a.xrep.SetMaxLength(a_in.xrep.length()+1);
   b.xrep.SetMaxLength(b_in.xrep.length()+1);

   long max_sz = max(a_in.xrep.length(), b_in.xrep.length());
   r.xrep.SetLength(max_sz+1);
   s.xrep.SetLength(max_sz+1);

   _ntl_ulong *rp = r.xrep.elts();
   _ntl_ulong *sp = s.xrep.elts();

   long i;
   for (i = 0; i <= max_sz; i++) {
      rp[i] = sp[i] = 0;
   }

   sp[0] = 1;

   long sr = 0;
   long ss = 1;

   a = a_in;
   b = b_in;

   _ntl_ulong *ap = a.xrep.elts();
   _ntl_ulong *bp = b.xrep.elts();

   long da = deg(a);
   long wa = da/NTL_BITS_PER_LONG;
   long ba = da - wa*NTL_BITS_PER_LONG;

   long db = deg(b);
   long wb = db/NTL_BITS_PER_LONG;
   long bb = db - wb*NTL_BITS_PER_LONG;

   long parity = 0;

   for (;;) {
      if (da < db) {
         swap(ap, bp);
         swap(da, db);
         swap(wa, wb);
         swap(ba, bb);
         parity = 1 - parity;

         swap(rp, sp);
         swap(sr, ss);
      }

      // da >= db

      if (db < m) break;

      ShiftAdd(ap, bp, wb+1, da-db);
      ShiftAdd(rp, sp, ss, da-db);
      long t = ss + (da-db+NTL_BITS_PER_LONG-1)/NTL_BITS_PER_LONG;
      if (t > sr) {
         while (t > 0 && rp[t-1] == 0) t--; 
         sr = t;
      }

      _ntl_ulong msk = 1UL << ba;
      _ntl_ulong aa = ap[wa];

      while ((aa & msk) == 0) {
         da--;
         msk = msk >> 1;
         ba--;
         if (!msk) {
            wa--;
            ba = NTL_BITS_PER_LONG-1;
            msk = 1UL << (NTL_BITS_PER_LONG-1);
            if (wa < 0) break;
            aa = ap[wa];
         }
      }
   }

   a.normalize();
   b.normalize();
   r.normalize();
   s.normalize();

   if (!parity) {
      h = s;
   }
   else {
      h = r;
   }
}


void DoMinPolyMod(GF2X& h, const GF2X& g, const GF2XModulus& F, long m, 
               const GF2X& R)
{
   GF2X x;

   ProjectPowers(x, R, 2*m, g, F);
   MinPolyInternal(h, x, m);
}

void MinPolySeq(GF2X& h, const vec_GF2& a, long m)
{
   if (m < 0 || NTL_OVERFLOW(m, 1, 0)) LogicError("MinPoly: bad args");
   if (a.length() < 2*m) LogicError("MinPoly: sequence too short");
   GF2X x;
   x.xrep = a.rep;
   x.normalize();
   MinPolyInternal(h, x, m);
}

void ProbMinPolyMod(GF2X& h, const GF2X& g, const GF2XModulus& F, long m)
{
   long n = F.n;
   if (m < 1 || m > n) LogicError("ProbMinPoly: bad args");

   GF2X R;
   random(R, n);

   DoMinPolyMod(h, g, F, m, R);
}

void ProbMinPolyMod(GF2X& h, const GF2X& g, const GF2XModulus& F)
{
   ProbMinPolyMod(h, g, F, F.n);
}

void MinPolyMod(GF2X& hh, const GF2X& g, const GF2XModulus& F, long m)
{
   GF2X h, h1;
   long n = F.n;
   if (m < 1 || m > n) LogicError("MinPoly: bad args");

   /* probabilistically compute min-poly */

   ProbMinPolyMod(h, g, F, m);
   if (deg(h) == m) { hh = h; return; }
   CompMod(h1, h, g, F);
   if (IsZero(h1)) { hh = h; return; }

   /* not completely successful...must iterate */


   GF2X h2, h3;
   GF2X R;
   GF2XTransMultiplier H1;
   

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

void IrredPolyMod(GF2X& h, const GF2X& g, const GF2XModulus& F, long m)
{
   if (m < 1 || m > F.n) LogicError("IrredPoly: bad args");

   GF2X R;
   set(R);

   DoMinPolyMod(h, g, F, m, R);
}



void IrredPolyMod(GF2X& h, const GF2X& g, const GF2XModulus& F)
{
   IrredPolyMod(h, g, F, F.n);
}



void MinPolyMod(GF2X& hh, const GF2X& g, const GF2XModulus& F)
{
   MinPolyMod(hh, g, F, F.n);
}



void MulByXMod(GF2X& c, const GF2X& a, const GF2XModulus& F)
{
   long da = deg(a);
   long df = deg(F);
   if (da >= df) LogicError("MulByXMod: bad args"); 

   MulByX(c, a);

   if (da >= 0 && da == df-1)
      add(c, c, F);
}

static
void MulByXModAux(GF2X& c, const GF2X& a, const GF2X& f)
{
   long da = deg(a);
   long df = deg(f);
   if (da >= df) LogicError("MulByXMod: bad args"); 

   MulByX(c, a);

   if (da >= 0 && da == df-1)
      add(c, c, f);
}

void MulByXMod(GF2X& h, const GF2X& a, const GF2X& f)
{
   if (&h == &f) {
      GF2X hh;
      MulByXModAux(hh, a, f);
      h = hh;
   }
   else
      MulByXModAux(h, a, f);
}




void power(GF2X& x, const GF2X& a, long e)
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

   if (da > (NTL_MAX_LONG-1)/e)
      ResourceError("overflow in power");

   GF2X res;
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
void FastTraceVec(vec_GF2& S, const GF2XModulus& f)
{
   long n = deg(f);

   if (n <= 0) LogicError("TraceVec: bad args");

   GF2X x = reverse(-LeftShift(reverse(diff(reverse(f)), n-1), n-1)/f, n-1);

   VectorCopy(S, x, n);
   S.put(0, to_GF2(n));
}

static
void PlainTraceVec(vec_GF2& S, const GF2X& f)
{
   long n = deg(f);

   if (n <= 0) 
      LogicError("TraceVec: bad args");

   if (n == 0) {
      S.SetLength(0);
      return;
   }

   GF2X x = reverse(-LeftShift(reverse(diff(reverse(f)), n-1), n-1)/f, n-1);

   VectorCopy(S, x, n); 
   S.put(0, to_GF2(n));
}


void TraceVec(vec_GF2& S, const GF2X& f)
{
   PlainTraceVec(S, f);
}

static
void ComputeTraceVec(vec_GF2& S, const GF2XModulus& F)
{
   if (F.method == GF2X_MOD_PLAIN) {
      PlainTraceVec(S, F.f);
   }
   else {
      FastTraceVec(S, F);
   }
}

void TraceMod(ref_GF2 x, const GF2X& a, const GF2XModulus& F)
{
   long n = F.n;

   if (deg(a) >= n)
      LogicError("trace: bad args");

   do { // NOTE: thread safe lazy init
      Lazy<vec_GF2>::Builder builder(F.tracevec.val()); 
      if (!builder()) break;
      UniquePtr<vec_GF2> p;
      p.make();
      ComputeTraceVec(*p, F);
      builder.move(p);
   } while (0);

   project(x, *F.tracevec.val(), a);
}

void TraceMod(ref_GF2 x, const GF2X& a, const GF2X& f)
{
   if (deg(a) >= deg(f) || deg(f) <= 0)
      LogicError("trace: bad args");

   project(x, TraceVec(f), a);
}



// New versions of GCD, XGCD, and MinPolyInternal
// and support routines

class _NTL_GF2XMatrix {
private:

   _NTL_GF2XMatrix(const _NTL_GF2XMatrix&);  // disable
   GF2X elts[2][2];

public:

   _NTL_GF2XMatrix() { }
   ~_NTL_GF2XMatrix() { }

   void operator=(const _NTL_GF2XMatrix&);
   GF2X& operator() (long i, long j) { return elts[i][j]; }
   const GF2X& operator() (long i, long j) const { return elts[i][j]; }
};


void _NTL_GF2XMatrix::operator=(const _NTL_GF2XMatrix& M)
{
   elts[0][0] = M.elts[0][0];
   elts[0][1] = M.elts[0][1];
   elts[1][0] = M.elts[1][0];
   elts[1][1] = M.elts[1][1];
}


static
void mul(GF2X& U, GF2X& V, const _NTL_GF2XMatrix& M)
// (U, V)^T = M*(U, V)^T
{
   GF2X t1, t2, t3;

   mul(t1, M(0,0), U);
   mul(t2, M(0,1), V);
   add(t3, t1, t2);
   mul(t1, M(1,0), U);
   mul(t2, M(1,1), V);
   add(V, t1, t2);
   U = t3;
}


static
void mul(_NTL_GF2XMatrix& A, _NTL_GF2XMatrix& B, _NTL_GF2XMatrix& C)
// A = B*C, B and C are destroyed
{
   GF2X t1, t2;

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

static
void IterHalfGCD(_NTL_GF2XMatrix& M_out, GF2X& U, GF2X& V, long d_red)
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

   GF2X Q, t(INIT_SIZE, d_red);

   while (deg(V) > goal) {
      DivRem(Q, U, U, V);
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



static
void HalfGCD(_NTL_GF2XMatrix& M_out, const GF2X& U, const GF2X& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));

      return;
   }


   long n = deg(U) - 2*d_red + 2;
   if (n < 0) n = 0;

   GF2X U1, V1;

   RightShift(U1, U, n);
   RightShift(V1, V, n);

   if (d_red <= NTL_GF2X_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U1, V1, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   _NTL_GF2XMatrix M1;

   HalfGCD(M1, U1, V1, d1);
   mul(U1, V1, M1);


   long d2 = deg(V1) - deg(U) + n + d_red;

   if (IsZero(V1) || d2 <= 0) {
      M_out = M1;
      return;
   }


   GF2X Q;
   _NTL_GF2XMatrix M2;

   DivRem(Q, U1, U1, V1);
   swap(U1, V1);

   HalfGCD(M2, U1, V1, d2);

   GF2X t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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

static
void HalfGCD(GF2X& U, GF2X& V)
{
   long d_red = (deg(U)+1)/2;

   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      return;
   }

   long du = deg(U);


   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   _NTL_GF2XMatrix M1;

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


   GF2X Q;

   DivRem(Q, U, U, V);
   swap(U, V);

   HalfGCD(M1, U, V, d2);

   mul(U, V, M1);
}


void GCD(GF2X& d, const GF2X& u, const GF2X& v)
{
   long su = u.xrep.length();
   long sv = v.xrep.length();

   if (su <= NTL_GF2X_GCD_CROSSOVER/NTL_BITS_PER_LONG &&
       sv <= NTL_GF2X_GCD_CROSSOVER/NTL_BITS_PER_LONG) {
      OldGCD(d, u, v);
      return;
   }
    
   GF2X u1, v1;

   u1 = u;
   v1 = v;

   long du1 = deg(u1);
   long dv1 = deg(v1);

   if (du1 == dv1) {
      if (IsZero(u1)) {
         clear(d);
         return;
      }

      rem(v1, v1, u1);
   }
   else if (du1 < dv1) {
      swap(u1, v1);
      du1 = dv1;
   }

   // deg(u1) > deg(v1)

   while (du1 >= NTL_GF2X_GCD_CROSSOVER && !IsZero(v1)) {
      HalfGCD(u1, v1);

      if (!IsZero(v1)) {
         rem(u1, u1, v1);
         swap(u1, v1);
      }

      du1 = deg(u1);
   }

   OldGCD(d, u1, v1);
}

static
void XHalfGCD(_NTL_GF2XMatrix& M_out, GF2X& U, GF2X& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));

      return;
   }

   long du = deg(U);

   if (d_red <= NTL_GF2X_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U, V, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   _NTL_GF2XMatrix M1;

   HalfGCD(M1, U, V, d1);
   mul(U, V, M1);

   long d2 = deg(V) - du + d_red;

   if (IsZero(V) || d2 <= 0) {
      M_out = M1;
      return;
   }


   GF2X Q;
   _NTL_GF2XMatrix M2;

   DivRem(Q, U, U, V);
   swap(U, V);

   XHalfGCD(M2, U, V, d2);


   GF2X t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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




void XGCD(GF2X& d, GF2X& s, GF2X& t, const GF2X& a, const GF2X& b)
{
   // GF2 w;

   long sa = a.xrep.length();
   long sb = b.xrep.length();

   if (sa <= NTL_GF2X_GCD_CROSSOVER/NTL_BITS_PER_LONG &&
       sb <= NTL_GF2X_GCD_CROSSOVER/NTL_BITS_PER_LONG) {
      OldXGCD(d, s, t, a, b);
      return;
   }

   GF2X U, V, Q;

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

   _NTL_GF2XMatrix M;

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

   // inv(w, LeadCoeff(d));
   // mul(d, d, w);
   // mul(s, s, w);
   // mul(t, t, w);
}


void MinPolyInternal(GF2X& h, const GF2X& x, long m)
{  
   if (m < NTL_GF2X_BERMASS_CROSSOVER) {
      OldMinPolyInternal(h, x, m);
      return;
   }

   GF2X a, b;
   _NTL_GF2XMatrix M;
      
   SetCoeff(b, 2*m);
   CopyReverse(a, x, 2*m-1);
   HalfGCD(M, b, a, m+1);

   h = M(1,1);
}



NTL_END_IMPL

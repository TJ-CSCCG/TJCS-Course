
#include <NTL/ZZ_pX.h>
#include <NTL/BasicThreadPool.h>






NTL_START_IMPL




long divide(ZZ_pX& q, const ZZ_pX& a, const ZZ_pX& b)
{
   if (IsZero(b)) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   ZZ_pX lq, r;
   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0; 
   q = lq;
   return 1;
}

long divide(const ZZ_pX& a, const ZZ_pX& b)
{
   if (IsZero(b)) return IsZero(a);
   ZZ_pX lq, r;
   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0; 
   return 1;
}




void ZZ_pXMatrix::operator=(const ZZ_pXMatrix& M)
{
   elts[0][0] = M.elts[0][0];
   elts[0][1] = M.elts[0][1];
   elts[1][0] = M.elts[1][0];
   elts[1][1] = M.elts[1][1];
}


void RightShift(ZZ_pX& x, const ZZ_pX& a, long n)
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

void LeftShift(ZZ_pX& x, const ZZ_pX& a, long n)
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


void ShiftAdd(ZZ_pX& U, const ZZ_pX& V, long n)
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

void ShiftSub(ZZ_pX& U, const ZZ_pX& V, long n)
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


void mul(ZZ_pX& U, ZZ_pX& V, const ZZ_pXMatrix& M)
// (U, V)^T = M*(U, V)^T
{
   long d = deg(U) - deg(M(1,1));
   long k = NextPowerOfTwo(d - 1);

   // When the GCD algorithm is run on polynomials of degree n, n-1, 
   // where n is a power of two, then d-1 is likely to be a power of two.
   // It would be more natural to set k = NextPowerOfTwo(d+1), but this
   // would be much less efficient in this case.

   // We optimize this case, as it does sometimes arise naturally
   // in some situations.

   long n = (1L << k);
   long xx;
   ZZ_p a0, a1, b0, b1, c0, d0, u0, u1, v0, v1, nu0, nu1, nv0;
   NTL_ZZRegister(t1);
   NTL_ZZRegister(t2);

   if (n == d-1)
      xx = 1;
   else if (n == d)
      xx = 2;
   else 
      xx = 3;

   switch (xx) {
   case 1:
      GetCoeff(a0, M(0,0), 0);
      GetCoeff(a1, M(0,0), 1);
      GetCoeff(b0, M(0,1), 0);
      GetCoeff(b1, M(0,1), 1);
      GetCoeff(c0, M(1,0), 0);
      GetCoeff(d0, M(1,1), 0);

      GetCoeff(u0, U, 0);
      GetCoeff(u1, U, 1);
      GetCoeff(v0, V, 0);
      GetCoeff(v1, V, 1);

      mul(t1, rep(a0), rep(u0));
      mul(t2, rep(b0), rep(v0));
      add(t1, t1, t2); 
      conv(nu0, t1);

      mul(t1, rep(a1), rep(u0));
      mul(t2, rep(a0), rep(u1));
      add(t1, t1, t2);
      mul(t2, rep(b1), rep(v0));
      add(t1, t1, t2);
      mul(t2, rep(b0), rep(v1));
      add(t1, t1, t2);
      conv(nu1, t1);

      mul(t1, rep(c0), rep(u0));
      mul(t2, rep(d0), rep(v0));
      add (t1, t1, t2);
      conv(nv0, t1);
   
      break;

   case 2:
      GetCoeff(a0, M(0,0), 0);
      GetCoeff(b0, M(0,1), 0);

      GetCoeff(u0, U, 0);
      GetCoeff(v0, V, 0);

      mul(t1, rep(a0), rep(u0));
      mul(t2, rep(b0), rep(v0));
      add(t1, t1, t2); 
      conv(nu0, t1);

      break;

   case 3:
      break;

   }

   FFTRep RU(INIT_SIZE, k), RV(INIT_SIZE, k), R1(INIT_SIZE, k), 
          R2(INIT_SIZE, k);

   ToFFTRep(RU, U, k);  
   ToFFTRep(RV, V, k);  

   ToFFTRep(R1, M(0,0), k);
   mul(R1, R1, RU);
   ToFFTRep(R2, M(0,1), k);
   mul(R2, R2, RV);
   add(R1, R1, R2);
   FromFFTRep(U, R1, 0, d);

   ToFFTRep(R1, M(1,0), k);
   mul(R1, R1, RU);
   ToFFTRep(R2, M(1,1), k);
   mul(R2, R2, RV);
   add(R1, R1, R2);
   FromFFTRep(V, R1, 0, d-1);

   // now fix-up results

   switch (xx) {
   case 1:
      GetCoeff(u0, U, 0);
      sub(u0, u0, nu0);
      SetCoeff(U, d-1, u0);
      SetCoeff(U, 0, nu0);

      GetCoeff(u1, U, 1);
      sub(u1, u1, nu1);
      SetCoeff(U, d, u1);
      SetCoeff(U, 1, nu1);

      GetCoeff(v0, V, 0);
      sub(v0, v0, nv0);
      SetCoeff(V, d-1, v0);
      SetCoeff(V, 0, nv0);

      break;
      

   case 2:
      GetCoeff(u0, U, 0);
      sub(u0, u0, nu0);
      SetCoeff(U, d, u0);
      SetCoeff(U, 0, nu0);

      break;

   }
}


void mul(ZZ_pXMatrix& A, ZZ_pXMatrix& B, ZZ_pXMatrix& C) 
// A = B*C, B and C are destroyed
{
   long db = deg(B(1,1));
   long dc = deg(C(1,1));
   long da = db + dc;

   long k = NextPowerOfTwo(da+1);

   FFTRep B00, B01, B10, B11, C0, C1, T1, T2;
   
   ToFFTRep(B00, B(0,0), k); B(0,0).kill();
   ToFFTRep(B01, B(0,1), k); B(0,1).kill();
   ToFFTRep(B10, B(1,0), k); B(1,0).kill();
   ToFFTRep(B11, B(1,1), k); B(1,1).kill();

   ToFFTRep(C0, C(0,0), k);  C(0,0).kill();
   ToFFTRep(C1, C(1,0), k);  C(1,0).kill();

   mul(T1, B00, C0);
   mul(T2, B01, C1);
   add(T1, T1, T2);
   FromFFTRep(A(0,0), T1, 0, da);

   mul(T1, B10, C0);
   mul(T2, B11, C1);
   add(T1, T1, T2);
   FromFFTRep(A(1,0), T1, 0, da);

   ToFFTRep(C0, C(0,1), k);  C(0,1).kill();
   ToFFTRep(C1, C(1,1), k);  C(1,1).kill();

   mul(T1, B00, C0);
   mul(T2, B01, C1);
   add(T1, T1, T2);
   FromFFTRep(A(0,1), T1, 0, da);

   mul(T1, B10, C0);
   mul(T2, B11, C1);
   add(T1, T1, T2);
   FromFFTRep(A(1,1), T1, 0, da);
}

void IterHalfGCD(ZZ_pXMatrix& M_out, ZZ_pX& U, ZZ_pX& V, long d_red)
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

   ZZVec tmp(deg(U)+1, ZZ_p::ExtendedModulusSize());
   ZZ_pX Q, t(INIT_SIZE, d_red);

   while (deg(V) > goal) {
      PlainDivRem(Q, U, U, V, tmp);
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
   


void HalfGCD(ZZ_pXMatrix& M_out, const ZZ_pX& U, const ZZ_pX& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));
 
      return;
   }


   long n = deg(U) - 2*d_red + 2;
   if (n < 0) n = 0;

   ZZ_pX U1, V1;

   RightShift(U1, U, n);
   RightShift(V1, V, n);

   if (d_red <= NTL_ZZ_pX_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U1, V1, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   ZZ_pXMatrix M1;

   HalfGCD(M1, U1, V1, d1);
   mul(U1, V1, M1);

   long d2 = deg(V1) - deg(U) + n + d_red;

   if (IsZero(V1) || d2 <= 0) {
      M_out = M1;
      return;
   }


   ZZ_pX Q;
   ZZ_pXMatrix M2;

   DivRem(Q, U1, U1, V1);
   swap(U1, V1);

   HalfGCD(M2, U1, V1, d2);

   ZZ_pX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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




void XHalfGCD(ZZ_pXMatrix& M_out, ZZ_pX& U, ZZ_pX& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));
 
      return;
   }

   long du = deg(U);

   if (d_red <= NTL_ZZ_pX_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U, V, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   ZZ_pXMatrix M1;

   HalfGCD(M1, U, V, d1);
   mul(U, V, M1);

   long d2 = deg(V) - du + d_red;

   if (IsZero(V) || d2 <= 0) {
      M_out = M1;
      return;
   }


   ZZ_pX Q;
   ZZ_pXMatrix M2;

   DivRem(Q, U, U, V);
   swap(U, V);

   XHalfGCD(M2, U, V, d2);

   ZZ_pX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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

void HalfGCD(ZZ_pX& U, ZZ_pX& V)
{
   long d_red = (deg(U)+1)/2;

   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      return;
   }

   long du = deg(U);


   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   ZZ_pXMatrix M1;

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


   ZZ_pX Q;

   DivRem(Q, U, U, V);
   swap(U, V);

   HalfGCD(M1, U, V, d2);

   mul(U, V, M1); 
}


void GCD(ZZ_pX& d, const ZZ_pX& u, const ZZ_pX& v)
{
   ZZ_pX u1, v1;

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

   while (deg(u1) > NTL_ZZ_pX_GCD_CROSSOVER && !IsZero(v1)) {
      HalfGCD(u1, v1);

      if (!IsZero(v1)) {
         rem(u1, u1, v1);
         swap(u1, v1);
      }
   }

   PlainGCD(d, u1, v1);
}



void XGCD(ZZ_pX& d, ZZ_pX& s, ZZ_pX& t, const ZZ_pX& a, const ZZ_pX& b)
{
   ZZ_p w;

   if (IsZero(a) && IsZero(b)) {
      clear(d);
      set(s);
      clear(t);
      return;
   }

   ZZ_pX U, V, Q;

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

   ZZ_pXMatrix M;

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

      





void IterBuild(ZZ_p* a, long n)
{
   long i, k;
   ZZ_p b, t;

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

void mul(ZZ_p* x, const ZZ_p* a, const ZZ_p* b, long n)
{
   NTL_ZZRegister(t);
   NTL_ZZRegister(accum);

   long i, j, jmin, jmax;

   long d = 2*n-1;

   for (i = 0; i <= d; i++) {
      jmin = max(0, i-(n-1));
      jmax = min(n-1, i);
      clear(accum);
      for (j = jmin; j <= jmax; j++) {
         mul(t, rep(a[j]), rep(b[i-j]));
         add(accum, accum, t);
      }
      if (i >= n) {
         add(accum, accum, rep(a[i-n]));
         add(accum, accum, rep(b[i-n]));
      }

      conv(x[i], accum);
   }
}


void BuildFromRoots(ZZ_pX& x, const vec_ZZ_p& a)
{
   long n = a.length();

   if (n == 0) {
      set(x);
      return;
   }

   long k0 = NextPowerOfTwo(NTL_ZZ_pX_FFT_CROSSOVER);
   long crossover = 1L << k0;

   if (n <= crossover) {
      x.rep.SetMaxLength(n+1);
      x.rep = a;
      IterBuild(&x.rep[0], n);
      x.rep.SetLength(n+1);
      SetCoeff(x, n);
      return;
   }

   long k = NextPowerOfTwo(n);

   long m = 1L << k;
   long i, j;
   long l, width;

   ZZ_pX b(INIT_SIZE, m+1);

   b.rep = a;
   b.rep.SetLength(m+1);
   for (i = n; i < m; i++)
      clear(b.rep[i]);

   set(b.rep[m]);
   
   FFTRep R1(INIT_SIZE, k), R2(INIT_SIZE, k);


   ZZ_p t1, one;
   set(one);

   vec_ZZ_p G(INIT_SIZE, crossover), H(INIT_SIZE, crossover);
   ZZ_p *g = G.elts();
   ZZ_p *h = H.elts();
   ZZ_p *tmp;
   
   for (i = 0; i < m; i+= crossover) {
      for (j = 0; j < crossover; j++)
         negate(g[j], b.rep[i+j]);

      if (k0 > 0) {
         for (j = 0; j < crossover; j+=2) {
            mul(t1, g[j], g[j+1]);
            add(g[j+1], g[j], g[j+1]);
            g[j] = t1;
         }
      }
   
      for (l = 1; l < k0; l++) {
         width = 1L << l;

         for (j = 0; j < crossover; j += 2*width)
            mul(&h[j], &g[j], &g[j+width], width);
      
         tmp = g; g = h; h = tmp;
      }

      for (j = 0; j < crossover; j++)
         b.rep[i+j] = g[j];
   }

   for (l = k0; l < k; l++) {
      width = 1L << l;
      for (i = 0; i < m; i += 2*width) {
         t1 = b.rep[i+width];
         set(b.rep[i+width]);
         ToFFTRep(R1, b, l+1, i, i+width);
         b.rep[i+width] = t1;
         t1 = b.rep[i+2*width];
         set(b.rep[i+2*width]);
         ToFFTRep(R2, b, l+1, i+width, i+2*width);
         b.rep[i+2*width] = t1;
         mul(R1, R1, R2);
         FromFFTRep(&b.rep[i], R1, 0, 2*width-1);
         sub(b.rep[i], b.rep[i], one);
      }
   }

   x.rep.SetLength(n+1);
   long delta = m-n;
   for (i = 0; i <= n; i++)
     x.rep[i] = b.rep[i+delta];

   // no need to normalize
}



void eval(ZZ_p& b, const ZZ_pX& f, const ZZ_p& a)
// does a Horner evaluation
{
   ZZ_p acc;
   long i;

   clear(acc);
   for (i = deg(f); i >= 0; i--) {
      mul(acc, acc, a);
      add(acc, acc, f.rep[i]);
   }

   b = acc;
}



void eval(vec_ZZ_p& b, const ZZ_pX& f, const vec_ZZ_p& a)
// naive algorithm:  repeats Horner
{
   if (&b == &f.rep) {
      vec_ZZ_p bb;
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




void interpolate(ZZ_pX& f, const vec_ZZ_p& a, const vec_ZZ_p& b)
{
   long m = a.length();
   if (b.length() != m) LogicError("interpolate: vector length mismatch");

   if (m == 0) {
      clear(f);
      return;
   }

   vec_ZZ_p prod;
   prod = a;

   ZZ_p t1, t2;

   long k, i;

   vec_ZZ_p res;
   res.SetLength(m);

   for (k = 0; k < m; k++) {

      const ZZ_p& aa = a[k];

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


   

NTL_TBDECL(InnerProduct)(ZZ_pX& x, const vec_ZZ_p& v, long low, long high, 
                   const vec_ZZ_pX& H, long n, ZZVec& t)
{
   NTL_ZZRegister(s);
   long i, j;

   for (j = 0; j < n; j++)
      clear(t[j]);

   high = min(high, v.length()-1);
   for (i = low; i <= high; i++) {
      const vec_ZZ_p& h = H[i-low].rep;
      long m = h.length();
      const ZZ& w = rep(v[i]);

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


#ifdef NTL_THREAD_BOOST

void InnerProduct(ZZ_pX& x, const vec_ZZ_p& v, long low, long high, 
                   const vec_ZZ_pX& H, long n, ZZVec& t)
{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1) {
      basic_InnerProduct(x, v, low, high, H, n, t);
      return;
   }

   high = min(high, v.length()-1);
   x.rep.SetLength(n);

   ZZ_pContext local_context;
   local_context.save();

   pool->exec_range(n,
   [low, high, &x, &t, &H, &v, &local_context](long first, long last) {

      local_context.restore();

      NTL_ZZRegister(s);
   
      for (long j = first; j < last; j++) clear(t[j]);

      for (long i = low; i <= high; i++) {
         const vec_ZZ_p& h = H[i-low].rep;
         long m = min(h.length(), last);
         const ZZ& w = rep(v[i]);
   
         for (long j = first; j < m; j++) {
            mul(s, w, rep(h[j]));
            add(t[j], t[j], s);
         }
      }

      for (long j = first; j < last; j++) conv(x.rep[j], t[j]);
   } );

   x.normalize();
}

#endif


void CompMod(ZZ_pX& x, const ZZ_pX& g, const ZZ_pXArgument& A, 
             const ZZ_pXModulus& F)
{
   if (deg(g) <= 0) {
      x = g;
      return;
   }


   ZZ_pX s, t;
   ZZVec scratch(F.n, ZZ_p::ExtendedModulusSize());

   long m = A.H.length() - 1;
   long l = ((g.rep.length()+m-1)/m) - 1;

   ZZ_pXMultiplier M;
   build(M, A.H[m], F);

   InnerProduct(t, g.rep, l*m, l*m + m - 1, A.H, F.n, scratch);
   for (long i = l-1; i >= 0; i--) {
      InnerProduct(s, g.rep, i*m, i*m + m - 1, A.H, F.n, scratch);
      MulMod(t, t, M, F);
      add(t, t, s);
   }

   x = t;
}


void build(ZZ_pXArgument& A, const ZZ_pX& h, const ZZ_pXModulus& F, long m)
{
   if (m <= 0 || deg(h) >= F.n) LogicError("build: bad args");

   if (m > F.n) m = F.n;

   long i;

   if (ZZ_pXArgBound > 0) {
      double sz = ZZ_p::storage();
      sz = sz*F.n;
      sz = sz + NTL_VECTOR_HEADER_SIZE + sizeof(vec_ZZ_p);
      sz = sz/1024;
      m = min(m, long(ZZ_pXArgBound/sz));
      m = max(m, 1);
   }

   ZZ_pXMultiplier M;

   build(M, h, F);

   A.H.SetLength(m+1);

   set(A.H[0]);
   A.H[1] = h;
   for (i = 2; i <= m; i++) 
      MulMod(A.H[i], A.H[i-1], M, F);
}




NTL_CHEAP_THREAD_LOCAL long ZZ_pXArgBound = 0;


void CompMod(ZZ_pX& x, const ZZ_pX& g, const ZZ_pX& h, const ZZ_pXModulus& F)
   // x = g(h) mod f
{
   long m = SqrRoot(g.rep.length());

   if (m == 0) {
      clear(x);
      return;
   }

   ZZ_pXNewArgument A;

   build(A, h, F, m);

   CompMod(x, g, A, F);
}




void Comp2Mod(ZZ_pX& x1, ZZ_pX& x2, const ZZ_pX& g1, const ZZ_pX& g2,
              const ZZ_pX& h, const ZZ_pXModulus& F)

{
   long m = SqrRoot(g1.rep.length() + g2.rep.length());

   if (m == 0) {
      clear(x1);
      clear(x2);
      return;
   }

   ZZ_pXNewArgument A;

   build(A, h, F, m);

   ZZ_pX xx1, xx2;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);

   x1 = xx1;
   x2 = xx2;
}

void Comp3Mod(ZZ_pX& x1, ZZ_pX& x2, ZZ_pX& x3, 
              const ZZ_pX& g1, const ZZ_pX& g2, const ZZ_pX& g3,
              const ZZ_pX& h, const ZZ_pXModulus& F)

{
   long m = SqrRoot(g1.rep.length() + g2.rep.length() + g3.rep.length());

   if (m == 0) {
      clear(x1);
      clear(x2);
      clear(x3);
      return;
   }

   ZZ_pXNewArgument A;

   build(A, h, F, m);

   ZZ_pX xx1, xx2, xx3;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);
   CompMod(xx3, g3, A, F);

   x1 = xx1;
   x2 = xx2;
   x3 = xx3;
}


static void StripZeroes(vec_ZZ_p& x)
{
   long n = x.length();
   while (n > 0 && IsZero(x[n-1]))
      n--;
   x.SetLength(n);
}


void PlainUpdateMap(vec_ZZ_p& xx, const vec_ZZ_p& a, 
                    const ZZ_pX& b, const ZZ_pX& f)
{
   long n = deg(f);
   long i, m;

   if (IsZero(b)) {
      xx.SetLength(0);
      return;
   }

   m = n-1 - deg(b);

   vec_ZZ_p x(INIT_SIZE, n);

   for (i = 0; i <= m; i++)
      InnerProduct(x[i], a, b.rep, i);

   if (deg(b) != 0) {
      ZZ_pX c(INIT_SIZE, n);
      LeftShift(c, b, m);

      for (i = m+1; i < n; i++) {
         MulByXMod(c, c, f);
         InnerProduct(x[i], a, c.rep);
      }
   }

   xx = x;
}
   

void UpdateMap(vec_ZZ_p& x, const vec_ZZ_p& aa, 
               const ZZ_pXMultiplier& B, const ZZ_pXModulus& F)
{
   long n = F.n;
   long i;


   vec_ZZ_p a;
   a = aa;
   StripZeroes(a);

   if (a.length() > n) LogicError("UpdateMap: bad args");

   if (!B.UseFFT) {
      PlainUpdateMap(x, a, B.b, F.f);
      StripZeroes(x);
      return;
   }

   FFTRep R1(INIT_SIZE, F.k), R2(INIT_SIZE, F.l);
   vec_ZZ_p V1(INIT_SIZE, n);


   RevToFFTRep(R1, a, F.k, 0, a.length()-1, 0);
   mul(R2, R1, F.FRep);
   RevFromFFTRep(V1, R2, 0, n-2);
   for (i = 0; i <= n-2; i++)  negate(V1[i], V1[i]);
   RevToFFTRep(R2, V1, F.l, 0, n-2, n-1);
   mul(R2, R2, B.B1);
   mul(R1, R1, B.B2);

   AddExpand(R2, R1);
   RevFromFFTRep(x, R2, 0, n-1);
   StripZeroes(x);
}

   

NTL_TBDECL(ProjectPowers)(vec_ZZ_p& x, const vec_ZZ_p& a, long k,
                   const ZZ_pXArgument& H, const ZZ_pXModulus& F)

{
   long n = F.n;

   if (a.length() > n || k < 0) 
      LogicError("ProjectPowers: bad args");
   if (NTL_OVERFLOW(k, 1, 0)) 
      ResourceError("ProjectPowers: excessive args");


   long m = H.H.length()-1;
   long l = (k+m-1)/m - 1;

   ZZ_pXMultiplier M;
   build(M, H.H[m], F);

   vec_ZZ_p s(INIT_SIZE, n);
   s = a;
   StripZeroes(s);

   x.SetLength(k);

   for (long i = 0; i <= l; i++) {
      long m1 = min(m, k-i*m);
      ZZ_p* w = &x[i*m];
      for (long j = 0; j < m1; j++)
         InnerProduct(w[j], H.H[j].rep, s);
      if (i < l)
         UpdateMap(s, s, M, F);
   }
}


#ifdef NTL_THREAD_BOOST

void ProjectPowers(vec_ZZ_p& x, const vec_ZZ_p& a, long k,
                   const ZZ_pXArgument& H, const ZZ_pXModulus& F)

{
   BasicThreadPool *pool = GetThreadPool();

   if (!pool || pool->active() || pool->NumThreads() == 1) {
      basic_ProjectPowers(x, a, k, H, F);
      return;
   }

   long n = F.n;

   if (a.length() > n || k < 0) 
      LogicError("ProjectPowers: bad args");
   if (NTL_OVERFLOW(k, 1, 0)) 
      ResourceError("ProjectPowers: excessive args");


   long m = H.H.length()-1;
   long l = (k+m-1)/m - 1;

   ZZ_pXMultiplier M;
   build(M, H.H[m], F);

   vec_ZZ_p s(INIT_SIZE, n);
   s = a;
   StripZeroes(s);

   x.SetLength(k);

   ZZ_pContext local_context;
   local_context.save();


   for (long i = 0; i <= l; i++) {
      long m1 = min(m, k-i*m);
      ZZ_p* w = &x[i*m];

      pool->exec_range(m1,
      [w, &H, &s, &local_context](long first, long last) {
         local_context.restore();
         for (long j = first; j < last; j++)
            InnerProduct(w[j], H.H[j].rep, s);
      } );


      if (i < l)
         UpdateMap(s, s, M, F);
   }
}


#endif


void ProjectPowers(vec_ZZ_p& x, const vec_ZZ_p& a, long k,
                   const ZZ_pX& h, const ZZ_pXModulus& F)

{
   if (a.length() > F.n || k < 0) LogicError("ProjectPowers: bad args");

   if (k == 0) {
      x.SetLength(0);
      return;
   }

   long m = SqrRoot(k);

   ZZ_pXNewArgument H;

   build(H, h, F, m);
   ProjectPowers(x, a, k, H, F);
}

#ifdef  NTL_USE_ZZ_pXNewArgument

// ZZ_pXNewArgument stuff


void build(ZZ_pXNewArgument& H, const ZZ_pX& h, const ZZ_pXModulus& F, long m)
{
   long n = F.n;

   if (m <= 0 || deg(h) >= n) LogicError("build: bad args");

   if (NTL_OVERFLOW(m, 1, 0)) 
      ResourceError("ZZ_pXNewArgument:build: m too big");

   // NOTE: we don't take ZZ_pXArgBound into account, as the 
   // new strategy anyway always uses space about (m + deg(g)/m)*n

   long width; // usually n, but may be smaller if h has very low degree
   // some messiness here to avoid overflow

   long dh = deg(h);

   if (dh < 0)
      width = 1;
   else if (dh == 0 || m-1 == 0)
      width = 1;
   else if (dh <= n/(m-1))
      width = min(n, dh*(m-1) + 1);
   else
      width = n;


   ZZ_pXMultiplier M;
   build(M, h, F);

   Mat<ZZ_p> mat;
   mat.SetDims(m, width);
 
   ZZ_pX poly;
   poly = 1;

   for (long i = 0; i < m; i++) {
      VectorCopy(mat[i], poly, width);
      MulMod(poly, poly, M, F);
   }

   
   H.mat.move(mat);
   H.poly.swap(poly);
}

void CompMod(ZZ_pX& x, const ZZ_pX& g, const ZZ_pXNewArgument& H, 
             const ZZ_pXModulus& F)
{
   long d = deg(g)+1;

   if (d <= 1) {
      x = g;
      return;
   }

   long m = H.mat.NumRows();

   if (m == 0) LogicError("CompMod: uninitialized argument");

   long l = (d+m-1)/m;

   Mat<ZZ_p> gmat;
   gmat.SetDims(l, m);

   for (long i = 0; i < l; i++) 
      for (long j = 0; j < m; j++)
         gmat[i][j] = coeff(g, i*m+j);

   Mat<ZZ_p> xmat;
   mul(xmat, gmat, H.mat);


   ZZ_pX t;
   conv(t, xmat[l-1]);

   if (l-2 >= 0) {
      ZZ_pXMultiplier M;
      build(M, H.poly, F);
      ZZ_pX s;

      for (long i = l-2; i >= 0; i--) {
	 conv(s, xmat[i]);
	 MulMod(t, t, M, F);
	 add(t, t, s);
      }
   }

   x = t;
}



void ProjectPowers(vec_ZZ_p& x, const vec_ZZ_p& a, long k,
                   const ZZ_pXNewArgument& H, const ZZ_pXModulus& F)

{
   long n = F.n;

   if (a.length() > n || k < 0)
      LogicError("ProjectPowers: bad args");
   if (NTL_OVERFLOW(k, 1, 0))
      ResourceError("ProjectPowers: excessive args");

   long m = H.mat.NumRows();

   if (m == 0) LogicError("CompMod: uninitialized argument");

   long width = H.mat.NumCols();
   long l = (k+m-1)/m;

   Mat<ZZ_p> amat, xmat;

   amat.SetDims(l, width);

   vec_ZZ_p s(INIT_SIZE, n);
   s = a;
   StripZeroes(s);

   VectorCopy(amat[0], s, width);

   if (l > 1) {
      ZZ_pXMultiplier M;
      build(M, H.poly, F);

      for (long i = 1; i < l; i++) {
	 UpdateMap(s, s, M, F);
	 VectorCopy(amat[i], s, width);
      }
   }

   mul_transpose(xmat, amat, H.mat);

   x.SetLength(k);
   for (long i = 0; i < l; i++) {
      long j_max = min(m, k-i*m);
      for (long j = 0; j < j_max; j++)
         x[i*m+j] = xmat[i][j];
   }

}


#endif




void BerlekampMassey(ZZ_pX& h, const vec_ZZ_p& a, long m)
{
   ZZ_pX Lambda, Sigma, Temp;
   long L;
   ZZ_p Delta, Delta1, t1;
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




void GCDMinPolySeq(ZZ_pX& h, const vec_ZZ_p& x, long m)
{
   long i;
   ZZ_pX a, b;
   ZZ_pXMatrix M;
   ZZ_p t;

   a.rep.SetLength(2*m);
   for (i = 0; i < 2*m; i++) a.rep[i] = x[2*m-1-i];
   a.normalize();

   SetCoeff(b, 2*m);

   HalfGCD(M, b, a, m+1);

   /* make monic */

   inv(t, LeadCoeff(M(1,1)));
   mul(h, M(1,1), t);
}


void MinPolySeq(ZZ_pX& h, const vec_ZZ_p& a, long m)
{
   if (m < 0) LogicError("MinPoly: bad args");
   if (NTL_OVERFLOW(m, 1, 0)) LogicError("MinPoly: bad args");
   if (a.length() < 2*m) LogicError("MinPoly: sequence too short");

   if (m > NTL_ZZ_pX_BERMASS_CROSSOVER)
      GCDMinPolySeq(h, a, m);
   else
      BerlekampMassey(h, a, m);
}


void DoMinPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F, long m,
               const vec_ZZ_p& R) 
{
   vec_ZZ_p x;

   ProjectPowers(x, R, 2*m, g, F);
   MinPolySeq(h, x, m);
}


void ProbMinPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F, long m) 
{
   long n = F.n;
   if (m < 1 || m > n) LogicError("ProbMinPoly: bad args");
   
   long i;
   vec_ZZ_p R(INIT_SIZE, n);

   for (i = 0; i < n; i++) random(R[i]);
   DoMinPolyMod(h, g, F, m, R);
}

void MinPolyMod(ZZ_pX& hh, const ZZ_pX& g, const ZZ_pXModulus& F, long m) 
{
   ZZ_pX h, h1;
   long n = F.n;
   if (m < 1 || m > n) LogicError("MinPoly: bad args");

   /* probabilistically compute min-poly */

   ProbMinPolyMod(h, g, F, m);
   if (deg(h) == m) { hh = h; return; }
   CompMod(h1, h, g, F);
   if (IsZero(h1)) { hh = h; return; }

   /* not completely successful...must iterate */

   long i;

   ZZ_pX h2, h3;
   ZZ_pXMultiplier H1;
   vec_ZZ_p R(INIT_SIZE, n);

   for (;;) {
      R.SetLength(n);
      for (i = 0; i < n; i++) random(R[i]);
      build(H1, h1, F);
      UpdateMap(R, R, H1, F);
      DoMinPolyMod(h2, g, F, m-deg(h), R);

      mul(h, h, h2);
      if (deg(h) == m) { hh = h; return; }
      CompMod(h3, h2, g, F);
      MulMod(h1, h3, H1, F);
      if (IsZero(h1)) { hh = h; return; }
   }
}



void IrredPolyMod(ZZ_pX& h, const ZZ_pX& g, const ZZ_pXModulus& F, long m) 
{
   vec_ZZ_p R(INIT_SIZE, 1);
   if (m < 1 || m > F.n) LogicError("IrredPoly: bad args");

   set(R[0]);
   DoMinPolyMod(h, g, F, m, R);
}



void diff(ZZ_pX& x, const ZZ_pX& a)
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


void MakeMonic(ZZ_pX& x)
{
   if (IsZero(x))
      return;

   if (IsOne(LeadCoeff(x)))
      return;

   ZZ_p t;

   inv(t, LeadCoeff(x));
   mul(x, x, t);
}


      
void PlainMulTrunc(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, long n)
{
   ZZ_pX y;
   mul(y, a, b);
   trunc(x, y, n);
}


void FFTMulTrunc(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, long n)
{
   if (IsZero(a) || IsZero(b)) {
      clear(x);
      return;
   }

   long d = deg(a) + deg(b);
   if (n > d + 1)
      n = d + 1;

   long k = NextPowerOfTwo(d + 1);
   FFTRep R1(INIT_SIZE, k), R2(INIT_SIZE, k);

   ToFFTRep(R1, a, k);
   ToFFTRep(R2, b, k);
   mul(R1, R1, R2);
   FromFFTRep(x, R1, 0, n-1);
}

void MulTrunc(ZZ_pX& x, const ZZ_pX& a, const ZZ_pX& b, long n)
{
   if (n < 0) LogicError("MulTrunc: bad args");

   if (deg(a) <= NTL_ZZ_pX_FFT_CROSSOVER || deg(b) <= NTL_ZZ_pX_FFT_CROSSOVER)
      PlainMulTrunc(x, a, b, n);
   else
      FFTMulTrunc(x, a, b, n);
}

      
void PlainSqrTrunc(ZZ_pX& x, const ZZ_pX& a, long n)
{
   ZZ_pX y;
   sqr(y, a);
   trunc(x, y, n);
}


void FFTSqrTrunc(ZZ_pX& x, const ZZ_pX& a, long n)
{
   if (IsZero(a)) {
      clear(x);
      return;
   }

   long d = 2*deg(a);
   if (n > d + 1)
      n = d + 1;

   long k = NextPowerOfTwo(d + 1);
   FFTRep R1(INIT_SIZE, k);

   ToFFTRep(R1, a, k);
   mul(R1, R1, R1);
   FromFFTRep(x, R1, 0, n-1);
}

void SqrTrunc(ZZ_pX& x, const ZZ_pX& a, long n)
{
   if (n < 0) LogicError("SqrTrunc: bad args");

   if (deg(a) <= NTL_ZZ_pX_FFT_CROSSOVER)
      PlainSqrTrunc(x, a, n);
   else
      FFTSqrTrunc(x, a, n);
}


void FastTraceVec(vec_ZZ_p& S, const ZZ_pX& f)
{
   long n = deg(f);

   if (n <= 0) 
      LogicError("FastTraceVec: bad args");

   if (n == 0) {
      S.SetLength(0);
      return;
   }

   if (n == 1) {
      S.SetLength(1);
      set(S[0]);
      return;
   }
   
   long i;
   ZZ_pX f1;

   f1.rep.SetLength(n-1);
   for (i = 0; i <= n-2; i++)
      f1.rep[i] = f.rep[n-i];
   f1.normalize();

   ZZ_pX f2;
   f2.rep.SetLength(n-1);
   for (i = 0; i <= n-2; i++)
      mul(f2.rep[i], f.rep[n-1-i], i+1);
   f2.normalize();

   ZZ_pX f3;
   InvTrunc(f3, f1, n-1);
   MulTrunc(f3, f3, f2, n-1);

   S.SetLength(n);

   S[0] = n;
   for (i = 1; i < n; i++)
      negate(S[i], coeff(f3, i-1));
}


void PlainTraceVec(vec_ZZ_p& S, const ZZ_pX& ff)
{
   if (deg(ff) <= 0)
      LogicError("TraceVec: bad args");

   ZZ_pX f;
   f = ff;

   MakeMonic(f);

   long n = deg(f);

   S.SetLength(n);

   if (n == 0)
      return;

   long k, i;
   ZZ acc, t;
   ZZ_p t1;

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

void TraceVec(vec_ZZ_p& S, const ZZ_pX& f)
{
   if (deg(f) <= NTL_ZZ_pX_TRACE_CROSSOVER)
      PlainTraceVec(S, f);
   else
      FastTraceVec(S, f);
}

static
void ComputeTraceVec(vec_ZZ_p& S, const ZZ_pXModulus& F)
{
   if (!F.UseFFT) {
      PlainTraceVec(S, F.f);
      return;
   }

   long i;
   long n = F.n;

   FFTRep R;
   ZZ_pX P, g;

   g.rep.SetLength(n-1);
   for (i = 1; i < n; i++)
      mul(g.rep[n-i-1], F.f.rep[n-i], i); 
   g.normalize();

   ToFFTRep(R, g, F.l);
   mul(R, R, F.HRep);
   FromFFTRep(P, R, n-2, 2*n-4);

   S.SetLength(n);

   S[0] = n;
   for (i = 1; i < n; i++)
      negate(S[i], coeff(P, n-1-i));
}

void TraceMod(ZZ_p& x, const ZZ_pX& a, const ZZ_pXModulus& F)
{
   long n = F.n;

   if (deg(a) >= n)
      LogicError("trace: bad args");

   do { // NOTE: thread safe lazy init
      Lazy<vec_ZZ_p>::Builder builder(F.tracevec.val());
      if (!builder()) break;
      UniquePtr<vec_ZZ_p> p;
      p.make();
      ComputeTraceVec(*p, F);
      builder.move(p);
   } while (0);
      
   InnerProduct(x, a.rep, *F.tracevec.val());
}

void TraceMod(ZZ_p& x, const ZZ_pX& a, const ZZ_pX& f)
{
   if (deg(a) >= deg(f) || deg(f) <= 0)
      LogicError("trace: bad args");

   project(x, TraceVec(f), a);
}

void PlainResultant(ZZ_p& rres, const ZZ_pX& a, const ZZ_pX& b)
{
   ZZ_p res;
 
   if (IsZero(a) || IsZero(b))
      clear(res);
   else if (deg(a) == 0 && deg(b) == 0) 
      set(res);
   else {
      long d0, d1, d2;
      ZZ_p lc;
      set(res);

      long n = max(deg(a),deg(b)) + 1;
      ZZ_pX u(INIT_SIZE, n), v(INIT_SIZE, n);
      ZZVec tmp(n, ZZ_p::ExtendedModulusSize());

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


void ResIterHalfGCD(ZZ_pXMatrix& M_out, ZZ_pX& U, ZZ_pX& V, long d_red,
                    vec_ZZ_p& cvec, vec_long& dvec)
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

   ZZVec tmp(deg(U)+1, ZZ_p::ExtendedModulusSize());
   ZZ_pX Q, t(INIT_SIZE, d_red);


   while (deg(V) > goal) {
      append(cvec, LeadCoeff(V));
      append(dvec, dvec[dvec.length()-1]-deg(U)+deg(V));
      PlainDivRem(Q, U, U, V, tmp);
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
   


void ResHalfGCD(ZZ_pXMatrix& M_out, const ZZ_pX& U, const ZZ_pX& V, long d_red,
                vec_ZZ_p& cvec, vec_long& dvec)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));
 
      return;
   }


   long n = deg(U) - 2*d_red + 2;
   if (n < 0) n = 0;

   ZZ_pX U1, V1;

   RightShift(U1, U, n);
   RightShift(V1, V, n);

   if (d_red <= NTL_ZZ_pX_HalfGCD_CROSSOVER) { 
      ResIterHalfGCD(M_out, U1, V1, d_red, cvec, dvec);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   ZZ_pXMatrix M1;

   ResHalfGCD(M1, U1, V1, d1, cvec, dvec);
   mul(U1, V1, M1);

   long d2 = deg(V1) - deg(U) + n + d_red;

   if (IsZero(V1) || d2 <= 0) {
      M_out = M1;
      return;
   }


   ZZ_pX Q;
   ZZ_pXMatrix M2;

   append(cvec, LeadCoeff(V1));
   append(dvec, dvec[dvec.length()-1]-deg(U1)+deg(V1));
   DivRem(Q, U1, U1, V1);
   swap(U1, V1);

   ResHalfGCD(M2, U1, V1, d2, cvec, dvec);

   ZZ_pX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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

void ResHalfGCD(ZZ_pX& U, ZZ_pX& V, vec_ZZ_p& cvec, vec_long& dvec)
{
   long d_red = (deg(U)+1)/2;

   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      return;
   }

   long du = deg(U);


   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   ZZ_pXMatrix M1;

   ResHalfGCD(M1, U, V, d1, cvec, dvec);
   mul(U, V, M1);

   long d2 = deg(V) - du + d_red;

   if (IsZero(V) || d2 <= 0) {
      return;
   }

   M1(0,0).kill();
   M1(0,1).kill();
   M1(1,0).kill();
   M1(1,1).kill();


   ZZ_pX Q;

   append(cvec, LeadCoeff(V));
   append(dvec, dvec[dvec.length()-1]-deg(U)+deg(V));
   DivRem(Q, U, U, V);
   swap(U, V);

   ResHalfGCD(M1, U, V, d2, cvec, dvec);

   mul(U, V, M1); 
}


void resultant(ZZ_p& rres, const ZZ_pX& u, const ZZ_pX& v)
{
   if (deg(u) <= NTL_ZZ_pX_GCD_CROSSOVER || deg(v) <= NTL_ZZ_pX_GCD_CROSSOVER) { 
      PlainResultant(rres, u, v);
      return;
   }

   ZZ_pX u1, v1;

   u1 = u;
   v1 = v;

   ZZ_p res, t;
   set(res);

   if (deg(u1) == deg(v1)) {
      rem(u1, u1, v1);
      swap(u1, v1);

      if (IsZero(v1)) {
         clear(rres);
         return;
      }

      power(t, LeadCoeff(u1), deg(u1) - deg(v1));
      mul(res, res, t);
      if (deg(u1) & 1)
         negate(res, res);
   }
   else if (deg(u1) < deg(v1)) {
      swap(u1, v1);
      if (deg(u1) & deg(v1) & 1)
         negate(res, res);
   }

   // deg(u1) > deg(v1) && v1 != 0

   vec_ZZ_p cvec;
   vec_long  dvec;

   cvec.SetMaxLength(deg(v1)+2);
   dvec.SetMaxLength(deg(v1)+2);

   append(cvec, LeadCoeff(u1));
   append(dvec, deg(u1));


   while (deg(u1) > NTL_ZZ_pX_GCD_CROSSOVER && !IsZero(v1)) { 
      ResHalfGCD(u1, v1, cvec, dvec);

      if (!IsZero(v1)) {
         append(cvec, LeadCoeff(v1));
         append(dvec, deg(v1));
         rem(u1, u1, v1);
         swap(u1, v1);
      }
   }

   if (IsZero(v1) && deg(u1) > 0) {
      clear(rres);
      return;
   }

   long i, l;
   l = dvec.length();

   if (deg(u1) == 0) {
      // we went all the way...

      for (i = 0; i <= l-3; i++) {
         power(t, cvec[i+1], dvec[i]-dvec[i+2]);
         mul(res, res, t);
         if (dvec[i] & dvec[i+1] & 1)
            negate(res, res);
      }

      power(t, cvec[l-1], dvec[l-2]);
      mul(res, res, t);
   }
   else {
      for (i = 0; i <= l-3; i++) {
         power(t, cvec[i+1], dvec[i]-dvec[i+2]);
         mul(res, res, t);
         if (dvec[i] & dvec[i+1] & 1)
            negate(res, res);
      }

      power(t, cvec[l-1], dvec[l-2]-deg(v1));
      mul(res, res, t);
      if (dvec[l-2] & dvec[l-1] & 1)
         negate(res, res);

      PlainResultant(t, u1, v1);
      mul(res, res, t);
   }

   rres = res;
}

void NormMod(ZZ_p& x, const ZZ_pX& a, const ZZ_pX& f)
{
   if (deg(f) <= 0 || deg(a) >= deg(f)) 
      LogicError("norm: bad args");

   if (IsZero(a)) {
      clear(x);
      return;
   }

   ZZ_p t;
   resultant(t, f, a);
   if (!IsOne(LeadCoeff(f))) {
      ZZ_p t1;
      power(t1, LeadCoeff(f), deg(a));
      inv(t1, t1);
      mul(t, t, t1);
   }

   x = t;
}

NTL_END_IMPL

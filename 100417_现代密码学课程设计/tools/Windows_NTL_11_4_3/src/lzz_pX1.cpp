
#include <NTL/lzz_pX.h>



NTL_START_IMPL



long divide(zz_pX& q, const zz_pX& a, const zz_pX& b)
{
   if (IsZero(b)) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   zz_pX lq, r;
   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0;
   q = lq;
   return 1;
}

long divide(const zz_pX& a, const zz_pX& b)
{
   if (IsZero(b)) return IsZero(a);
   zz_pX lq, r;
   DivRem(lq, r, a, b);
   if (!IsZero(r)) return 0;
   return 1;
}



void zz_pXMatrix::operator=(const zz_pXMatrix& M)
{
   elts[0][0] = M.elts[0][0];
   elts[0][1] = M.elts[0][1];
   elts[1][0] = M.elts[1][0];
   elts[1][1] = M.elts[1][1];
}


void RightShift(zz_pX& x, const zz_pX& a, long n)
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

void LeftShift(zz_pX& x, const zz_pX& a, long n)
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


void ShiftAdd(zz_pX& U, const zz_pX& V, long n)
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

void ShiftSub(zz_pX& U, const zz_pX& V, long n)
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

void mul(zz_pX& U, zz_pX& V, const zz_pXMatrix& M)
// (U, V)^T = M*(U, V)^T
{
   long d = deg(U) - deg(M(1,1));
   long k = NextPowerOfTwo(d - 1);

   // When the GCD algorithm is run on polynomials of degree n, n-1, 
   // where n is a power of two, then d-1 is likely to be a power of two.
   // It would be more natural to set k = NextPowerOfTwo(d+1), but this
   // would be much less efficient in this case.

   long n = (1L << k);
   long xx;
   zz_p a0, a1, b0, b1, c0, d0, u0, u1, v0, v1, nu0, nu1, nv0;
   zz_p t1, t2;

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

      mul(t1, (a0), (u0));
      mul(t2, (b0), (v0));
      add(t1, t1, t2); 
      nu0 = t1;

      mul(t1, (a1), (u0));
      mul(t2, (a0), (u1));
      add(t1, t1, t2);
      mul(t2, (b1), (v0));
      add(t1, t1, t2);
      mul(t2, (b0), (v1));
      add(t1, t1, t2);
      nu1 = t1;

      mul(t1, (c0), (u0));
      mul(t2, (d0), (v0));
      add (t1, t1, t2);
      nv0 = t1;
   
      break;

   case 2:
      GetCoeff(a0, M(0,0), 0);
      GetCoeff(b0, M(0,1), 0);

      GetCoeff(u0, U, 0);
      GetCoeff(v0, V, 0);

      mul(t1, (a0), (u0));
      mul(t2, (b0), (v0));
      add(t1, t1, t2); 
      nu0 = t1;

      break;

   case 3:
      break;

   }

   fftRep RU(INIT_SIZE, k), RV(INIT_SIZE, k), R1(INIT_SIZE, k), 
          R2(INIT_SIZE, k);

   TofftRep(RU, U, k);  
   TofftRep(RV, V, k);  

   TofftRep(R1, M(0,0), k);
   mul(R1, R1, RU);
   TofftRep(R2, M(0,1), k);
   mul(R2, R2, RV);
   add(R1, R1, R2);
   FromfftRep(U, R1, 0, d);

   TofftRep(R1, M(1,0), k);
   mul(R1, R1, RU);
   TofftRep(R2, M(1,1), k);
   mul(R2, R2, RV);
   add(R1, R1, R2);
   FromfftRep(V, R1, 0, d-1);

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


void mul(zz_pXMatrix& A, zz_pXMatrix& B, zz_pXMatrix& C)
// A = B*C, B and C are destroyed
{
   long db = deg(B(1,1));
   long dc = deg(C(1,1));
   long da = db + dc;

   long k = NextPowerOfTwo(da+1);

   fftRep B00, B01, B10, B11, C0, C1, T1, T2;
   
   TofftRep(B00, B(0,0), k); B(0,0).kill();
   TofftRep(B01, B(0,1), k); B(0,1).kill();
   TofftRep(B10, B(1,0), k); B(1,0).kill();
   TofftRep(B11, B(1,1), k); B(1,1).kill();

   TofftRep(C0, C(0,0), k);  C(0,0).kill();
   TofftRep(C1, C(1,0), k);  C(1,0).kill();

   mul(T1, B00, C0);
   mul(T2, B01, C1);
   add(T1, T1, T2);
   FromfftRep(A(0,0), T1, 0, da);

   mul(T1, B10, C0);
   mul(T2, B11, C1);
   add(T1, T1, T2);
   FromfftRep(A(1,0), T1, 0, da);

   TofftRep(C0, C(0,1), k);  C(0,1).kill();
   TofftRep(C1, C(1,1), k);  C(1,1).kill();

   mul(T1, B00, C0);
   mul(T2, B01, C1);
   add(T1, T1, T2);
   FromfftRep(A(0,1), T1, 0, da);

   mul(T1, B10, C0);
   mul(T2, B11, C1);
   add(T1, T1, T2);
   FromfftRep(A(1,1), T1, 0, da);
}

void IterHalfGCD(zz_pXMatrix& M_out, zz_pX& U, zz_pX& V, long d_red)
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

   zz_pX Q, t(INIT_SIZE, d_red);

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
   


void HalfGCD(zz_pXMatrix& M_out, const zz_pX& U, const zz_pX& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));
 
      return;
   }


   long n = deg(U) - 2*d_red + 2;
   if (n < 0) n = 0;

   zz_pX U1, V1;

   RightShift(U1, U, n);
   RightShift(V1, V, n);

   if (d_red <= NTL_zz_pX_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U1, V1, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   zz_pXMatrix M1;

   HalfGCD(M1, U1, V1, d1);
   mul(U1, V1, M1);

   long d2 = deg(V1) - deg(U) + n + d_red;

   if (IsZero(V1) || d2 <= 0) {
      M_out = M1;
      return;
   }


   zz_pX Q;
   zz_pXMatrix M2;

   DivRem(Q, U1, U1, V1);
   swap(U1, V1);

   HalfGCD(M2, U1, V1, d2);

   zz_pX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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




void XHalfGCD(zz_pXMatrix& M_out, zz_pX& U, zz_pX& V, long d_red)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));
 
      return;
   }

   long du = deg(U);

   if (d_red <= NTL_zz_pX_HalfGCD_CROSSOVER) {
      IterHalfGCD(M_out, U, V, d_red);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   zz_pXMatrix M1;

   HalfGCD(M1, U, V, d1);
   mul(U, V, M1);

   long d2 = deg(V) - du + d_red;

   if (IsZero(V) || d2 <= 0) {
      M_out = M1;
      return;
   }


   zz_pX Q;
   zz_pXMatrix M2;

   DivRem(Q, U, U, V);
   swap(U, V);

   XHalfGCD(M2, U, V, d2);

   zz_pX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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

void HalfGCD(zz_pX& U, zz_pX& V)
{
   long d_red = (deg(U)+1)/2;

   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      return;
   }

   long du = deg(U);


   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   zz_pXMatrix M1;

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


   zz_pX Q;

   DivRem(Q, U, U, V);
   swap(U, V);

   HalfGCD(M1, U, V, d2);

   mul(U, V, M1); 
}


void GCD(zz_pX& d, const zz_pX& u, const zz_pX& v)
{
   zz_pX u1, v1;

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

   while (deg(u1) > NTL_zz_pX_GCD_CROSSOVER && !IsZero(v1)) {
      HalfGCD(u1, v1);

      if (!IsZero(v1)) {
         rem(u1, u1, v1);
         swap(u1, v1);
      }
   }

   PlainGCD(d, u1, v1);
}



void XGCD(zz_pX& d, zz_pX& s, zz_pX& t, const zz_pX& a, const zz_pX& b)
{
   zz_p w;

   if (IsZero(a) && IsZero(b)) {
      clear(d);
      set(s);
      clear(t);
      return;
   }

   zz_pX U, V, Q;

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

   zz_pXMatrix M;

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

      





void IterBuild(zz_p* a, long n)
{
   long i, k;
   zz_p b, t;

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

void mul(zz_p* x, const zz_p* a, const zz_p* b, long n)
{
   zz_p t, accum;

   long i, j, jmin, jmax;

   long d = 2*n-1;

   for (i = 0; i <= d; i++) {
      jmin = max(0, i-(n-1));
      jmax = min(n-1, i);
      clear(accum);
      for (j = jmin; j <= jmax; j++) {
         mul(t, (a[j]), (b[i-j]));
         add(accum, accum, t);
      }
      if (i >= n) {
         add(accum, accum, (a[i-n]));
         add(accum, accum, (b[i-n]));
      }

      x[i] = accum;
   }
}


void BuildFromRoots(zz_pX& x, const vec_zz_p& a)
{
   long n = a.length();

   if (n == 0) {
      set(x);
      return;
   }

   long k0 = NextPowerOfTwo(NTL_zz_pX_MUL_CROSSOVER)-1;
   long crossover = 1L << k0;

   if (n <= NTL_zz_pX_MUL_CROSSOVER) {
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

   zz_pX b(INIT_SIZE, m+1);

   b.rep = a;
   b.rep.SetLength(m+1);
   for (i = n; i < m; i++)
      clear(b.rep[i]);

   set(b.rep[m]);
   
   fftRep R1(INIT_SIZE, k), R2(INIT_SIZE, k);


   zz_p t1, one;
   set(one);

   vec_zz_p G(INIT_SIZE, crossover), H(INIT_SIZE, crossover);
   zz_p *g = G.elts();
   zz_p *h = H.elts();
   zz_p *tmp;
   
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
         TofftRep(R1, b, l+1, i, i+width);
         b.rep[i+width] = t1;
         t1 = b.rep[i+2*width];
         set(b.rep[i+2*width]);
         TofftRep(R2, b, l+1, i+width, i+2*width);
         b.rep[i+2*width] = t1;
         mul(R1, R1, R2);
         FromfftRep(&b.rep[i], R1, 0, 2*width-1);
         sub(b.rep[i], b.rep[i], one);
      }
   }

   x.rep.SetLength(n+1);
   long delta = m-n;
   for (i = 0; i <= n; i++)
     x.rep[i] = b.rep[i+delta];

   // no need to normalize
}



void eval(zz_p& b, const zz_pX& f, zz_p a)
// does a Horner evaluation
{
   zz_p acc;
   long i;

   clear(acc);
   for (i = deg(f); i >= 0; i--) {
      mul(acc, acc, a);
      add(acc, acc, f.rep[i]);
   }

   b = acc;
}



void eval(vec_zz_p& b, const zz_pX& f, const vec_zz_p& a)
// naive algorithm:  repeats Horner
{
   if (&b == &f.rep) {
      vec_zz_p bb;
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




void interpolate(zz_pX& f, const vec_zz_p& a, const vec_zz_p& b)
{
   long m = a.length();
   if (b.length() != m) LogicError("interpolate: vector length mismatch");

   if (m == 0) {
      clear(f);
      return;
   }

   vec_zz_p prod;
   prod = a;

   zz_p t1, t2;

   long k, i;

   vec_zz_p res;
   res.SetLength(m);

   for (k = 0; k < m; k++) {

      const zz_p& aa = a[k];

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


   
void InnerProduct(zz_pX& x, const vec_zz_p& v, long low, long high, 
                   const vec_zz_pX& H, long n, vec_zz_p& t)
{
   zz_p s;
   long i, j;

   zz_p *tp = t.elts();

   for (j = 0; j < n; j++)
      clear(tp[j]);


   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();

   high = min(high, v.length()-1);
   for (i = low; i <= high; i++) {
      const vec_zz_p& h = H[i-low].rep;
      long m = h.length();
      zz_p w = (v[i]);

      long W = rep(w);
      mulmod_precon_t Wpinv = PrepMulModPrecon(W, p, pinv); 
      const zz_p *hp = h.elts();

      for (j = 0; j < m; j++) {
         long S = MulModPrecon(rep(hp[j]), W, p, Wpinv);
         S = AddMod(S, rep(tp[j]), p);
         tp[j].LoopHole() = S;
      }
   }

   x.rep = t;
   x.normalize();
}


void CompMod(zz_pX& x, const zz_pX& g, const zz_pXArgument& A, 
             const zz_pXModulus& F)
{
   if (deg(g) <= 0) {
      x = g;
      return;
   }


   zz_pX s, t;
   vec_zz_p scratch(INIT_SIZE, F.n);

   long m = A.H.length() - 1;
   long l = ((g.rep.length()+m-1)/m) - 1;

   zz_pXMultiplier M;
   build(M, A.H[m], F);

   InnerProduct(t, g.rep, l*m, l*m + m - 1, A.H, F.n, scratch);
   for (long i = l-1; i >= 0; i--) {
      InnerProduct(s, g.rep, i*m, i*m + m - 1, A.H, F.n, scratch);
      MulMod(t, t, M, F);
      add(t, t, s);
   }

   x = t;
}


void build(zz_pXArgument& A, const zz_pX& h, const zz_pXModulus& F, long m)
{
   if (m <= 0 || deg(h) >= F.n) LogicError("build: bad args");

   if (m > F.n) m = F.n;

   long i;

   if (zz_pXArgBound > 0) {
      double sz = 1;
      sz = sz*F.n;
      sz = sz+6;
      sz = sz*(sizeof (long));
      sz = sz/1024;
      m = min(m, long(zz_pXArgBound/sz));
      m = max(m, 1);
   }

   zz_pXMultiplier M;

   build(M, h, F);

   A.H.SetLength(m+1);

   set(A.H[0]);
   A.H[1] = h;
   for (i = 2; i <= m; i++) 
      MulMod(A.H[i], A.H[i-1], M, F);
}




NTL_CHEAP_THREAD_LOCAL long zz_pXArgBound = 0;


void CompMod(zz_pX& x, const zz_pX& g, const zz_pX& h, const zz_pXModulus& F)
   // x = g(h) mod f
{
   long m = SqrRoot(g.rep.length());

   if (m == 0) {
      clear(x);
      return;
   }

   zz_pXNewArgument A;

   build(A, h, F, m);

   CompMod(x, g, A, F);
}




void Comp2Mod(zz_pX& x1, zz_pX& x2, const zz_pX& g1, const zz_pX& g2,
              const zz_pX& h, const zz_pXModulus& F)

{
   long m = SqrRoot(g1.rep.length() + g2.rep.length());

   if (m == 0) {
      clear(x1);
      clear(x2);
      return;
   }

   zz_pXNewArgument A;

   build(A, h, F, m);

   zz_pX xx1, xx2;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);

   x1 = xx1;
   x2 = xx2;
}

void Comp3Mod(zz_pX& x1, zz_pX& x2, zz_pX& x3, 
              const zz_pX& g1, const zz_pX& g2, const zz_pX& g3,
              const zz_pX& h, const zz_pXModulus& F)

{
   long m = SqrRoot(g1.rep.length() + g2.rep.length() + g3.rep.length());

   if (m == 0) {
      clear(x1);
      clear(x2);
      clear(x3);
      return;
   }

   zz_pXNewArgument A;

   build(A, h, F, m);

   zz_pX xx1, xx2, xx3;

   CompMod(xx1, g1, A, F);
   CompMod(xx2, g2, A, F);
   CompMod(xx3, g3, A, F);

   x1 = xx1;
   x2 = xx2;
   x3 = xx3;
}




static void StripZeroes(vec_zz_p& x)
{
   long n = x.length();
   while (n > 0 && IsZero(x[n-1]))
      n--;
   x.SetLength(n);
}


void PlainUpdateMap(vec_zz_p& xx, const vec_zz_p& a, 
                    const zz_pX& b, const zz_pX& f)
{
   long n = deg(f);
   long i, m;

   if (IsZero(b)) {
      xx.SetLength(0);
      return;
   }

   m = n-1 - deg(b);

   vec_zz_p x(INIT_SIZE, n);

   for (i = 0; i <= m; i++)
      InnerProduct(x[i], a, b.rep, i);

   if (deg(b) != 0) {
      zz_pX c(INIT_SIZE, n);
      LeftShift(c, b, m);

      for (i = m+1; i < n; i++) {
         MulByXMod(c, c, f);
         InnerProduct(x[i], a, c.rep);
      }
   }

   xx = x;
}
   



void UpdateMap(vec_zz_p& x, const vec_zz_p& aa, 
               const zz_pXMultiplier& B, const zz_pXModulus& F)
{
   long n = F.n;

   vec_zz_p a;
   a = aa;
   StripZeroes(a);

   if (a.length() > n) LogicError("UpdateMap: bad args");
   long i;

   if (!B.UseFFT) {
      PlainUpdateMap(x, a, B.b, F.f);
      StripZeroes(x);
      return;
   }

   fftRep R1(INIT_SIZE, F.k), R2(INIT_SIZE, F.l);
   vec_zz_p V1(INIT_SIZE, n);


   RevTofftRep(R1, a, F.k, 0, a.length()-1, 0);
   mul(R2, R1, F.FRep);
   RevFromfftRep(V1, R2, 0, n-2);
   for (i = 0; i <= n-2; i++)  negate(V1[i], V1[i]);
   RevTofftRep(R2, V1, F.l, 0, n-2, n-1);
   mul(R2, R2, B.B1);
   mul(R1, R1, B.B2);

   AddExpand(R2, R1);
   RevFromfftRep(x, R2, 0, n-1);
   StripZeroes(x);
}

   

void ProjectPowers(vec_zz_p& x, const vec_zz_p& a, long k,
                   const zz_pXArgument& H, const zz_pXModulus& F)

{
   long n = F.n;

   if (a.length() > n || k < 0)
      LogicError("ProjectPowers: bad args");
   if (NTL_OVERFLOW(k, 1, 0))
      ResourceError("ProjectPowers: excessive args");

   long m = H.H.length()-1;
   long l = (k+m-1)/m - 1;

   zz_pXMultiplier M;
   build(M, H.H[m], F);

   vec_zz_p s(INIT_SIZE, n);
   s = a;
   StripZeroes(s);

   x.SetLength(k);

   for (long i = 0; i <= l; i++) {
      long m1 = min(m, k-i*m);
      zz_p* w = &x[i*m];
      for (long j = 0; j < m1; j++)
         InnerProduct(w[j], H.H[j].rep, s);
      if (i < l)
         UpdateMap(s, s, M, F);
   }
}

// zz_pXNewArgument stuff


void build(zz_pXNewArgument& H, const zz_pX& h, const zz_pXModulus& F, long m)
{
   long n = F.n;

   if (m <= 0 || deg(h) >= n) LogicError("build: bad args");

   if (NTL_OVERFLOW(m, 1, 0)) 
      ResourceError("zz_pXNewArgument:build: m too big");

   // NOTE: we don't take zz_pXArgBound into account, as the 
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


   zz_pXMultiplier M;
   build(M, h, F);

   Mat<zz_p> mat;
   mat.SetDims(m, width);
 
   zz_pX poly;
   poly = 1;

   for (long i = 0; i < m; i++) {
      VectorCopy(mat[i], poly, width);
      MulMod(poly, poly, M, F);
   }

   mat.swap(H.mat);
   poly.swap(H.poly);
}

void CompMod(zz_pX& x, const zz_pX& g, const zz_pXNewArgument& H, 
             const zz_pXModulus& F)
{
   long d = deg(g)+1;

   if (d <= 1) {
      x = g;
      return;
   }

   long m = H.mat.NumRows();

   if (m == 0) LogicError("CompMod: uninitialized argument");

   long l = (d+m-1)/m;

   Mat<zz_p> gmat;
   gmat.SetDims(l, m);

   for (long i = 0; i < l; i++) 
      for (long j = 0; j < m; j++)
         gmat[i][j] = coeff(g, i*m+j);

   Mat<zz_p> xmat;
   mul(xmat, gmat, H.mat);


   zz_pX t;
   conv(t, xmat[l-1]);

   if (l-2 >= 0) {
      zz_pXMultiplier M;
      build(M, H.poly, F);
      zz_pX s;

      for (long i = l-2; i >= 0; i--) {
	 conv(s, xmat[i]);
	 MulMod(t, t, M, F);
	 add(t, t, s);
      }
   }

   x = t;
}

void reduce(zz_pXNewArgument& H, const zz_pXModulus& F)
{
   long m = H.mat.NumRows();

   if (m == 0) LogicError("reduce: uninitialized argument");

   zz_pX h;
   if (m > 1)
      conv(h, H.mat[1]);
   else
      h = H.poly;

   rem(h, h, F);
   build(H, h, F, m);
}





void ProjectPowers(vec_zz_p& x, const vec_zz_p& a, long k,
                   const zz_pXNewArgument& H, const zz_pXModulus& F)

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

   Mat<zz_p> hmat, amat, xmat;
   transpose(hmat, H.mat);
   // NOTE: it would be better if we could compute
   // matrix*transpose(matrix), and avoid this
   // transposition altogether
   
   // NOTE: if we want to save on some memory usage,
   // we could kill H.mat at this point


   amat.SetDims(l, width);

   vec_zz_p s(INIT_SIZE, n);
   s = a;
   StripZeroes(s);

   VectorCopy(amat[0], s, width);

   if (l > 1) {
      zz_pXMultiplier M;
      build(M, H.poly, F);

      for (long i = 1; i < l; i++) {
	 UpdateMap(s, s, M, F);
	 VectorCopy(amat[i], s, width);
      }
   }

   mul(xmat, amat, hmat);

   x.SetLength(k);
   for (long i = 0; i < l; i++) {
      long j_max = min(m, k-i*m);
      for (long j = 0; j < j_max; j++)
         x[i*m+j] = xmat[i][j];
   }

}



void ProjectPowers(vec_zz_p& x, const vec_zz_p& a, long k,
                   const zz_pX& h, const zz_pXModulus& F)

{
   if (a.length() > F.n || k < 0) LogicError("ProjectPowers: bad args");

   if (k == 0) {
      x.SetLength(0);
      return;
   }

   long m = SqrRoot(k);

   zz_pXNewArgument H;

   build(H, h, F, m);
   ProjectPowers(x, a, k, H, F);
}









void BerlekampMassey(zz_pX& h, const vec_zz_p& a, long m)
{
   zz_pX Lambda, Sigma, Temp;
   long L;
   zz_p Delta, Delta1, t1;
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


void GCDMinPolySeq(zz_pX& h, const vec_zz_p& x, long m)
{
   long i;
   zz_pX a, b;
   zz_pXMatrix M;
   zz_p t;

   a.rep.SetLength(2*m);
   for (i = 0; i < 2*m; i++) a.rep[i] = x[2*m-1-i];
   a.normalize();

   SetCoeff(b, 2*m);

   HalfGCD(M, b, a, m+1);

   /* make monic */

   inv(t, LeadCoeff(M(1,1)));
   mul(h, M(1,1), t);
}


void MinPolySeq(zz_pX& h, const vec_zz_p& a, long m)
{
   if (m < 0 || NTL_OVERFLOW(m, 1, 0)) LogicError("MinPoly: bad args");
   if (a.length() < 2*m) LogicError("MinPoly: sequence too short");

   if (m > NTL_zz_pX_BERMASS_CROSSOVER)
      GCDMinPolySeq(h, a, m);
   else
      BerlekampMassey(h, a, m);
}


void DoMinPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F, long m,
               const vec_zz_p& R) 
{
   vec_zz_p x;

   ProjectPowers(x, R, 2*m, g, F);
   MinPolySeq(h, x, m);
}


void ProbMinPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F, long m)
{
   long n = F.n;
   if (m < 1 || m > n) LogicError("ProbMinPoly: bad args");

   long i;
   vec_zz_p R(INIT_SIZE, n);

   for (i = 0; i < n; i++) random(R[i]);
   DoMinPolyMod(h, g, F, m, R);
}

void MinPolyMod(zz_pX& hh, const zz_pX& g, const zz_pXModulus& F, long m)
{
   zz_pX h, h1;
   long n = F.n;
   if (m < 1 || m > n) LogicError("MinPoly: bad args");

   /* probabilistically compute min-poly */

   ProbMinPolyMod(h, g, F, m);
   if (deg(h) == m) { hh = h; return; }
   CompMod(h1, h, g, F);
   if (IsZero(h1)) { hh = h; return; }

   /* not completely successful...must iterate */

   long i;

   zz_pX h2, h3;
   zz_pXMultiplier H1;
   vec_zz_p R(INIT_SIZE, n);

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

void IrredPolyMod(zz_pX& h, const zz_pX& g, const zz_pXModulus& F, long m)
{
   vec_zz_p R(INIT_SIZE, 1);
   if (m < 1 || m > F.n) LogicError("IrredPoly: bad args");

   set(R[0]);
   DoMinPolyMod(h, g, F, m, R);
}



void diff(zz_pX& x, const zz_pX& a)
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

void MakeMonic(zz_pX& x)
{
   if (IsZero(x))
      return;

   if (IsOne(LeadCoeff(x)))
      return;

   zz_p t;

   inv(t, LeadCoeff(x));
   mul(x, x, t);
}




      
void PlainMulTrunc(zz_pX& x, const zz_pX& a, const zz_pX& b, long n)
{
   zz_pX y;
   mul(y, a, b);
   trunc(x, y, n);
}


void FFTMulTrunc(zz_pX& x, const zz_pX& a, const zz_pX& b, long n)
{
   if (IsZero(a) || IsZero(b)) {
      clear(x);
      return;
   }

   long d = deg(a) + deg(b);
   if (n > d + 1)
      n = d + 1;

   long k = NextPowerOfTwo(d + 1);
   fftRep R1(INIT_SIZE, k), R2(INIT_SIZE, k);

   TofftRep(R1, a, k);
   TofftRep(R2, b, k);
   mul(R1, R1, R2);
   FromfftRep(x, R1, 0, n-1);
}

void MulTrunc(zz_pX& x, const zz_pX& a, const zz_pX& b, long n)
{
   if (n < 0) LogicError("MulTrunc: bad args");

   if (deg(a) <= NTL_zz_pX_MUL_CROSSOVER || deg(b) <= NTL_zz_pX_MUL_CROSSOVER)
      PlainMulTrunc(x, a, b, n);
   else
      FFTMulTrunc(x, a, b, n);
}

void PlainSqrTrunc(zz_pX& x, const zz_pX& a, long n)
{
   zz_pX y;
   sqr(y, a);
   trunc(x, y, n);
}


void FFTSqrTrunc(zz_pX& x, const zz_pX& a, long n)
{
   if (IsZero(a)) {
      clear(x);
      return;
   }

   long d = 2*deg(a);
   if (n > d + 1)
      n = d + 1;

   long k = NextPowerOfTwo(d + 1);
   fftRep R1(INIT_SIZE, k);

   TofftRep(R1, a, k);
   mul(R1, R1, R1);
   FromfftRep(x, R1, 0, n-1);
}

void SqrTrunc(zz_pX& x, const zz_pX& a, long n)
{
   if (n < 0) LogicError("SqrTrunc: bad args");

   if (deg(a) <= NTL_zz_pX_MUL_CROSSOVER)
      PlainSqrTrunc(x, a, n);
   else
      FFTSqrTrunc(x, a, n);
}



void FastTraceVec(vec_zz_p& S, const zz_pX& f)
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
   zz_pX f1;

   f1.rep.SetLength(n-1);
   for (i = 0; i <= n-2; i++)
      f1.rep[i] = f.rep[n-i];
   f1.normalize();

   zz_pX f2;
   f2.rep.SetLength(n-1);
   for (i = 0; i <= n-2; i++)
      mul(f2.rep[i], f.rep[n-1-i], i+1);
   f2.normalize();

   zz_pX f3;
   InvTrunc(f3, f1, n-1);
   MulTrunc(f3, f3, f2, n-1);

   S.SetLength(n);

   S[0] = n;
   for (i = 1; i < n; i++)
      negate(S[i], coeff(f3, i-1));
}


void PlainTraceVec(vec_zz_p& S, const zz_pX& ff)
{
   if (deg(ff) <= 0)
      LogicError("TraceVec: bad args");

   zz_pX f;
   f = ff;

   MakeMonic(f);

   long n = deg(f);

   S.SetLength(n);

   if (n == 0)
      return;

   long k, i;
   zz_p acc, t;

   const zz_p *fp = f.rep.elts();;
   zz_p *sp = S.elts();

   sp[0] = n;

   for (k = 1; k < n; k++) {
      mul(acc, fp[n-k], k);

      for (i = 1; i < k; i++) {
         mul(t, fp[n-i], rep(sp[k-i]));
         add(acc, acc, t);
      }

      negate(sp[k], acc);
   }
}

void TraceVec(vec_zz_p& S, const zz_pX& f)
{
   if (deg(f) <= NTL_zz_pX_TRACE_CROSSOVER)
      PlainTraceVec(S, f);
   else
      FastTraceVec(S, f);
}

void ComputeTraceVec(vec_zz_p& S, const zz_pXModulus& F)
{
   if (!F.UseFFT) {
      PlainTraceVec(S, F.f);
      return;
   }

   long i;
   long n = F.n;

   fftRep R;
   zz_pX P, g;

   g.rep.SetLength(n-1);
   for (i = 1; i < n; i++)
      mul(g.rep[n-i-1], F.f.rep[n-i], i); 
   g.normalize();

   TofftRep(R, g, F.l);
   mul(R, R, F.HRep);
   FromfftRep(P, R, n-2, 2*n-4);

   S.SetLength(n);

   S[0] = n;
   for (i = 1; i < n; i++)
      negate(S[i], coeff(P, n-1-i));
}

void TraceMod(zz_p& x, const zz_pX& a, const zz_pXModulus& F)
{
   long n = F.n;

   if (deg(a) >= n)
      LogicError("trace: bad args");

   do { // NOTE: thread safe lazy init
      Lazy<vec_zz_p>::Builder builder(F.tracevec.val());
      if (!builder()) break;
      UniquePtr<vec_zz_p> p;
      p.make();
      ComputeTraceVec(*p, F);
      builder.move(p);
   } while (0);

   InnerProduct(x, a.rep, *F.tracevec.val());
}


void TraceMod(zz_p& x, const zz_pX& a, const zz_pX& f)
{
   if (deg(a) >= deg(f) || deg(f) <= 0)
      LogicError("trace: bad args");

   project(x, TraceVec(f), a);
}


void PlainResultant(zz_p& rres, const zz_pX& a, const zz_pX& b)
{
   zz_p res;
 
   if (IsZero(a) || IsZero(b))
      clear(res);
   else if (deg(a) == 0 && deg(b) == 0) 
      set(res);
   else {
      long d0, d1, d2;
      zz_p lc;
      set(res);

      long n = max(deg(a),deg(b)) + 1;
      zz_pX u(INIT_SIZE, n), v(INIT_SIZE, n);

      u = a;
      v = b;

      for (;;) {
         d0 = deg(u);
         d1 = deg(v);
         lc = LeadCoeff(v);

         PlainRem(u, u, v);
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


void ResIterHalfGCD(zz_pXMatrix& M_out, zz_pX& U, zz_pX& V, long d_red,
                    vec_zz_p& cvec, vec_long& dvec)
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

   zz_pX Q, t(INIT_SIZE, d_red);


   while (deg(V) > goal) {
      append(cvec, LeadCoeff(V));
      append(dvec, dvec[dvec.length()-1]-deg(U)+deg(V));
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
   


void ResHalfGCD(zz_pXMatrix& M_out, const zz_pX& U, const zz_pX& V, long d_red,
                vec_zz_p& cvec, vec_long& dvec)
{
   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      set(M_out(0,0));   clear(M_out(0,1));
      clear(M_out(1,0)); set(M_out(1,1));
 
      return;
   }


   long n = deg(U) - 2*d_red + 2;
   if (n < 0) n = 0;

   zz_pX U1, V1;

   RightShift(U1, U, n);
   RightShift(V1, V, n);

   if (d_red <= NTL_zz_pX_HalfGCD_CROSSOVER) { 
      ResIterHalfGCD(M_out, U1, V1, d_red, cvec, dvec);
      return;
   }

   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   zz_pXMatrix M1;

   ResHalfGCD(M1, U1, V1, d1, cvec, dvec);
   mul(U1, V1, M1);

   long d2 = deg(V1) - deg(U) + n + d_red;

   if (IsZero(V1) || d2 <= 0) {
      M_out = M1;
      return;
   }


   zz_pX Q;
   zz_pXMatrix M2;

   append(cvec, LeadCoeff(V1));
   append(dvec, dvec[dvec.length()-1]-deg(U1)+deg(V1));
   DivRem(Q, U1, U1, V1);
   swap(U1, V1);

   ResHalfGCD(M2, U1, V1, d2, cvec, dvec);

   zz_pX t(INIT_SIZE, deg(M1(1,1))+deg(Q)+1);

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

void ResHalfGCD(zz_pX& U, zz_pX& V, vec_zz_p& cvec, vec_long& dvec)
{
   long d_red = (deg(U)+1)/2;

   if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      return;
   }

   long du = deg(U);


   long d1 = (d_red + 1)/2;
   if (d1 < 1) d1 = 1;
   if (d1 >= d_red) d1 = d_red - 1;

   zz_pXMatrix M1;

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


   zz_pX Q;

   append(cvec, LeadCoeff(V));
   append(dvec, dvec[dvec.length()-1]-deg(U)+deg(V));
   DivRem(Q, U, U, V);
   swap(U, V);

   ResHalfGCD(M1, U, V, d2, cvec, dvec);

   mul(U, V, M1); 
}


void resultant(zz_p& rres, const zz_pX& u, const zz_pX& v)
{
   if (deg(u) <= NTL_zz_pX_GCD_CROSSOVER || deg(v) <= NTL_zz_pX_GCD_CROSSOVER) { 
      PlainResultant(rres, u, v);
      return;
   }

   zz_pX u1, v1;

   u1 = u;
   v1 = v;

   zz_p res, t;
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

   vec_zz_p cvec;
   vec_long  dvec;

   cvec.SetMaxLength(deg(v1)+2);
   dvec.SetMaxLength(deg(v1)+2);

   append(cvec, LeadCoeff(u1));
   append(dvec, deg(u1));


   while (deg(u1) > NTL_zz_pX_GCD_CROSSOVER && !IsZero(v1)) { 
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

void NormMod(zz_p& x, const zz_pX& a, const zz_pX& f)
{
   if (deg(f) <= 0 || deg(a) >= deg(f)) 
      LogicError("norm: bad args");

   if (IsZero(a)) {
      clear(x);
      return;
   }

   zz_p t;
   resultant(t, f, a);
   if (!IsOne(LeadCoeff(f))) {
      zz_p t1;
      power(t1, LeadCoeff(f), deg(a));
      inv(t1, t1);
      mul(t, t, t1);
   }

   x = t;
}


NTL_END_IMPL

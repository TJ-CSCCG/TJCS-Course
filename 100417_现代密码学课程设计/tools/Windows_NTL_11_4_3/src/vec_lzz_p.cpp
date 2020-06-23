
#include <NTL/vec_lzz_p.h>

NTL_START_IMPL


// NOTE: the signature for this is in lzz_p.h
void conv(vec_zz_p& x, const vec_ZZ& a)
{
   long i, n;

   n = a.length();
   x.SetLength(n);

   VectorConv(n, x.elts(), a.elts());
}

// NOTE: the signature for this is in lzz_p.h
void conv(vec_zz_p& x, const Vec<long>& a)
{
   long i, n;

   n = a.length();
   x.SetLength(n);

   VectorConv(n, x.elts(), a.elts());
}




void InnerProduct(zz_p& x, const vec_zz_p& a, const vec_zz_p& b)
{
   long n = min(a.length(), b.length());
   long i;

   long accum, t;
   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();

   const zz_p *ap = a.elts();
   const zz_p *bp = b.elts();

   accum = 0;
   for (i = 0; i < n; i++) {
      t = MulMod(rep(ap[i]), rep(bp[i]), p, pinv);
      accum = AddMod(accum, t, p);
   }

   x.LoopHole() = accum;
}

void InnerProduct(zz_p& x, const vec_zz_p& a, const vec_zz_p& b,
                  long offset)
{
   if (offset < 0) LogicError("InnerProduct: negative offset");
   if (NTL_OVERFLOW(offset, 1, 0)) ResourceError("InnerProduct: offset too big");

   long n = min(a.length(), b.length()+offset);
   long i;

   long accum, t;
   long p = zz_p::modulus();
   mulmod_t pinv = zz_p::ModulusInverse();


   const zz_p *ap = a.elts();
   const zz_p *bp = b.elts();

   accum = 0;
   for (i = offset; i < n; i++) {
      t = MulMod(rep(ap[i]), rep(bp[i-offset]), p, pinv);
      accum = AddMod(accum, t, p);
   }

   x.LoopHole() = accum;
}

long CRT(vec_ZZ& gg, ZZ& a, const vec_zz_p& G)
{
   long n = gg.length();
   if (G.length() != n) LogicError("CRT: vector length mismatch");

   long p = zz_p::modulus();

   ZZ new_a;
   mul(new_a, a, p);

   long a_inv;
   a_inv = rem(a, p);
   a_inv = InvMod(a_inv, p);

   long p1;
   p1 = p >> 1;

   ZZ a1;
   RightShift(a1, a, 1);

   long p_odd = (p & 1);

   long modified = 0;

   long h;

   ZZ g;
   long i;
   for (i = 0; i < n; i++) {
      if (!CRTInRange(gg[i], a)) {
         modified = 1;
         rem(g, gg[i], a);
         if (g > a1) sub(g, g, a);
      }
      else
         g = gg[i];
   
      h = rem(g, p);
      h = SubMod(rep(G[i]), h, p);
      h = MulMod(h, a_inv, p);
      if (h > p1)
         h = h - p;
   
      if (h != 0) {
         modified = 1;
   
         if (!p_odd && g > 0 && (h == p1))
            MulSubFrom(g, a, h);
         else
            MulAddTo(g, a, h);
      }

      gg[i] = g;
   }

   a = new_a;

   return modified;
}



void mul(vec_zz_p& x, const vec_zz_p& a, zz_p b)
{
   long n = a.length();
   x.SetLength(n);

   long i;

   if (n <= 1) {

      for (i = 0; i < n; i++)
         mul(x[i], a[i], b);

   }
   else {
 
      long p = zz_p::modulus();
      mulmod_t pinv = zz_p::ModulusInverse();
      long bb = rep(b);
      mulmod_precon_t bpinv = PrepMulModPrecon(bb, p, pinv);
      
      
      const zz_p *ap = a.elts();
      zz_p *xp = x.elts();

      for (i = 0; i < n; i++)
         xp[i].LoopHole() = MulModPrecon(rep(ap[i]), bb, p, bpinv);

   }
}

void mul(vec_zz_p& x, const vec_zz_p& a, long b_in)
{
   zz_p b;
   b = b_in;
   mul(x, a, b);
}



void add(vec_zz_p& x, const vec_zz_p& a, const vec_zz_p& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector add: dimension mismatch");

   long p = zz_p::modulus();

   x.SetLength(n);

   const zz_p *ap = a.elts();
   const zz_p *bp = b.elts();
   zz_p *xp = x.elts();

   long i;
   for (i = 0; i < n; i++)
      xp[i].LoopHole() = AddMod(rep(ap[i]), rep(bp[i]), p);
}

void sub(vec_zz_p& x, const vec_zz_p& a, const vec_zz_p& b)
{
   long n = a.length();
   if (b.length() != n) LogicError("vector sub: dimension mismatch");

   long p = zz_p::modulus();

   x.SetLength(n);


   const zz_p *ap = a.elts();
   const zz_p *bp = b.elts();
   zz_p *xp = x.elts();

   long i;
   for (i = 0; i < n; i++)
      xp[i].LoopHole() = SubMod(rep(ap[i]), rep(bp[i]), p);
}

void clear(vec_zz_p& x)
{
   long n = x.length();


   zz_p *xp = x.elts();

   long i;
   for (i = 0; i < n; i++)
      clear(xp[i]);
}

void negate(vec_zz_p& x, const vec_zz_p& a)
{
   long n = a.length();
   long p = zz_p::modulus();

   x.SetLength(n);


   const zz_p *ap = a.elts();
   zz_p *xp = x.elts();


   long i;
   for (i = 0; i < n; i++)
      xp[i].LoopHole() = NegateMod(rep(ap[i]), p);
}


long IsZero(const vec_zz_p& a)
{
   long n = a.length();


   const zz_p *ap = a.elts();

   long i;
   for (i = 0; i < n; i++)
      if (!IsZero(ap[i]))
         return 0;

   return 1;
}

vec_zz_p operator+(const vec_zz_p& a, const vec_zz_p& b)
{
   vec_zz_p res;
   add(res, a, b);
   NTL_OPT_RETURN(vec_zz_p, res);
}

vec_zz_p operator-(const vec_zz_p& a, const vec_zz_p& b)
{
   vec_zz_p res;
   sub(res, a, b);
   NTL_OPT_RETURN(vec_zz_p, res);
}


vec_zz_p operator-(const vec_zz_p& a)
{
   vec_zz_p res;
   negate(res, a);
   NTL_OPT_RETURN(vec_zz_p, res);
}


zz_p operator*(const vec_zz_p& a, const vec_zz_p& b)
{
   zz_p res;
   InnerProduct(res, a, b);
   return res;
}


void VectorCopy(vec_zz_p& x, const vec_zz_p& a, long n)
{
   if (n < 0) LogicError("VectorCopy: negative length");
   if (NTL_OVERFLOW(n, 1, 0)) ResourceError("overflow in VectorCopy");

   long m = min(n, a.length());

   x.SetLength(n);


   const zz_p *ap = a.elts();
   zz_p *xp = x.elts();

  
   long i;

   for (i = 0; i < m; i++)
      xp[i] = ap[i];

   for (i = m; i < n; i++)
      clear(xp[i]);
}


void random(vec_zz_p& x, long n)
{
   x.SetLength(n);
   VectorRandom(n, x.elts());
}

NTL_END_IMPL

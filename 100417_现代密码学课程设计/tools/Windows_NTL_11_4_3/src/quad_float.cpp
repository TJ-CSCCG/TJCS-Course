
// The quad_float module is derived from the doubledouble
// library originally developed by Keith Briggs:
//    http://keithbriggs.info/doubledouble.html
// I attach the original copyright notice.


/*

Copyright (C) 1997 Keith Martin Briggs

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

// The configure script tries to prevent this, but we
// double check here.  Note that while it is strongly 
// discouraged, other parts of NTL probably work even with 
// "fast math"; however, quad_float will definitely break.

#if (defined(__GNUC__) && __FAST_MATH__)
#error "do not compile quad_float.cpp with -ffast-math!!"
#endif

// The configure script should define NTL_FP_CONTRACT_OFF
// for icc via the NOCONTRACT variable
#ifdef NTL_FP_CONTRACT_OFF
#pragma fp_contract(off)
#endif

#if 0
// The configure script should ensure that all NTL files
// are compiled with --fp-model precise on icc.
#ifdef __INTEL_COMPILER
#pragma float_control(precise,on)
#endif
#endif

#include <NTL/quad_float.h>
#include <cfloat>


NTL_START_IMPL

#if (NTL_EXT_DOUBLE && defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))

#if (!defined(NTL_X86_FIX) && !defined(NTL_NO_X86_FIX))

#define NTL_X86_FIX

#endif

#endif


#if (NTL_EXT_DOUBLE && !defined(NTL_X86_FIX))

#define DOUBLE volatile double

#else

#define DOUBLE double

#endif


#ifdef NTL_X86_FIX


#define START_FIX \
  unsigned short __old_cw, __new_cw; \
  __asm__ volatile ("fnstcw %0":"=m" (__old_cw)::"memory"); \
  __new_cw = (__old_cw & ~0x300) | 0x200; \
  __asm__ volatile ("fldcw %0"::"m" (__new_cw):"memory");


#define END_FIX  __asm__ volatile ("fldcw %0": :"m" (__old_cw));

// NOTE: "asm volatile" does not guarantee that the asm does 
// not move.  However, the "memory" clobber makes these
// memory barriers that cannot move past a load/store

#define NO_INLINE __attribute__ ((noinline))
// to protect against LTO inlining which could break the memory
// barriers in START_FIX and END_FIX.  I've done some testing
// on gcc, clang, and icc.  The noinline attribute and the volatile
// asm together should ensure that the function gets called
// and doesn't get inlined during LTO.
// That said, I wouln't really recommend applying LTO to NTL...
// and especially to quad_float.cpp.


// NOTE: gcc 8.1 seems a bit buggy: it warns when overloading a function
// with different inline atrributes.  Earlier versions are fine.
// ICC and CLANG are fine.

// NOTE: starting with gcc 8.1, there is a function attribute called
// "noipa" which really does exactly what I want.  It would also be useful
// for ForceToMem, for example.  

#else

#define START_FIX
#define END_FIX

#define NO_INLINE

#endif




NO_INLINE void quad_float_normalize(quad_float& z, const double& xhi, const double& xlo)
{
START_FIX
   DOUBLE u, v;

   u = xhi + xlo; 
   v = xhi - u;    
   v = v + xlo;    

   z.hi = u;
   z.lo = v;
END_FIX
}

NO_INLINE void quad_float_in_place_add(quad_float& x, const quad_float& y ) {
START_FIX
        DOUBLE    H, h, T, t, S, s, e, f;
        DOUBLE    t1;

        S = x.hi + y.hi;
        T = x.lo + y.lo;
        e = S - x.hi;
        f = T - x.lo;

        t1 = S-e;
        t1 = x.hi-t1;
        s = y.hi-e;
        s = s + t1;
        
        t1 = T-f;
        t1 = x.lo-t1;
        t = y.lo-f;
        t = t + t1;


        s = s + T;
        H = S + s;
        h = S - H;
        h = h + s;

        h = h + t;
        e = H + h; 
        f = H - e;
        f = f + h;

        x.hi = e;
        x.lo = f;
END_FIX
}


NO_INLINE void quad_float_in_place_sub(quad_float& x, const quad_float& y ) {
START_FIX
        DOUBLE    H, h, T, t, S, s, e, f;
        DOUBLE    t1, yhi, ylo;

        yhi = -y.hi;
        ylo = -y.lo;

        S = x.hi + yhi;
        T = x.lo + ylo;
        e = S - x.hi;
        f = T - x.lo;

        t1 = S-e;
        t1 = x.hi-t1;
        s = yhi-e;
        s = s + t1;
        
        t1 = T-f;
        t1 = x.lo-t1;
        t = ylo-f;
        t = t + t1;


        s = s + T;
        H = S + s;
        h = S - H;
        h = h + s;

        h = h + t;
        e = H + h; 
        f = H - e;
        f = f + h;

        x.hi = e;
        x.lo = f;
END_FIX
}

NO_INLINE void quad_float_in_place_negate(quad_float& x)
{
START_FIX
   DOUBLE xhi, xlo, u, v;

   xhi = -x.hi;
   xlo = -x.lo;

   // it is a good idea to renormalize here, just in case
   // the rounding rule depends on sign, and thus we will
   // maintain the "normal form" for quad_float's.
  
   u = xhi + xlo;
   v = xhi - u;
   v = v + xlo;

   x.hi = u;
   x.lo = v;
END_FIX
}



#if (NTL_FMA_DETECTED && !defined(NTL_CONTRACTION_FIXED))


// The configure script should ensure that no FMA's are issued
// fo most compilers (at least gcc, clang, and icc), but if not,
// this is a last ditch effort to fix the problem (which seems to work).

double quad_float_zero = 0;

static inline
double Protect(double x) { return x + quad_float_zero; }

#else


static inline
double Protect(double x) { return x; }


#endif




NO_INLINE void quad_float_in_place_mul(quad_float& x,const quad_float& y ) {
START_FIX
  DOUBLE hx, tx, hy, ty, C, c;
  DOUBLE t1, t2;

  C = Protect(NTL_QUAD_FLOAT_SPLIT*x.hi);
  hx = C-x.hi;
  c = Protect(NTL_QUAD_FLOAT_SPLIT*y.hi);
  hx = C-hx;
  tx = x.hi-hx;
  hy = c-y.hi;
  C = Protect(x.hi*y.hi);
  hy = c-hy;
  ty = y.hi-hy;

  // c = ((((hx*hy-C)+hx*ty)+tx*hy)+tx*ty)+(x.hi*y.lo+x.lo*y.hi);
  
  t1 = Protect(hx*hy);
  t1 = t1-C;
  t2 = Protect(hx*ty);
  t1 = t1+t2;
  t2 = Protect(tx*hy);
  t1 = t1+t2;
  t2 = Protect(tx*ty);
  c = t1+t2;
  t1 = Protect(x.hi*y.lo);
  t2 = Protect(x.lo*y.hi);
  t1 = t1+t2;
  c = c + t1;


  hx = C+c;
  tx = C-hx;
  tx = tx+c;

  x.hi = hx;
  x.lo = tx;
END_FIX
}


NO_INLINE void quad_float_in_place_div(quad_float& x, const quad_float& y ) {
START_FIX
  DOUBLE hc, tc, hy, ty, C, c, U, u;
  DOUBLE t1;

  C = x.hi/y.hi;
  c = Protect(NTL_QUAD_FLOAT_SPLIT*C);
  hc = c-C;
  u = Protect(NTL_QUAD_FLOAT_SPLIT*y.hi);
  hc = c-hc;
  tc = C-hc;
  hy = u-y.hi;
  U = Protect(C * y.hi);
  hy = u-hy;
  ty = y.hi-hy;

  // u = (((hc*hy-U)+hc*ty)+tc*hy)+tc*ty;

  u = Protect(hc*hy);
  u = u-U;
  t1 = Protect(hc*ty);
  u = u+t1;
  t1 = Protect(tc*hy);
  u = u+t1;
  t1 = Protect(tc*ty);
  u = u+t1;

  // c = ((((x.hi-U)-u)+x.lo)-C*y.lo)/y.hi;

  c = x.hi-U;
  c = c-u;
  c = c+x.lo;
  t1 = Protect(C*y.lo);
  c = c - t1;
  c = c/y.hi;
  
  hy = C+c;
  ty = C-hy;
  ty = ty+c;

  x.hi = hy;
  x.lo = ty;
END_FIX
}


NO_INLINE void quad_float_in_place_sqrt(quad_float& y, double& c_ref) {
START_FIX
  DOUBLE c = c_ref;
  DOUBLE p,q,hx,tx,u,uu,cc;
  DOUBLE t1;

  p = Protect(NTL_QUAD_FLOAT_SPLIT*c); 
  hx = (c-p); 
  hx = hx+p; 
  tx = c-hx;
  p = Protect(hx*hx);
  q = Protect(hx*tx);
  q = q+q;

  u = p+q;
  uu = p-u;
  uu = uu+q;
  t1 = Protect(tx*tx);
  uu = uu+t1;


  cc = y.hi-u;
  cc = cc-uu;
  cc = cc+y.lo;
  t1 = c+c;
  cc = cc/t1;

  hx = c+cc;
  tx = c-hx;
  tx = tx+cc;

  y.hi = hx;
  y.lo = tx;
END_FIX
}


NO_INLINE void quad_float_PrecisionOK(long& res, const double& one)
{
START_FIX
   long k;
   DOUBLE l1 = one;
   DOUBLE lh = one/double(2);
   DOUBLE epsilon;
   DOUBLE fudge, oldfudge;

   epsilon = l1;
   fudge = l1+l1;

   k = 0;

   do {
      k++;
      epsilon = epsilon * lh;
      oldfudge = fudge;
      fudge = l1 + epsilon;
   } while (fudge > l1 && fudge < oldfudge);

   res = (k == NTL_DOUBLE_PRECISION);
END_FIX
}




NTL_END_IMPL


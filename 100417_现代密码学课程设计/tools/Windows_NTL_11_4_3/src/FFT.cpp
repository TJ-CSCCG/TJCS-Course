
#include <NTL/FFT.h>
#include <NTL/FFT_impl.h>

#ifdef NTL_ENABLE_AVX_FFT
#include <NTL/SmartPtr.h>
#include <NTL/pd_FFT.h>
#endif


/********************************************************************

This is an implementation of a "small prime" FFT, which lies at the heart of
ZZ_pX and zz_pX arithmetic, and impacts many other applications as well
(such as arithmetic in ZZ_pEX, zz_pEX, and ZZX).

The algorithm is a Truncated FFT based on code originally developed by David
Harvey.  David's code built on the single-precision modular multiplication
technique introduced in NTL many years ago, but also uses a "lazy
multiplication" technique, which reduces the number of "correction" steps that
need to be performed in each butterfly (see below for more details).  It also
implements a version of the Truncated FFT algorithm introduced by Joris van der
Hoeven at ISSAC 2004.  Also see "A cache-friendly truncated FFT", David Harvey,
Theoretical Computer Science Volume 410, Issues 27-29, 28 June 2009, Pages
2649-2658.

I have almost completely re-written David's original code to make it fit into
NTL's software framework; however, all all of the key logic is still based on
David's code.  David's original code also implemented a 2D transformation which
is more cache friendly for *very* large transforms.  However, my experimens
indicated this was only beneficial for transforms of size at least 2^20, and so
I did not incorporate this variant.

Here is the Copyright notice from David's original code:


==============================================================================

fft62: a library for number-theoretic transforms

Copyright (C) 2013, David Harvey

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


==============================================================================


SINGLE-PRECISION MODULAR ARITHMETIC

The implementation of arithmetic modulo n, where n is a "word sized" integer is
critical to the performance of the FFT.  Such word-sized modular arithmetic is
used throughout many other parts of NTL, and is a part of the external,
documented interface.

As NTL was initially built on top of Arjen Lenstra's LIP software, I stole a
lot of ideas from LIP.  One very nice ideas was LIP's way of handling
single-precision modular arithmetic.  Back in those days (the early 1990's), I
was targeting 32-machines, mainly SPARC stations.  LIP's stratgey was to
restrict n to 30 bits, and to compute a*b % n, where 0 <= a, b < n, the
follwong was computed:

   long q = long(double(a) * double(b) / double(n));
   long r = a*b - q*n;
   if (r >= n) 
      r -= n;
   else if (r < 0)
      r += n;

With quite reasonable assumptions about floating point (certainly, anything
even remotely close to IEEE 64-bit doubles), the computation of q always gives
the true quotient floor(a*b / n), plus or minus 1.  The computation of r is
done modulo the 2^{word size}, and the following if/then/else adjusts r as
necessary.  To be more portable, some of these computations should really be
done using unsigned arithmetic, but that is not so important here.  Also, the
adjustment steps can be replaced by simple non-branching instrictions sequences
involving SHIFT, AND, and ADD/SUB instructions.  On some modern machines, this
is usually faster and NTL uses this non-branching strategy.  However, on other
machines (modern x86's are  an example of this), conditional move instructions
can be used in place of branching, and this code can be faster than the
non-branching code.  NTL's performance-tuning script will figure out the best
way to do this.


Other simple optimizations can be done, such as precomputing 1/double(n) when n
remains fixed for many computations, as is often the case.  

Note also that this strategy works perfectly well even when a or b are larger
than n, but the quotient itself is bounded by 2^30.

This strategy worked well for many years.  I had considered employing
"Montgomery multiplication", but did not do so for a couple of reasons:
  1) it would require non-portable code, because Montgomery multiplication
     requires the computation of two-word products,
  2) I did not like the idea of working with "alternative representations"
     for integers mod n, as this would make the interfaces more awkward.

At some point in the early 2000's, this strategy was starting to slow things
down, as floating point arithmetic, especially the integer/floating point
conversions, was starting to slow down relative to integer arithmetic.  This
was especially true on x86 machines, which by this time was starting to become
the most important target.  As it happens, later in the 2000's, as the x86
platforms started to use SSE instructions in lieu of the old x87 FPU
instructions, this speed differential again became less of a problem.
Nevertheless, I introduced some new techniques that speed things up across a
variety of platforms.  I introduced this new technique in NTL 5.4 back in 2005.
I never claimed it was particularly new, and I never really documented many
details about it, but since then, it has come to be known as "Shoup
multiplcation" in a few papers, so I'll accept that. :-)  The paper "Faster
arithmetic for number-theoretic transforms" [David Harvey, J. Symb. Comp. 60
(2014)] seems to be the first place where it is discussed in detail,
and Harvey's paper also contains some improvements which I discuss below.

The basic idea is that in many computations, not only n, but one of the
arguments, say b, remains fixed for many computatations of a*b % n, and so we
can afford to do a little precomputation, based on b and n, to speed things up.
This approach does require the ability to compute double-word products
(actually, just the high word of the product), but it still presents the same
basic interface as before (i.e., no awkward, alternative representations);
moreover, on platforms where we can't get double-word products, the
implementation falls back to the old floating point strategy, and client code
need not be aware of this.

The basic idea is this: suppose 0 <= n < 2^w, and 0 <= a < 2^w, and 0 <= b < n.
We precompute bninv = floor(2^w*b/n).  Then if we compute q =
floor(a*bninv/2^w), it can be argued that q is either floor(a*b/n), or is 1 too
small.  The computation of bninv can be done using the floating point
techniques described above.  The computation of q can be done by computing the
high word of a double-word product (it helps if bninv is left-shifted an
appropriate amount first).  Once we have q, we can compute a*b - q*n as before,
and adjust (but now only one adjustment is needed).  So after the
precomputation.  the whole operation takes 3 multiplies (one doube-word and two
single-word), and a small handful of simple instructions (adds, shifts, etc).
Moreover, two of the three multiplies can start in parallel, on platforms where
this is possible.

David Harvey noticed that because on modern machines, multiplies are really not
that slow compared to additions, the cost of all of the adjustments (in the
MulMod, as well as in the AddMod and SubMod's in the basic FFT butterfly steps)
starts to dominate the cost of the FFT. Indeed, with a straightforward
implementation of the above ideas, there are three multiplies and three
adjustment steps in each butterfly step.  David's idea was to work with
redundant representations mod n, in the range [0..4*n), and thus reduce the
number of adjustments per butterfly from three to one.  I've implemented this
idea here, and it does indeed make a significant difference, which is even more
pronounced when all of the FFT multipliers b and corresponding bninv values are
precomputed.  My initial implementation of David's ideas (v6.0 in 2013) only
implemented his approach with these precomputated tables: it seemed that
without these tables, it was not a significant improvement.  However, I later
figured out how to reduce the cost of computing all the necessary data "on the
fly", in a way that seems only slightly (10-15%) slower overall.  I introduced
this in v9.1 in 2015, and set things up so that now the pre-computed tables are
still used, but not exclusively, in such a way as to reduce the memory used by
these tables for very large polynomials (either very high degree or lots of FFT
primes).   The idea here is simple, but I haven't seen it discussed elsewhere,
so I'll document the basic idea here.

Suppose we have the preconditioners for a and b, and want a*b % n along with
the preconditioner for a*b % n.

For a, let us suppose that we have both q1 and r1, where:
   2^w*a = n*q1 + r1
We can obtain both q1 and r1 using floating point techniques.

Step 1. Compute a*b % n, using the integer-only MulMod, using
either the preconditioner for either a or b.

Step 2. Compute q2 and r2 such that
   r1*b = n*q2 + r2
We can obtain these using the integer-only MulMod, preconditioned on b.
Actually, we only need q2, not r2.

Step 3. Compute
   q3 = q1*b + q2 mod 2^w
which we can compute with just a single-word multiply and an addition.

One can easily show that the value q3 computed above is indeed the
preconditioner for a*b % n.  

Note that, in theory, if the computation in Step 2 is done using the
preconditioner for a (i.e., q1), then the multiplication q1*b in Step 3 should
not really be necessary (assuming that computing both high and low words of a
doube-wprd product is no more expensive than just computing the low word).
However, none of the compilers I've used have been able to perform that
optimization (in NTL v11.1, I added code that hand-codes this optimization).


64-BIT MACHINES

Current versions of NTL use (by default) 60-bit moduli based
on all-integer arithemtic.


Prior to v9.0 of NTL, on 64 bits, the modulus n was restricted to 50 bits, in
order to allow the use of double-precision techniques, as double's have 53 bits
of precision.  However, NTL now supports 60-bit moduli.  Actually, 62 bits can
be supported by setting the NTL_MAXIMIZE_SP_NBITS configuraton flag, but other
things (namely, the TBL_REM implementation in lip.cpp) start to slow down if 62
bits are used, so 60 seems like a good compromise.  Currently,  60-bit moduli
are available only when compiling NTL with GMP, and when some kind of extended
integer of floating point arithmetic is available. 


FUTURE TRENDS


* The following papers

   https://eprint.iacr.org/2017/727
   https://eprint.iacr.org/2016/504
   https://eprint.iacr.org/2015/382

present FFTs that access the pre-computed tables in a somewhat more efficent
fashion, so that we only need to read from the tables O(n) times, rather than
O(n log n) times.  

I've partially implemented this, and have gotten mixed results.
For smallish FFT's (below k=10 or 11), this code is somewhat slower.
For larger FFT's (say, k=17), I see a speedup of 3-10%.


********************************************************************/



#define NTL_FFT_BIGTAB_LIMIT (180)
#define NTL_FFT_BIGTAB_MAXROOT (17)
#define NTL_FFT_BIGTAB_MINROOT (7)

// table sizes are bounded by 2^bound, where 
// bound = NTL_FFT_BIGTAB_MAXROOT-index/NTL_FFT_BIGTAB_LIMIT.
// Here, index is the index of an FFT prime, or 0 for a user FFT prime.
// If bound <= NTL_FFT_BIGTAB_MINROOT, then big tables are not used,
// so only the first 
//    (NTL_FFT_BIGTAB_MAXROOT-NTL_FFT_BIGTAB_MINROOT)*NTL_FFT_BIGTAB_LIMIT
// FFT primes will have big tables.

// NOTE: in newer versions of NTL (v9.1 and later), the BIGTAB
// code is only about 5-15% faster than the non-BIGTAB code, so
// this is not a great time/space trade-off.
// However, some futher optimizations may only be implemented 
// if big tables are used.

// NOTE: NTL_FFT_BIGTAB_MAXROOT is set independently of the parameter
// NTL_FFTMaxRoot defined in FFT.h (and which is typically 25).
// The space for the LazyTable FFTMultipliers could be reduced a bit
// by using min(NTL_FFT_BIGTAB_MAXROOT, NTL_FFTMaxRoot) + 1 for the
// size of these tables.



NTL_START_IMPL



class FFTVectorPair {
public:
   Vec<long> wtab_precomp;
   Vec<mulmod_precon_t> wqinvtab_precomp;
};

typedef LazyTable<FFTVectorPair, NTL_FFTMaxRoot+1> FFTMultipliers;


#ifdef NTL_ENABLE_AVX_FFT
class pd_FFTVectorPair {
public:
   AlignedArray<double> wtab_precomp;
   AlignedArray<double> wqinvtab_precomp;
};

typedef LazyTable<pd_FFTVectorPair, NTL_FFTMaxRoot+1> pd_FFTMultipliers;
#endif



class FFTMulTabs {
public:

#ifndef NTL_ENABLE_AVX_FFT
   long bound;
   FFTMultipliers MulTab;
#else
   pd_FFTMultipliers pd_MulTab[2];
#endif

};

void FFTMulTabsDeleterPolicy::deleter(FFTMulTabs *p) { delete p; }



FFTTablesType FFTTables;
// a truly GLOBAL variable, shared among all threads



long IsFFTPrime(long n, long& w)
{
   long  m, x, y, z;
   long j, k;


   if (n <= 1 || n >= NTL_SP_BOUND) return 0;

   if (n % 2 == 0) return 0;

   if (n % 3 == 0) return 0;

   if (n % 5 == 0) return 0;

   if (n % 7 == 0) return 0;
   
   m = n - 1;
   k = 0;
   while ((m & 1) == 0) {
      m = m >> 1;
      k++;
   }

   for (;;) {
      x = RandomBnd(n);

      if (x == 0) continue;
      z = PowerMod(x, m, n);
      if (z == 1) continue;

      x = z;
      j = 0;
      do {
         y = z;
         z = MulMod(y, y, n);
         j++;
      } while (j != k && z != 1);

      if (z != 1 || y !=  n-1) return 0;

      if (j == k) 
         break;
   }

   /* x^{2^k} = 1 mod n, x^{2^{k-1}} = -1 mod n */

   long TrialBound;

   TrialBound = m >> k;
   if (TrialBound > 0) {
      if (!ProbPrime(n, 5)) return 0;
   
      /* we have to do trial division by special numbers */
   
      TrialBound = SqrRoot(TrialBound);
   
      long a, b;
   
      for (a = 1; a <= TrialBound; a++) {
         b = (a << k) + 1;
         if (n % b == 0) return 0; 
      }
   }

   /* n is an FFT prime */


   for (j = NTL_FFTMaxRoot; j < k; j++) {
      x = MulMod(x, x, n);
   }

   w = x;

   return 1;
}


static
void NextFFTPrime(long& q, long& w, long index)
{
   static long m = NTL_FFTMaxRootBnd + 1;
   static long k = 0;
   // m and k are truly GLOBAL variables, shared among
   // all threads.  Access is protected by a critical section
   // guarding FFTTables

   static long last_index = -1;
   static long last_m = 0;
   static long last_k = 0;

   if (index == last_index) {
      // roll back m and k...part of a simple error recovery
      // strategy if an exception was thrown in the last 
      // invocation of UseFFTPrime...probably of academic 
      // interest only

      m = last_m;
      k = last_k;
   }
   else {
      last_index = index;
      last_m = m;
      last_k = k;
   }

   long t, cand;

   for (;;) {
      if (k == 0) {
         m--;
         if (m < 5) ResourceError("ran out of FFT primes");
         k = 1L << (NTL_SP_NBITS-m-2);
      }

      k--;

      cand = (1L << (NTL_SP_NBITS-1)) + (k << (m+1)) + (1L << m) + 1;

      if (!IsFFTPrime(cand, t)) continue;
      q = cand;
      w = t;
      return;
   }
}


long CalcMaxRoot(long p)
{
   p = p-1;
   long k = 0;
   while ((p & 1) == 0) {
      p = p >> 1;
      k++;
   }

   if (k > NTL_FFTMaxRoot)
      return NTL_FFTMaxRoot;
   else
      return k; 
}




#ifndef NTL_WIZARD_HACK
SmartPtr<zz_pInfoT> Build_zz_pInfo(FFTPrimeInfo *info);
#else
SmartPtr<zz_pInfoT> Build_zz_pInfo(FFTPrimeInfo *info) { return 0; }
#endif

void UseFFTPrime(long index)
{
   if (index < 0) LogicError("invalud FFT prime index");
   if (index >= NTL_MAX_FFTPRIMES) ResourceError("FFT prime index too large");

   if (index+1 >= NTL_NSP_BOUND) ResourceError("FFT prime index too large");
   // largely acacedemic, but it is a convenient assumption

   do {  // NOTE: thread safe lazy init
      FFTTablesType::Builder bld(FFTTables, index+1);
      long amt = bld.amt();
      if (!amt) break;

      long first = index+1-amt;
      // initialize entries first..index

      long i;
      for (i = first; i <= index; i++) {
         UniquePtr<FFTPrimeInfo> info;
         info.make();

         long q, w;
         NextFFTPrime(q, w, i);

         long bigtab_index = -1;

#ifdef NTL_FFT_BIGTAB
         bigtab_index = i;
#endif

         InitFFTPrimeInfo(*info, q, w, bigtab_index);
         info->zz_p_context = Build_zz_pInfo(info.get());
         bld.move(info);
      }

   } while (0);
}


#ifdef NTL_FFT_LAZYMUL 
// we only honor the FFT_LAZYMUL flag if either the SPMM_ULL_VIABLE or LONGLONG_SP_MULMOD 
// flags are set

#if (!defined(NTL_SPMM_ULL_VIABLE) && !defined(NTL_LONGLONG_SP_MULMOD))
#undef NTL_FFT_LAZYMUL

// raise an error if running the wizard
#if (defined(NTL_WIZARD_HACK))
#error "cannot honor NTL_FFT_LAZYMUL"
#endif

#endif

#endif




#ifdef NTL_FFT_LAZYMUL
// FFT with  lazy multiplication

#ifdef NTL_CLEAN_INT
#define NTL_FFT_USEBUF
#endif
// DIRT: with the lazy multiplication strategy, we have to work
// with unisgned long's rather than long's.  To avoid unnecessary
// copying, we simply cast long* to unsigned long*.
// Is this standards compliant? Does it evoke Undefined Behavior?
// The C++ standard before C++14 were actually somewhat inconsistent 
// on this point.

// In all versions of the C++ and C standards, the "strict aliasing"
// rules [basic.lval] have always said that signed/unsigned can
// always alias each other.  So this does not break the strict
// aliasing rules.  However, prior to C++14, the section
// on Lvalue-to-rvalue conversion [conv.lval] said that
// this was actually UB.  This has been cleared up in C++14,
// where now it is no longer UB.  Actally, it seems that the change
// to C++14 was cleaning up an inconsistency in the standard
// itself, and not really a change in the language definition.

// In practice, it does make a significant difference in performance
// to avoid all these copies, so the default is avoid them.

// See: https://stackoverflow.com/questions/30048135/efficient-way-to-bit-copy-a-signed-integer-to-an-unsigned-integer

// See: https://stackoverflow.com/questions/27109701/aliasing-of-otherwise-equivalent-signed-and-unsigned-types 
// Especially comments by Columbo regarding N3797 and [conv.lval] 






#if (defined(NTL_LONGLONG_SP_MULMOD))


#if (NTL_BITS_PER_LONG >= NTL_SP_NBITS+4) 

static inline unsigned long 
sp_NormalizedLazyPrepMulModPreconWithRem(unsigned long& rres, long b, long n, unsigned long ninv)
{
   unsigned long H = cast_unsigned(b);
   unsigned long Q = ll_mul_hi(H << 4, ninv);
   unsigned long L = cast_unsigned(b) << (NTL_SP_NBITS+2);
   long r = L - Q*cast_unsigned(n);  // r in [0..2*n)

   r = sp_CorrectExcessQuo(Q, r, n);
   rres = r;
   return Q; // NOTE: not shifted
}

static inline unsigned long 
sp_NormalizedLazyPrepMulModPrecon(long b, long n, unsigned long ninv)
{
   unsigned long H = cast_unsigned(b);
   unsigned long Q = ll_mul_hi(H << 4, ninv);
   unsigned long L = cast_unsigned(b) << (NTL_SP_NBITS+2);
   long r = L - Q*cast_unsigned(n);  // r in [0..2*n)

   Q += 1L + sp_SignMask(r-n);
   return Q; // NOTE: not shifted
}


#else

// NTL_BITS_PER_LONG == NTL_SP_NBITS+2
static inline unsigned long 
sp_NormalizedLazyPrepMulModPreconWithRem(unsigned long& rres, long b, long n, unsigned long ninv)
{
   unsigned long H = cast_unsigned(b) << 2;
   unsigned long Q = ll_mul_hi(H, (ninv << 1)) + H;
   unsigned long rr = -Q*cast_unsigned(n);  // r in [0..3*n)

   long r = sp_CorrectExcessQuo(Q, rr, n);
   r = sp_CorrectExcessQuo(Q, r, n);
   rres = r;
   return Q;  // NOTE: not shifted
}

static inline unsigned long 
sp_NormalizedLazyPrepMulModPrecon(long b, long n, unsigned long ninv)
{
   unsigned long H = cast_unsigned(b) << 2;
   unsigned long Q = ll_mul_hi(H, (ninv << 1)) + H;
   unsigned long rr = -Q*cast_unsigned(n);  // r in [0..3*n)
   Q += 2L + sp_SignMask(rr-n) + sp_SignMask(rr-2*n);
   return Q; // NOTE: not shifted
}


#endif


static inline unsigned long
LazyPrepMulModPrecon(long b, long n, sp_inverse ninv)
{
   return sp_NormalizedLazyPrepMulModPrecon(b << ninv.shamt, n << ninv.shamt, ninv.inv) << (NTL_BITS_PER_LONG-NTL_SP_NBITS-2);
}


static inline unsigned long
LazyPrepMulModPreconWithRem(unsigned long& rres, long b, long n, sp_inverse ninv)
{
   unsigned long qq, rr;
   qq = sp_NormalizedLazyPrepMulModPreconWithRem(rr, b << ninv.shamt, n << ninv.shamt, ninv.inv); 
   rres = rr >> ninv.shamt;
   return qq << (NTL_BITS_PER_LONG-NTL_SP_NBITS-2);
}








#elif (NTL_BITS_PER_LONG - NTL_SP_NBITS >= 4 && NTL_WIDE_DOUBLE_PRECISION - NTL_SP_NBITS >= 4)


// slightly faster functions, which should kick in on x86-64, where 
//    NTL_BITS_PER_LONG == 64
//    NTL_SP_NBITS == 60 (another reason for holding this back to 60 bits)
//    NTL_WIDE_DOUBLE_PRECISION == 64

// DIRT: if the relative error in floating point calcuations (muls and reciprocals)
//   is <= epsilon, the relative error in the calculations is <= 3*epsilon +
//   O(epsilon^2), and we require that this relative error is at most
//   2^{-(NTL_SP_NBITS+2)}, so it should be pretty safe as long as
//   epsilon is at most, or not much geater than, 2^{-NTL_WIDE_DOUBLE_PRECISION}.

static inline 
unsigned long LazyPrepMulModPrecon(long b, long n, wide_double ninv)
{
   long q = (long) ( (((wide_double) b) * wide_double(4*NTL_SP_BOUND)) * ninv ); 

   unsigned long rr = (cast_unsigned(b) << (NTL_SP_NBITS+2)) 
                       - cast_unsigned(q)*cast_unsigned(n);

   q += sp_SignMask(rr) + sp_SignMask(rr-n) + 1L;

   return cast_unsigned(q) << (NTL_BITS_PER_LONG - NTL_SP_NBITS - 2);
}

static inline 
unsigned long LazyPrepMulModPreconWithRem(unsigned long& rres, long b, long n, wide_double ninv)
{
   long q = (long) ( (((wide_double) b) * wide_double(4*NTL_SP_BOUND)) * ninv ); 

   unsigned long rr = (cast_unsigned(b) << (NTL_SP_NBITS+2)) 
                       - cast_unsigned(q)*cast_unsigned(n);

   long r = sp_CorrectDeficitQuo(q, rr, n);
   r = sp_CorrectExcessQuo(q, r, n);

   unsigned long qres = cast_unsigned(q) << (NTL_BITS_PER_LONG - NTL_SP_NBITS - 2);
   rres = r;
   return qres;
}

#else


static inline 
unsigned long LazyPrepMulModPrecon(long b, long n, wide_double ninv)
{
   long q = (long) ( (((wide_double) b) * wide_double(NTL_SP_BOUND)) * ninv ); 

   unsigned long rr = (cast_unsigned(b) << (NTL_SP_NBITS)) 
                       - cast_unsigned(q)*cast_unsigned(n);

   long r = sp_CorrectDeficitQuo(q, rr, n);
   r = sp_CorrectExcessQuo(q, r, n);

   unsigned long qq = q;

   qq = 2*qq;
   r = 2*r;
   r = sp_CorrectExcessQuo(qq, r, n);

   qq = 2*qq;
   r = 2*r;
   qq += sp_SignMask(r-n) + 1L;

   return qq << (NTL_BITS_PER_LONG - NTL_SP_NBITS - 2);
}





static inline 
unsigned long LazyPrepMulModPreconWithRem(unsigned long& rres, long b, long n, wide_double ninv)
{
   long q = (long) ( (((wide_double) b) * wide_double(NTL_SP_BOUND)) * ninv ); 

   unsigned long rr = (cast_unsigned(b) << (NTL_SP_NBITS)) 
                       - cast_unsigned(q)*cast_unsigned(n);

   long r = sp_CorrectDeficitQuo(q, rr, n);
   r = sp_CorrectExcessQuo(q, r, n);

   unsigned long qq = q;

   qq = 2*qq;
   r = 2*r;
   r = sp_CorrectExcessQuo(qq, r, n);

   qq = 2*qq;
   r = 2*r;
   r = sp_CorrectExcessQuo(qq, r, n);

   rres = r;
   return qq << (NTL_BITS_PER_LONG - NTL_SP_NBITS - 2);
}

#endif



static inline
unsigned long LazyMulModPreconQuo(unsigned long a, unsigned long b, 
                                  unsigned long n, unsigned long bninv)
{
   unsigned long q = ll_mul_hi(a, bninv);
   unsigned long r = a*b - q*n;
   q += sp_SignMask(r-n) + 1L;
   return q << (NTL_BITS_PER_LONG - NTL_SP_NBITS - 2);
}


static inline 
unsigned long LazyMulModPrecon(unsigned long a, unsigned long b, 
                               unsigned long n, unsigned long bninv)
{
   unsigned long q = ll_mul_hi(a, bninv);
   unsigned long res = a*b - q*n;
   return res;
}


typedef long mint_t;
typedef unsigned long umint_t;
// For readability and to make it easier to adapt this
// code to other settings

static inline 
umint_t LazyReduce1(umint_t a, mint_t q)
{
  return sp_CorrectExcess(mint_t(a), q);
}

static inline 
umint_t LazyReduce2(umint_t a, mint_t q)
{
  return sp_CorrectExcess(a, 2*q);
}


// inputs in [0, 2*n), output in [0, 4*n)
static inline 
umint_t LazyAddMod(umint_t a, umint_t b, mint_t n)
{
   return a+b;
}

// inputs in [0, 2*n), output in [0, 4*n)
static inline 
umint_t LazySubMod(umint_t a, umint_t b, mint_t n)
{
   return a-b+2*n;
}

// inputs in [0, 2*n), output in [0, 2*n)
static inline 
umint_t LazyAddMod2(umint_t a, umint_t b, mint_t n)
{
   umint_t r = a+b;
   return sp_CorrectExcess(r, 2*n);
}

// inputs in [0, 2*n), output in [0, 2*n)
static inline 
umint_t LazySubMod2(umint_t a, umint_t b, mint_t n)
{
   umint_t r = a-b;
   return sp_CorrectDeficit(r, 2*n);
}

#ifdef NTL_AVOID_BRANCHING

// x, y in [0, 4*m)
// returns x + y mod 4*m, in [0, 4*m)
inline static umint_t 
LazyAddMod4(umint_t x, umint_t y, mint_t m)
{
   x = LazyReduce2(x, m);
   y = LazyReduce2(y, m);
   return x+y;
}

// x, y in [0, 4*m)
// returns x - y mod 4*m, in [0, 4*m)
inline static umint_t 
LazySubMod4(umint_t x, umint_t y, mint_t m)
{
   x = LazyReduce2(x, m);
   y = LazyReduce2(y, m);
   return x-y+2*m;
}

#else

static inline umint_t 
LazyAddMod4(umint_t x, umint_t y, umint_t m)
{
  y = 4*m - y;
  umint_t z = x - y;
  z += (x < y) ? 4*m : 0;
  return z;
}


static inline umint_t 
LazySubMod4(umint_t x, umint_t y, umint_t m)
{
  umint_t z = x - y;
  z += (x < y) ? 4*m : 0;
  return z;
}

#endif

// Input and output in [0, 4*n)
static inline umint_t
LazyDoubleMod4(umint_t a, mint_t n)
{
   return 2 * LazyReduce2(a, n);
}

// Input and output in [0, 2*n)
static inline umint_t
LazyDoubleMod2(umint_t a, mint_t n)
{
   return 2 * LazyReduce1(a, n);
}

void ComputeMultipliers(Vec<FFTVectorPair>& v, long k, mint_t q, mulmod_t qinv, const mint_t* root)
{

   long old_len = v.length();
   v.SetLength(k+1);

   for (long s = max(old_len, 1); s <= k; s++) {
      v[s].wtab_precomp.SetLength(1L << (s-1));
      v[s].wqinvtab_precomp.SetLength(1L << (s-1));
   }

   if (k >= 1) {
      v[1].wtab_precomp[0] = 1;
      v[1].wqinvtab_precomp[0] = LazyPrepMulModPrecon(1, q, qinv);
   }

   if (k >= 2) {
      v[2].wtab_precomp[0] = v[1].wtab_precomp[0];
      v[2].wtab_precomp[1] = root[2];
      v[2].wqinvtab_precomp[0] = v[1].wqinvtab_precomp[0];
      v[2].wqinvtab_precomp[1] = LazyPrepMulModPrecon(root[2], q, qinv);
   }

   for (long s = 3; s <= k; s++) {
      long m = 1L << s;
      long m_half = 1L << (s-1);
      long m_fourth = 1L << (s-2);
      mint_t* NTL_RESTRICT wtab = v[s].wtab_precomp.elts();
      mint_t* NTL_RESTRICT wtab1 = v[s-1].wtab_precomp.elts();
      mulmod_precon_t* NTL_RESTRICT wqinvtab = v[s].wqinvtab_precomp.elts();
      mulmod_precon_t* NTL_RESTRICT wqinvtab1 = v[s-1].wqinvtab_precomp.elts();

      mint_t w = root[s];
      umint_t wqinv_rem;
      mulmod_precon_t wqinv = LazyPrepMulModPreconWithRem(wqinv_rem, w, q, qinv);


      for (long i = m_half-1, j = m_fourth-1; i >= 0; i -= 2, j--) {
         mint_t w_j = wtab1[j];
         mulmod_precon_t wqi_j = wqinvtab1[j];

#if 0
         mint_t w_i = LazyReduce1(LazyMulModPrecon(w_j, w, q, wqinv), q);
         mulmod_precon_t wqi_i = LazyMulModPreconQuo(wqinv_rem, w_j, q, wqi_j) 
                                   + cast_unsigned(w_j)*wqinv;
#else
         // This code sequence makes sure the compiler sees
         // that the product w_j*wqinv needs to be computed just once
         ll_type x;
         ll_mul(x, w_j, wqinv);
         umint_t hi = ll_get_hi(x);
         umint_t lo = ll_get_lo(x);
         umint_t r = cast_unsigned(w_j)*cast_unsigned(w) - hi*cast_unsigned(q);

         mint_t w_i = LazyReduce1(r, q);
         mulmod_precon_t wqi_i = lo+LazyMulModPreconQuo(wqinv_rem, w_j, q, wqi_j); 
#endif

         wtab[i-1] = w_j;
         wqinvtab[i-1] = wqi_j;
         wtab[i] = w_i;
         wqinvtab[i] = wqi_i;
      }
   }

#if 0
   // verify result
   for (long s = 1; s <= k; s++) {
      mint_t *wtab = v[s].wtab_precomp.elts();
      mulmod_precon_t *wqinvtab = v[s].wqinvtab_precomp.elts();
      long m_half = 1L << (s-1);

      mint_t w = root[s];
      mint_t w_i = 1;
      for (long i = 0; i < m_half; i++) {
         if (wtab[i] != w_i || wqinvtab[i] != LazyPrepMulModPrecon(w_i, q, qinv))
            Error("bad table entry");
         w_i = MulMod(w_i, w, q, qinv);
      }
   }
#endif
}


#else


// Hacks to make the LAZY code work with ordinary modular arithmetic

typedef long mint_t;
typedef long umint_t;

static inline mint_t IdentityMod(mint_t a, mint_t q) { return a; }
static inline mint_t DoubleMod(mint_t a, mint_t q) { return AddMod(a, a, q); }

#define LazyPrepMulModPrecon PrepMulModPrecon
#define LazyMulModPrecon MulModPrecon

#define LazyReduce1 IdentityMod
#define LazyReduce2 IdentityMod
#define LazyAddMod AddMod
#define LazySubMod SubMod
#define LazyAddMod2 AddMod
#define LazySubMod2 SubMod
#define LazyAddMod4 AddMod
#define LazySubMod4 SubMod
#define LazyDoubleMod2 DoubleMod
#define LazyDoubleMod4 DoubleMod


void ComputeMultipliers(Vec<FFTVectorPair>& v, long k, mint_t q, mulmod_t qinv, const mint_t* root)
{

   long old_len = v.length();
   v.SetLength(k+1);

   for (long s = max(old_len, 1); s <= k; s++) {
      v[s].wtab_precomp.SetLength(1L << (s-1));
      v[s].wqinvtab_precomp.SetLength(1L << (s-1));
   }

   if (k >= 1) {
      v[1].wtab_precomp[0] = 1;
      v[1].wqinvtab_precomp[0] = PrepMulModPrecon(1, q, qinv);
   }

   if (k >= 2) {
      v[2].wtab_precomp[0] = v[1].wtab_precomp[0];
      v[2].wtab_precomp[1] = root[2];
      v[2].wqinvtab_precomp[0] = v[1].wqinvtab_precomp[0];
      v[2].wqinvtab_precomp[1] = PrepMulModPrecon(root[2], q, qinv);
   }

   for (long s = 3; s <= k; s++) {
      long m = 1L << s;
      long m_half = 1L << (s-1);
      long m_fourth = 1L << (s-2);
      mint_t* NTL_RESTRICT wtab = v[s].wtab_precomp.elts();
      mint_t* NTL_RESTRICT wtab1 = v[s-1].wtab_precomp.elts();
      mulmod_precon_t* NTL_RESTRICT wqinvtab = v[s].wqinvtab_precomp.elts();
      mulmod_precon_t* NTL_RESTRICT wqinvtab1 = v[s-1].wqinvtab_precomp.elts();

      mint_t w = root[s];
      mulmod_precon_t wqinv = PrepMulModPrecon(w, q, qinv);


      for (long i = m_half-1, j = m_fourth-1; i >= 0; i -= 2, j--) {
         mint_t w_j = wtab1[j];
         mulmod_precon_t wqi_j = wqinvtab1[j];

         mint_t w_i = MulModPrecon(w_j, w, q, wqinv);
         mulmod_precon_t wqi_i = PrepMulModPrecon(w_i, q, qinv); 

         wtab[i-1] = w_j;
         wqinvtab[i-1] = wqi_j;
         wtab[i] = w_i;
         wqinvtab[i] = wqi_i;
      }
   }

#if 0
   // verify result
   for (long s = 1; s <= k; s++) {
      mint_t *wtab = v[s].wtab_precomp.elts();
      mulmod_precon_t *wqinvtab = v[s].wqinvtab_precomp.elts();
      long m_half = 1L << (s-1);

      mint_t w = root[s];
      mint_t w_i = 1;
      for (long i = 0; i < m_half; i++) {
         if (wtab[i] != w_i || wqinvtab[i] != PrepMulModPrecon(w_i, q, qinv))
            Error("bad table entry");
         w_i = MulMod(w_i, w, q, qinv);
      }
   }
#endif
}

#endif



static
void LazyPrecompFFTMultipliers(long k, mint_t q, mulmod_t qinv, const mint_t *root, const FFTMultipliers& tab)
{
   if (k < 1) LogicError("LazyPrecompFFTMultipliers: bad input");

   do { // NOTE: thread safe lazy init
      FFTMultipliers::Builder bld(tab, k+1);
      long amt = bld.amt();
      if (!amt) break;

      long first = k+1-amt;
      // initialize entries first..k


      for (long s = first; s <= k; s++) {
         UniquePtr<FFTVectorPair> item;

         if (s == 0) {
            bld.move(item); // position 0 not used
            continue;
         }

         if (s == 1) {
            item.make();
            item->wtab_precomp.SetLength(1);
            item->wqinvtab_precomp.SetLength(1);
            item->wtab_precomp[0] = 1;
            item->wqinvtab_precomp[0] = LazyPrepMulModPrecon(1, q, qinv);
            bld.move(item);
            continue;
         }

         item.make();
         item->wtab_precomp.SetLength(1L << (s-1));
         item->wqinvtab_precomp.SetLength(1L << (s-1));

         long m = 1L << s;
         long m_half = 1L << (s-1);
         long m_fourth = 1L << (s-2);

         const mint_t *wtab_last = tab[s-1]->wtab_precomp.elts();
         const mulmod_precon_t *wqinvtab_last = tab[s-1]->wqinvtab_precomp.elts();

         mint_t *wtab = item->wtab_precomp.elts();
         mulmod_precon_t *wqinvtab = item->wqinvtab_precomp.elts();

         for (long i = 0; i < m_fourth; i++) {
            wtab[i] = wtab_last[i];
            wqinvtab[i] = wqinvtab_last[i];
         } 

         mint_t w = root[s];
         mulmod_precon_t wqinv = LazyPrepMulModPrecon(w, q, qinv);

         // prepare wtab...

         if (s == 2) {
            wtab[1] = LazyReduce1(LazyMulModPrecon(wtab[0], w, q, wqinv), q);
            wqinvtab[1] = LazyPrepMulModPrecon(wtab[1], q, qinv);
         }
         else {
            long i, j;

            i = m_half-1; j = m_fourth-1;
            wtab[i-1] = wtab[j];
            wqinvtab[i-1] = wqinvtab[j];
            wtab[i] = LazyReduce1(LazyMulModPrecon(wtab[i-1], w, q, wqinv), q);

            i -= 2; j --;

            for (; i >= 0; i -= 2, j --) {
               mint_t wp2 = wtab[i+2];
               mint_t wm1 = wtab[j];
               wqinvtab[i+2] = LazyPrepMulModPrecon(wp2, q, qinv);
               wtab[i-1] = wm1;
               wqinvtab[i-1] = wqinvtab[j];
               wtab[i] = LazyReduce1(LazyMulModPrecon(wm1, w, q, wqinv), q);
            }

            wqinvtab[1] = LazyPrepMulModPrecon(wtab[1], q, qinv);
         }

         bld.move(item);
      }
   } while (0);
}


//===================================================================

// TRUNCATED FFT

// This code is derived from code originally developed
// by David Harvey.  I include his original documentation,
// annotated appropriately to highlight differences in
// the implemebtation (see NOTEs).

/*
  The DFT is defined as follows.

  Let the input sequence be a_0, ..., a_{N-1}.

  Let w = standard primitive N-th root of 1, i.e. w = g^(2^FFT62_MAX_LGN / N),
  where g = some fixed element of Z/pZ of order 2^FFT62_MAX_LGN.

  Let Z = an element of (Z/pZ)^* (twisting parameter).

  Then the output sequence is
    b_j = \sum_{0 <= i < N} Z^i a_i w^(ij'), for 0 <= j < N,
  where j' is the length-lgN bit-reversal of j.

  Some of the FFT routines can operate on truncated sequences of certain
  "admissible" sizes. A size parameter n is admissible if 1 <= n <= N, and n is
  divisible by a certain power of 2. The precise power depends on the recursive
  array decomposition of the FFT. The smallest admissible n' >= n can be
  obtained via fft62_next_size().
*/

// NOTE: the twising parameter is not implemented.
// NOTE: the next admissible size function is called FFTRoundUp,
//   and is defined in FFT.h.  


/*
  Truncated FFT interface is as follows:

  xn and yn must be admissible sizes for N.

  Input in xp[] is a_0, a_1, ..., a_{xn-1}. Assumes a_i = 0 for xn <= i < N.

  Output in yp[] is b_0, ..., b_{yn-1}, i.e. only first yn outputs are computed.

  Twisting parameter Z is described by z and lgH. If z == 0, then Z = basic
  2^lgH-th root of 1, and must have lgH >= lgN + 1. If z != 0, then Z = z
  (and lgH is ignored).

  The buffers {xp,xn} and {yp,yn} may overlap, but only if xp == yp.

  Inputs are in [0, 2p), outputs are in [0, 2p).

  threads = number of OpenMP threads to use.
*/



/*
  Inverse truncated FFT interface is as follows.

  xn and yn must be admissible sizes for N, with yn <= xn.

  Input in xp[] is b_0, b_1, ..., b_{yn-1}, N*a_{yn}, ..., N*a_{xn-1}.

  Assumes a_i = 0 for xn <= i < N.

  Output in yp[] is N*a_0, ..., N*a_{yn-1}.

  Twisting parameter Z is described by z and lgH. If z == 0, then Z = basic
  2^lgH-th root of 1, and must have lgH >= lgN + 1. If z != 0, then Z = z^(-1)
  (and lgH is ignored).

  The buffers {xp,xn} and {yp,yn} may overlap, but only if xp == yp.

  Inputs are in [0, 4p), outputs are in [0, 4p).

  threads = number of OpenMP threads to use.

  (note: no function actually implements this interface in full generality!
  This is because it is tricky (and not that useful) to implement the twisting
  parameter when xn != yn.)
*/

// NOTE: threads and twisting parameter are not used here. 
// NOTE: the code has been re-written and simplified so that
//   everything is done in place, so xp == yp.




//===================================================================






// NOTE: these could be inlined, but I found the code generation
// to be extremely sensitive to seemingly trivial changes,
// so it seems safest to use macros instead.
// w and wqinv are read only once.
// q is read several times.
// xx0, xx1 are read once and written once

#define fwd_butterfly(xx0, xx1, w, q, wqinv)  \
do \
{ \
   umint_t x0_ = xx0; \
   umint_t x1_ = xx1; \
   umint_t t_  = LazySubMod(x0_, x1_, q); \
   xx0 = LazyAddMod2(x0_, x1_, q); \
   xx1 = LazyMulModPrecon(t_, w, q, wqinv); \
}  \
while (0)

#define fwd_butterfly_neg(xx0, xx1, w, q, wqinv)  \
do \
{ \
   umint_t x0_ = xx0; \
   umint_t x1_ = xx1; \
   umint_t t_  = LazySubMod(x1_, x0_, q); /* NEG */ \
   xx0 = LazyAddMod2(x0_, x1_, q); \
   xx1 = LazyMulModPrecon(t_, w, q, wqinv); \
}  \
while (0)

#define fwd_butterfly1(xx0, xx1, w, q, wqinv, w1, w1qinv)  \
do \
{ \
   umint_t x0_ = xx0; \
   umint_t x1_ = xx1; \
   umint_t t_  = LazySubMod(x0_, x1_, q); \
   xx0 = LazyAddMod2(x0_, x1_, q); \
   xx1 = LazyMulModPrecon(LazyMulModPrecon(t_, w1, q, w1qinv), w, q, wqinv); \
}  \
while (0)


#define fwd_butterfly0(xx0, xx1, q) \
do   \
{  \
   umint_t x0_ = xx0;  \
   umint_t x1_ = xx1;  \
   xx0 = LazyAddMod2(x0_, x1_, q);  \
   xx1 = LazySubMod2(x0_, x1_, q);  \
}  \
while (0)


#define NTL_NEW_FFT_THRESH (11)

struct new_mod_t {
   mint_t q;
   const mint_t **wtab;
   const mulmod_precon_t **wqinvtab;
};





// requires size divisible by 8
static void
new_fft_layer(umint_t* xp, long blocks, long size,
              const mint_t* NTL_RESTRICT wtab, 
              const mulmod_precon_t* NTL_RESTRICT wqinvtab, 
              mint_t q)
{
  size /= 2;

  do
    {
      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + size;

      // first 4 butterflies
      fwd_butterfly0(xp0[0+0], xp1[0+0], q);
      fwd_butterfly(xp0[0+1], xp1[0+1], wtab[0+1], q, wqinvtab[0+1]);
      fwd_butterfly(xp0[0+2], xp1[0+2], wtab[0+2], q, wqinvtab[0+2]);
      fwd_butterfly(xp0[0+3], xp1[0+3], wtab[0+3], q, wqinvtab[0+3]);

      // 4-way unroll
      for (long j = 4; j < size; j += 4) {
        fwd_butterfly(xp0[j+0], xp1[j+0], wtab[j+0], q, wqinvtab[j+0]);
        fwd_butterfly(xp0[j+1], xp1[j+1], wtab[j+1], q, wqinvtab[j+1]);
        fwd_butterfly(xp0[j+2], xp1[j+2], wtab[j+2], q, wqinvtab[j+2]);
        fwd_butterfly(xp0[j+3], xp1[j+3], wtab[j+3], q, wqinvtab[j+3]);
      }

      xp += 2 * size;
    }
  while (--blocks != 0);
}


static void
new_fft_last_two_layers(umint_t* xp, long blocks,
			  const mint_t* wtab, const mulmod_precon_t* wqinvtab, 
                          mint_t q)
{
  // 4th root of unity
  mint_t w = wtab[1];
  mulmod_precon_t wqinv = wqinvtab[1];

  do
    {
      umint_t u0 = xp[0];
      umint_t u1 = xp[1];
      umint_t u2 = xp[2];
      umint_t u3 = xp[3];

      umint_t v0 = LazyAddMod2(u0, u2, q);
      umint_t v2 = LazySubMod2(u0, u2, q);
      umint_t v1 = LazyAddMod2(u1, u3, q);
      umint_t t  = LazySubMod(u1, u3, q);
      umint_t v3 = LazyMulModPrecon(t, w, q, wqinv);

      xp[0] = LazyAddMod2(v0, v1, q);
      xp[1] = LazySubMod2(v0, v1, q);
      xp[2] = LazyAddMod2(v2, v3, q);
      xp[3] = LazySubMod2(v2, v3, q);

      xp += 4;
    }
  while (--blocks != 0);
}



void new_fft_base(umint_t* xp, long lgN, const new_mod_t& mod)
{
  if (lgN == 0) return;

  mint_t q = mod.q;

  if (lgN == 1)
    {
      umint_t x0 = xp[0];
      umint_t x1 = xp[1];
      xp[0] = LazyAddMod2(x0, x1, q);
      xp[1] = LazySubMod2(x0, x1, q);
      return;
    }

  const mint_t** wtab = mod.wtab;
  const mulmod_precon_t** wqinvtab = mod.wqinvtab;

  long N = 1L << lgN;

  for (long j = lgN, size = N, blocks = 1; 
       j > 2; j--, blocks <<= 1, size >>= 1)
    new_fft_layer(xp, blocks, size, wtab[j], wqinvtab[j], q);

  new_fft_last_two_layers(xp, N/4, wtab[2], wqinvtab[2], q);
}


// Implements the truncated FFT interface, described above.
// All computations done in place, and xp should point to 
// an array of size N, all of which may be overwitten
// during the computation.
static
void new_fft_short(umint_t* xp, long yn, long xn, long lgN, 
                   const new_mod_t& mod)
{
  long N = 1L << lgN;

  if (yn == N)
    {
      if (xn == N && lgN <= NTL_NEW_FFT_THRESH)
	{
	  // no truncation
	  new_fft_base(xp, lgN, mod);
	  return;
	}
    }

  // divide-and-conquer algorithm

  long half = N >> 1;
  mint_t q = mod.q;

  if (yn <= half)
    {
      if (xn <= half)
	{
	  new_fft_short(xp, yn, xn, lgN - 1, mod);
	}
      else
	{
	  xn -= half;

	  // (X, Y) -> X + Y
	  for (long j = 0; j < xn; j++)
	    xp[j] = LazyAddMod2(xp[j], xp[j + half], q);

	  new_fft_short(xp, yn, half, lgN - 1, mod);
	}
    }
  else
    {
      yn -= half;
      
      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + half;
      const mint_t* NTL_RESTRICT wtab = mod.wtab[lgN];
      const mulmod_precon_t* NTL_RESTRICT wqinvtab = mod.wqinvtab[lgN];

      if (xn <= half)
	{
	  // X -> (X, w*X)
	  for (long j = 0; j < xn; j++)
	    xp1[j] = LazyMulModPrecon(xp0[j], wtab[j], q, wqinvtab[j]);

	  new_fft_short(xp0, half, xn, lgN - 1, mod);
	  new_fft_short(xp1, yn, xn, lgN - 1, mod);
	}
      else
	{
	  xn -= half;

	  // (X, Y) -> (X + Y, w*(X - Y))
          // DIRT: assumes xn is a multiple of 4
          fwd_butterfly0(xp0[0], xp1[0], q);
          fwd_butterfly(xp0[1], xp1[1], wtab[1], q, wqinvtab[1]);
          fwd_butterfly(xp0[2], xp1[2], wtab[2], q, wqinvtab[2]);
          fwd_butterfly(xp0[3], xp1[3], wtab[3], q, wqinvtab[3]);
	  for (long j = 4; j < xn; j+=4) {
            fwd_butterfly(xp0[j+0], xp1[j+0], wtab[j+0], q, wqinvtab[j+0]);
            fwd_butterfly(xp0[j+1], xp1[j+1], wtab[j+1], q, wqinvtab[j+1]);
            fwd_butterfly(xp0[j+2], xp1[j+2], wtab[j+2], q, wqinvtab[j+2]);
            fwd_butterfly(xp0[j+3], xp1[j+3], wtab[j+3], q, wqinvtab[j+3]);
          }

	  // X -> (X, w*X)
	  for (long j = xn; j < half; j++)
	    xp1[j] = LazyMulModPrecon(xp0[j], wtab[j], q, wqinvtab[j]);

	  new_fft_short(xp0, half, half, lgN - 1, mod);
	  new_fft_short(xp1, yn, half, lgN - 1, mod);
	}
    }
}

static
void new_fft_short_notab(umint_t* xp, long yn, long xn, long lgN, 
                   const new_mod_t& mod, mint_t w, mulmod_precon_t wqinv)
// This version assumes that we only have tables up to level lgN-1,
// and w generates the values at level lgN.
// DIRT: requires xn even
{
  long N = 1L << lgN;

  // divide-and-conquer algorithm

  long half = N >> 1;
  mint_t q = mod.q;

  if (yn <= half)
    {
      if (xn <= half)
	{
	  new_fft_short(xp, yn, xn, lgN - 1, mod);
	}
      else
	{
	  xn -= half;

	  // (X, Y) -> X + Y
	  for (long j = 0; j < xn; j++)
	    xp[j] = LazyAddMod2(xp[j], xp[j + half], q);

	  new_fft_short(xp, yn, half, lgN - 1, mod);
	}
    }
  else
    {
      yn -= half;
      
      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + half;
      const mint_t* NTL_RESTRICT wtab = mod.wtab[lgN-1];
      const mulmod_precon_t* NTL_RESTRICT wqinvtab = mod.wqinvtab[lgN-1];

      if (xn <= half)
	{
	  // X -> (X, w*X)
	  for (long j = 0, j_half = 0; j < xn; j+=2, j_half++) {
	    xp1[j] = LazyMulModPrecon(xp0[j], wtab[j_half], q, wqinvtab[j_half]);
	    xp1[j+1] = LazyMulModPrecon(LazyMulModPrecon(xp0[j+1], w, q, wqinv), 
                                        wtab[j_half], q, wqinvtab[j_half]);
          }

	  new_fft_short(xp0, half, xn, lgN - 1, mod);
	  new_fft_short(xp1, yn, xn, lgN - 1, mod);
	}
      else
	{
	  xn -= half;

	  // (X, Y) -> (X + Y, w*(X - Y))
          fwd_butterfly0(xp0[0], xp1[0], q);
          fwd_butterfly(xp0[1], xp1[1], w, q, wqinv);
          long j = 2;
          long j_half = 1;
	  for (; j < xn; j+=2, j_half++) {
            fwd_butterfly(xp0[j], xp1[j], wtab[j_half], q, wqinvtab[j_half]);
            fwd_butterfly1(xp0[j+1], xp1[j+1], wtab[j_half], q, wqinvtab[j_half], w, wqinv);
          }

	  // X -> (X, w*X)
	  for (; j < half; j+=2, j_half++) {
	    xp1[j] = LazyMulModPrecon(xp0[j], wtab[j_half], q, wqinvtab[j_half]);
	    xp1[j+1] = LazyMulModPrecon(LazyMulModPrecon(xp0[j+1], w, q, wqinv), 
                                        wtab[j_half], q, wqinvtab[j_half]);
          }

	  new_fft_short(xp0, half, half, lgN - 1, mod);
	  new_fft_short(xp1, yn, half, lgN - 1, mod);
	}
    }
}


//=====


// NOTE: these "flipped" routines perform the same
// functions as their normal, "unflipped" counter-parts,
// except that they work with inverted roots.
// They also perform no truncation, just to keep things simple.
// All of this is necessary only to implement the UpdateMap
// routines for ZZ_pX and zz_pX.

// requires size divisible by 8
static void
new_fft_layer_flipped(umint_t* xp, long blocks, long size,
              const mint_t* wtab, 
              const mulmod_precon_t* wqinvtab, 
              mint_t q)
{
  size /= 2;

  const mint_t* NTL_RESTRICT wtab1 = wtab + size;
  const mulmod_precon_t* NTL_RESTRICT wqinvtab1 = wqinvtab + size;

  do
    {
      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + size;

      // first 4 butterflies
      fwd_butterfly0(xp0[0+0], xp1[0+0], q);
      fwd_butterfly_neg(xp0[0+1], xp1[0+1], wtab1[-(0+1)], q, wqinvtab1[-(0+1)]);
      fwd_butterfly_neg(xp0[0+2], xp1[0+2], wtab1[-(0+2)], q, wqinvtab1[-(0+2)]);
      fwd_butterfly_neg(xp0[0+3], xp1[0+3], wtab1[-(0+3)], q, wqinvtab1[-(0+3)]);

      // 4-way unroll
      for (long j = 4; j < size; j += 4) {
        fwd_butterfly_neg(xp0[j+0], xp1[j+0], wtab1[-(j+0)], q, wqinvtab1[-(j+0)]);
        fwd_butterfly_neg(xp0[j+1], xp1[j+1], wtab1[-(j+1)], q, wqinvtab1[-(j+1)]);
        fwd_butterfly_neg(xp0[j+2], xp1[j+2], wtab1[-(j+2)], q, wqinvtab1[-(j+2)]);
        fwd_butterfly_neg(xp0[j+3], xp1[j+3], wtab1[-(j+3)], q, wqinvtab1[-(j+3)]);
      }

      xp += 2 * size;
    }
  while (--blocks != 0);
}



static void
new_fft_last_two_layers_flipped(umint_t* xp, long blocks,
			  const mint_t* wtab, const mulmod_precon_t* wqinvtab, 
                          mint_t q)
{
  // 4th root of unity
  mint_t w = wtab[1];
  mulmod_precon_t wqinv = wqinvtab[1];

  do
    {
      umint_t u0 = xp[0];
      umint_t u1 = xp[1];
      umint_t u2 = xp[2];
      umint_t u3 = xp[3];

      umint_t v0 = LazyAddMod2(u0, u2, q);
      umint_t v2 = LazySubMod2(u0, u2, q);
      umint_t v1 = LazyAddMod2(u1, u3, q);
      umint_t t  = LazySubMod(u3, u1, q); // NEG
      umint_t v3 = LazyMulModPrecon(t, w, q, wqinv);

      xp[0] = LazyAddMod2(v0, v1, q);
      xp[1] = LazySubMod2(v0, v1, q);
      xp[2] = LazyAddMod2(v2, v3, q); 
      xp[3] = LazySubMod2(v2, v3, q); 

      xp += 4;
    }
  while (--blocks != 0);
}



void new_fft_base_flipped(umint_t* xp, long lgN, const new_mod_t& mod)
{
  if (lgN == 0) return;

  mint_t q = mod.q;

  if (lgN == 1)
    {
      umint_t x0 = xp[0];
      umint_t x1 = xp[1];
      xp[0] = LazyAddMod2(x0, x1, q);
      xp[1] = LazySubMod2(x0, x1, q);
      return;
    }

  const mint_t** wtab = mod.wtab;
  const mulmod_precon_t** wqinvtab = mod.wqinvtab;

  long N = 1L << lgN;

  for (long j = lgN, size = N, blocks = 1; 
       j > 2; j--, blocks <<= 1, size >>= 1)
    new_fft_layer_flipped(xp, blocks, size, wtab[j], wqinvtab[j], q);

  new_fft_last_two_layers_flipped(xp, N/4, wtab[2], wqinvtab[2], q);
}


static
void new_fft_short_flipped(umint_t* xp, long lgN, const new_mod_t& mod)
{
  long N = 1L << lgN;

  if (lgN <= NTL_NEW_FFT_THRESH)
    {
      new_fft_base_flipped(xp, lgN, mod);
      return;
    }

  // divide-and-conquer algorithm

  long half = N >> 1;
  mint_t q = mod.q;

  umint_t* NTL_RESTRICT xp0 = xp;
  umint_t* NTL_RESTRICT xp1 = xp + half;
  const mint_t* NTL_RESTRICT wtab = mod.wtab[lgN] + half;
  const mulmod_precon_t* NTL_RESTRICT wqinvtab = mod.wqinvtab[lgN] + half;

  // (X, Y) -> (X + Y, w*(X - Y))

  fwd_butterfly0(xp0[0], xp1[0], q);
  fwd_butterfly_neg(xp0[1], xp1[1], wtab[-1], q, wqinvtab[-1]);
  fwd_butterfly_neg(xp0[2], xp1[2], wtab[-2], q, wqinvtab[-2]);
  fwd_butterfly_neg(xp0[3], xp1[3], wtab[-3], q, wqinvtab[-3]);
  for (long j = 4; j < half; j+=4) {
    fwd_butterfly_neg(xp0[j+0], xp1[j+0], wtab[-(j+0)], q, wqinvtab[-(j+0)]);
    fwd_butterfly_neg(xp0[j+1], xp1[j+1], wtab[-(j+1)], q, wqinvtab[-(j+1)]);
    fwd_butterfly_neg(xp0[j+2], xp1[j+2], wtab[-(j+2)], q, wqinvtab[-(j+2)]);
    fwd_butterfly_neg(xp0[j+3], xp1[j+3], wtab[-(j+3)], q, wqinvtab[-(j+3)]);
  }

  new_fft_short_flipped(xp0, lgN - 1, mod);
  new_fft_short_flipped(xp1, lgN - 1, mod);
}



// IFFT (inverse truncated FFT)


#define inv_butterfly0(xx0, xx1, q)  \
do   \
{  \
   umint_t x0_ = LazyReduce2(xx0, q);  \
   umint_t x1_ = LazyReduce2(xx1, q);  \
   xx0 = LazyAddMod(x0_, x1_, q);  \
   xx1 = LazySubMod(x0_, x1_, q);  \
} while (0)  


#define inv_butterfly_neg(xx0, xx1, w, q, wqinv)  \
do  \
{  \
   umint_t x0_ = LazyReduce2(xx0, q);  \
   umint_t x1_ = xx1;  \
   umint_t t_ = LazyMulModPrecon(x1_, w, q, wqinv);   \
   xx0 = LazySubMod(x0_, t_, q);  /* NEG */   \
   xx1 = LazyAddMod(x0_, t_, q);  /* NEG */   \
} while (0)
   
#define inv_butterfly(xx0, xx1, w, q, wqinv)  \
do  \
{  \
   umint_t x0_ = LazyReduce2(xx0, q);  \
   umint_t x1_ = xx1;  \
   umint_t t_ = LazyMulModPrecon(x1_, w, q, wqinv);   \
   xx0 = LazyAddMod(x0_, t_, q);    \
   xx1 = LazySubMod(x0_, t_, q);    \
} while (0)
   
#define inv_butterfly1_neg(xx0, xx1, w, q, wqinv, w1, w1qinv)  \
do  \
{  \
   umint_t x0_ = LazyReduce2(xx0, q);  \
   umint_t x1_ = xx1;  \
   umint_t t_ = LazyMulModPrecon(LazyMulModPrecon(x1_, w1, q, w1qinv), w, q, wqinv);   \
   xx0 = LazySubMod(x0_, t_, q);  /* NEG */   \
   xx1 = LazyAddMod(x0_, t_, q);  /* NEG */   \
} while (0)


static
void new_ifft_short2(umint_t* yp, long yn, long lgN, const new_mod_t& mod);



// requires size divisible by 8
static void
new_ifft_layer(umint_t* xp, long blocks, long size,
		 const mint_t* wtab, 
                 const mulmod_precon_t* wqinvtab, mint_t q)
{

  size /= 2;
  const mint_t* NTL_RESTRICT wtab1 = wtab + size;
  const mulmod_precon_t* NTL_RESTRICT wqinvtab1 = wqinvtab + size;

  do
    {

      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + size;


      // first 4 butterflies
      inv_butterfly0(xp0[0], xp1[0], q);
      inv_butterfly_neg(xp0[1], xp1[1], wtab1[-1], q, wqinvtab1[-1]); 
      inv_butterfly_neg(xp0[2], xp1[2], wtab1[-2], q, wqinvtab1[-2]); 
      inv_butterfly_neg(xp0[3], xp1[3], wtab1[-3], q, wqinvtab1[-3]); 

      // 4-way unroll
      for (long j = 4; j < size; j+= 4) {
	 inv_butterfly_neg(xp0[j+0], xp1[j+0], wtab1[-(j+0)], q, wqinvtab1[-(j+0)]); 
	 inv_butterfly_neg(xp0[j+1], xp1[j+1], wtab1[-(j+1)], q, wqinvtab1[-(j+1)]); 
	 inv_butterfly_neg(xp0[j+2], xp1[j+2], wtab1[-(j+2)], q, wqinvtab1[-(j+2)]); 
	 inv_butterfly_neg(xp0[j+3], xp1[j+3], wtab1[-(j+3)], q, wqinvtab1[-(j+3)]); 
      }

      xp += 2 * size;
    }
  while (--blocks != 0);
}


static void
new_ifft_first_two_layers(umint_t* xp, long blocks, const mint_t* wtab, 
                          const mulmod_precon_t* wqinvtab, mint_t q)
{
  // 4th root of unity
  mint_t w = wtab[1];
  mulmod_precon_t wqinv = wqinvtab[1];

  do
    {
      umint_t u0 = LazyReduce2(xp[0], q);
      umint_t u1 = LazyReduce2(xp[1], q);
      umint_t u2 = LazyReduce2(xp[2], q);
      umint_t u3 = LazyReduce2(xp[3], q);

      umint_t v0 = LazyAddMod2(u0, u1, q);
      umint_t v1 = LazySubMod2(u0, u1, q);
      umint_t v2 = LazyAddMod2(u2, u3, q);
      umint_t t  = LazySubMod(u2, u3, q);
      umint_t v3 = LazyMulModPrecon(t, w, q, wqinv);

      xp[0] = LazyAddMod(v0, v2, q);
      xp[2] = LazySubMod(v0, v2, q);
      xp[1] = LazySubMod(v1, v3, q);  // NEG
      xp[3] = LazyAddMod(v1, v3, q);  // NEG

      xp += 4;
    }
  while (--blocks != 0);
}



static
void new_ifft_base(umint_t* xp, long lgN, const new_mod_t& mod)
{
  if (lgN == 0) return;

  mint_t q = mod.q;

  if (lgN == 1)
    {
      umint_t x0 = LazyReduce2(xp[0], q);
      umint_t x1 = LazyReduce2(xp[1], q);
      xp[0] = LazyAddMod(x0, x1, q);
      xp[1] = LazySubMod(x0, x1, q);
      return;
    }

  const mint_t** wtab = mod.wtab;
  const mulmod_precon_t** wqinvtab = mod.wqinvtab;

  long blocks = 1L << (lgN - 2);
  new_ifft_first_two_layers(xp, blocks, wtab[2], wqinvtab[2], q);
  blocks >>= 1;

  long size = 8;
  for (long j = 3; j <= lgN; j++, blocks >>= 1, size <<= 1)
    new_ifft_layer(xp, blocks, size, wtab[j], wqinvtab[j], q);
}


static
void new_ifft_short1(umint_t* xp, long yn, long lgN, const new_mod_t& mod)

// Implements truncated inverse FFT interface, but with xn==yn.
// All computations are done in place.

{
  long N = 1L << lgN;

  if (yn == N && lgN <= NTL_NEW_FFT_THRESH)
    {
      // no truncation
      new_ifft_base(xp, lgN, mod);
      return;
    }

  // divide-and-conquer algorithm

  long half = N >> 1;
  mint_t q = mod.q;

  if (yn <= half)
    {
      // X -> 2X
      for (long j = 0; j < yn; j++)
      	xp[j] = LazyDoubleMod4(xp[j], q);

      new_ifft_short1(xp, yn, lgN - 1, mod);
    }
  else
    {
      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + half;
      const mint_t* NTL_RESTRICT wtab = mod.wtab[lgN];
      const mulmod_precon_t* NTL_RESTRICT wqinvtab = mod.wqinvtab[lgN];

      new_ifft_short1(xp0, half, lgN - 1, mod);

      yn -= half;

      // X -> (2X, w*X)
      for (long j = yn; j < half; j++)
	{
	  umint_t x0 = xp0[j];
	  xp0[j] = LazyDoubleMod4(x0, q);
	  xp1[j] = LazyMulModPrecon(x0, wtab[j], q, wqinvtab[j]);
	}

      new_ifft_short2(xp1, yn, lgN - 1, mod);

      // (X, Y) -> (X + Y/w, X - Y/w)
      {
	const mint_t* NTL_RESTRICT wtab1 = wtab + half;
	const mulmod_precon_t* NTL_RESTRICT wqinvtab1 =  wqinvtab + half;

	// DIRT: assumes yn is a multiple of 4
	inv_butterfly0(xp0[0], xp1[0], q);
	inv_butterfly_neg(xp0[1], xp1[1], wtab1[-1], q, wqinvtab1[-1]);
	inv_butterfly_neg(xp0[2], xp1[2], wtab1[-2], q, wqinvtab1[-2]);
	inv_butterfly_neg(xp0[3], xp1[3], wtab1[-3], q, wqinvtab1[-3]);
	for (long j = 4; j < yn; j+=4) {
	  inv_butterfly_neg(xp0[j+0], xp1[j+0], wtab1[-(j+0)], q, wqinvtab1[-(j+0)]);
	  inv_butterfly_neg(xp0[j+1], xp1[j+1], wtab1[-(j+1)], q, wqinvtab1[-(j+1)]);
	  inv_butterfly_neg(xp0[j+2], xp1[j+2], wtab1[-(j+2)], q, wqinvtab1[-(j+2)]);
	  inv_butterfly_neg(xp0[j+3], xp1[j+3], wtab1[-(j+3)], q, wqinvtab1[-(j+3)]);
	}
      }
    }
}


static
void new_ifft_short1_notab(umint_t* xp, long yn, long lgN, const new_mod_t& mod,
                           mint_t w, mulmod_precon_t wqinv,
                           mint_t iw, mulmod_precon_t iwqinv)
// This version assumes that we only have tables up to level lgN-1,
// and w generates the values at level lgN.
// DIRT: requires yn even
{
  long N = 1L << lgN;

  // divide-and-conquer algorithm

  long half = N >> 1;
  mint_t q = mod.q;

  if (yn <= half)
    {
      // X -> 2X
      for (long j = 0; j < yn; j++)
      	xp[j] = LazyDoubleMod4(xp[j], q);

      new_ifft_short1(xp, yn, lgN - 1, mod);
    }
  else
    {
      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + half;
      const mint_t* NTL_RESTRICT wtab = mod.wtab[lgN-1];
      const mulmod_precon_t* NTL_RESTRICT wqinvtab = mod.wqinvtab[lgN-1];

      new_ifft_short1(xp0, half, lgN - 1, mod);

      yn -= half;

      // X -> (2X, w*X)
      for (long j = yn, j_half = yn/2; j < half; j+=2, j_half++) {
	{
	  umint_t x0 = xp0[j+0];
	  xp0[j+0] = LazyDoubleMod4(x0, q);
	  xp1[j+0] = LazyMulModPrecon(x0, wtab[j_half], q, wqinvtab[j_half]);
	}
	{
	  umint_t x0 = xp0[j+1];
	  xp0[j+1] = LazyDoubleMod4(x0, q);
	  xp1[j+1] = LazyMulModPrecon(LazyMulModPrecon(x0, w, q, wqinv), 
                                      wtab[j_half], q, wqinvtab[j_half]);
	}
      }

      new_ifft_short2(xp1, yn, lgN - 1, mod);

      // (X, Y) -> (X + Y/w, X - Y/w)
      {
	const mint_t* NTL_RESTRICT wtab1 = wtab + half/2;
	const mulmod_precon_t* NTL_RESTRICT wqinvtab1 =  wqinvtab + half/2;

	inv_butterfly0(xp0[0], xp1[0], q);
	inv_butterfly(xp0[1], xp1[1], iw, q, iwqinv);
	for (long j = 2, j_half = 1; j < yn; j+=2, j_half++) {
	  inv_butterfly_neg(xp0[j+0], xp1[j+0], wtab1[-j_half], q, wqinvtab1[-j_half]);
	  inv_butterfly1_neg(xp0[j+1], xp1[j+1], wtab1[-j_half], q, wqinvtab1[-j_half], iw, iwqinv);
	}
      }
    }
}



//=========


// requires size divisible by 8
static void
new_ifft_layer_flipped(umint_t* xp, long blocks, long size,
		 const mint_t* NTL_RESTRICT wtab, 
                 const mulmod_precon_t* NTL_RESTRICT wqinvtab, mint_t q)
{

  size /= 2;

  do
    {

      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + size;


      // first 4 butterflies
      inv_butterfly0(xp0[0], xp1[0], q);
      inv_butterfly(xp0[1], xp1[1], wtab[1], q, wqinvtab[1]); 
      inv_butterfly(xp0[2], xp1[2], wtab[2], q, wqinvtab[2]); 
      inv_butterfly(xp0[3], xp1[3], wtab[3], q, wqinvtab[3]); 

      // 4-way unroll
      for (long j = 4; j < size; j+= 4) {
	 inv_butterfly(xp0[j+0], xp1[j+0], wtab[j+0], q, wqinvtab[j+0]); 
	 inv_butterfly(xp0[j+1], xp1[j+1], wtab[j+1], q, wqinvtab[j+1]); 
	 inv_butterfly(xp0[j+2], xp1[j+2], wtab[j+2], q, wqinvtab[j+2]); 
	 inv_butterfly(xp0[j+3], xp1[j+3], wtab[j+3], q, wqinvtab[j+3]); 
      }

      xp += 2 * size;
    }
  while (--blocks != 0);
}


static void
new_ifft_first_two_layers_flipped(umint_t* xp, long blocks, const mint_t* wtab, 
                          const mulmod_precon_t* wqinvtab, mint_t q)
{
  // 4th root of unity
  mint_t w = wtab[1];
  mulmod_precon_t wqinv = wqinvtab[1];

  do
    {
      umint_t u0 = LazyReduce2(xp[0], q);
      umint_t u1 = LazyReduce2(xp[1], q);
      umint_t u2 = LazyReduce2(xp[2], q);
      umint_t u3 = LazyReduce2(xp[3], q);

      umint_t v0 = LazyAddMod2(u0, u1, q);
      umint_t v1 = LazySubMod2(u0, u1, q);
      umint_t v2 = LazyAddMod2(u2, u3, q);
      umint_t t  = LazySubMod(u2, u3, q);
      umint_t v3 = LazyMulModPrecon(t, w, q, wqinv);

      xp[0] = LazyAddMod(v0, v2, q);
      xp[2] = LazySubMod(v0, v2, q);
      xp[1] = LazyAddMod(v1, v3, q);  
      xp[3] = LazySubMod(v1, v3, q); 

      xp += 4;
    }
  while (--blocks != 0);
}



static
void new_ifft_base_flipped(umint_t* xp, long lgN, const new_mod_t& mod)
{
  if (lgN == 0) return;

  mint_t q = mod.q;

  if (lgN == 1)
    {
      umint_t x0 = LazyReduce2(xp[0], q);
      umint_t x1 = LazyReduce2(xp[1], q);
      xp[0] = LazyAddMod(x0, x1, q);
      xp[1] = LazySubMod(x0, x1, q);
      return;
    }

  const mint_t** wtab = mod.wtab;
  const mulmod_precon_t** wqinvtab = mod.wqinvtab;

  long blocks = 1L << (lgN - 2);
  new_ifft_first_two_layers_flipped(xp, blocks, wtab[2], wqinvtab[2], q);
  blocks >>= 1;

  long size = 8;
  for (long j = 3; j <= lgN; j++, blocks >>= 1, size <<= 1)
    new_ifft_layer_flipped(xp, blocks, size, wtab[j], wqinvtab[j], q);
}


static
void new_ifft_short1_flipped(umint_t* xp, long lgN, const new_mod_t& mod)
{
  long N = 1L << lgN;

  if (lgN <= NTL_NEW_FFT_THRESH)
    {
      new_ifft_base_flipped(xp, lgN, mod);
      return;
    }

  // divide-and-conquer algorithm

  long half = N >> 1;
  mint_t q = mod.q;

  umint_t* NTL_RESTRICT xp0 = xp;
  umint_t* NTL_RESTRICT xp1 = xp + half;
  const mint_t* NTL_RESTRICT wtab = mod.wtab[lgN];
  const mulmod_precon_t* NTL_RESTRICT wqinvtab = mod.wqinvtab[lgN];

  new_ifft_short1_flipped(xp0, lgN - 1, mod);
  new_ifft_short1_flipped(xp1, lgN - 1, mod);

  // (X, Y) -> (X + Y*w, X - Y*w)

  inv_butterfly0(xp0[0], xp1[0], q);
  inv_butterfly(xp0[1], xp1[1], wtab[1], q, wqinvtab[1]);
  inv_butterfly(xp0[2], xp1[2], wtab[2], q, wqinvtab[2]);
  inv_butterfly(xp0[3], xp1[3], wtab[3], q, wqinvtab[3]);
  for (long j = 4; j < half; j+=4) {
    inv_butterfly(xp0[j+0], xp1[j+0], wtab[j+0], q, wqinvtab[j+0]);
    inv_butterfly(xp0[j+1], xp1[j+1], wtab[j+1], q, wqinvtab[j+1]);
    inv_butterfly(xp0[j+2], xp1[j+2], wtab[j+2], q, wqinvtab[j+2]);
    inv_butterfly(xp0[j+3], xp1[j+3], wtab[j+3], q, wqinvtab[j+3]);
  }
}

//=========



static
void new_ifft_short2(umint_t* xp, long yn, long lgN, const new_mod_t& mod)

// Implements truncated inverse FFT interface, but with xn==N.
// All computations are done in place.

{
  long N = 1L << lgN;

  if (yn == N && lgN <= NTL_NEW_FFT_THRESH)
    {
      // no truncation
      new_ifft_base(xp, lgN, mod);
      return;
    }

  // divide-and-conquer algorithm

  long half = N >> 1;
  mint_t q = mod.q;

  if (yn <= half)
    {
      // X -> 2X
      for (long j = 0; j < yn; j++)
     	xp[j] = LazyDoubleMod4(xp[j], q);
      // (X, Y) -> X + Y
      for (long j = yn; j < half; j++)
	xp[j] = LazyAddMod4(xp[j], xp[j + half], q);

      new_ifft_short2(xp, yn, lgN - 1, mod);

      // (X, Y) -> X - Y
      for (long j = 0; j < yn; j++)
	xp[j] = LazySubMod4(xp[j], xp[j + half], q);
    }
  else
    {
      umint_t* NTL_RESTRICT xp0 = xp;
      umint_t* NTL_RESTRICT xp1 = xp + half;
      const mint_t* NTL_RESTRICT wtab = mod.wtab[lgN];
      const mulmod_precon_t* NTL_RESTRICT wqinvtab = mod.wqinvtab[lgN];

      new_ifft_short1(xp0, half, lgN - 1, mod);

      yn -= half;


      // (X, Y) -> (2X - Y, w*(X - Y))
      for (long j = yn; j < half; j++)
	{
	  umint_t x0 = xp0[j];
	  umint_t x1 = xp1[j];
	  umint_t u = LazySubMod4(x0, x1, q);
	  xp0[j] = LazyAddMod4(x0, u, q);
	  xp1[j] = LazyMulModPrecon(u, wtab[j], q, wqinvtab[j]);
	}

      new_ifft_short2(xp1, yn, lgN - 1, mod);

      // (X, Y) -> (X + Y/w, X - Y/w)
      {
	const mint_t* NTL_RESTRICT wtab1 = wtab + half;
	const mulmod_precon_t* NTL_RESTRICT wqinvtab1 =  wqinvtab + half;

	// DIRT: assumes yn is a multiple of 4
	inv_butterfly0(xp0[0], xp1[0], q);
	inv_butterfly_neg(xp0[1], xp1[1], wtab1[-1], q, wqinvtab1[-1]);
	inv_butterfly_neg(xp0[2], xp1[2], wtab1[-2], q, wqinvtab1[-2]);
	inv_butterfly_neg(xp0[3], xp1[3], wtab1[-3], q, wqinvtab1[-3]);
	for (long j = 4; j < yn; j+=4) {
	  inv_butterfly_neg(xp0[j+0], xp1[j+0], wtab1[-(j+0)], q, wqinvtab1[-(j+0)]);
	  inv_butterfly_neg(xp0[j+1], xp1[j+1], wtab1[-(j+1)], q, wqinvtab1[-(j+1)]);
	  inv_butterfly_neg(xp0[j+2], xp1[j+2], wtab1[-(j+2)], q, wqinvtab1[-(j+2)]);
	  inv_butterfly_neg(xp0[j+3], xp1[j+3], wtab1[-(j+3)], q, wqinvtab1[-(j+3)]);
	}
      }
    }
}


//=============================================

// HIGH LEVEL ROUTINES

//=========== FFT without tables ===========


NTL_TLS_GLOBAL_DECL(Vec<umint_t>, AA_store)

NTL_TLS_GLOBAL_DECL(Vec<FFTVectorPair>, mul_vec)

void new_fft_notab(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info,
             long yn, long xn)

// Performs a high-level FFT.  Inputs and outputs are in the range [0,q). 
// xn and yn are as described above in the truncated FFT interface.
// Both A and a should point to arrays of size 2^k,
// and should either be the same or not overlap at all.
// This version does not use precomputed tables.

{
   mint_t q = info.q;

   if (k <= 1) {
      if (k == 0) {
	 A[0] = a[0];
	 return;
      }
      if (k == 1) {
         mint_t A0 = AddMod(a[0], a[1], q);
         mint_t A1 = SubMod(a[0], a[1], q);
         A[0] = A0;
         A[1] = A1;
	 return;
      }
   }

   // assume k > 1
   const mint_t *root = info.RootTable[0].elts();
   mulmod_t qinv = info.qinv;

   NTL_TLS_GLOBAL_ACCESS(mul_vec);
   ComputeMultipliers(mul_vec, k-1, q, qinv, root);

   long n = 1L << k;

   const mint_t *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k-1; s++) wtab[s] = mul_vec[s].wtab_precomp.elts();

   const mulmod_precon_t *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k-1; s++) wqinvtab[s] = mul_vec[s].wqinvtab_precomp.elts();

   new_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];

   mint_t w = info.RootTable[0][k];
   mulmod_precon_t wqinv = LazyPrepMulModPrecon(w, q, info.qinv);

#ifdef NTL_FFT_USEBUF
   NTL_TLS_GLOBAL_ACCESS(AA_store);
   AA_store.SetLength(1L << k);
   umint_t *AA = AA_store.elts();

   for (long i = 0; i < xn; i++) AA[i] = a[i];

   new_fft_short_notab(AA, yn, xn, k, mod, w, wqinv);

   for (long i = 0; i < yn; i++) {
      A[i] = LazyReduce1(AA[i], q);
   }
#else
   umint_t *AA = (umint_t *) A;
   if (a != A) for (long i = 0; i < xn; i++) AA[i] = a[i];

   new_fft_short_notab(AA, yn, xn, k, mod, w, wqinv);

   for (long i = 0; i < yn; i++) {
      AA[i] = LazyReduce1(AA[i], q);
   }
#endif
}


void new_fft_flipped_notab(mint_t* A, const mint_t* a, long k, 
             const FFTPrimeInfo& info)

// Performs a high-level FFT.  Inputs and outputs are in the range [0,q). 
// Both A and a should point to arrays of size 2^k,
// and should either be the same or not overlap at all.
// This version is "flipped" -- it uses inverted roots, 
// multiplies by 2^{-k}, and performs no truncations.
// This version does not use precomputed tables.

{
   mint_t q = info.q;

   if (k <= 1) {
      if (k == 0) {
	 A[0] = a[0];
	 return;
      }
      if (k == 1) {
         mint_t two_inv = info.TwoInvTable[1];
         mulmod_precon_t two_inv_aux = info.TwoInvPreconTable[1];
         mint_t A0 = AddMod(a[0], a[1], q);
         mint_t A1 = SubMod(a[0], a[1], q);
         A[0] = LazyReduce1(LazyMulModPrecon(A0, two_inv, q, two_inv_aux), q);
         A[1] = LazyReduce1(LazyMulModPrecon(A1, two_inv, q, two_inv_aux), q);
	 return;
      }
   }

   // assume k > 1
   const mint_t *root = info.RootTable[1].elts();
   mulmod_t qinv = info.qinv;

   NTL_TLS_GLOBAL_ACCESS(mul_vec);
   ComputeMultipliers(mul_vec, k-1, q, qinv, root);

   long n = 1L << k;

   const mint_t *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k-1; s++) wtab[s] = mul_vec[s].wtab_precomp.elts();

   const mulmod_precon_t *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k-1; s++) wqinvtab[s] = mul_vec[s].wqinvtab_precomp.elts();

   new_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];

   mint_t w = info.RootTable[1][k];
   mulmod_precon_t wqinv = LazyPrepMulModPrecon(w, q, info.qinv);

   mint_t two_inv = info.TwoInvTable[k];
   mulmod_precon_t two_inv_aux = info.TwoInvPreconTable[k];

#ifdef NTL_FFT_USEBUF
   NTL_TLS_GLOBAL_ACCESS(AA_store);
   AA_store.SetLength(1L << k);
   umint_t *AA = AA_store.elts();

   for (long i = 0; i < n; i++) AA[i] = a[i];

   new_fft_short_notab(AA, n, n, k, mod, w, wqinv);

   for (long i = 0; i < n; i++) {
      umint_t tmp = LazyMulModPrecon(AA[i], two_inv, q, two_inv_aux); 
      A[i] = LazyReduce1(tmp, q);
   }
#else
   umint_t *AA = (umint_t *) A;
   if (a != A) for (long i = 0; i < n; i++) AA[i] = a[i];

   new_fft_short_notab(AA, n, n, k, mod, w, wqinv);

   for (long i = 0; i < n; i++) {
      umint_t tmp = LazyMulModPrecon(AA[i], two_inv, q, two_inv_aux); 
      AA[i] = LazyReduce1(tmp, q);
   }

#endif
}


//=========== Inverse FFT without tables  ===========

void new_ifft_notab(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info,
              long yn)

// Performs a high-level IFFT.  Inputs and outputs are in the range [0,q). 
// yn==xn are as described above in the truncated FFT interface.
// Both A and a should point to arrays of size 2^k,
// and should either be the same or not overlap at all.
// Multiplies by 2^{-k}.
// This version does not use precomputed tables.

{
   mint_t q = info.q;

   if (k <= 1) {
      if (k == 0) {
	 A[0] = a[0];
	 return;
      }
      if (k == 1) {
         mint_t two_inv = info.TwoInvTable[1];
         mulmod_precon_t two_inv_aux = info.TwoInvPreconTable[1];
         mint_t A0 = AddMod(a[0], a[1], q);
         mint_t A1 = SubMod(a[0], a[1], q);
         A[0] = LazyReduce1(LazyMulModPrecon(A0, two_inv, q, two_inv_aux), q);
         A[1] = LazyReduce1(LazyMulModPrecon(A1, two_inv, q, two_inv_aux), q);
	 return;
      }
   }

   // assume k > 1
   const mint_t *root = info.RootTable[0].elts();
   mulmod_t qinv = info.qinv;

   NTL_TLS_GLOBAL_ACCESS(mul_vec);
   ComputeMultipliers(mul_vec, k-1, q, qinv, root);

   long n = 1L << k;

   const mint_t *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k-1; s++) wtab[s] = mul_vec[s].wtab_precomp.elts();

   const mulmod_precon_t *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k-1; s++) wqinvtab[s] = mul_vec[s].wqinvtab_precomp.elts();

   new_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];


   mint_t w = info.RootTable[0][k];
   mulmod_precon_t wqinv = LazyPrepMulModPrecon(w, q, info.qinv);

   mint_t iw = info.RootTable[1][k];
   mulmod_precon_t iwqinv = LazyPrepMulModPrecon(iw, q, info.qinv);

   mint_t two_inv = info.TwoInvTable[k];
   mulmod_precon_t two_inv_aux = info.TwoInvPreconTable[k];

#ifdef NTL_FFT_USEBUF
   NTL_TLS_GLOBAL_ACCESS(AA_store);
   AA_store.SetLength(1L << k);
   umint_t *AA = AA_store.elts();

   for (long i = 0; i < yn; i++) AA[i] = a[i];

   new_ifft_short1_notab(AA, yn, k, mod, w, wqinv, iw, iwqinv);

   for (long i = 0; i < yn; i++) {
      umint_t tmp = LazyMulModPrecon(AA[i], two_inv, q, two_inv_aux); 
      A[i] = LazyReduce1(tmp, q);
   }
#else
   umint_t *AA = (umint_t *) A;
   if (a != A) for (long i = 0; i < yn; i++) AA[i] = a[i];

   new_ifft_short1_notab(AA, yn, k, mod, w, wqinv, iw, iwqinv);

   for (long i = 0; i < yn; i++) {
      umint_t tmp = LazyMulModPrecon(AA[i], two_inv, q, two_inv_aux); 
      AA[i] = LazyReduce1(tmp, q);
   }

#endif
}


void new_ifft_flipped_notab(mint_t* A, const mint_t* a, long k, 
              const FFTPrimeInfo& info)

// Performs a high-level IFFT.  Inputs and outputs are in the range [0,q). 
// Flipped means inverse roots are used an no truncation and
// no multiplication by 2^{-k}.
// Both A and a should point to arrays of size 2^k,
// and should either be the same or not overlap at all.
// This version does not use precomputed tables.

{
   mint_t q = info.q;

   if (k <= 1) {
      if (k == 0) {
	 A[0] = a[0];
	 return;
      }
      if (k == 1) {
         mint_t A0 = AddMod(a[0], a[1], q);
         mint_t A1 = SubMod(a[0], a[1], q);
         A[0] = A0;
         A[1] = A1;
	 return;
      }
   }

   // assume k > 1
   const mint_t *root = info.RootTable[1].elts();
   mulmod_t qinv = info.qinv;

   NTL_TLS_GLOBAL_ACCESS(mul_vec);
   ComputeMultipliers(mul_vec, k-1, q, qinv, root);

   long n = 1L << k;

   const mint_t *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k-1; s++) wtab[s] = mul_vec[s].wtab_precomp.elts();

   const mulmod_precon_t *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k-1; s++) wqinvtab[s] = mul_vec[s].wqinvtab_precomp.elts();

   new_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];

   mint_t w = info.RootTable[1][k];
   mulmod_precon_t wqinv = LazyPrepMulModPrecon(w, q, info.qinv);

   mint_t iw = info.RootTable[0][k];
   mulmod_precon_t iwqinv = LazyPrepMulModPrecon(iw, q, info.qinv);

#ifdef NTL_FFT_USEBUF
   NTL_TLS_GLOBAL_ACCESS(AA_store);
   AA_store.SetLength(1L << k);
   umint_t *AA = AA_store.elts();

   for (long i = 0; i < n; i++) AA[i] = a[i];


   new_ifft_short1_notab(AA, n, k, mod, w, wqinv, iw, iwqinv);

   for (long i = 0; i < n; i++) {
      umint_t tmp = LazyReduce2(AA[i], q);
      A[i] = LazyReduce1(tmp, q);
   }
#else
   umint_t *AA = (umint_t *) A;
   if (a != A) for (long i = 0; i < n; i++) AA[i] = a[i];

   new_ifft_short1_notab(AA, n, k, mod, w, wqinv, iw, iwqinv);

   for (long i = 0; i < n; i++) {
      umint_t tmp = LazyReduce2(AA[i], q);
      AA[i] = LazyReduce1(tmp, q);
   }
#endif
}


#ifndef NTL_ENABLE_AVX_FFT

//================ FFT with tables ==============


void new_fft(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info, 
             long yn, long xn)

// Performs a high-level FFT.  Inputs and outputs are in the range [0,q). 
// xn and yn are as described above in the truncated FFT interface.
// Both A and a should point to arrays of size 2^k,
// and should either be the same or not overlap at all.

{
   if (!info.bigtab || k > info.bigtab->bound) {
      new_fft_notab(A, a, k, info, yn, xn);
      return;
   }

   mint_t q = info.q;

   if (k <= 1) {
      if (k == 0) {
	 A[0] = a[0];
	 return;
      }
      if (k == 1) {
         mint_t A0 = AddMod(a[0], a[1], q);
         mint_t A1 = SubMod(a[0], a[1], q);
         A[0] = A0;
         A[1] = A1;
	 return;
      }
   }

   // assume k > 1
   const mint_t *root = info.RootTable[0].elts();
   mulmod_t qinv = info.qinv;
   const FFTMultipliers& tab = info.bigtab->MulTab;

   if (k >= tab.length()) LazyPrecompFFTMultipliers(k, q, qinv, root, tab);


   long n = 1L << k;


   const mint_t *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab[s] = tab[s]->wtab_precomp.elts();

   const mulmod_precon_t *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab[s] = tab[s]->wqinvtab_precomp.elts();

   new_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];



#ifdef NTL_FFT_USEBUF
   NTL_TLS_GLOBAL_ACCESS(AA_store);
   AA_store.SetLength(1L << k);
   umint_t *AA = AA_store.elts();

   for (long i = 0; i < xn; i++) AA[i] = a[i];

   new_fft_short(AA, yn, xn, k, mod);

   for (long i = 0; i < yn; i++) {
      A[i] = LazyReduce1(AA[i], q);
   }
#else
   umint_t *AA = (umint_t *) A;
   if (a != A) for (long i = 0; i < xn; i++) AA[i] = a[i];

   new_fft_short(AA, yn, xn, k, mod);

   for (long i = 0; i < yn; i++) {
      AA[i] = LazyReduce1(AA[i], q);
   }
#endif

}

void new_fft_flipped(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info)

// Performs a high-level FFT.  Inputs and outputs are in the range [0,q). 
// Both A and a should point to arrays of size 2^k,
// and should either be the same or not overlap at all.
// This version is "flipped" -- it uses inverted roots, 
// multiplies by 2^{-k}, and performs no truncations.

{
   if (!info.bigtab || k > info.bigtab->bound) {
      new_fft_flipped_notab(A, a, k, info);
      return;
   }

   mint_t q = info.q;

   if (k <= 1) {
      if (k == 0) {
	 A[0] = a[0];
	 return;
      }
      if (k == 1) {
         mint_t two_inv = info.TwoInvTable[1];
         mulmod_precon_t two_inv_aux = info.TwoInvPreconTable[1];
         mint_t A0 = AddMod(a[0], a[1], q);
         mint_t A1 = SubMod(a[0], a[1], q);
         A[0] = LazyReduce1(LazyMulModPrecon(A0, two_inv, q, two_inv_aux), q);
         A[1] = LazyReduce1(LazyMulModPrecon(A1, two_inv, q, two_inv_aux), q);
	 return;
      }
   }

   // assume k > 1
   const mint_t *root = info.RootTable[0].elts();
   mulmod_t qinv = info.qinv;
   const FFTMultipliers& tab = info.bigtab->MulTab;

   if (k >= tab.length()) LazyPrecompFFTMultipliers(k, q, qinv, root, tab);


   long n = 1L << k;


   const mint_t *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab[s] = tab[s]->wtab_precomp.elts();

   const mulmod_precon_t *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab[s] = tab[s]->wqinvtab_precomp.elts();

   new_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];

   mint_t two_inv = info.TwoInvTable[k];
   mulmod_precon_t two_inv_aux = info.TwoInvPreconTable[k];


#ifdef NTL_FFT_USEBUF
   NTL_TLS_GLOBAL_ACCESS(AA_store);
   AA_store.SetLength(1L << k);
   umint_t *AA = AA_store.elts();

   for (long i = 0; i < n; i++) AA[i] = a[i];

   new_fft_short_flipped(AA, k, mod);

   for (long i = 0; i < n; i++) {
      umint_t tmp = LazyMulModPrecon(AA[i], two_inv, q, two_inv_aux); 
      A[i] = LazyReduce1(tmp, q);
   }
#else
   umint_t *AA = (umint_t *) A;
   if (a != A) for (long i = 0; i < n; i++) AA[i] = a[i];

   new_fft_short_flipped(AA, k, mod);

   for (long i = 0; i < n; i++) {
      umint_t tmp = LazyMulModPrecon(AA[i], two_inv, q, two_inv_aux); 
      AA[i] = LazyReduce1(tmp, q);
   }
#endif
}

//=======  Inverse FFT with tables ==============


void new_ifft(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info, 
              long yn)

// Performs a high-level IFFT.  Inputs and outputs are in the range [0,q). 
// yn==xn are as described above in the truncated FFT interface.
// Both A and a should point to arrays of size 2^k,
// and should either be the same or not overlap at all.
// Multiples by 2^{-k}.

{
   if (!info.bigtab || k > info.bigtab->bound) {
      new_ifft_notab(A, a, k, info, yn);
      return;
   }

   mint_t q = info.q;

   if (k <= 1) {
      if (k == 0) {
	 A[0] = a[0];
	 return;
      }
      if (k == 1) {
         mint_t two_inv = info.TwoInvTable[1];
         mulmod_precon_t two_inv_aux = info.TwoInvPreconTable[1];
         mint_t A0 = AddMod(a[0], a[1], q);
         mint_t A1 = SubMod(a[0], a[1], q);
         A[0] = LazyReduce1(LazyMulModPrecon(A0, two_inv, q, two_inv_aux), q);
         A[1] = LazyReduce1(LazyMulModPrecon(A1, two_inv, q, two_inv_aux), q);
	 return;
      }
   }

   // assume k > 1
   const mint_t *root = info.RootTable[0].elts();
   mulmod_t qinv = info.qinv;
   const FFTMultipliers& tab = info.bigtab->MulTab;

   if (k >= tab.length()) LazyPrecompFFTMultipliers(k, q, qinv, root, tab);


   long n = 1L << k;


   const mint_t *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab[s] = tab[s]->wtab_precomp.elts();

   const mulmod_precon_t *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab[s] = tab[s]->wqinvtab_precomp.elts();

   new_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];

   mint_t two_inv = info.TwoInvTable[k];
   mulmod_precon_t two_inv_aux = info.TwoInvPreconTable[k];

#ifdef NTL_FFT_USEBUF
   NTL_TLS_GLOBAL_ACCESS(AA_store);
   AA_store.SetLength(1L << k);
   umint_t *AA = AA_store.elts();

   for (long i = 0; i < yn; i++) AA[i] = a[i];

   new_ifft_short1(AA, yn, k, mod);

   for (long i = 0; i < yn; i++) {
      umint_t tmp = LazyMulModPrecon(AA[i], two_inv, q, two_inv_aux); 
      A[i] = LazyReduce1(tmp, q);
   }
#else
   umint_t *AA = (umint_t *) A;
   if (a != A) for (long i = 0; i < yn; i++) AA[i] = a[i];

   new_ifft_short1(AA, yn, k, mod);

   for (long i = 0; i < yn; i++) {
      umint_t tmp = LazyMulModPrecon(AA[i], two_inv, q, two_inv_aux); 
      AA[i] = LazyReduce1(tmp, q);
   }
#endif
}


void new_ifft_flipped(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info)


// Performs a high-level IFFT.  Inputs and outputs are in the range [0,q). 
// Flipped means inverse roots are used an no truncation and
// no multiplication by 2^{-k}.
// Both A and a should point to arrays of size 2^k,
// and should either be the same or not overlap at all.


{
   if (!info.bigtab || k > info.bigtab->bound) {
      new_ifft_flipped_notab(A, a, k, info);
      return;
   }

   mint_t q = info.q;

   if (k <= 1) {
      if (k == 0) {
	 A[0] = a[0];
	 return;
      }
      if (k == 1) {
         mint_t A0 = AddMod(a[0], a[1], q);
         mint_t A1 = SubMod(a[0], a[1], q);
         A[0] = A0;
         A[1] = A1;
	 return;
      }
   }

   // assume k > 1
   const mint_t *root = info.RootTable[0].elts();
   mulmod_t qinv = info.qinv;
   const FFTMultipliers& tab = info.bigtab->MulTab;

   if (k >= tab.length()) LazyPrecompFFTMultipliers(k, q, qinv, root, tab);


   long n = 1L << k;


   const mint_t *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab[s] = tab[s]->wtab_precomp.elts();

   const mulmod_precon_t *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab[s] = tab[s]->wqinvtab_precomp.elts();

   new_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];


#ifdef NTL_FFT_USEBUF
   NTL_TLS_GLOBAL_ACCESS(AA_store);
   AA_store.SetLength(1L << k);
   umint_t *AA = AA_store.elts();

   for (long i = 0; i < n; i++) AA[i] = a[i];

   new_ifft_short1_flipped(AA, k, mod);

   for (long i = 0; i < n; i++) {
      umint_t tmp = LazyReduce2(AA[i], q);
      A[i] = LazyReduce1(tmp, q);
   }
#else
   umint_t *AA = (umint_t *) A;
   if (a != A) for (long i = 0; i < n; i++) AA[i] = a[i];

   new_ifft_short1_flipped(AA, k, mod);

   for (long i = 0; i < n; i++) {
      umint_t tmp = LazyReduce2(AA[i], q);
      AA[i] = LazyReduce1(tmp, q);
   }
#endif
}

#endif

//===============================================

void InitFFTPrimeInfo(FFTPrimeInfo& info, long q, long w, long bigtab_index)
{
   mulmod_t qinv = PrepMulMod(q);

   long mr = CalcMaxRoot(q);

   info.q = q;
   info.qinv = qinv;
   info.qrecip = 1/double(q);
   info.zz_p_context = 0;


   info.RootTable[0].SetLength(mr+1);
   info.RootTable[1].SetLength(mr+1);
   info.TwoInvTable.SetLength(mr+1);
   info.TwoInvPreconTable.SetLength(mr+1);

   long *rt = &info.RootTable[0][0];
   long *rit = &info.RootTable[1][0];
   long *tit = &info.TwoInvTable[0];
   mulmod_precon_t *tipt = &info.TwoInvPreconTable[0];

   long j;
   long t;

   rt[mr] = w;
   for (j = mr-1; j >= 0; j--)
      rt[j] = MulMod(rt[j+1], rt[j+1], q);

   rit[mr] = InvMod(w, q);
   for (j = mr-1; j >= 0; j--)
      rit[j] = MulMod(rit[j+1], rit[j+1], q);

   t = InvMod(2, q);
   tit[0] = 1;
   for (j = 1; j <= mr; j++)
      tit[j] = MulMod(tit[j-1], t, q);

   for (j = 0; j <= mr; j++)
      tipt[j] = LazyPrepMulModPrecon(tit[j], q, qinv);

#ifndef NTL_ENABLE_AVX_FFT
   if (bigtab_index != -1) {
      long bound = NTL_FFT_BIGTAB_MAXROOT-bigtab_index/NTL_FFT_BIGTAB_LIMIT;
      if (bound > NTL_FFT_BIGTAB_MINROOT) {
         info.bigtab.make();
         info.bigtab->bound = bound;
      }
   }
#else
   // with the AVX implementation, we unconditionally use tables
   info.bigtab.make();
#endif
}


//===================================================================

#ifdef NTL_ENABLE_AVX_FFT

static void
pd_LazyPrepMulModPrecon(double *bninv, const double *b, double n, long len)
{
   CSRPush push;
   pd_LazyPrepMulModPrecon_impl(bninv, b, n, len);
}

static
void LazyPrecompFFTMultipliers(long k, mint_t q, mulmod_t qinv, const mint_t *root, const pd_FFTMultipliers& tab)
{
   if (k < 1) LogicError("LazyPrecompFFTMultipliers: bad input");

   do { // NOTE: thread safe lazy init
      pd_FFTMultipliers::Builder bld(tab, k+1);
      long amt = bld.amt();
      if (!amt) break;

      long first = k+1-amt;
      // initialize entries first..k


      for (long s = first; s <= k; s++) {
         UniquePtr<pd_FFTVectorPair> item;

         if (s == 0) {
            bld.move(item); // position 0 not used
            continue;
         }

         long m = 1L << s;
         long m_half = 1L << (s-1);

         item.make();
         item->wtab_precomp.SetLength(m_half);
         item->wqinvtab_precomp.SetLength(m_half);

         double *wtab = item->wtab_precomp.elts();
         double *wqinvtab = item->wqinvtab_precomp.elts();

         mint_t w = root[s];
         mulmod_precon_t wqinv = PrepMulModPrecon(w, q, qinv);

         mint_t wi = 1;
         wtab[0] = wi;
         for (long i = 1; i < m_half; i++) {
            wi = MulModPrecon(wi, w, q, wqinv);
            wtab[i] = wi;
         }
         pd_LazyPrepMulModPrecon(wqinvtab, wtab, q, m_half);

         bld.move(item);
      }
   } while (0);
}

NTL_TLS_GLOBAL_DECL(AlignedArray<double>, pd_AA_store)
static NTL_CHEAP_THREAD_LOCAL long pd_AA_store_len = 0;


#define PD_MIN_K (NTL_LG2_PDSZ+3)
// k must be at least PD_MIN_K

void new_fft(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info,
            long yn, long xn)
{
   if (k < PD_MIN_K) {
      new_fft_notab(A, a, k, info, yn, xn);
      return;
   }

   long dir = 0;

   mint_t q = info.q;
   const mint_t *root = info.RootTable[dir].elts();
   mulmod_t qinv = info.qinv;
   const pd_FFTMultipliers& tab = info.bigtab->pd_MulTab[dir];

   if (k >= tab.length()) LazyPrecompFFTMultipliers(k, q, qinv, root, tab);

   const double *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab[s] = tab[s]->wtab_precomp.elts();

   const double *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab[s] = tab[s]->wqinvtab_precomp.elts();

   pd_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];

   long n = 1L << k;

   NTL_TLS_GLOBAL_ACCESS(pd_AA_store);
   if (pd_AA_store_len < n) pd_AA_store.SetLength(n);
   double *AA = pd_AA_store.elts();

   CSRPush push;
   pd_fft_trunc_impl(A, a, AA, k, mod, yn, xn);
}



void new_fft_flipped(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info)
{
   if (k < PD_MIN_K) {
      new_fft_flipped_notab(A, a, k, info);
      return;
   }

   long dir = 1;

   mint_t q = info.q;
   const mint_t *root = info.RootTable[dir].elts();
   mulmod_t qinv = info.qinv;
   const pd_FFTMultipliers& tab = info.bigtab->pd_MulTab[dir];

   if (k >= tab.length()) LazyPrecompFFTMultipliers(k, q, qinv, root, tab);

   const double *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab[s] = tab[s]->wtab_precomp.elts();

   const double *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab[s] = tab[s]->wqinvtab_precomp.elts();

   pd_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];

   long n = 1L << k;

   NTL_TLS_GLOBAL_ACCESS(pd_AA_store);
   if (pd_AA_store_len < n) pd_AA_store.SetLength(n);
   double *AA = pd_AA_store.elts();

   CSRPush push;
   pd_fft_trunc_impl(A, a, AA, k, mod, n, n, info.TwoInvTable[k]);
}


void new_ifft(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info,
            long yn)
{
   if (k < PD_MIN_K) {
      new_ifft_notab(A, a, k, info, yn);
      return;
   }

   long dir = 0;

   mint_t q = info.q;
   const mint_t *root = info.RootTable[1-dir].elts();
   const mint_t *root1 = info.RootTable[dir].elts();
   mulmod_t qinv = info.qinv;
   const pd_FFTMultipliers& tab = info.bigtab->pd_MulTab[1-dir];
   const pd_FFTMultipliers& tab1 = info.bigtab->pd_MulTab[dir];

   if (k >= tab.length()) LazyPrecompFFTMultipliers(k, q, qinv, root, tab);
   if (k >= tab1.length()) LazyPrecompFFTMultipliers(k, q, qinv, root1, tab1);

   const double *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab[s] = tab[s]->wtab_precomp.elts();

   const double *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab[s] = tab[s]->wqinvtab_precomp.elts();

   const double *wtab1[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab1[s] = tab1[s]->wtab_precomp.elts();

   const double *wqinvtab1[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab1[s] = tab1[s]->wqinvtab_precomp.elts();

   pd_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];
   mod.wtab1 = &wtab1[0];
   mod.wqinvtab1 = &wqinvtab1[0];

   long n = 1L << k;

   NTL_TLS_GLOBAL_ACCESS(pd_AA_store);
   if (pd_AA_store_len < n) pd_AA_store.SetLength(n);
   double *AA = pd_AA_store.elts();

   CSRPush push;
   pd_ifft_trunc_impl(A, a, AA, k, mod, yn, info.TwoInvTable[k]);
}


void new_ifft_flipped(mint_t* A, const mint_t* a, long k, const FFTPrimeInfo& info)
{
   if (k < PD_MIN_K) {
      new_ifft_flipped_notab(A, a, k, info);
      return;
   }

   long dir = 1;

   mint_t q = info.q;
   const mint_t *root = info.RootTable[1-dir].elts();
   const mint_t *root1 = info.RootTable[dir].elts();
   mulmod_t qinv = info.qinv;
   const pd_FFTMultipliers& tab = info.bigtab->pd_MulTab[1-dir];
   const pd_FFTMultipliers& tab1 = info.bigtab->pd_MulTab[dir];

   if (k >= tab.length()) LazyPrecompFFTMultipliers(k, q, qinv, root, tab);
   if (k >= tab1.length()) LazyPrecompFFTMultipliers(k, q, qinv, root1, tab1);

   const double *wtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab[s] = tab[s]->wtab_precomp.elts();

   const double *wqinvtab[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab[s] = tab[s]->wqinvtab_precomp.elts();

   const double *wtab1[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wtab1[s] = tab1[s]->wtab_precomp.elts();

   const double *wqinvtab1[NTL_FFTMaxRoot+1];
   for (long s = 1; s <= k; s++) wqinvtab1[s] = tab1[s]->wqinvtab_precomp.elts();

   pd_mod_t mod;
   mod.q = q;
   mod.wtab = &wtab[0];
   mod.wqinvtab = &wqinvtab[0];
   mod.wtab1 = &wtab1[0];
   mod.wqinvtab1 = &wqinvtab1[0];

   long n = 1L << k;

   NTL_TLS_GLOBAL_ACCESS(pd_AA_store);
   if (pd_AA_store_len < n) pd_AA_store.SetLength(n);
   double *AA = pd_AA_store.elts();

   CSRPush push;
   pd_ifft_trunc_impl(A, a, AA, k, mod, n);
}

#endif


NTL_END_IMPL

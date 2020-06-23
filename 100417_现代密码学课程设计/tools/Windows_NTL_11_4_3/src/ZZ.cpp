
#include <NTL/ZZ.h>
#include <NTL/vec_ZZ.h>
#include <NTL/Lazy.h>
#include <NTL/fileio.h>
#include <NTL/SmartPtr.h>

#include <NTL/BasicThreadPool.h>



#if defined(NTL_HAVE_AVX2)
#include <immintrin.h>
#elif defined(NTL_HAVE_SSSE3)
#include <emmintrin.h>
#include <tmmintrin.h>
#endif





NTL_START_IMPL





const ZZ& ZZ::zero()
{
   
   static const ZZ z; // GLOBAL (relies on C++11 thread-safe init)
   return z;
}


const ZZ& ZZ_expo(long e)
{
   NTL_TLS_LOCAL(ZZ, expo_helper);

   conv(expo_helper, e);
   return expo_helper;
}



void AddMod(ZZ& x, const ZZ& a, long b, const ZZ& n)
{
   NTL_ZZRegister(B);
   conv(B, b);
   AddMod(x, a, B, n);
}


void SubMod(ZZ& x, const ZZ& a, long b, const ZZ& n)
{
   NTL_ZZRegister(B);
   conv(B, b);
   SubMod(x, a, B, n);
}

void SubMod(ZZ& x, long a, const ZZ& b, const ZZ& n)
{
   NTL_ZZRegister(A);
   conv(A, a);
   SubMod(x, A, b, n);
}



// ****** input and output


static NTL_CHEAP_THREAD_LOCAL long iodigits = 0;
static NTL_CHEAP_THREAD_LOCAL long ioradix = 0;
// iodigits is the greatest integer such that 10^{iodigits} < NTL_WSP_BOUND
// ioradix = 10^{iodigits}

static void InitZZIO()
{
   long x;

   x = (NTL_WSP_BOUND-1)/10;
   iodigits = 0;
   ioradix = 1;

   while (x) {
      x = x / 10;
      iodigits++;
      ioradix = ioradix * 10;
   }

   if (iodigits <= 0) TerminalError("problem with I/O");
}


istream& operator>>(istream& s, ZZ& x)
{
   long c;
   long cval;
   long sign;
   long ndigits;
   long acc;
   NTL_ZZRegister(a);

   if (!s) NTL_INPUT_ERROR(s, "bad ZZ input");

   if (!iodigits) InitZZIO();

   a = 0;

   SkipWhiteSpace(s);
   c = s.peek();

   if (c == '-') {
      sign = -1;
      s.get();
      c = s.peek();
   }
   else
      sign = 1;

   cval = CharToIntVal(c);

   if (cval < 0 || cval > 9) NTL_INPUT_ERROR(s, "bad ZZ input");

   ndigits = 0;
   acc = 0;
   while (cval >= 0 && cval <= 9) {
      acc = acc*10 + cval;
      ndigits++;

      if (ndigits == iodigits) {
         mul(a, a, ioradix);
         add(a, a, acc);
         ndigits = 0;
         acc = 0;
      }

      s.get();
      c = s.peek();
      cval = CharToIntVal(c);
   }

   if (ndigits != 0) {
      long mpy = 1;
      while (ndigits > 0) {
         mpy = mpy * 10;
         ndigits--;
      }

      mul(a, a, mpy);
      add(a, a, acc);
   }

   if (sign == -1)
      negate(a, a);

   x = a;
   return s;
}


// The class _ZZ_local_stack should be defined in an empty namespace,
// but since I don't want to rely on namespaces, we just give it a funny 
// name to avoid accidental name clashes.

struct _ZZ_local_stack {
   long top;
   Vec<long> data;

   _ZZ_local_stack() { top = -1; }

   long pop() { return data[top--]; }
   long empty() { return (top == -1); }
   void push(long x);
};

void _ZZ_local_stack::push(long x)
{
   if (top+1 >= data.length()) 
      data.SetLength(max(32, long(1.414*data.length())));

   top++;
   data[top] = x;
}


static
void PrintDigits(ostream& s, long d, long justify)
{
   NTL_TLS_LOCAL_INIT(Vec<char>, buf, (INIT_SIZE, iodigits));

   long i = 0;

   while (d) {
      buf[i] = IntValToChar(d % 10);
      d = d / 10;
      i++;
   }

   if (justify) {
      long j = iodigits - i;
      while (j > 0) {
         s << "0";
         j--;
      }
   }

   while (i > 0) {
      i--;
      s << buf[i];
   }
}
      

   

ostream& operator<<(ostream& s, const ZZ& a)
{
   ZZ b;
   _ZZ_local_stack S;
   long r;
   long k;

   if (!iodigits) InitZZIO();

   b = a;

   k = sign(b);

   if (k == 0) {
      s << "0";
      return s;
   }

   if (k < 0) {
      s << "-";
      negate(b, b);
   }

   do {
      r = DivRem(b, b, ioradix);
      S.push(r);
   } while (!IsZero(b));

   r = S.pop();
   PrintDigits(s, r, 0);

   while (!S.empty()) {
      r = S.pop();
      PrintDigits(s, r, 1);
   }
      
   return s;
}



long GCD(long a, long b)
{
   long u, v, t, x;

   if (a < 0) {
      if (a < -NTL_MAX_LONG) ResourceError("GCD: integer overflow");
      a = -a;
   }

   if (b < 0) {
      if (b < -NTL_MAX_LONG) ResourceError("GCD: integer overflow");
      b = -b;
   }


   if (b==0)
      x = a;
   else {
      u = a;
      v = b;
      do {
         t = u % v;
         u = v; 
         v = t;
      } while (v != 0);

      x = u;
   }

   return x;
}

         

void XGCD(long& d, long& s, long& t, long a, long b)
{
   long  u, v, u0, v0, u1, v1, u2, v2, q, r;

   long aneg = 0, bneg = 0;

   if (a < 0) {
      if (a < -NTL_MAX_LONG) ResourceError("XGCD: integer overflow");
      a = -a;
      aneg = 1;
   }

   if (b < 0) {
      if (b < -NTL_MAX_LONG) ResourceError("XGCD: integer overflow");
      b = -b;
      bneg = 1;
   }

   u1=1; v1=0;
   u2=0; v2=1;
   u = a; v = b;

   while (v != 0) {
      q = u / v;
      r = u % v;
      u = v;
      v = r;
      u0 = u2;
      v0 = v2;
      u2 =  u1 - q*u2;
      v2 = v1- q*v2;
      u1 = u0;
      v1 = v0;
   }

   if (aneg)
      u1 = -u1;

   if (bneg)
      v1 = -v1;

   d = u;
   s = u1;
   t = v1;
}
   
long InvModStatus(long& x, long a, long n)
{
   long d, s, t;

   XGCD(d, s, t, a, n);
   if (d != 1) {
      x = d;
      return 1;
   }
   else {
      if (s < 0)
         x = s + n;
      else
         x = s;

      return 0;
   }
}

long InvMod(long a, long n)
{
   long d, s, t;

   XGCD(d, s, t, a, n);
   if (d != 1) {
      InvModError("InvMod: inverse undefined");
   }
   if (s < 0)
      return s + n;
   else
      return s;
}


long PowerMod(long a, long ee, long n)
{
   long x, y;

   unsigned long e;

   if (ee < 0)
      e = - ((unsigned long) ee);
   else
      e = ee;

   x = 1;
   y = a;
   while (e) {
      if (e & 1) x = MulMod(x, y, n);
      y = MulMod(y, y, n);
      e = e >> 1;
   }

   if (ee < 0) x = InvMod(x, n);

   return x;
}

static
long MillerWitness_sp(long n, long x)
{
   long m, y, z;
   long j, k;

   if (x == 0) return 0;

   m = n - 1;
   k = 0;
   while((m & 1) == 0) {
      m = m >> 1;
      k++;
   }
   // n - 1 == 2^k * m, m odd

   z = PowerMod(x, m, n);
   if (z == 1) return 0;

   j = 0;
   do {
      y = z;
      z = MulMod(y, y, n);
      j++;
   } while (j != k && z != 1);

   if (z != 1 || y != n-1) return 1;
   return 0;
}

long ProbPrime(long n, long NumTrials)
{
   if (NumTrials < 0) NumTrials = 0;

   long m, x, y, z;
   long i, j, k;

   if (n <= 1) return 0;


   if (n == 2) return 1;
   if (n % 2 == 0) return 0;

   if (n == 3) return 1;
   if (n % 3 == 0) return 0;

   if (n == 5) return 1;
   if (n % 5 == 0) return 0;

   if (n == 7) return 1;
   if (n % 7 == 0) return 0;

   if (n == 11) return 1;
   if (n % 11 == 0) return 0;

   if (n == 13) return 1;
   if (n % 13 == 0) return 0;

   if (n >= NTL_SP_BOUND) {
      return ProbPrime(to_ZZ(n), NumTrials);
   }

   m = n - 1;
   k = 0;
   while((m & 1) == 0) {
      m = m >> 1;
      k++;
   }

   // n - 1 == 2^k * m, m odd

   for (i = 0; i < NumTrials+1; i++) {
      // for consistency with the multi-precision version,
      // we first see if 2 is a witness, so we really do 
      // NumTrials+1 tests

      if (i == 0) 
         x = 2;
      else {
	 do {
	    x = RandomBnd(n);
	 } while (x == 0);
         // x == 0 is not a useful candidate for a witness!
      }

      z = PowerMod(x, m, n);
      if (z == 1) continue;
   
      j = 0;
      do {
         y = z;
         z = MulMod(y, y, n);
         j++;
      } while (j != k && z != 1);

      if (z != 1 || y !=  n-1) return 0;
   }

   return 1;
}


long MillerWitness(const ZZ& n, const ZZ& x)
{
   if (n.SinglePrecision()) {
      return MillerWitness_sp(to_long(n), to_long(x));
   }

   ZZ m, y, z;

   long j, k;

   if (x == 0) return 0;

   add(m, n, -1);
   k = MakeOdd(m);
   // n - 1 == 2^k * m, m odd

   PowerMod(z, x, m, n);
   if (z == 1) return 0;

   j = 0;
   do {
      y = z;
      SqrMod(z, y, n);
      j++;
   } while (j != k && z != 1);

   if (z != 1) return 1;
   add(y, y, 1);
   if (y != n) return 1;
   return 0;
}


// ComputePrimeBound computes a reasonable bound for trial
// division in the Miller-Rabin test.
// It is computed a bit on the "low" side, since being a bit
// low doesn't hurt much, but being too high can hurt a lot.

// See the paper "Fast generation of prime numbers and secure
// public-key cryptographic parameters" by Ueli Maurer.
// In that paper, it is calculated that the optimal bound in
// roughly T_exp/T_div, where T_exp is the time for an exponentiation
// and T_div is is the time for a single precision division.
// Of course, estimating these times is a bit tricky, and
// the values we use are based on experimentation, assuming
// GMP is being used.  I've tested this on various bit lengths
// up to 16,000, and they seem to be pretty close to optimal. 

static
long ComputePrimeBound(long bn)
{
   long wn = (bn+NTL_ZZ_NBITS-1)/NTL_ZZ_NBITS;

   long fn;

   if (wn <= 36)
      fn = wn/4 + 1;
   else
      fn = long(1.67*sqrt(double(wn)));

   long prime_bnd;

   if (NumBits(bn) + NumBits(fn) > NTL_SP_NBITS)
      prime_bnd = NTL_SP_BOUND;
   else
      prime_bnd = bn*fn;

   return prime_bnd;


}


long ProbPrime(const ZZ& n, long NumTrials)
{
   if (NumTrials < 0) NumTrials = 0;

   if (n <= 1) return 0;

   if (n.SinglePrecision()) {
      return ProbPrime(to_long(n), NumTrials);
   }


   long prime_bnd = ComputePrimeBound(NumBits(n));


   PrimeSeq s;
   long p;

   p = s.next();
   while (p && p < prime_bnd) {
      if (rem(n, p) == 0)
         return 0;

      p = s.next();
   }

   ZZ W;
   W = 2;

   // first try W == 2....the exponentiation
   // algorithm runs slightly faster in this case

   if (MillerWitness(n, W))
      return 0;


   long i;

   for (i = 0; i < NumTrials; i++) {
      do {
         RandomBnd(W, n);
      } while (W == 0);
      // W == 0 is not a useful candidate for a witness!

      if (MillerWitness(n, W)) 
         return 0;
   }

   return 1;
}


static
void MultiThreadedRandomPrime(ZZ& n, long l, long NumTrials)
{

   long nt = AvailableThreads();



   const long LOCAL_ITER_BOUND = 8; 
   // since resetting the PRG comes at a certain cost,
   // we perform a few iterations with each reset to
   // amortize the reset cost. 

   unsigned long initial_counter = 0;
   ZZ seed;
   RandomBits(seed, 256);

   for (;;) {

      AtomicLowWaterMark low_water_mark(-1UL);
      AtomicCounter counter(initial_counter);

      Vec< UniquePtr<ZZ> > result(INIT_SIZE, nt);
      Vec<unsigned long> result_ctr(INIT_SIZE, nt, -1UL);

      NTL_EXEC_INDEX(nt, index)

         RandomStreamPush push;

	 SetSeed(seed);
	 RandomStream& stream = GetCurrentRandomStream();

	 ZZ cand;

	 while (low_water_mark == -1UL) {

	    unsigned long local_ctr = counter.inc();
            if (local_ctr >> (NTL_BITS_PER_NONCE-1)) {
               // counter overflow...rather academic
               break;
            }

	    stream.set_nonce(local_ctr);
	    
	    for (long iter = 0; iter < LOCAL_ITER_BOUND && 
				local_ctr <= low_water_mark; iter++) {

	       RandomLen(cand, l);
	       if (!IsOdd(cand)) add(cand, cand, 1);

	       if (ProbPrime(cand, 0)) { 
		  result[index].make(cand);
		  result_ctr[index] = local_ctr;
		  low_water_mark.UpdateMin(local_ctr);
		  break;
	       }
	    }
	 }

      NTL_EXEC_INDEX_END

      // find index of low_water_mark

      unsigned long low_water_mark1 = low_water_mark;
      long low_water_index = -1;

      for (long index = 0; index < nt; index++) {
	 if (result_ctr[index] == low_water_mark1) {
	    low_water_index = index;
	    break;
	 }
      }

      if (low_water_index == -1) {
         // counter overflow...rather academic
         initial_counter = 0;
         RandomBits(seed, 256);
         continue;
      }

      ZZ N;
      N = *result[low_water_index];

      Vec<ZZ> W(INIT_SIZE, NumTrials);

      for (long i = 0; i < NumTrials; i++) {
         do { 
            RandomBnd(W[i], N);
         } while (W[i] == 0);
      }

      AtomicBool tests_pass(true);

      NTL_EXEC_RANGE(NumTrials, first, last)

         for (long i = first; i < last && tests_pass; i++) {
            if (MillerWitness(N, W[i])) tests_pass = false;
         }

      NTL_EXEC_RANGE_END

      if (tests_pass) {
         n = N;
         return;
      }

      // very unlikey to get here
      initial_counter = low_water_mark1 + 1;
   }
}


void RandomPrime(ZZ& n, long l, long NumTrials)
{
   if (NumTrials < 0) NumTrials = 0;

   if (l >= 256) { 
      MultiThreadedRandomPrime(n, l, NumTrials); 
      return;
   }

   if (l <= 1)
      LogicError("RandomPrime: l out of range");

   if (l == 2) {
      if (RandomBnd(2))
         n = 3;
      else
         n = 2;

      return;
   }

   do {
      RandomLen(n, l);
      if (!IsOdd(n)) add(n, n, 1);
   } while (!ProbPrime(n, NumTrials));
}

void OldRandomPrime(ZZ& n, long l, long NumTrials)
{
   if (l <= 1)
      LogicError("RandomPrime: l out of range");

   if (l == 2) {
      if (RandomBnd(2))
         n = 3;
      else
         n = 2;

      return;
   }

   do {
      RandomLen(n, l);
      if (!IsOdd(n)) add(n, n, 1);
   } while (!ProbPrime(n, NumTrials));
}

void NextPrime(ZZ& n, const ZZ& m, long NumTrials)
{
   ZZ x;

   if (m <= 2) {
      n = 2;
      return;
   }

   x = m;

   while (!ProbPrime(x, NumTrials))
      add(x, x, 1);

   n = x;
}

long NextPrime(long m, long NumTrials)
{
   long x;

   if (m <= 2) 
      return 2;

   x = m;

   while (x < NTL_SP_BOUND && !ProbPrime(x, NumTrials))
      x++;

   if (x >= NTL_SP_BOUND)
      ResourceError("NextPrime: no more primes");

   return x;
}



long NextPowerOfTwo(long m)
{
   long k; 
   unsigned long n, um;

   if (m < 0) return 0;

   um = m;
   n = 1;
   k = 0;

   while (n < um) {
      n = n << 1;
      k++;
   }

   if (k >= NTL_BITS_PER_LONG-1)
      ResourceError("NextPowerOfTwo: overflow");

   return k;
}




long bit(long a, long k)
{
   unsigned long aa;
   if (a < 0)
      aa = - ((unsigned long) a);
   else
      aa = a;

   if (k < 0 || k >= NTL_BITS_PER_LONG) 
      return 0;
   else
      return long((aa >> k) & 1);
}



long divide(ZZ& q, const ZZ& a, const ZZ& b)
{
   NTL_ZZRegister(qq);
   NTL_ZZRegister(r);

   if (IsZero(b)) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }


   if (IsOne(b)) {
      q = a;
      return 1;
   }

   DivRem(qq, r, a, b);
   if (!IsZero(r)) return 0;
   q = qq;
   return 1;
}

long divide(const ZZ& a, const ZZ& b)
{
   NTL_ZZRegister(r);

   if (IsZero(b)) return IsZero(a);
   if (IsOne(b)) return 1;

   rem(r, a, b);
   return IsZero(r);
}

long divide(ZZ& q, const ZZ& a, long b)
{
   NTL_ZZRegister(qq);

   if (!b) {
      if (IsZero(a)) {
         clear(q);
         return 1;
      }
      else
         return 0;
   }

   if (b == 1) {
      q = a;
      return 1;
   }

   long r = DivRem(qq, a, b);
   if (r) return 0;
   q = qq;
   return 1;
}

long divide(const ZZ& a, long b)
{
   if (!b) return IsZero(a);
   if (b == 1) {
      return 1;
   }

   long r = rem(a,  b);
   return (r == 0);
}


void InvMod(ZZ& x, const ZZ& a, const ZZ& n)
{
   // NOTE: the underlying LIP routines write to the first argument,
   // even if inverse is undefined

   NTL_ZZRegister(xx);
   if (InvModStatus(xx, a, n)) 
      InvModError("InvMod: inverse undefined", a, n);
   x = xx;
}

void PowerMod(ZZ& x, const ZZ& a, const ZZ& e, const ZZ& n)
{
   // NOTE: this ensures that all modular inverses are computed
   // in the routine InvMod above, rather than the LIP-internal
   // modular inverse routine
   if (e < 0) {
      ZZ a_inv;
      ZZ e_neg;

      InvMod(a_inv, a, n);
      negate(e_neg, e);
      LowLevelPowerMod(x, a_inv, e_neg, n);
   }
   else
      LowLevelPowerMod(x, a, e, n); 
}
   
#ifdef NTL_EXCEPTIONS

void InvModError(const char *s, const ZZ& a, const ZZ& n)
{
   throw InvModErrorObject(s, a, n); 
}

#else

void InvModError(const char *s, const ZZ& a, const ZZ& n)
{
   TerminalError(s);
}


#endif

long RandomPrime_long(long l, long NumTrials)
{
   if (NumTrials < 0) NumTrials = 0;

   if (l <= 1 || l >= NTL_BITS_PER_LONG)
      ResourceError("RandomPrime: length out of range");

   long n;
   do {
      n = RandomLen_long(l);
   } while (!ProbPrime(n, NumTrials));

   return n;
}


static Lazy< Vec<char> > lowsieve_storage;
// This is a GLOBAL VARIABLE


PrimeSeq::PrimeSeq()
{
   movesieve = 0;
   pshift = -1;
   pindex = -1;
   exhausted = 0;
}


long PrimeSeq::next()
{
   if (exhausted) {
      return 0;
   }

   if (pshift < 0) {
      shift(0);
      return 2;
   }

   for (;;) {
      const char *p = movesieve;
      long i = pindex;

      while ((++i) < NTL_PRIME_BND) {
         if (p[i]) {
            pindex = i;
            return pshift + 2 * i + 3;
         }
      }

      long newshift = pshift + 2*NTL_PRIME_BND;

      if (newshift > 2 * NTL_PRIME_BND * (2 * NTL_PRIME_BND + 1)) {
         /* end of the road */
         exhausted = 1;
         return 0;
      }

      shift(newshift);
   }
}

void PrimeSeq::shift(long newshift)
{
   long i;
   long j;
   long jstep;
   long jstart;
   long ibound;
   char *p;

   if (!lowsieve_storage.built())
      start();

   const char *lowsieve = lowsieve_storage->elts();


   if (newshift < 0) {
      pshift = -1;
   }
   else if (newshift == 0) {
      pshift = 0;
      movesieve = lowsieve;
   } 
   else if (newshift != pshift) {
      if (movesieve_mem.length() == 0) {
         movesieve_mem.SetLength(NTL_PRIME_BND);
      }

      pshift = newshift;
      movesieve = p = movesieve_mem.elts();
      for (i = 0; i < NTL_PRIME_BND; i++)
         p[i] = 1;

      jstep = 3;
      ibound = pshift + 2 * NTL_PRIME_BND + 1;
      for (i = 0; jstep * jstep <= ibound; i++) {
         if (lowsieve[i]) {
            if (!((jstart = (pshift + 2) / jstep + 1) & 1))
               jstart++;
            if (jstart <= jstep)
               jstart = jstep;
            jstart = (jstart * jstep - pshift - 3) / 2;
            for (j = jstart; j < NTL_PRIME_BND; j += jstep)
               p[j] = 0;
         }
         jstep += 2;
      }
   }

   pindex = -1;
   exhausted = 0;
}


void PrimeSeq::start()
{
   long i;
   long j;
   long jstep;
   long jstart;
   long ibnd;
   char *p;

   do {
      Lazy< Vec<char> >::Builder builder(lowsieve_storage);
      if (!builder()) break;

      UniquePtr< Vec<char> > ptr;
      ptr.make();
      ptr->SetLength(NTL_PRIME_BND);

      p = ptr->elts();

      for (i = 0; i < NTL_PRIME_BND; i++)
         p[i] = 1;
         
      jstep = 1;
      jstart = -1;
      ibnd = (SqrRoot(2 * NTL_PRIME_BND + 1) - 3) / 2;
      for (i = 0; i <= ibnd; i++) {
         jstart += 2 * ((jstep += 2) - 1);
         if (p[i])
            for (j = jstart; j < NTL_PRIME_BND; j += jstep)
               p[j] = 0;
      }

      builder.move(ptr);
   } while (0);

}

void PrimeSeq::reset(long b)
{
   if (b > (2*NTL_PRIME_BND+1)*(2*NTL_PRIME_BND+1)) {
      exhausted = 1;
      return;
   }

   if (b <= 2) {
      shift(-1);
      return;
   }

   if ((b & 1) == 0) b++;

   shift(((b-3) / (2*NTL_PRIME_BND))* (2*NTL_PRIME_BND));
   pindex = (b - pshift - 3)/2 - 1;
}
 
long Jacobi(const ZZ& aa, const ZZ& nn)
{
   ZZ a, n;
   long t, k;
   long d;

   a = aa;
   n = nn;
   t = 1;

   while (a != 0) {
      k = MakeOdd(a);
      d = trunc_long(n, 3);
      if ((k & 1) && (d == 3 || d == 5)) t = -t;

      if (trunc_long(a, 2) == 3 && (d & 3) == 3) t = -t;
      swap(a, n);
      rem(a, a, n);
   }

   if (n == 1)
      return t;
   else
      return 0;
}


void SqrRootMod(ZZ& x, const ZZ& aa, const ZZ& nn)
{
   if (aa == 0 || aa == 1) {
      x = aa;
      return;
   }

   // at this point, we must have nn >= 5

   if (trunc_long(nn, 2) == 3) {  // special case, n = 3 (mod 4)
      ZZ n, a, e, z;

      n = nn;
      a  = aa;

      add(e, n, 1);
      RightShift(e, e, 2);

      PowerMod(z, a, e, n);
      x = z;

      return;
   }

   ZZ n, m;
   int h, nlen;

   n = nn;
   nlen = NumBits(n);

   sub(m, n, 1);
   h = MakeOdd(m);  // h >= 2


   if (nlen > 50 && h < SqrRoot(nlen)) {
      long i, j;
      ZZ a, b, a_inv, c, r, m1, d;

      a = aa;
      InvMod(a_inv, a, n);

      if (h == 2) 
         b = 2;
      else {
         do {
            RandomBnd(b, n);
         } while (Jacobi(b, n) != -1);
      }


      PowerMod(c, b, m, n);
      
      add(m1, m, 1);
      RightShift(m1, m1, 1);
      PowerMod(r, a, m1, n);

      for (i = h-2; i >= 0; i--) {
         SqrMod(d, r, n);
         MulMod(d, d, a_inv, n);
         for (j = 0; j < i; j++)
            SqrMod(d, d, n);
         if (!IsOne(d))
            MulMod(r, r, c, n);
         SqrMod(c, c, n);
      } 

      x = r;
      return;
   } 





   long i, k;
   ZZ ma, t, u, v, e;
   ZZ t1, t2, t3, t4;

   n = nn;
   NegateMod(ma, aa, n);

   // find t such that t^2 - 4*a is not a square

   MulMod(t1, ma, 4, n);
   do {
      RandomBnd(t, n);
      SqrMod(t2, t, n);
      AddMod(t2, t2, t1, n);
   } while (Jacobi(t2, n) != -1);

   // compute u*X + v = X^{(n+1)/2} mod f, where f = X^2 - t*X + a

   add(e, n, 1);
   RightShift(e, e, 1);

   u = 0;
   v = 1;

   k = NumBits(e);

   for (i = k - 1; i >= 0; i--) {
      add(t2, u, v);
      sqr(t3, t2);  // t3 = (u+v)^2
      sqr(t1, u);
      sqr(t2, v);
      sub(t3, t3, t1);
      sub(t3, t3, t2); // t1 = u^2, t2 = v^2, t3 = 2*u*v
      rem(t1, t1, n);
      mul(t4, t1, t);
      add(t4, t4, t3);
      rem(u, t4, n);

      mul(t4, t1, ma);
      add(t4, t4, t2);
      rem(v, t4, n);
      
      if (bit(e, i)) {
         MulMod(t1, u, t, n);
         AddMod(t1, t1, v, n);
         MulMod(v, u, ma, n);
         u = t1;
      }

   }

   x = v;
}



// Chinese Remaindering.
//
// This version in new to v3.7, and is significantly
// simpler and faster than the previous version.
//
// This function takes as input g, a, G, p,
// such that a > 0, 0 <= G < p, and gcd(a, p) = 1.
// It computes a' = a*p and g' such that 
//   * g' = g (mod a);
//   * g' = G (mod p);
//   * -a'/2 < g' <= a'/2.
// It then sets g := g' and a := a', and returns 1 iff g has changed.
//
// Under normal use, the input value g satisfies -a/2 < g <= a/2;
// however, this was not documented or enforced in earlier versions,
// so to maintain backward compatability, no restrictions are placed
// on g.  This routine runs faster, though, if -a/2 < g <= a/2,
// and the first thing the routine does is to make this condition
// hold.
//
// Also, under normal use, both a and p are odd;  however, the routine
// will still work even if this is not so.
//
// The routine is based on the following simple fact.
//
// Let -a/2 < g <= a/2, and let h satisfy
//   * g + a h = G (mod p);
//   * -p/2 < h <= p/2.
// Further, if p = 2*h and g > 0, set
//   g' := g - a h;
// otherwise, set
//   g' := g + a h.
// Then g' so defined satisfies the above requirements.
//
// It is trivial to see that g's satisfies the congruence conditions.
// The only thing is to check that the "balancing" condition
// -a'/2 < g' <= a'/2 also holds.


long CRT(ZZ& gg, ZZ& a, long G, long p)
{
   if (p >= NTL_SP_BOUND) {
      ZZ GG, pp;
      conv(GG, G);
      conv(pp, p);
      return CRT(gg, a, GG, pp);
   }

   long modified = 0;

   NTL_ZZRegister(g);

   if (!CRTInRange(gg, a)) {
      modified = 1;
      ZZ a1;
      rem(g, gg, a);
      RightShift(a1, a, 1);
      if (g > a1) sub(g, g, a);
   }
   else
      g = gg;


   long p1;
   p1 = p >> 1;

   long a_inv;
   a_inv = rem(a, p);
   a_inv = InvMod(a_inv, p);

   long h;
   h = rem(g, p);
   h = SubMod(G, h, p);
   h = MulMod(h, a_inv, p);
   if (h > p1)
      h = h - p;

   if (h != 0) {
      modified = 1;

      if (!(p & 1) && g > 0 && (h == p1))
         MulSubFrom(g, a, h);
      else
         MulAddTo(g, a, h);
   }

   mul(a, a, p);
   gg = g;

   return modified;
}

long CRT(ZZ& gg, ZZ& a, const ZZ& G, const ZZ& p)
{
   long modified = 0;

   ZZ g;

   if (!CRTInRange(gg, a)) {
      modified = 1;
      ZZ a1;
      rem(g, gg, a);
      RightShift(a1, a, 1);
      if (g > a1) sub(g, g, a);
   }
   else
      g = gg;


   ZZ p1;
   RightShift(p1, p, 1);

   ZZ a_inv;
   rem(a_inv, a, p);
   InvMod(a_inv, a_inv, p);

   ZZ h;
   rem(h, g, p);
   SubMod(h, G, h, p);
   MulMod(h, h, a_inv, p);
   if (h > p1)
      sub(h, h, p);

   if (h != 0) {
      modified = 1;
      ZZ ah;
      mul(ah, a, h);

      if (!IsOdd(p) && g > 0 &&  (h == p1))
         sub(g, g, ah);
      else
         add(g, g, ah);
   }

   mul(a, a, p);
   gg = g;

   return modified;
}



void sub(ZZ& x, long a, const ZZ& b)
{
   NTL_ZZRegister(A);
   conv(A, a);
   sub(x, A, b);
}


void power2(ZZ& x, long e)
{
   if (e < 0) ArithmeticError("power2: negative exponent");
   set(x);
   LeftShift(x, x, e);
}

   

void bit_and(ZZ& x, const ZZ& a, long b)
{
   NTL_ZZRegister(B);
   conv(B, b);
   bit_and(x, a, B);
}

void bit_or(ZZ& x, const ZZ& a, long b)
{
   NTL_ZZRegister(B);
   conv(B, b);
   bit_or(x, a, B);
}

void bit_xor(ZZ& x, const ZZ& a, long b)
{
   NTL_ZZRegister(B);
   conv(B, b);
   bit_xor(x, a, B);
}


long power_long(long a, long e)
{
   if (e < 0) ArithmeticError("power_long: negative exponent");

   if (e == 0) return 1;

   if (a == 1) return 1;
   if (a == -1) {
      if (e & 1)
         return -1;
      else
         return 1;
   }

   // no overflow check --- result is computed correctly
   // modulo word size

   unsigned long res = 1;
   unsigned long aa = a;
   long i;

   for (i = 0; i < e; i++)
      res *= aa;

   return to_long(res);
}



// ======================= new PRG stuff ======================




#if (NTL_BITS_PER_INT32 == 32)
#define INT32MASK(x) (x)
#else
#define INT32MASK(x) ((x) & _ntl_uint32(0xffffffff))
#endif



// SHA256 code adapted from an implementauin by Brad Conte.
// The following is from his original source files.
/*********************************************************************
* Filename:   sha256.c
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Implementation of the SHA-256 hashing algorithm.
              SHA-256 is one of the three algorithms in the SHA2
              specification. The others, SHA-384 and SHA-512, are not
              offered in this implementation.
              Algorithm specification can be found here:
               * http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
              This implementation uses little endian byte order.
*********************************************************************/

// And the following is from the description at 
// https://github.com/B-Con/crypto-algorithms

/*********************************************************************

These are basic implementations of standard cryptography algorithms, written by
Brad Conte (brad@bradconte.com) from scratch and without any cross-licensing.
They exist to provide publically accessible, restriction-free implementations
of popular cryptographic algorithms, like AES and SHA-1. These are primarily
intended for educational and pragmatic purposes (such as comparing a
specification to actual implementation code, or for building an internal
application that computes test vectors for a product). The algorithms have been
tested against standard test vectors.

This code is released into the public domain free of any restrictions. The
author requests acknowledgement if the code is used, but does not require it.
This code is provided free of any liability and without any quality claims by
the author.

Note that these are not cryptographically secure implementations. They have no
resistence to side-channel attacks and should not be used in contexts that need
cryptographically secure implementations.

These algorithms are not optimized for speed or space. They are primarily
designed to be easy to read, although some basic optimization techniques have
been employed.

*********************************************************************/






#define SHA256_BLOCKSIZE (64)
#define SHA256_HASHSIZE  (32)

// DBL_INT_ADD treats two unsigned ints a and b as one 64-bit integer and adds c to it
static inline
void DBL_INT_ADD(_ntl_uint32& a, _ntl_uint32& b, _ntl_uint32 c)
{
   _ntl_uint32 aa = INT32MASK(a);
   if (aa > INT32MASK(_ntl_uint32(0xffffffff) - c)) b++;
   a = aa + c;
}

#define ROTLEFT(a,b) (((a) << (b)) | (INT32MASK(a) >> (32-(b))))
#define ROTRIGHT(a,b) ((INT32MASK(a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ (INT32MASK(x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ (INT32MASK(x) >> 10))

struct SHA256_CTX {
   unsigned char data[64];
   _ntl_uint32 datalen;
   _ntl_uint32 bitlen[2];
   _ntl_uint32 state[8];
};

static const _ntl_uint32 sha256_const[64] = {
   0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
   0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
   0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
   0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
   0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
   0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
   0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
   0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};


static
void sha256_transform(SHA256_CTX& ctx, unsigned char *data)
{  
   _ntl_uint32 a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
      
   for (i=0,j=0; i < 16; ++i, j += 4)
      m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
   for ( ; i < 64; ++i)
      m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

   a = ctx.state[0];
   b = ctx.state[1];
   c = ctx.state[2];
   d = ctx.state[3];
   e = ctx.state[4];
   f = ctx.state[5];
   g = ctx.state[6];
   h = ctx.state[7];
   
   for (i = 0; i < 64; ++i) {
      t1 = h + EP1(e) + CH(e,f,g) + sha256_const[i] + m[i];
      t2 = EP0(a) + MAJ(a,b,c);
      h = g;
      g = f;
      f = e;
      e = d + t1;
      d = c;
      c = b;
      b = a;
      a = t1 + t2;
   }
   
   ctx.state[0] += a;
   ctx.state[1] += b;
   ctx.state[2] += c;
   ctx.state[3] += d;
   ctx.state[4] += e;
   ctx.state[5] += f;
   ctx.state[6] += g;
   ctx.state[7] += h;
}  

static
void sha256_init(SHA256_CTX& ctx)
{  
   ctx.datalen = 0; 
   ctx.bitlen[0] = 0; 
   ctx.bitlen[1] = 0; 
   ctx.state[0] = 0x6a09e667;
   ctx.state[1] = 0xbb67ae85;
   ctx.state[2] = 0x3c6ef372;
   ctx.state[3] = 0xa54ff53a;
   ctx.state[4] = 0x510e527f;
   ctx.state[5] = 0x9b05688c;
   ctx.state[6] = 0x1f83d9ab;
   ctx.state[7] = 0x5be0cd19;
}

static
void sha256_update(SHA256_CTX& ctx, const unsigned char *data, _ntl_uint32 len)
{  
   _ntl_uint32 i;
   
   for (i=0; i < len; ++i) { 
      ctx.data[ctx.datalen] = data[i]; 
      ctx.datalen++; 
      if (ctx.datalen == 64) { 
         sha256_transform(ctx,ctx.data);
         DBL_INT_ADD(ctx.bitlen[0],ctx.bitlen[1],512); 
         ctx.datalen = 0; 
      }  
   }  
}  

static
void sha256_final(SHA256_CTX& ctx, unsigned char *hash, 
                  long hlen=SHA256_HASHSIZE)
{  
   _ntl_uint32 i, j; 
   
   i = ctx.datalen; 
   
   // Pad whatever data is left in the buffer. 
   if (ctx.datalen < 56) { 
      ctx.data[i++] = 0x80; 
      while (i < 56) 
         ctx.data[i++] = 0x00; 
   }  
   else { 
      ctx.data[i++] = 0x80; 
      while (i < 64) 
         ctx.data[i++] = 0x00; 
      sha256_transform(ctx,ctx.data);
      memset(ctx.data,0,56); 
   }  
   
   // Append to the padding the total message's length in bits and transform. 
   DBL_INT_ADD(ctx.bitlen[0],ctx.bitlen[1],ctx.datalen * 8);

   ctx.data[63] = ctx.bitlen[0]; 
   ctx.data[62] = ctx.bitlen[0] >> 8; 
   ctx.data[61] = ctx.bitlen[0] >> 16; 
   ctx.data[60] = ctx.bitlen[0] >> 24; 
   ctx.data[59] = ctx.bitlen[1]; 
   ctx.data[58] = ctx.bitlen[1] >> 8; 
   ctx.data[57] = ctx.bitlen[1] >> 16;  
   ctx.data[56] = ctx.bitlen[1] >> 24; 
   sha256_transform(ctx,ctx.data);
   
   for (i = 0; i < 8; i++) {
      _ntl_uint32 w = ctx.state[i];
      for (j = 0; j < 4; j++) {
         if (hlen <= 0) break;
         hash[4*i + j] = w >> (24-j*8); 
         hlen--;
      }
   }

}  



static
void sha256(const unsigned char *data, long dlen, unsigned char *hash, 
            long hlen=SHA256_HASHSIZE)
{
   if (dlen < 0) dlen = 0;
   if (hlen < 0) hlen = 0;

   SHA256_CTX ctx;
   sha256_init(ctx);

   const long BLKSIZE = 4096;

   long i;
   for (i = 0; i <= dlen-BLKSIZE; i += BLKSIZE) 
      sha256_update(ctx, data + i, BLKSIZE);

   if (i < dlen)
      sha256_update(ctx, data + i, dlen - i);

   sha256_final(ctx, hash, hlen);
}


static
void hmac_sha256(const unsigned char *key, long klen, 
                 const unsigned char *data, long dlen,
                 unsigned char *hash, long hlen=SHA256_HASHSIZE)
{
   if (klen < 0) klen = 0;
   if (dlen < 0) dlen = 0;
   if (hlen < 0) hlen = 0;

   unsigned char K[SHA256_BLOCKSIZE];
   unsigned char tmp[SHA256_HASHSIZE];

   long i;

   if (klen <= SHA256_BLOCKSIZE) {
      for (i = 0; i < klen; i++)
         K[i] = key[i];
      for (i = klen; i < SHA256_BLOCKSIZE; i++) 
         K[i] = 0;
   }
   else {
      sha256(key, klen, K, SHA256_BLOCKSIZE); 
      for (i = SHA256_HASHSIZE; i < SHA256_BLOCKSIZE; i++)
         K[i] = 0;
   }

   for (i = 0; i < SHA256_BLOCKSIZE; i++)
      K[i] ^= 0x36;

   SHA256_CTX ctx;
   sha256_init(ctx);
   sha256_update(ctx, K, SHA256_BLOCKSIZE);
   sha256_update(ctx, data, dlen);
   sha256_final(ctx, tmp);

   for (i = 0; i < SHA256_BLOCKSIZE; i++)
      K[i] ^= (0x36 ^ 0x5C);

   sha256_init(ctx);
   sha256_update(ctx, K, SHA256_BLOCKSIZE);
   sha256_update(ctx, tmp, SHA256_HASHSIZE);
   sha256_final(ctx, hash, hlen);
}


// This key derivation uses HMAC with a zero key to derive
// an intermediate key K from the data, and then uses HMAC
// as a PRF in counter mode with key K to derive the final key

void DeriveKey(unsigned char *key, long klen,  
               const unsigned char *data, long dlen)
{
   if (dlen < 0) LogicError("DeriveKey: bad args");
   if (klen < 0) LogicError("DeriveKey: bad args");

   long i, j;


   unsigned char K[SHA256_HASHSIZE];
   hmac_sha256(0, 0, data, dlen, K); 

   // initialize 64-bit counter to zero
   unsigned char counter[8];
   for (j = 0; j < 8; j++) counter[j] = 0;

   for (i = 0; i <= klen-SHA256_HASHSIZE; i += SHA256_HASHSIZE) {
      hmac_sha256(K, SHA256_HASHSIZE, counter, 8, key+i); 

      // increment counter
      for (j = 0; j < 8; j++) {
         counter[j]++;
         if (counter[j] != 0) break; 
      }
   }

   if (i < klen) 
      hmac_sha256(K, SHA256_HASHSIZE, counter, 8, key+i, klen-i);
}




// ******************** ChaCha20 stuff ***********************

// ============= old stuff

#define LE(p) (((_ntl_uint32)((p)[0])) + ((_ntl_uint32)((p)[1]) << 8) + \
    ((_ntl_uint32)((p)[2]) << 16) + ((_ntl_uint32)((p)[3]) << 24))

#define FROMLE(p, x) (p)[0] = (x), (p)[1] = ((x) >> 8), \
   (p)[2] = ((x) >> 16), (p)[3] = ((x) >> 24)


#define QUARTERROUND(x, a, b, c, d) \
    x[a] += x[b], x[d] = ROTLEFT(x[d] ^ x[a], 16), \
    x[c] += x[d], x[b] = ROTLEFT(x[b] ^ x[c], 12), \
    x[a] += x[b], x[d] = ROTLEFT(x[d] ^ x[a], 8), \
    x[c] += x[d], x[b] = ROTLEFT(x[b] ^ x[c], 7)


static
void salsa20_core(_ntl_uint32* data)
{
   long i;

   for (i = 0; i < 10; i++) {
      QUARTERROUND(data, 0, 4, 8, 12);
      QUARTERROUND(data, 1, 5, 9, 13);
      QUARTERROUND(data, 2, 6, 10, 14);
      QUARTERROUND(data, 3, 7, 11, 15);
      QUARTERROUND(data, 0, 5, 10, 15);
      QUARTERROUND(data, 1, 6, 11, 12);
      QUARTERROUND(data, 2, 7, 8, 13);
      QUARTERROUND(data, 3, 4, 9, 14);
   }
}


// key K must be exactly 32 bytes
static
void salsa20_init(_ntl_uint32 *state, const unsigned char *K)  
{
   static const _ntl_uint32 chacha_const[4] = 
      { 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574 };

   long i;

   for (i = 0; i < 4; i++)
      state[i] = chacha_const[i];

   for (i = 4; i < 12; i++)
      state[i] = LE(K + 4*(i-4));

   for (i = 12; i < 16; i++)
      state[i] = 0;
}



// state and data are of length 16
static
void salsa20_apply(_ntl_uint32 *state, _ntl_uint32 *data)
{
   long i;

   for (i = 0; i < 16; i++) data[i] = state[i];

   salsa20_core(data);

   for (i = 0; i < 16; i++) data[i] += state[i];

   for (i = 12; i < 14; i++) {
      state[i]++;
      state[i] = INT32MASK(state[i]);
      if (state[i] != 0) break;
   }
}



old_RandomStream::old_RandomStream(const unsigned char *key)
{
   salsa20_init(state, key);
   pos = 64;
}


void old_RandomStream::do_get(unsigned char *res, long n)
{
   if (n < 0) LogicError("RandomStream::get: bad args");

   long i, j;

   if (n <= 64-pos) {
      for (i = 0; i < n; i++) res[i] = buf[pos+i];
      pos += n;
      return;
   }

   // read remainder of buffer
   for (i = 0; i < 64-pos; i++) res[i] = buf[pos+i];
   n -= 64-pos;
   res += 64-pos;
   pos = 64;

   _ntl_uint32 wdata[16];

   // read 64-byte chunks
   for (i = 0; i <= n-64; i += 64) {
      salsa20_apply(state, wdata);
      for (j = 0; j < 16; j++)
         FROMLE(res + i + 4*j, wdata[j]);
   }

   if (i < n) { 
      salsa20_apply(state, wdata);

      for (j = 0; j < 16; j++)
         FROMLE(buf + 4*j, wdata[j]);

      pos = n-i;
      for (j = 0; j < pos; j++)
         res[i+j] = buf[j];
   }
}

#if (defined(NTL_HAVE_AVX2) || defined(NTL_HAVE_SSSE3))


/*****************************************************************

This AVX2 implementation is derived from public domain code 
originally developed by Martin Goll Shay Gueron, and obtained from 
here:

https://github.com/floodyberry/supercop/tree/master/crypto_stream/chacha20/goll_gueron

On a Haswell machine, ths code is about 4.x faster than the vanilla 
C code.

The following is the README from that page

==================================================================

This code implements Daniel J. Bernstein's ChaCha stream cipher in C,
targeting architectures with AVX2 and future AVX512 vector extensions.

The implementation improves the slightly modified implementations of Ted Krovetz in the Chromium Project
(http://src.chromium.org/viewvc/chrome/trunk/deps/third_party/nss/nss/lib/freebl/chacha20/chacha20_vec.c and
http://src.chromium.org/viewvc/chrome/trunk/deps/third_party/openssl/openssl/crypto/chacha/chacha_vec.c)
by using the Advanced Vector Extensions AVX2 and, if available in future, AVX512 to widen the vectorization
to 256-bit, respectively 512-bit.

On Intel's Haswell architecture this implementation (using AVX2) is almost ~2x faster than the fastest 
implementation here, when encrypting (decrypting) 2 blocks and more. Also, this implementation is expected 
to double the speed again, when encrypting (decrypting) 4 blocks and more, running on a future architecture
with support for AVX512.

Further details and our measurement results are provided in:
Goll, M., and Gueron,S.: Vectorization of ChaCha Stream Cipher. Cryptology ePrint Archive, 
Report 2013/759, November, 2013, http://eprint.iacr.org/2013/759.pdf

Developers and authors:
*********************************************************
Martin Goll (1) and Shay Gueron (2, 3), 
(1) Ruhr-University Bochum, Germany
(2) University of Haifa, Israel
(3) Intel Corporation, Israel Development Center, Haifa, Israel
*********************************************************

Intellectual Property Notices
-----------------------------

There are no known present or future claims by a copyright holder that the
distribution of this software infringes the copyright. In particular, the author
of the software is not making such claims and does not intend to make such
claims.

There are no known present or future claims by a patent holder that the use of
this software infringes the patent. In particular, the author of the software is
not making such claims and does not intend to make such claims.

Our implementation is in public domain.

*****************************************************************/


// round selector, specified values:
//  8:  low security - high speed
// 12:  mid security -  mid speed
// 20: high security -  low speed
#ifndef CHACHA_RNDS
#define CHACHA_RNDS 20
#endif


#if (defined(NTL_HAVE_AVX2))

typedef __m256i ivec_t;

#define DELTA	_mm256_set_epi64x(0,2,0,2)
#define START   _mm256_set_epi64x(0,1,0,0)
#define NONCE(nonce) _mm256_set_epi64x(nonce, 1, nonce, 0)   

#define STOREU_VEC(m,r)	_mm256_storeu_si256((__m256i*)(m), r)
#define STORE_VEC(m,r)	_mm256_store_si256((__m256i*)(m), r)

#define LOAD_VEC(r,m) r = _mm256_load_si256((const __m256i *)(m))
#define LOADU_VEC(r,m) r = _mm256_loadu_si256((const __m256i *)(m))

#define LOADU_VEC_128(r, m) r = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)(m)))



#define ADD_VEC_32(a,b)	_mm256_add_epi32(a, b)
#define ADD_VEC_64(a,b)	_mm256_add_epi64(a, b)
#define XOR_VEC(a,b)	_mm256_xor_si256(a, b)


#define ROR_VEC_V1(x)	_mm256_shuffle_epi32(x,_MM_SHUFFLE(0,3,2,1))
#define ROR_VEC_V2(x)	_mm256_shuffle_epi32(x,_MM_SHUFFLE(1,0,3,2))
#define ROR_VEC_V3(x)	_mm256_shuffle_epi32(x,_MM_SHUFFLE(2,1,0,3))
#define ROL_VEC_7(x)	XOR_VEC(_mm256_slli_epi32(x, 7), _mm256_srli_epi32(x,25))
#define ROL_VEC_12(x)	XOR_VEC(_mm256_slli_epi32(x,12), _mm256_srli_epi32(x,20))

#define ROL_VEC_8(x)	_mm256_shuffle_epi8(x,_mm256_set_epi8(14,13,12,15,10,9,8,11,6,5,4,7,2,1,0,3,14,13,12,15,10,9,8,11,6,5,4,7,2,1,0,3))


#define ROL_VEC_16(x)	_mm256_shuffle_epi8(x,_mm256_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2,13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2))



#define WRITEU_VEC(op, d, v0, v1, v2, v3)						\
    STOREU_VEC(op + (d + 0*4), _mm256_permute2x128_si256(v0, v1, 0x20));	\
    STOREU_VEC(op + (d + 8*4), _mm256_permute2x128_si256(v2, v3, 0x20));	\
    STOREU_VEC(op + (d +16*4), _mm256_permute2x128_si256(v0, v1, 0x31));	\
    STOREU_VEC(op + (d +24*4), _mm256_permute2x128_si256(v2, v3, 0x31));

#define WRITE_VEC(op, d, v0, v1, v2, v3)						\
    STORE_VEC(op + (d + 0*4), _mm256_permute2x128_si256(v0, v1, 0x20));	\
    STORE_VEC(op + (d + 8*4), _mm256_permute2x128_si256(v2, v3, 0x20));	\
    STORE_VEC(op + (d +16*4), _mm256_permute2x128_si256(v0, v1, 0x31));	\
    STORE_VEC(op + (d +24*4), _mm256_permute2x128_si256(v2, v3, 0x31));

#define SZ_VEC (32)

#define RANSTREAM_NCHUNKS (2)
// leads to a BUFSZ of 512


#elif defined(NTL_HAVE_SSSE3)

typedef __m128i ivec_t;

#define DELTA	_mm_set_epi32(0,0,0,1)
#define START   _mm_setzero_si128()
#define NONCE(nonce) _mm_set_epi64x(nonce,0)

#define STOREU_VEC(m,r)	_mm_storeu_si128((__m128i*)(m), r)
#define STORE_VEC(m,r)	_mm_store_si128((__m128i*)(m), r)

#define LOAD_VEC(r,m) r = _mm_load_si128((const __m128i *)(m))
#define LOADU_VEC(r,m) r = _mm_loadu_si128((const __m128i *)(m))

#define LOADU_VEC_128(r, m) r = _mm_loadu_si128((const __m128i*)(m))

#define ADD_VEC_32(a,b)	_mm_add_epi32(a, b)
#define ADD_VEC_64(a,b)	_mm_add_epi64(a, b)
#define XOR_VEC(a,b)	_mm_xor_si128(a, b)


#define ROR_VEC_V1(x)	_mm_shuffle_epi32(x,_MM_SHUFFLE(0,3,2,1))
#define ROR_VEC_V2(x)	_mm_shuffle_epi32(x,_MM_SHUFFLE(1,0,3,2))
#define ROR_VEC_V3(x)	_mm_shuffle_epi32(x,_MM_SHUFFLE(2,1,0,3))
#define ROL_VEC_7(x)	XOR_VEC(_mm_slli_epi32(x, 7), _mm_srli_epi32(x,25))
#define ROL_VEC_12(x)	XOR_VEC(_mm_slli_epi32(x,12), _mm_srli_epi32(x,20))

#define ROL_VEC_8(x)	_mm_shuffle_epi8(x,_mm_set_epi8(14,13,12,15,10,9,8,11,6,5,4,7,2,1,0,3))


#define ROL_VEC_16(x)	_mm_shuffle_epi8(x,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2))


#define WRITEU_VEC(op, d, v0, v1, v2, v3)	\
    STOREU_VEC(op + (d + 0*4), v0);	\
    STOREU_VEC(op + (d + 4*4), v1);	\
    STOREU_VEC(op + (d + 8*4), v2);	\
    STOREU_VEC(op + (d +12*4), v3);

#define WRITE_VEC(op, d, v0, v1, v2, v3)	\
    STORE_VEC(op + (d + 0*4), v0);	\
    STORE_VEC(op + (d + 4*4), v1);	\
    STORE_VEC(op + (d + 8*4), v2);	\
    STORE_VEC(op + (d +12*4), v3);

#define SZ_VEC (16)

#define RANSTREAM_NCHUNKS (4)
// leads to a BUFSZ of 512

#else

#error "unsupported architecture"

#endif


#define DQROUND_VECTORS_VEC(a,b,c,d)				\
    a = ADD_VEC_32(a,b); d = XOR_VEC(d,a); d = ROL_VEC_16(d);	\
    c = ADD_VEC_32(c,d); b = XOR_VEC(b,c); b = ROL_VEC_12(b);	\
    a = ADD_VEC_32(a,b); d = XOR_VEC(d,a); d = ROL_VEC_8(d);	\
    c = ADD_VEC_32(c,d); b = XOR_VEC(b,c); b = ROL_VEC_7(b);	\
    b = ROR_VEC_V1(b); c = ROR_VEC_V2(c); d = ROR_VEC_V3(d);	\
    a = ADD_VEC_32(a,b); d = XOR_VEC(d,a); d = ROL_VEC_16(d);	\
    c = ADD_VEC_32(c,d); b = XOR_VEC(b,c); b = ROL_VEC_12(b);	\
    a = ADD_VEC_32(a,b); d = XOR_VEC(d,a); d = ROL_VEC_8(d);	\
    c = ADD_VEC_32(c,d); b = XOR_VEC(b,c); b = ROL_VEC_7(b);	\
    b = ROR_VEC_V3(b); c = ROR_VEC_V2(c); d = ROR_VEC_V1(d);



#define RANSTREAM_STATESZ (4*SZ_VEC)

#define RANSTREAM_CHUNKSZ (2*RANSTREAM_STATESZ)
#define RANSTREAM_BUFSZ   (RANSTREAM_NCHUNKS*RANSTREAM_CHUNKSZ)


struct RandomStream_impl {

   AlignedArray<unsigned char> state_store;
   AlignedArray<unsigned char> buf_store;
   long chunk_count;

   void allocate_space() 
   {
      state_store.SetLength(RANSTREAM_STATESZ);
      buf_store.SetLength(RANSTREAM_BUFSZ);
   }


   explicit
   RandomStream_impl(const unsigned char *key) 
   {
      allocate_space();

      unsigned char *state = state_store.elts();

      unsigned int chacha_const[] = {
	      0x61707865,0x3320646E,0x79622D32,0x6B206574
      };


      ivec_t d0, d1, d2, d3;
      LOADU_VEC_128(d0, chacha_const);
      LOADU_VEC_128(d1, key);
      LOADU_VEC_128(d2, key+16);

      d3 = START;


      STORE_VEC(state + 0*SZ_VEC, d0); 
      STORE_VEC(state + 1*SZ_VEC, d1); 
      STORE_VEC(state + 2*SZ_VEC, d2); 
      STORE_VEC(state + 3*SZ_VEC, d3); 

      chunk_count = 0;
   }

   RandomStream_impl(const RandomStream_impl& other) 
   {
      allocate_space();
      *this = other;
   }

   RandomStream_impl& operator=(const RandomStream_impl& other) 
   {
      std::memcpy(state_store.elts(), other.state_store.elts(), RANSTREAM_STATESZ);
      std::memcpy(buf_store.elts(), other.buf_store.elts(), RANSTREAM_BUFSZ);
      chunk_count = other.chunk_count;
      return *this;
   }

   const unsigned char *
   get_buf() const
   {
      return buf_store.elts(); 
   }

   long
   get_buf_len() const
   {
      return RANSTREAM_BUFSZ;
   }

   // bytes are generated in chunks of RANSTREAM_BUFSZ bytes, except that
   // initially, we may generate a few chunks of RANSTREAM_CHUNKSZ
   // bytes.  This optimizes a bit for short bursts following a reset.

   long
   get_bytes(unsigned char *NTL_RESTRICT res, 
             long n, long pos)
   {
      if (n < 0) LogicError("RandomStream::get: bad args");
      if (n == 0) return pos;

      unsigned char *NTL_RESTRICT buf = buf_store.elts();

      if (n <= RANSTREAM_BUFSZ-pos) {
	 std::memcpy(&res[0], &buf[pos], n);
	 pos += n;
	 return pos;
      }

      unsigned char *NTL_RESTRICT state = state_store.elts();

      ivec_t d0, d1, d2, d3;
      LOAD_VEC(d0, state + 0*SZ_VEC);
      LOAD_VEC(d1, state + 1*SZ_VEC);
      LOAD_VEC(d2, state + 2*SZ_VEC);
      LOAD_VEC(d3, state + 3*SZ_VEC);


      // read remainder of buffer
      std::memcpy(&res[0], &buf[pos], RANSTREAM_BUFSZ-pos);
      n -= RANSTREAM_BUFSZ-pos;
      res += RANSTREAM_BUFSZ-pos;
      pos = RANSTREAM_BUFSZ;

      long i = 0;
      for (;  i <= n-RANSTREAM_BUFSZ; i += RANSTREAM_BUFSZ) {

         chunk_count |= RANSTREAM_NCHUNKS;  // disable small buffer strategy

	 for (long j = 0; j < RANSTREAM_NCHUNKS; j++) {
	    ivec_t v0=d0, v1=d1, v2=d2, v3=d3;
	    ivec_t v4=d0, v5=d1, v6=d2, v7=ADD_VEC_64(d3, DELTA);

	    for (long k = 0; k < CHACHA_RNDS/2; k++) {
		    DQROUND_VECTORS_VEC(v0,v1,v2,v3)
		    DQROUND_VECTORS_VEC(v4,v5,v6,v7)
	    }

	    WRITEU_VEC(res+i+j*(8*SZ_VEC), 0, ADD_VEC_32(v0,d0), ADD_VEC_32(v1,d1), ADD_VEC_32(v2,d2), ADD_VEC_32(v3,d3))
	    d3 = ADD_VEC_64(d3, DELTA);
	    WRITEU_VEC(res+i+j*(8*SZ_VEC), 4*SZ_VEC, ADD_VEC_32(v4,d0), ADD_VEC_32(v5,d1), ADD_VEC_32(v6,d2), ADD_VEC_32(v7,d3))
	    d3 = ADD_VEC_64(d3, DELTA);

	 }

      }

      if (i < n) {

         long nchunks;

         if (chunk_count < RANSTREAM_NCHUNKS) {
            nchunks = long(cast_unsigned((n-i)+RANSTREAM_CHUNKSZ-1)/RANSTREAM_CHUNKSZ);
            chunk_count += nchunks;
         }
         else
            nchunks = RANSTREAM_NCHUNKS;

         long pos_offset = RANSTREAM_BUFSZ - nchunks*RANSTREAM_CHUNKSZ;
         buf += pos_offset;

	 for (long j = 0; j < nchunks; j++) {
	    ivec_t v0=d0, v1=d1, v2=d2, v3=d3;
	    ivec_t v4=d0, v5=d1, v6=d2, v7=ADD_VEC_64(d3, DELTA);

	    for (long k = 0; k < CHACHA_RNDS/2; k++) {
               DQROUND_VECTORS_VEC(v0,v1,v2,v3)
               DQROUND_VECTORS_VEC(v4,v5,v6,v7)
	    }

	    WRITE_VEC(buf+j*(8*SZ_VEC), 0, ADD_VEC_32(v0,d0), ADD_VEC_32(v1,d1), ADD_VEC_32(v2,d2), ADD_VEC_32(v3,d3))
	    d3 = ADD_VEC_64(d3, DELTA);
	    WRITE_VEC(buf+j*(8*SZ_VEC), 4*SZ_VEC, ADD_VEC_32(v4,d0), ADD_VEC_32(v5,d1), ADD_VEC_32(v6,d2), ADD_VEC_32(v7,d3))
	    d3 = ADD_VEC_64(d3, DELTA);
	 }

	 pos = n-i+pos_offset;
	 std::memcpy(&res[i], &buf[0], n-i);
      }

      STORE_VEC(state + 3*SZ_VEC, d3); 

      return pos;
   }

   void set_nonce(unsigned long nonce)
   {
      unsigned char *state = state_store.elts();
      ivec_t d3;
      d3 = NONCE(nonce);
      STORE_VEC(state + 3*SZ_VEC, d3);
      chunk_count = 0;
   }

};


#else

struct RandomStream_impl {
   _ntl_uint32 state[16];
   unsigned char buf[64];

   explicit
   RandomStream_impl(const unsigned char *key)
   {
      salsa20_init(state, key);
   }

   const unsigned char *
   get_buf() const
   {
      return &buf[0];
   }

   long
   get_buf_len() const
   {
      return 64;
   }

   long get_bytes(unsigned char *res, long n, long pos) 
   {
      if (n < 0) LogicError("RandomStream::get: bad args");

      long i, j;

      if (n <= 64-pos) {
	 for (i = 0; i < n; i++) res[i] = buf[pos+i];
	 pos += n;
	 return pos;
      }

      // read remainder of buffer
      for (i = 0; i < 64-pos; i++) res[i] = buf[pos+i];
      n -= 64-pos;
      res += 64-pos;
      pos = 64;

      _ntl_uint32 wdata[16];

      // read 64-byte chunks
      for (i = 0; i <= n-64; i += 64) {
	 salsa20_apply(state, wdata);
	 for (j = 0; j < 16; j++)
	    FROMLE(res + i + 4*j, wdata[j]);
      }

      if (i < n) { 
	 salsa20_apply(state, wdata);

	 for (j = 0; j < 16; j++)
	    FROMLE(buf + 4*j, wdata[j]);

	 pos = n-i;
	 for (j = 0; j < pos; j++)
	    res[i+j] = buf[j];
      }

      return pos;
   }

   void set_nonce(unsigned long nonce)
   {
      _ntl_uint32 nonce0, nonce1;

      nonce0 = nonce;
      nonce0 = INT32MASK(nonce0);

      nonce1 = 0;

#if (NTL_BITS_PER_LONG > 32)
      nonce1 = nonce >> 32;
      nonce1 = INT32MASK(nonce1);
#endif

      state[12] = 0;
      state[13] = 0;
      state[14] = nonce0;
      state[15] = nonce1;
   }
};


#endif



// Boilerplate PIMPL code

RandomStream_impl *
RandomStream_impl_build(const unsigned char *key)
{
   UniquePtr<RandomStream_impl> p;
   p.make(key);
   return p.release();
}

RandomStream_impl *
RandomStream_impl_build(const RandomStream_impl& other)
{
   UniquePtr<RandomStream_impl> p;
   p.make(other);
   return p.release();
}

void
RandomStream_impl_copy(RandomStream_impl& x, const RandomStream_impl& y)
{
   x = y;
}

void
RandomStream_impl_delete(RandomStream_impl* p)
{
   delete p;
}

const unsigned char *
RandomStream_impl_get_buf(const RandomStream_impl& x)
{
   return x.get_buf();
}

long
RandomStream_impl_get_buf_len(const RandomStream_impl& x)
{
   return x.get_buf_len();
}

long
RandomStream_impl_get_bytes(RandomStream_impl& impl, 
   unsigned char *res, long n, long pos)
{
   return impl.get_bytes(res, n, pos);
}


void 
RandomStream_impl_set_nonce(RandomStream_impl& impl, unsigned long nonce)
{
   impl.set_nonce(nonce);
}





NTL_TLS_GLOBAL_DECL(UniquePtr<RandomStream>,  CurrentRandomStream);


void SetSeed(const RandomStream& s)
{
   NTL_TLS_GLOBAL_ACCESS(CurrentRandomStream);

   if (!CurrentRandomStream)
      CurrentRandomStream.make(s);
   else
      *CurrentRandomStream = s;
}


void SetSeed(const unsigned char *data, long dlen)
{
   if (dlen < 0) LogicError("SetSeed: bad args");

   Vec<unsigned char> key;
   key.SetLength(NTL_PRG_KEYLEN);
   DeriveKey(key.elts(), NTL_PRG_KEYLEN, data, dlen);
 
   SetSeed(RandomStream(key.elts()));
}

void SetSeed(const ZZ& seed)
{
   long nb = NumBytes(seed);

   Vec<unsigned char> buf;
   buf.SetLength(nb);

   BytesFromZZ(buf.elts(), seed, nb);

   SetSeed(buf.elts(), nb);
}


static
void InitRandomStream()
{
   const std::string& id = UniqueID();
   SetSeed((const unsigned char *) id.c_str(), id.length());
}

static inline
RandomStream& LocalGetCurrentRandomStream()
{
   NTL_TLS_GLOBAL_ACCESS(CurrentRandomStream);

   if (!CurrentRandomStream) InitRandomStream();
   return *CurrentRandomStream;
}

RandomStream& GetCurrentRandomStream()
{
   return LocalGetCurrentRandomStream();
}







static inline
unsigned long WordFromBytes(const unsigned char *buf, long n)
{
   unsigned long res = 0;
   long i;

   for (i = n-1; i >= 0; i--)
      res = (res << 8) | buf[i];

   return res;
}


unsigned long RandomWord()
{
   RandomStream& stream = LocalGetCurrentRandomStream();
   unsigned char buf[NTL_BITS_PER_LONG/8];

   stream.get(buf, NTL_BITS_PER_LONG/8);
   return WordFromBytes(buf, NTL_BITS_PER_LONG/8);
}


void VectorRandomWord(long k, unsigned long* x)
{
   RandomStream& stream = LocalGetCurrentRandomStream();
   unsigned char buf[NTL_BITS_PER_LONG/8];

   for (long i = 0; i < k; i++) {
      stream.get(buf, NTL_BITS_PER_LONG/8);
      x[i] = WordFromBytes(buf, NTL_BITS_PER_LONG/8);
   }
}

long RandomBits_long(long l)
{
   if (l <= 0) return 0;
   if (l >= NTL_BITS_PER_LONG) 
      ResourceError("RandomBits: length too big");

   RandomStream& stream = LocalGetCurrentRandomStream();
   unsigned char buf[NTL_BITS_PER_LONG/8];
   long nb = (l+7)/8;
   stream.get(buf, nb);

   return long(WordFromBytes(buf, nb) & ((1UL << l)-1UL)); 
}

unsigned long RandomBits_ulong(long l)
{
   if (l <= 0) return 0;
   if (l > NTL_BITS_PER_LONG) 
      ResourceError("RandomBits: length too big");

   RandomStream& stream = LocalGetCurrentRandomStream();
   unsigned char buf[NTL_BITS_PER_LONG/8];
   long nb = (l+7)/8;
   stream.get(buf, nb);
   unsigned long res = WordFromBytes(buf, nb);
   if (l < NTL_BITS_PER_LONG)
      res = res & ((1UL << l)-1UL);
   return res;
}

long RandomLen_long(long l)
{
   if (l <= 0) return 0;
   if (l == 1) return 1;
   if (l >= NTL_BITS_PER_LONG) 
      ResourceError("RandomLen: length too big");

   RandomStream& stream = LocalGetCurrentRandomStream();
   unsigned char buf[NTL_BITS_PER_LONG/8];
   long nb = ((l-1)+7)/8;
   stream.get(buf, nb);
   unsigned long res = WordFromBytes(buf, nb);
   unsigned long mask = (1UL << (l-1)) - 1UL;
   return long((res & mask) | (mask+1UL)); 
}


long RandomBnd(long bnd)
{
   if (bnd <= 1) return 0;

   RandomStream& stream = LocalGetCurrentRandomStream();
   unsigned char buf[NTL_BITS_PER_LONG/8];
   long l = NumBits(bnd-1);
   long nb = (l+7)/8;

   long tmp;
   do {
      stream.get(buf, nb);
      tmp = long(WordFromBytes(buf, nb) & ((1UL << l)-1UL));
   } while (tmp >= bnd);

   return tmp;
}



void RandomBits(ZZ& x, long l)
{
   if (l <= 0) {
      x = 0;
      return;
   }

   if (NTL_OVERFLOW(l, 1, 0))
      ResourceError("RandomBits: length too big");

   RandomStream& stream = LocalGetCurrentRandomStream();

   long nb = (l+7)/8;
   unsigned long mask = (1UL << (8 - nb*8 + l)) - 1UL;

   NTL_TLS_LOCAL(Vec<unsigned char>, buf_mem);
   Vec<unsigned char>::Watcher watch_buf_mem(buf_mem);

   buf_mem.SetLength(nb);
   unsigned char *buf = buf_mem.elts();

   x.SetSize((l + NTL_ZZ_NBITS - 1)/NTL_ZZ_NBITS);
   // pre-allocate to ensure strong ES

   stream.get(buf, nb);
   buf[nb-1] &= mask;
   
   ZZFromBytes(x, buf, nb);
}


void RandomLen(ZZ& x, long l)
{
   if (l <= 0) {
      x = 0;
      return;
   }

   if (l == 1) {
      x = 1;
      return;
   }

   if (NTL_OVERFLOW(l, 1, 0))
      ResourceError("RandomLen: length too big");

   RandomStream& stream = LocalGetCurrentRandomStream();

   long nb = (l+7)/8;
   unsigned long mask = (1UL << (8 - nb*8 + l)) - 1UL;

   NTL_TLS_LOCAL(Vec<unsigned char>, buf_mem);
   Vec<unsigned char>::Watcher watch_buf_mem(buf_mem);

   buf_mem.SetLength(nb);
   unsigned char *buf = buf_mem.elts();

   x.SetSize((l + NTL_ZZ_NBITS - 1)/NTL_ZZ_NBITS);
   // pre-allocate to ensure strong ES

   stream.get(buf, nb);
   buf[nb-1] &= mask;
   buf[nb-1] |= ((mask >> 1) + 1UL);
   
   ZZFromBytes(x, buf, nb);
}





/**********************************************************

The following implementation of RandomBnd is designed
for speed.  It certainly is not resilient against a
timing side-channel attack (but then again, none of these
PRG routines are designed to be).

The naive strategy generates random candidates of the right 
bit length until the candidate < bnd.
The idea in this implementation is to generate the high
order two bytes of the candidate first, and compare this
to the high order two bytes of tmp.  We can discard the
candidate if this is already too large.

***********************************************************/

void RandomBnd(ZZ& x, const ZZ& bnd)
{
   if (bnd <= 1) {
      x = 0;
      return;
   }

   RandomStream& stream = LocalGetCurrentRandomStream();

   long l = NumBits(bnd);
   long nb = (l+7)/8;

   if (nb <= 3) {
      long lbnd = conv<long>(bnd);
      unsigned char lbuf[3];
      long ltmp;
      
      x.SetSize((l + NTL_ZZ_NBITS - 1)/NTL_ZZ_NBITS);
      // pre-allocate to ensure strong ES
      do {
         stream.get(lbuf, nb);
         ltmp = long(WordFromBytes(lbuf, nb) & ((1UL << l)-1UL));
      } while (ltmp >= lbnd);

     conv(x, ltmp);
     return;
   }

   // deal with possible alias
   NTL_ZZRegister(tmp_store);
   const ZZ& bnd_ref = ((&x == &bnd) ? (tmp_store = bnd) : bnd); 


   NTL_ZZRegister(hbnd);
   RightShift(hbnd, bnd_ref, (nb-2)*8);
   long lhbnd = conv<long>(hbnd);

   unsigned long mask = (1UL << (16 - nb*8 + l)) - 1UL;

   NTL_TLS_LOCAL(Vec<unsigned char>, buf_mem);
   Vec<unsigned char>::Watcher watch_buf_mem(buf_mem);
   buf_mem.SetLength(nb);
   unsigned char *buf = buf_mem.elts();

   unsigned char hbuf[2];

   x.SetSize((l + NTL_ZZ_NBITS - 1)/NTL_ZZ_NBITS);
   // pre-allocate to ensure strong ES
   for (;;) {
      stream.get(hbuf, 2);
      long hpart = long(WordFromBytes(hbuf, 2) & mask);

      if (hpart > lhbnd) continue;

      stream.get(buf, nb-2);
      buf[nb-2] = ((unsigned long) hpart);
      buf[nb-1] = ((unsigned long) hpart) >> 8; 

      ZZFromBytes(x, buf, nb);
      if (hpart < lhbnd || x < bnd_ref) break;
   }
}




// More prime generation stuff...

static
double Log2(double x)
{
   static const double log2 = log(2.0); // GLOBAL (relies on C++11 thread-safe init)
   return log(x)/log2;
}

// Define p(k,t) to be the conditional probability that a random, odd, k-bit 
// number is composite, given that it passes t iterations of the 
// Miller-Rabin test.
// If this routine returns a non-zero value, then
//    p(k,t) <= 2^{-n}.
// This basically encodes the estimates of Damgard, Landrock, and Pomerance;
// it uses floating point arithmetic, but is coded in such a way
// that its results should be correct, assuming that the log function
// is computed with reasonable precision.
// 
// It is assumed that k >= 3 and t >= 1; if this does not hold,
// then 0 is returned.

static
long ErrBoundTest(long kk, long tt, long nn)

{
   const double fudge = (1.0 + 1024.0/NTL_FDOUBLE_PRECISION);
   const double log2_3 = Log2(3.0);
   const double log2_7 = Log2(7.0);
   const double log2_20 = Log2(20.0);

   double k = kk;
   double t = tt;
   double n = nn;

   if (k < 3 || t < 1) return 0;
   if (n < 1) return 1;

   // the following test is largely academic
   if (9*t > NTL_FDOUBLE_PRECISION) LogicError("ErrBoundTest: t too big");

   double log2_k = Log2(k);

   if ((n + log2_k)*fudge <= 2*t)
      return 1;

   if ((2*log2_k + 4.0 + n)*fudge <= 2*sqrt(k))
      return 2;

   if ((t == 2 && k >= 88) || (3 <= t && 9*t <= k && k >= 21)) {
      if ((1.5*log2_k + t + 4.0 + n)*fudge <= 0.5*Log2(t) + 2*(sqrt(t*k)))
         return 3;
   }

   if (k <= 9*t && 4*t <= k && k >= 21) {
      if ( ((log2_3 + log2_7 + log2_k + n)*fudge <= log2_20 + 5*t)  &&
           ((log2_3 + (15.0/4.0)*log2_k + n)*fudge <= log2_7 + k/2 + 2*t) &&
           ((2*log2_3 + 2 + log2_k + n)*fudge <= k/4 + 3*t) )
         return 4; 
   }

   if (4*t >= k && k >= 21) {
      if (((15.0/4.0)*log2_k + n)*fudge <= log2_7 + k/2 + 2*t)
         return 5;
   }

   return 0;
}


void GenPrime(ZZ& n, long k, long err)
{
   if (k <= 1) LogicError("GenPrime: bad length");

   if (k > (1L << 20)) ResourceError("GenPrime: length too large");

   if (err < 1) err = 1;
   if (err > 512) err = 512;

   if (k == 2) {
      if (RandomBnd(2))
         n = 3;
      else
         n = 2;

      return;
   }


   long t;

   t = 1;
   while (!ErrBoundTest(k, t, err))
      t++;

   RandomPrime(n, k, t);
}


long GenPrime_long(long k, long err)
{
   if (k <= 1) LogicError("GenPrime: bad length");

   if (k >= NTL_BITS_PER_LONG) ResourceError("GenPrime: length too large");

   if (err < 1) err = 1;
   if (err > 512) err = 512;

   if (k == 2) {
      if (RandomBnd(2))
         return 3;
      else
         return 2;
   }

   long t;

   t = 1;
   while (!ErrBoundTest(k, t, err))
      t++;

   return RandomPrime_long(k, t);
}

void MultiThreadedGenGermainPrime(ZZ& n, long k, long err)
{
   long nt = AvailableThreads();


   long prime_bnd = ComputePrimeBound(k);

   if (NumBits(prime_bnd) >= k/2)
      prime_bnd = (1L << (k/2-1));

   ZZ two;
   two = 2;

   const long LOCAL_ITER_BOUND = 8;
   // since resetting the PRG comes at a certain cost,
   // we perform a few iterations with each reset to
   // amortize the reset cost. 

   unsigned long initial_counter = 0;
   ZZ seed;
   RandomBits(seed, 256);


   ZZ overflow_counter;

   for (;;) {

      AtomicLowWaterMark low_water_mark(-1UL);
      AtomicCounter counter(initial_counter);

      Vec< UniquePtr<ZZ> > result(INIT_SIZE, nt);
      Vec<unsigned long> result_ctr(INIT_SIZE, nt, -1UL);

      NTL_EXEC_INDEX(nt, index)

         RandomStreamPush push;

	 SetSeed(seed);
	 RandomStream& stream = GetCurrentRandomStream();

	 ZZ cand, n1;
         PrimeSeq s;

	 while (low_water_mark == -1UL) {

	    unsigned long local_ctr = counter.inc();
            if (local_ctr >> (NTL_BITS_PER_NONCE-1)) {
               // counter overflow...rather academic
               break;
            }

	    stream.set_nonce(local_ctr);
	    
	    for (long iter = 0; iter < LOCAL_ITER_BOUND && 
				local_ctr <= low_water_mark; iter++) {


	       RandomLen(cand, k);
	       if (!IsOdd(cand)) add(cand, cand, 1);

	       s.reset(3);
	       long p;

	       long sieve_passed = 1;

	       p = s.next();
	       while (p && p < prime_bnd) {
		  long r = rem(cand, p);

		  if (r == 0) {
		     sieve_passed = 0;
		     break;
		  }

		  // test if 2*r + 1 = 0 (mod p)
		  if (r == p-r-1) {
		     sieve_passed = 0;
		     break;
		  }

		  p = s.next();
	       }

               if (!sieve_passed) continue;


               if (MillerWitness(cand, two)) continue;

	       // n1 = 2*cand+1
	       mul(n1, cand, 2);
	       add(n1, n1, 1);


               if (MillerWitness(n1, two)) continue;

	       result[index].make(cand);
	       result_ctr[index] = local_ctr;
	       low_water_mark.UpdateMin(local_ctr);
	       break;
            }
         }

      NTL_EXEC_INDEX_END

      // find index of low_water_mark

      unsigned long low_water_mark1 = low_water_mark;
      long low_water_index = -1;

      for (long index = 0; index < nt; index++) {
	 if (result_ctr[index] == low_water_mark1) {
	    low_water_index = index;
	    break;
	 }
      }

      if (low_water_index == -1) {
         // counter overflow...rather academic
         overflow_counter++;
         initial_counter = 0;
         RandomBits(seed, 256);
         continue;
      }

      ZZ N;
      N = *result[low_water_index];

      ZZ iter = ((overflow_counter << (NTL_BITS_PER_NONCE-1)) +
                 conv<ZZ>(low_water_mark1) + 1)*LOCAL_ITER_BOUND;

      // now do t M-R iterations...just to make sure
 
      // First compute the appropriate number of M-R iterations, t
      // The following computes t such that 
      //       p(k,t)*8/k <= 2^{-err}/(5*iter^{1.25})
      // which suffices to get an overall error probability of 2^{-err}.
      // Note that this method has the advantage of not requiring 
      // any assumptions on the density of Germain primes.

      long err1 = max(1, err + 7 + (5*NumBits(iter) + 3)/4 - NumBits(k));
      long t;
      t = 1;
      while (!ErrBoundTest(k, t, err1))
         t++;

      Vec<ZZ> W(INIT_SIZE, t);

      for (long i = 0; i < t; i++) {
         do { 
            RandomBnd(W[i], N);
         } while (W[i] == 0);
      }

      AtomicBool tests_pass(true);

      NTL_EXEC_RANGE(t, first, last)

         for (long i = first; i < last && tests_pass; i++) {
            if (MillerWitness(N, W[i])) tests_pass = false;
         }

      NTL_EXEC_RANGE_END

      if (tests_pass) {
         n = N;
         return;
      }

      // very unlikey to get here
      initial_counter = low_water_mark1 + 1;
   }
}

void GenGermainPrime(ZZ& n, long k, long err)
{
   if (k <= 1) LogicError("GenGermainPrime: bad length");

   if (k > (1L << 20)) ResourceError("GenGermainPrime: length too large");

   if (err < 1) err = 1;
   if (err > 512) err = 512;

   if (k == 2) {
      if (RandomBnd(2))
         n = 3;
      else
         n = 2;

      return;
   }

   if (k >= 192) {
      MultiThreadedGenGermainPrime(n, k, err);
      return;
   }


   long prime_bnd = ComputePrimeBound(k);

   if (NumBits(prime_bnd) >= k/2)
      prime_bnd = (1L << (k/2-1));


   ZZ two;
   two = 2;

   ZZ n1;

   
   PrimeSeq s;

   ZZ iter;
   iter = 0;


   for (;;) {
      iter++;

      RandomLen(n, k);
      if (!IsOdd(n)) add(n, n, 1);

      s.reset(3);
      long p;

      long sieve_passed = 1;

      p = s.next();
      while (p && p < prime_bnd) {
         long r = rem(n, p);

         if (r == 0) {
            sieve_passed = 0;
            break;
         }

         // test if 2*r + 1 = 0 (mod p)
         if (r == p-r-1) {
            sieve_passed = 0;
            break;
         }

         p = s.next();
      }

      if (!sieve_passed) continue;


      if (MillerWitness(n, two)) continue;

      // n1 = 2*n+1
      mul(n1, n, 2);
      add(n1, n1, 1);


      if (MillerWitness(n1, two)) continue;

      // now do t M-R iterations...just to make sure
 
      // First compute the appropriate number of M-R iterations, t
      // The following computes t such that 
      //       p(k,t)*8/k <= 2^{-err}/(5*iter^{1.25})
      // which suffices to get an overall error probability of 2^{-err}.
      // Note that this method has the advantage of not requiring 
      // any assumptions on the density of Germain primes.

      long err1 = max(1, err + 7 + (5*NumBits(iter) + 3)/4 - NumBits(k));
      long t;
      t = 1;
      while (!ErrBoundTest(k, t, err1))
         t++;

      ZZ W;
      long MR_passed = 1;

      long i;
      for (i = 1; i <= t; i++) {
         do {
            RandomBnd(W, n);
         } while (W == 0);
         // W == 0 is not a useful candidate witness!

         if (MillerWitness(n, W)) {
            MR_passed = 0;
            break;
         }
      }

      if (MR_passed) break;
   }
}

void OldGenGermainPrime(ZZ& n, long k, long err)
{
   if (k <= 1) LogicError("GenGermainPrime: bad length");

   if (k > (1L << 20)) ResourceError("GenGermainPrime: length too large");

   if (err < 1) err = 1;
   if (err > 512) err = 512;

   if (k == 2) {
      if (RandomBnd(2))
         n = 3;
      else
         n = 2;

      return;
   }


   long prime_bnd = ComputePrimeBound(k);

   if (NumBits(prime_bnd) >= k/2)
      prime_bnd = (1L << (k/2-1));


   ZZ two;
   two = 2;

   ZZ n1;

   
   PrimeSeq s;

   ZZ iter;
   iter = 0;


   for (;;) {
      iter++;

      RandomLen(n, k);
      if (!IsOdd(n)) add(n, n, 1);

      s.reset(3);
      long p;

      long sieve_passed = 1;

      p = s.next();
      while (p && p < prime_bnd) {
         long r = rem(n, p);

         if (r == 0) {
            sieve_passed = 0;
            break;
         }

         // test if 2*r + 1 = 0 (mod p)
         if (r == p-r-1) {
            sieve_passed = 0;
            break;
         }

         p = s.next();
      }

      if (!sieve_passed) continue;


      if (MillerWitness(n, two)) continue;

      // n1 = 2*n+1
      mul(n1, n, 2);
      add(n1, n1, 1);


      if (MillerWitness(n1, two)) continue;

      // now do t M-R iterations...just to make sure
 
      // First compute the appropriate number of M-R iterations, t
      // The following computes t such that 
      //       p(k,t)*8/k <= 2^{-err}/(5*iter^{1.25})
      // which suffices to get an overall error probability of 2^{-err}.
      // Note that this method has the advantage of not requiring 
      // any assumptions on the density of Germain primes.

      long err1 = max(1, err + 7 + (5*NumBits(iter) + 3)/4 - NumBits(k));
      long t;
      t = 1;
      while (!ErrBoundTest(k, t, err1))
         t++;

      ZZ W;
      long MR_passed = 1;

      long i;
      for (i = 1; i <= t; i++) {
         do {
            RandomBnd(W, n);
         } while (W == 0);
         // W == 0 is not a useful candidate witness!

         if (MillerWitness(n, W)) {
            MR_passed = 0;
            break;
         }
      }

      if (MR_passed) break;
   }
}

long GenGermainPrime_long(long k, long err)
{
   if (k >= NTL_BITS_PER_LONG-1)
      ResourceError("GenGermainPrime_long: length too long");

   ZZ n;
   GenGermainPrime(n, k, err);
   return to_long(n);
}


NTL_END_IMPL

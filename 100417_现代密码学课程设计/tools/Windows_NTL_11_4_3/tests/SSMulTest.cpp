#include <NTL/ZZX.h>

NTL_CLIENT


#define TIME_IT(t, action) \
do { \
   double _t0, _t1; \
   long _iter = 1; \
   long _cnt = 0; \
   do { \
      _t0 = GetTime(); \
      for (long _i = 0; _i < _iter; _i++) { action; _cnt++; } \
      _t1 = GetTime(); \
   } while ( _t1 - _t0 < 2 && (_iter *= 2)); \
   t = (_t1 - _t0)/_iter; \
} while(0)

void FillRandom(ZZX& f, long n, long k)
{
   long sw = RandomBnd(2);
   f.SetLength(n);
   for (long i = 0; i < n; i++) {
      if (sw) {
         long kk = 1 + RandomBnd(k);
         RandomBits(f[i], kk);
      }
      else {
        long kk = RandomBnd(k);
        SetBit(f[i], kk);
      }
      if (RandomBnd(2)) NTL::negate(f[i], f[i]);
   }
   f.normalize();
}

int main()
{

   for (long iter = 0; iter < 4000; iter++) {
     if (iter % 100 == 0) cerr << ".";
     long na, nb, k;

     long sw = RandomBnd(3);

     if (sw == 0) {
        na = RandomBnd(20) + 1;
        nb = RandomBnd(20) + 1;
        k = RandomBnd(20) + 1;
     }
     else if (sw == 1) {
        na = RandomBnd(200) + 10;
        nb = RandomBnd(200) + 10;
        k = RandomBnd(200) + 10;
     }
     else {
        na = RandomBnd(3000) + 100;
        nb = RandomBnd(3000) + 100;
        k = RandomBnd(3000) + 100;
     }

     ZZX a, b, c, c1;
     FillRandom(a, na, k);
     FillRandom(b, nb, k);
    
     if (RandomBnd(2)) {
        SSMul(c, a, b);
        KarMul(c1, a, b);
        if (c != c1) Error("oops");
     }
     else {
        SSSqr(c, a);
        KarSqr(c1, a);
        if (c != c1) Error("oops");
     }
   }

   cerr << "\n";
}




#include <NTL/LLL.h>

NTL_CLIENT

long SubsetSumSolution(const vec_ZZ& z)
{
   long n = z.length()-3;
   long j;

   if (z(n+1) != 0) return 0;
   if (z(n+2) != -1 && z(n+2) != 1) return 0;
   for (j = 1; j <= n; j++) 
      if (z(j) != -1 && z(j) != 1) return 0;

   return 1;
}



int main()
{
   RR::SetPrecision(150);
   long n, b, size;

   cerr << "n: ";
   cin >> n;

   cerr << "b: ";
   cin >> b;

   cerr << "size: ";
   cin >> size;

   cerr << "prune: ";
   long prune;
   cin >> prune;

   ZZ seed;
   cerr << "seed: ";
   cin >> seed;

   if (seed != 0)
      SetSeed(seed);

   char alg;
   cerr << "alg [fqQxr]: ";
   cin >> alg;

   double TotalTime = 0;
   long TotalSucc = 0;

   long iter;

   for (iter = 1; iter <= 20; iter++) {
      vec_ZZ a;
      a.SetLength(n);
   
      ZZ bound;
   
      LeftShift(bound, to_ZZ(1), b);
   
      long i;
      for (i = 1; i <= n; i++) {
         RandomBnd(a(i), bound);
         a(i) += 1;
      }
   
      ZZ S;
   
      do {
         RandomLen(S, n+1);
      } while (weight(S) != n/2+1);
   
      ZZ s;
      clear(s);
      for (i = 1; i <= n; i++)
         if (bit(S, i-1))
            s += a(i);
   
      mat_ZZ B(INIT_SIZE, n+1, n+3);
   
      for (i = 1; i <= n; i++) {
         B(i, i) = 2;
         B(i, n+1) = a(i) * n;
         B(i, n+3) = n;
      }
   
      for (i = 1; i <= n; i++)
         B(n+1, i) = 1;
   
      B(n+1, n+1) = s * n;
      B(n+1, n+2) = 1;
      B(n+1, n+3) = n;
      B(n+1, n+3) *= n/2;
   
      swap(B(1), B(n+1)); 
   
      for (i = 2; i <= n; i++) {
         long j = RandomBnd(n-i+2) + i;
         swap(B(i), B(j));
      }
   
      double t;

      LLLStatusInterval = 10;
   
      t = GetTime();
      switch (alg) {
      case 'f':
         BKZ_FP(B, 0.99, size, prune, SubsetSumSolution);
         break;
      case 'q':
         BKZ_QP(B, 0.99, size, prune, SubsetSumSolution);
         break;
      case 'Q':
         BKZ_QP1(B, 0.99, size, prune, SubsetSumSolution);
         break;
      case 'x':
         BKZ_XD(B, 0.99, size, prune, SubsetSumSolution);
         break;
      case 'r':
         BKZ_RR(B, 0.99, size, prune, SubsetSumSolution);
         break;
      default:
         TerminalError("invalid algorithm");
      }


      t = GetTime()-t;
   
      long succ = 0;
      for (i = 1; i <= n+1; i++)
         if (SubsetSumSolution(B(i)))
            succ = 1;

      TotalTime += t;
      TotalSucc += succ;

      if (succ)
         cerr << "+";
      else
         cerr << "-";
   }

   cerr << "\n";

   cerr << "number of success: " << TotalSucc << "\n";
   cerr << "average time: " << TotalTime/20 << "\n";

   return 0;
}
      



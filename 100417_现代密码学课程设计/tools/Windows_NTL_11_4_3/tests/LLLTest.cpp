
#include <NTL/LLL.h>

NTL_CLIENT

int main()
{
   mat_ZZ B;
   long s;

#if 1
   cin >> B;
#else
   long i, j;
   long n;
   cerr << "n: ";
   cin >> n;

   long m;
   cerr << "m: ";
   cin >> m;

   long k;
   cerr << "k: ";
   cin >> k;

   B.SetDims(n, m);
   for (i = 1; i <= n; i++)
      for (j = 1; j <= m; j++) {
         RandomLen(B(i,j), k);
         if (RandomBnd(2)) negate(B(i,j), B(i,j));
      }


#endif

   mat_ZZ U, B0, B1, B2;

   B0 = B;

   double t;
   ZZ d;

   B = B0;
   cerr << "LLL_FP...";
   t = GetTime();
   s = LLL_FP(B, U, 0.99);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");
   LLL(d, B, 90, 100);
   if (B1 != B) TerminalError("bad LLLTest (2)");

   B = B0;
   cerr << "LLL_QP...";
   t = GetTime();
   s = LLL_QP(B, U, 0.99);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");
   LLL(d, B, 90, 100);
   if (B1 != B) TerminalError("bad LLLTest (2)");


   B = B0;
   cerr << "LLL_XD...";
   t = GetTime();
   s = LLL_XD(B, U, 0.99);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");
   LLL(d, B, 90, 100);
   if (B1 != B) TerminalError("bad LLLTest (2)");

   B = B0;
   cerr << "LLL_RR...";
   t = GetTime();
   s = LLL_RR(B, U, 0.99);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");
   LLL(d, B, 90, 100);
   if (B1 != B) TerminalError("bad LLLTest (2)");

   B = B0;
   cerr << "G_LLL_FP...";
   t = GetTime();
   s = G_LLL_FP(B, U, 0.99);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");
   LLL(d, B, 90, 100);
   if (B1 != B) TerminalError("bad LLLTest (2)");

   B = B0;
   cerr << "G_LLL_QP...";
   t = GetTime();
   s = G_LLL_QP(B, U, 0.99);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");
   LLL(d, B, 90, 100);
   if (B1 != B) TerminalError("bad LLLTest (2)");

   B = B0;
   cerr << "G_LLL_XD...";
   t = GetTime();
   s = G_LLL_XD(B, U, 0.99);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");
   LLL(d, B, 90, 100);
   if (B1 != B) TerminalError("bad LLLTest (2)");

   B = B0;
   cerr << "G_LLL_RR...";
   t = GetTime();
   s = G_LLL_RR(B, U, 0.99);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");
   LLL(d, B, 90, 100);
   if (B1 != B) TerminalError("bad LLLTest (2)");


   B = B0;
   cerr << "LLL...";
   t = GetTime();
   s = LLL(d, B, U);
   cerr << (GetTime()-t) << "\n";
   mul(B1, U, B0);
   if (B1 != B) TerminalError("bad LLLTest (1)");

   cout << "rank = " << s << "\n";
   cout << "det = " << d << "\n";
   cout << "B = " << B << "\n";
   cout << "U = " << U << "\n";
}


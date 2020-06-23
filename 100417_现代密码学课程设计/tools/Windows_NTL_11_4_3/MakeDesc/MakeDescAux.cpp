
#include <cstdlib>
using namespace std;

int val_int(int x) { return x; }
unsigned int val_uint(unsigned int x) { return x; }
 
long val_long(long x) { return x; }
unsigned long val_ulong(unsigned long x) { return x; }
 
size_t val_size_t(size_t x) { return x; }

double val_double(double x) { return x; }
long double val_ldouble(double x) { return x; }
 
void touch_int(int* x) {}
void touch_uint(unsigned int* x) {}
 
void touch_long(long* x) {}
void touch_ulong(unsigned long* x) {}

void touch_size_t(size_t* x) {}
 
void touch_double(double* x) {}
void touch_ldouble(long double* x) {}

double sum_double(double *x, long n)
{
   long i;
   double acc = 0;

   for (i = 0; i < n; i++)
      acc += x[i];

   return acc;
}

double fma_test(double a, double b, double c)
{
   double t1 = a*b;
   double t2 = t1 + c;
   return t2;
}

double reassoc_test(double a, double b, double c, double d)
{
   double t1 = a*c + a*d;
   double t2 = b*c + b*d;
   return t1 + t2;
   // an optimizing compiler that reassociates will almost
   // surely compute this as (a+b)*(c+d).
}

double power2(long k)
{
   long i;
   double res;

   res = 1;

   for (i = 1; i <= k; i++)
      res = res * 2;

   return res;
}

#include <sys/time.h>
#include <sys/resource.h>

// some (old?) Solaris systems don't seem
// to supply a getrusage prototype

extern "C" int getrusage(int, struct rusage*);


double _ntl_GetTime()
{
   struct rusage used;

   getrusage(RUSAGE_SELF, &used);
   return (used.ru_utime.tv_sec + used.ru_stime.tv_sec +
      (used.ru_utime.tv_usec + used.ru_stime.tv_usec) / 1e6);
}


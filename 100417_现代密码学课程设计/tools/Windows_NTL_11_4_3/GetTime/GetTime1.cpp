#include <sys/time.h>
#include <sys/resource.h>

// FIXME: it would be nice to have a per-thread
// timing function, but it seems very difficult
// to get a cross-platform solution to this.


double _ntl_GetTime()
{
   struct rusage used;

   getrusage(RUSAGE_SELF, &used);
   return (used.ru_utime.tv_sec + used.ru_stime.tv_sec +
      (used.ru_utime.tv_usec + used.ru_stime.tv_usec) / 1e6);
}


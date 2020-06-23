#include <sys/time.h>
#include <sys/resource.h>
#include <syscall.h>



double _ntl_GetTime()
{
   struct rusage used;

   syscall(SYS_getrusage, RUSAGE_SELF, &used);
   return (used.ru_utime.tv_sec + used.ru_stime.tv_sec +
      (used.ru_utime.tv_usec + used.ru_stime.tv_usec) / 1e6);
}


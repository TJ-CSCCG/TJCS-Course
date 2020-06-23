
#include <sys/types.h>
#include <unistd.h>

unsigned long _ntl_GetPID()
{
   return getpid();
}


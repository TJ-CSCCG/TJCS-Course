#include <ctime>

using namespace std;

// FIXME: this is the GetTime that ends up getting used
// on Windows. However, it returns the wall time, not CPU time.
// We could perhaps switch to using GetProcessTimes.
// See: http://nadeausoftware.com/articles/2012/03/c_c_tip_how_measure_cpu_time_benchmarking

// NOTE: on Windows, CLOCKS_PER_SEC is 1000 and clock_t is a 32 bit integer,
// this can go for almost 600 hours without overflowing.

// NOTE: on Windows, clock does not necessarily wrap-around,
// but instead may return -1 after an overflow.

double _ntl_GetTime()
{
   clock_t this_clock = clock();
   return double(this_clock)/double(CLOCKS_PER_SEC);
}


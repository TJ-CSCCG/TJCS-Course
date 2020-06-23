
#include <NTL/thread.h>

#ifdef NTL_THREADS

#include <thread>
#include <sstream>

#endif



NTL_START_IMPL


const std::string& CurrentThreadID()
{
   NTL_TLS_LOCAL(std::string, ID);
   static NTL_CHEAP_THREAD_LOCAL bool initialized = false;

   if (!initialized) {
#ifdef NTL_THREADS
      std::stringstream ss;
      ss << std::this_thread::get_id();
      ID = ss.str();
#else
      ID = "0";
#endif
      initialized = true;
   }

   return ID;
}



NTL_END_IMPL

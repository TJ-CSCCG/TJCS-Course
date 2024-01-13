
#include <NTL/BasicThreadPool.h>

// make a global symbol, just to supress warnings
int _ntl_BasicThreadPool_dummy_symbol = 0;

#ifdef NTL_THREAD_BOOST

NTL_START_IMPL


NTL_TLS_GLOBAL_DECL(UniquePtr<BasicThreadPool>, NTLThreadPool_stg)

NTL_CHEAP_THREAD_LOCAL BasicThreadPool *NTLThreadPool_ptr = 0;

void ResetThreadPool(BasicThreadPool *pool)
{
   NTL_TLS_GLOBAL_ACCESS(NTLThreadPool_stg);
   NTLThreadPool_stg.reset(pool);
   NTLThreadPool_ptr = pool;
}

BasicThreadPool *ReleaseThreadPool()
{
   NTL_TLS_GLOBAL_ACCESS(NTLThreadPool_stg);
   BasicThreadPool *pool = NTLThreadPool_stg.release();
   NTLThreadPool_ptr = 0;
   return pool;
}



NTL_END_IMPL

#endif


#ifndef NTL_BasicThreadPool__H
#define NTL_BasicThreadPool__H

#include <NTL/tools.h>
#include <NTL/vector.h>
#include <NTL/SmartPtr.h>
#include <NTL/thread.h>


NTL_OPEN_NNS


inline long AvailableThreads();

struct PartitionInfo {
   long nintervals;  // number of intervals
   long intervalsz;  // interval size
   long nsintervals; // number of small intervals

   explicit
   PartitionInfo(long sz, long nt = AvailableThreads()) 
   // partitions [0..sz) into nintervals intervals,
   // so that there are nsintervals of size intervalsz-1
   // and nintervals-nsintervals of size intervalsz
   {
      if (sz <= 0) {
         nintervals = intervalsz = nsintervals = 0;
         return;
      }

      if (nt <= 0) LogicError("PartitionInfo: bad args");

      // NOTE: this overflow check probably unnecessary
      if (NTL_OVERFLOW(sz, 1, 0) || NTL_OVERFLOW(nt, 1, 0))
         ResourceError("PartitionInfo: arg too big");

      if (sz < nt) {
         nintervals = sz;
         intervalsz = 1;
         nsintervals = 0;
         return;
      }

      nintervals = nt;

      long q, r;
      q = sz/nt;
      r = sz - nt*q;

      if (r == 0) {
         intervalsz = q;
         nsintervals = 0;
      }
      else {
         intervalsz = q+1;
         nsintervals = nt - r;
      }
   }

   long NumIntervals() const { return nintervals; }

   void interval(long& first, long& last, long i) const
   // [first..last) is the ith interval -- no range checking is done
   {

#if 0
      // this is the logic, naturally expressed
      if (i < nsintervals) {
         first = i*(intervalsz-1);
         last = first + (intervalsz-1);
      }
      else {
         first = nsintervals*(intervalsz-1) + (i-nsintervals)*intervalsz;
         last = first + intervalsz;
      }
#else
      // this is the same logic, but branch-free (and portable)
      // ...probably unnecessary optimization
      
      long mask = -long(cast_unsigned(i-nsintervals) >> (NTL_BITS_PER_LONG-1));
      // mask == -1 if i < nsintervals, 0 o/w
 
      long lfirst = i*(intervalsz-1);
      lfirst += long((~cast_unsigned(mask)) & cast_unsigned(i-nsintervals));
      // lfirst += max(0, i-nsintervals)

      long llast = lfirst + intervalsz + mask;

      first = lfirst;
      last = llast;
#endif
   }

};



NTL_CLOSE_NNS



#ifdef NTL_THREADS


#include <thread>
#include <condition_variable>
#include <exception>


NTL_OPEN_NNS

/*************************************************************

Some simple thread pooling.

You create a thread pool by constructing a BasicThreadPool object.
For example:

   long nthreads = 4;
   BasicThreadPool pool(nthreads);

creates a thread pool of 4 threads.  These threads will exist
until the destructor for pool is called.  

The simplest way to use a thread pools is as follows.
Suppose you have a task that consists of N subtasks,
indexed 0..N-1.  Then you can write:


   pool.exec_range(N, 
      [&](long first, long last) {
         for (long i = first; i < last; i++) {
            ... code to process subtask i ...
         }
      }
   );

The second argument to exec1 is a C++11 "lambda".
The "[&]" indicates that all local variables in the calling
context are captured by reference, so the lambda body can 
reference all visible local variables directly.

A lower-level interface is also provided.
One can write:

   pool.exec_index(n,
      [&](long index) {
         ... code to process index i ...
      }
   );

This will activate n threads with indices 0..n-1, and execute
the given code on each index.  The parameter n must be
in the range 1..nthreads, otherwise an error is raised.

This lower-level interface is useful in some cases,
especially when memory is managed in some special way.
For convenience, a method is provided to break
subtasks up into smaller, almost-equal-sized groups
of subtasks:

   Vec<long> pvec;
   long n = pool.SplitProblems(N, pvec);

can be used for this.  N is the number of subtasks, indexed 0..N-1.
This method will compute n as needed by exec, and 
the range of subtasks to be processed by a given index in the range
0..n-1 is pvec[index]..pvec[index+1]-1
Thus, the logic of the above exec1 example can be written
using the lower-level exec interface as follows:

   
   Vec<long> pvec;
   long n = pool.SplitProblems(N, pvec);
   pool.exec_index(n,
      [&](long index) {
         long first = pvec[index];
         long last = pvec[index+1];
         for (long i = first; i < last; i++) {
            ... code to process subtask i ...
         }
      }
   );

However, with this approach, memory or other resources can be
assigned to each index = 0..n-1, and managed externally. 




*************************************************************/


class BasicThreadPool {
friend struct RecursiveThreadPool;

private:

// lots of nested stuff

   template<class T>
   class SimpleSignal {
   private:
     T val; 
     std::mutex m;
     std::condition_variable cv;
   
     SimpleSignal(const SimpleSignal&); // disabled
     void operator=(const SimpleSignal&); // disabled
   
   public:
     SimpleSignal() : val(0) { }
   
     T wait() 
     {
       std::unique_lock<std::mutex> lock(m);
       cv.wait(lock, [&]() { return val; } );
       T old_val = val;
       val = 0;
       return old_val;
     }
   
     void send(T new_val)
     {
       std::lock_guard<std::mutex> lock(m);
       val = new_val;
       cv.notify_one();
     }
   };
   
   
   template<class T, class T1>
   class CompositeSignal {
   private:
     T val; 
     T1 val1;
     std::mutex m;
     std::condition_variable cv;
   
     CompositeSignal(const CompositeSignal&); // disabled
     void operator=(const CompositeSignal&); // disabled
   
   public:
     CompositeSignal() : val(0) { }
   
     T wait(T1& _val1) 
     {
       std::unique_lock<std::mutex> lock(m);
       cv.wait(lock, [&]() { return val; } );
       T _val = val;
       _val1 = val1;
       val = 0;
       return _val;
     }
   
     void send(T _val, T1 _val1)
     {
       std::lock_guard<std::mutex> lock(m);
       val = _val;
       val1 = _val1;
       cv.notify_one();
     }
   };
   
   
   
   class ConcurrentTask {
     BasicThreadPool *pool;
   public:
     ConcurrentTask(BasicThreadPool *_pool) : pool(_pool) { }
     BasicThreadPool *getBasicThreadPool() const { return pool; }
   
     virtual void run(long index) = 0;
   };
   
   
   
   // dummy class, used for signalling termination
   class ConcurrentTaskTerminate : public ConcurrentTask {
   public:
     ConcurrentTaskTerminate() : ConcurrentTask(0) { }
     void run(long index) { }
   };
   
   
   
   template<class Fct>
   class ConcurrentTaskFct : public ConcurrentTask {
   public:
     const Fct& fct;
   
     ConcurrentTaskFct(BasicThreadPool *_pool, const Fct& _fct) : 
       ConcurrentTask(_pool), fct(_fct) { }
   
     void run(long index) { fct(index); }
   };
   
   template<class Fct>
   class ConcurrentTaskFct1 : public ConcurrentTask {
   public:
      const Fct& fct;
      const PartitionInfo& pinfo;
 
      ConcurrentTaskFct1(BasicThreadPool *_pool, const Fct& _fct, 
         const PartitionInfo& _pinfo) : 
         ConcurrentTask(_pool), fct(_fct), pinfo(_pinfo)  { }
    
      void run(long index) 
      { 
         long first, last;
         pinfo.interval(first, last, index);
         fct(first, last); 
      }
   };
   
   
   
   struct AutomaticThread {
      CompositeSignal< ConcurrentTask *, long > localSignal;
      ConcurrentTaskTerminate term;
      std::thread t;
   
   
      AutomaticThread() : t(worker, &localSignal) 
      { 
         // cerr << "starting thread " << t.get_id() << "\n";
      }
   
      ~AutomaticThread()
      {
        // cerr << "stopping thread " << t.get_id() << "...";
        localSignal.send(&term, -1);
        t.join();
        // cerr << "\n";
      }
   };



// BasicThreadPool data members

  long nthreads;

  bool active_flag;

  std::atomic<long> counter;
  SimpleSignal<bool> globalSignal;

  Vec< UniquePtr<AutomaticThread> > threadVec;

  std::exception_ptr eptr;
  std::mutex eptr_guard;

// BasicThreadPool private member functions

  BasicThreadPool(const BasicThreadPool&); // disabled
  void operator=(const BasicThreadPool&); // disabled

  void launch(ConcurrentTask *task, long index)
  {
    threadVec[index-1]->localSignal.send(task, index);
    // we use threadVec[index-1] to allow for the fact
    // that we want the current thread to have index 0
  }

  void begin(long cnt)
  {

    active_flag = true;
    counter = cnt;
  }

  void end()
  {
    globalSignal.wait();

    active_flag = false;

    if (eptr) {
      std::exception_ptr eptr1 = eptr;
      eptr = nullptr;
      std::rethrow_exception(eptr1);
    }
  }

  static void runOneTask(ConcurrentTask *task, long index)
  {
    BasicThreadPool *pool = task->getBasicThreadPool();
  
    try {
       task->run(index);
    }
    catch (...) {
       std::lock_guard<std::mutex> lock(pool->eptr_guard);
       if (!pool->eptr) pool->eptr = std::current_exception();
    }

    if (--(pool->counter) == 0) pool->globalSignal.send(true);
  }

   static void worker(CompositeSignal< ConcurrentTask *, long > *localSignal)
   {
     for (;;) {
       long index = -1;
       ConcurrentTask *task = localSignal->wait(index);
       if (index == -1) return; 
   
       runOneTask(task, index);
     }
   }


public:

  long NumThreads() const { return nthreads; }
  bool active() const { return active_flag; }

  explicit
  BasicThreadPool(long _nthreads) : 
    nthreads(_nthreads), active_flag(false), counter(0)
  {
    if (nthreads <= 0) LogicError("BasicThreadPool::BasicThreadPool: bad args");
    if (nthreads == 1) return;

    if (NTL_OVERFLOW(nthreads, 1, 0)) 
      ResourceError("BasicThreadPool::BasicThreadPool: arg too big");


    threadVec.SetLength(nthreads-1);

    for (long i = 0; i < nthreads-1; i++) {
      threadVec[i].make();
    }
  }

  ~BasicThreadPool() 
  {
    if (active()) TerminalError("BasicThreadPool: destructor called while active");
  }
   

  // adding, deleting, moving threads

  void add(long n = 1)
  {
    if (active()) LogicError("BasicThreadPool: illegal operation while active");
    if (n <= 0) LogicError("BasicThreadPool::add: bad args");
    if (NTL_OVERFLOW(n, 1, 0)) 
      ResourceError("BasicThreadPool::add: arg too big");

    Vec< UniquePtr<AutomaticThread> > newThreads;

    newThreads.SetLength(n);
    for (long i = 0; i < n; i++)
      newThreads[i].make();

    threadVec.SetLength(n + nthreads - 1);
    for (long i = 0; i < n; i++)
      threadVec[nthreads-1+i].move(newThreads[i]); 

    nthreads += n;
  }


  void remove(long n = 1)
  {
    if (active()) LogicError("BasicThreadPool: illegal operation while active");
    if (n <= 0 || n >= nthreads) LogicError("BasicThreadPool::remove: bad args");

    for (long i = nthreads-1-n; i < nthreads-1; i++)
      threadVec[i] = 0;

    threadVec.SetLength(nthreads-1-n);
    nthreads -= n;
  }

  
  void move(BasicThreadPool& other, long n = 1) 
  {
    if (active() || other.active()) 
      LogicError("BasicThreadPool: illegal operation while active");
    if (n <= 0 || n >= other.nthreads) LogicError("BasicThreadPool::move: bad args");

    if (this == &other) return;

    threadVec.SetLength(n + nthreads - 1);
    for (long i = 0; i < n; i++)
       threadVec[nthreads-1+i].move(other.threadVec[other.nthreads-1-n+i]);

    other.threadVec.SetLength(other.nthreads-1-n);
    other.nthreads -= n;

    nthreads += n;
  }



  // High level interfaces, intended to be used with lambdas

  // In this version, fct takes one argument, which is
  // an index in [0..cnt)

  template<class Fct>
  void exec_index(long cnt, const Fct& fct) 
  {
    if (active()) LogicError("BasicThreadPool: illegal operation while active");
    if (cnt <= 0) return;
    if (cnt > nthreads) LogicError("BasicThreadPool::exec_index: bad args");

    ConcurrentTaskFct<Fct> task(this, fct);

    begin(cnt);
    for (long t = 1; t < cnt; t++) launch(&task, t);
    runOneTask(&task, 0);
    end();
  }

  template<class Fct>
  static void relaxed_exec_index(BasicThreadPool *pool, long cnt, const Fct& fct) 
  {
    if (cnt > 0) {
      if (cnt == 1) {
	fct(0);
      }
      else if (pool && !pool->active()) {
	pool->exec_index(cnt, fct);
      }
      else {
	LogicError("relaxed_exec_index: not enough threads");
      }
    }
  }

  // even higher level version: sz is the number of subproblems,
  // and fct takes two args, first and last, so that subproblems
  // [first..last) are processed.

  template<class Fct>
  void exec_range(long sz, const Fct& fct) 
  {
    if (active()) LogicError("BasicThreadPool: illegal operation while active");
    if (sz <= 0) return;

    PartitionInfo pinfo(sz, nthreads);

    long cnt = pinfo.NumIntervals();
    ConcurrentTaskFct1<Fct> task(this, fct, pinfo);

    begin(cnt);
    for (long t = 1; t < cnt; t++) launch(&task, t);
    runOneTask(&task, 0);
    end();
  }

  template<class Fct>
  static void relaxed_exec_range(BasicThreadPool *pool, long sz, const Fct& fct) 
  {
    if (sz <= 0) return;
    if (!pool || pool->active() || sz == 1) {
      fct(0, sz);
    }
    else {
      pool->exec_range(sz, fct);
    }
  }

};


// NOTE: BasicThreadPool's are non-relocatable

struct RecursiveThreadPool : BasicThreadPool {
   BasicThreadPool *base_pool;
   long lo, hi; // range of indices is [lo..hi)

   RecursiveThreadPool(BasicThreadPool* _base_pool, long _lo, long _hi) :
      BasicThreadPool(1), base_pool(_base_pool), lo(_lo), hi(_hi) 
   {
      if (lo == 0 && hi == base_pool->nthreads)
         base_pool->active_flag = true;
   }

   ~RecursiveThreadPool() 
   {
      if (lo == 0 && hi == base_pool->nthreads)
         base_pool->active_flag = false;
   }
         

   template<class Fct0, class Fct1>
   void exec_pair(long mid, const Fct0& fct0, const Fct1& fct1) 
   {
     ConcurrentTaskFct<Fct0> task0(this, fct0);
     ConcurrentTaskFct<Fct1> task1(this, fct1);

     begin(2);
     base_pool->launch(&task1, mid);
     runOneTask(&task0, lo);
     end();
   }
};

// NOTE: RecursiveThreadPool's are non-relocatable

inline
SmartPtr<RecursiveThreadPool> StartRecursion(BasicThreadPool *base_pool)
{
   if (!base_pool || base_pool->active()) return 0;
   long nthreads = base_pool->NumThreads();
   if (nthreads <= 1) return 0;
   return MakeSmart<RecursiveThreadPool>(base_pool, 0, nthreads);
}

// NOTE: returning some kind of smart pointer ensures that
// the object itself will stay alive until the end of the
// largest enclosing expression, and then be destroyed.
// I could have also used a UniquePtr, and relied on the move
// constructor to be called.  However, NTL still has a DISABLE_MOVE
// option that would break that.  I could also have used 
// std::unique_ptr; however, I'm generally avoiding those parts
// of the standard library. A SmartPtr has some additional
// overhead, but this will only be called once at the outermost
// recursion, so it should be OK.




struct RecursiveThreadPoolHelper {
   UniquePtr<RecursiveThreadPool> subpool_stg[2];
   RecursiveThreadPool *subpool_ptr[2];
   long mid;

   bool concurrent() { return mid != 0; }
   RecursiveThreadPool* subpool(long i) { return subpool_ptr[i]; } 

   RecursiveThreadPoolHelper(RecursiveThreadPool *pool, bool seq, double load0)
   {
      mid = 0;
      subpool_ptr[0] = subpool_ptr[1] = 0;

      if (seq || !pool) return; 
      long n = pool->hi - pool->lo;
      if (n <= 1) return;
     
      long n0 = long(load0*n + 0.5);
      if (n0 < 0 || n0 > n) LogicError("RecursiveThreadPoolHelper: bad load0");

      if (n0 == 0) {
         subpool_ptr[1] = pool;
         return;
      }

      if (n0 == n) {
         subpool_ptr[0] = pool;
         return;
      }

      mid = pool->lo + n0;

      long n1 = n-n0;
      if (n0 > 1) subpool_stg[0].make(pool->base_pool, pool->lo, mid);
      if (n1 > 1) subpool_stg[1].make(pool->base_pool, mid, pool->hi);

      subpool_ptr[0] = subpool_stg[0].get();
      subpool_ptr[1] = subpool_stg[1].get();
   }
};



NTL_CLOSE_NNS


#endif



#ifdef NTL_THREAD_BOOST

#ifndef NTL_THREADS
#error "NTL_THREAD_BOOST requires NTL_THREADS"
#endif

NTL_OPEN_NNS

extern
NTL_CHEAP_THREAD_LOCAL BasicThreadPool *NTLThreadPool_ptr;

inline
BasicThreadPool *GetThreadPool()
{
   return NTLThreadPool_ptr;
}

void ResetThreadPool(BasicThreadPool *pool = 0);
BasicThreadPool *ReleaseThreadPool();

inline void SetNumThreads(long n) 
{ 
   BasicThreadPool *p = (n == 1 ? 0 : MakeRaw<BasicThreadPool>(n));
   ResetThreadPool(p);
}

inline long AvailableThreads()
{
   BasicThreadPool *pool = GetThreadPool();
   if (!pool || pool->active())
      return 1;
   else
      return pool->NumThreads();
}


NTL_CLOSE_NNS


#define NTL_EXEC_RANGE(n, first, last)  \
{  \
   NTL_NNS BasicThreadPool::relaxed_exec_range(NTL_NNS GetThreadPool(), (n), \
     [&](long first, long last) {  \


#define NTL_EXEC_RANGE_END  \
   } ); \
}  \


#define NTL_GEXEC_RANGE(seq, n, first, last)  \
{  \
   NTL_NNS BasicThreadPool::relaxed_exec_range((seq) ? 0 : NTL_NNS GetThreadPool(), (n), \
     [&](long first, long last) {  \


#define NTL_GEXEC_RANGE_END  \
   } ); \
}  \


#define NTL_EXEC_INDEX(n, index)  \
{  \
   NTL_NNS BasicThreadPool::relaxed_exec_index(NTL_NNS GetThreadPool(), (n), \
     [&](long index) {  \


#define NTL_EXEC_INDEX_END  \
   } ); \
}  \



// NOTE: at least with gcc >= 4.9.2, the GEXEC versions will evaluate seq, and
// if it is true, jump directly (more or less) to the body


#define NTL_TBDECL(x) static void basic_ ## x
#define NTL_TBDECL_static(x) static void basic_ ## x

#define NTL_IMPORT(x) auto _ntl_hidden_variable_IMPORT__ ## x = x; auto x = _ntl_hidden_variable_IMPORT__ ##x;


#define NTL_INIT_DIVIDE StartRecursion(GetThreadPool()).get()

#define NTL_EXEC_DIVIDE(seq, pool, helper, load0, F0, F1) \
{ \
  NTL::RecursiveThreadPoolHelper helper(pool, seq, load0); \
  if (!helper.mid) { \
    { F0; } \
    { F1; } \
  } \
  else { \
    pool->exec_pair(helper.mid,  \
      [&](long){ F0; }, \
      [&](long){ F1; } ); \
  } \
}





#else

NTL_OPEN_NNS


inline void SetNumThreads(long n) { }

inline long AvailableThreads() { return 1; }

struct RecursiveThreadPool;

struct RecursiveThreadPoolDummyHelper {
   bool concurrent() { return false; }
   RecursiveThreadPool* subpool(long i) { return 0; }
};


NTL_CLOSE_NNS

#define NTL_EXEC_RANGE(n, first, last)  \
{  \
   long _ntl_par_exec_n = (n);  \
   if (_ntl_par_exec_n > 0) {  \
      long first = 0;  \
      long last = _ntl_par_exec_n;  \
      {  \
   

#define NTL_EXEC_RANGE_END  }}}

#define NTL_GEXEC_RANGE(seq, n, first, last)  \
{  \
   long _ntl_par_exec_n = (n);  \
   if (_ntl_par_exec_n > 0) {  \
      long first = 0;  \
      long last = _ntl_par_exec_n;  \
      {  \
   

#define NTL_GEXEC_RANGE_END  }}}




#define NTL_EXEC_INDEX(n, index)  \
{  \
   long _ntl_par_exec_n = (n);  \
   if (_ntl_par_exec_n > 0) {  \
      if (_ntl_par_exec_n > 1) NTL_NNS LogicError("NTL_EXEC_INDEX: not enough threads"); \
      long index = 0;  \
      {  \

   
#define NTL_EXEC_INDEX_END  }}}



#define NTL_TBDECL(x) void x
#define NTL_TBDECL_static(x) static void x

#define NTL_IMPORT(x)

#define NTL_INIT_DIVIDE ((RecursiveThreadPool*) 0)

#define NTL_EXEC_DIVIDE(seq, pool, helper, load0, F0, F1) \
{ \
  NTL::RecursiveThreadPoolDummyHelper helper; \
  { F0; } \
  { F1; } \
}

#endif





#endif


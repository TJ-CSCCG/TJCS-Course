
#ifndef NTL_LazyTable__H
#define NTL_LazyTable__H

#include <NTL/tools.h>
#include <NTL/SmartPtr.h>
#include <NTL/thread.h>

NTL_OPEN_NNS


/***************************************************************************


LazyTable<T,MAX>: template class for lazy initialization of objects whose
values do not change after initialization.
In a multi-threaded environment, this makes use of "double checked locking"
for an efficient, thread-safe solution.

Usage:

   LazyTable<T,MAX> tab; // declaration of the lazy table, with max size == MAX

    ...

   do {
      LazyTable<T,MAX>::Builder builder(tab, n); // request length n
      long amt = builder.amt();
      if (!amt) break;      

      ... initialize elements i = n-amt..n-1 
          using builder.move(p), where p is a UnqiuePtr<T>
          note that each move application appends one element
                             
   } while(0);               // When this scope closes, 
                             // the table is fully initialized to length n


   const T* val = table[i];  // read-only access to table elements 0..n-1
                             

It is important to follow this recipe carefully.  In particular,
the builder must be enclosed in a scope, as it's destructor
plays a crucial role in finalizing the initialization.


template<class T, long MAX>
class LazyTable {
public:
   LazyTable();


   const T * operator[] (long i) const;

   ~LazyTable();

   long length() const; 

   class Builder {
      Builder(const LazyTable&, long request); 
     ~Builder()

      long amt() const;
      void move(UniquePtr<T>& p);
private:
   LazyTable(const LazyTable&);             // disabled
   LazyTable& operator=(const LazyTable&);

};
   


****************************************************************************/


// NOTE: For more on double-checked locking, see
// http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/

// NOTE: when compiled with the NTL_THREADS option, the LazyTable
// class may contain data members from the standard library
// that may not satisfy the requirements of the Vec class
// (i.e., relocatability).  One can wrap it in a pointer 
// class (e.g., OptionalVal) to deal with this.



template<class T, long MAX>
class LazyTable {
private:
   mutable AtomicLong len; 
   mutable MutexProxy mtx;

   mutable UniqueArray< UniquePtr<T> > data;

   LazyTable(const LazyTable&); // disabled
   void operator=(const LazyTable&); // disabled

public:
   LazyTable() : len(0) { }

   const T * operator[] (long i) const 
   { 
      // FIXME: add optional range checking

      return data[i].get(); 
   }

   long length() const { return len; }

   class Builder {
   private:
      const LazyTable& ref;
      long request;
      GuardProxy guard;

      long amount;
      long curlen;

      Builder(const Builder&); // disabled
      void operator=(const Builder&); // disabled

   public:
      Builder(const LazyTable& _ref, long _request) 
      : ref(_ref), request(_request), guard(_ref.mtx), amount(0), curlen(0)
      {
         if (request < 0 || request > MAX) 
            LogicError("request out of range in LazyTable::Builder"); 


         // Double-checked locking
         if (request <= ref.len || (guard.lock(), request <= ref.len)) 
            return;

         curlen = ref.len;
         amount = request - curlen;
         if (!ref.data) ref.data.SetLength(MAX);
      }

      ~Builder() { if (amount) ref.len = curlen; }

      void move(UniquePtr<T>& p)
      {
         if (!amount || curlen >= request) LogicError("LazyTable::Builder illegal move");
         ref.data[curlen].move(p);
         curlen++;
      }

      long amt() const { return amount; }
   };
};


// NOTE: LazyTable's are non-relocatable


NTL_CLOSE_NNS

#endif

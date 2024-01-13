
/***************************************************************************


Lazy<T>: template class for lazy initialization of objects whose
values do not change after initialization.
In a multi-threaded environment, this makes use of "double checked locking"
for an efficient, thread-safe solution.

Usage:

   Lazy<T> obj; // declaration of the lazy object

    ...

   do {
      Lazy<T>::Builder builder(obj);
      if (!builder()) break; // if we are not building, the break out

      UniquePtr<T> p;        // create a pointer 

         ...

      builder.move(p);       // move p into the object to complete the initialization
                             // We can then complete the initialization process.
   } while(0);               // When this scope closes, the object is fully initialized.
                             // subsequent attempts to build the object will yield
                             // !builder.built()


   T objCopy = *obj;         // *obj returns a read-only reference
                             // one can also use -> operator

It is important to follow this recipe carefully.  In particular,
the builder must be enclosed in a scope, as it's destructor
plays a crucial role in finalizing the initialization.

NOTE: if p is null in builder.move(p), the object is still considered
built.


template<class T>
class Lazy {
public:
   Lazy();

   Lazy(const Lazy&);             // "deep" copies
   Lazy& operator=(const Lazy&);

   const T& operator*()  const;     // pointer access
   const T* operator->() const;
   const T* get() const;
   operator fake_null_type() const; // test for null pointer
   
   ~Lazy();

   kill();  // destroy and reset

   bool built() const; // test if already built




   class Builder {
      Builder(const Lazy&); 
     ~Builder()

      bool operator()() const; // test if we are building
      void move(UniquePtr<T>&);

   };
   


****************************************************************************/

#ifndef NTL_Lazy__H
#define NTL_Lazy__H


#include <NTL/tools.h>
#include <NTL/SmartPtr.h>
#include <NTL/thread.h>


NTL_OPEN_NNS



// NOTE: For more on double-checked locking, see
// http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/

// NOTE: when compiled with the NTL_THREADS option, the Lazy
// class may contain data members from the standard library
// that may not satisfy the requirements of the Vec class
// (i.e., relocatability).  One can wrap it in a pointer 
// class (e.g., OptionalVal) to deal with this.

template<class T, class P=DefaultDeleterPolicy>
class Lazy {
private:
   /* we make data members mutable so that Lazy members of
      other classes don't have to be. */

   mutable AtomicBool initialized; 
   mutable MutexProxy mtx;

   mutable UniquePtr<T, P> data;


   class Dummy { };
   typedef void (Lazy::*fake_null_type)(Dummy) const;
   void fake_null_function(Dummy) const {}


public:
   Lazy() : initialized(false) { }

   // EXCEPTIONS: This always succeeds in killing the object
   void kill() 
   { 
      UniquePtr<T, P> tmp;
      tmp.swap(data);
      initialized = false;  
   }

   // This is provided for convenience for some legacy code.
   // It us up to the client code to ensure there are no race conditions.

   // EXCEPTIONS: strong ES 
   Lazy& operator=(const Lazy& other) 
   {
      if (this == &other) return *this;

      if (other.initialized) {
         UniquePtr<T, P> p;
         if (other.data) p.make(*other.data);
         p.swap(data);
         initialized = true;
      }
      else
         kill();

      return *this;
   }
   
   Lazy(const Lazy& other) : initialized(false)
   {
      *this = other;
   }

   const T& operator*()  const { return *data; }
   const T* operator->() const { return data.operator->(); }
   const T* get() const { return data.get(); }

   bool built()  const { return initialized; }

   operator fake_null_type() const 
   {
      return data ?  &Lazy::fake_null_function : 0;
   }


   class Builder {
   private:
      bool building;
      bool moved;
      const Lazy& ref;
      GuardProxy guard;

      Builder(const Builder&); // disabled
      void operator=(const Builder&); // disabled



   public:
      Builder(const Lazy& _ref) : building(false), moved(false),
                                  ref(_ref), guard(_ref.mtx)
      {
         // Double-checked locking
         if (ref.initialized || (guard.lock(), ref.initialized)) 
            return;

         building = true; // we set this to true after we lock the mutex
                          // and see the the object is still uninitialized
      }

      ~Builder() { if (moved) ref.initialized = true; }

      void move(UniquePtr<T, P>& p) 
      {
         if (!building || moved) LogicError("Lazy::Builder illegal call to move");
         ref.data.move(p); 
         moved = true; 
      }

      bool operator()()  const { return building; }
   };
};

// NOTE: Lazy's are non-relocatable


NTL_CLOSE_NNS


#endif


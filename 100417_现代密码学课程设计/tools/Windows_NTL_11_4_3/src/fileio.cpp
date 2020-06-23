
#include <NTL/fileio.h>
#include <NTL/thread.h>

#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>



NTL_START_IMPL


void OpenWrite(ofstream& s, const char *name)
{
   s.open(name, std::ios::out);

   if (!s) {
      FileError("write open failed");
   }
}

void OpenWrite(ofstream& s, const char *name, FileList& flist)
{
   // post condition: file is successfully opened iff 
   //   name is added to flist (even if exception is thrown).
   //   We do the AddFile first, since that can conceivably fail.

   flist.AddFile(name);
   s.open(name, std::ios::out);

   if (!s) {
      flist.RemoveLast();
      FileError("write open failed");
   }
}


void OpenRead(ifstream& s, const char *name)
{
   s.open(name, std::ios::in);
   if (!s) {
      FileError("read open failed");
   }
}

void CloseWrite(ofstream& s)
{
   s.close();
   if (s.fail()) FileError("close failed");
}


void FileList::AddFile(const char *name)
{
   Vec<char> item;
   item.SetLength(strlen(name)+1);
   strcpy(item.elts(), name);

   data.append(item);
}

void FileList::RemoveLast()
{
   data.SetLength(data.length()-1);
}


FileList::~FileList()
{
   long i, n;
 
   n = data.length();
   for (i = 0; i < n; i++)
      remove(data[i].elts());
}




const char *FileName(const char* stem, long d)
{
   NTL_TLS_LOCAL(std::string, sbuf);

   std::stringstream ss;
   ss << "tmp-ntl-" << stem;
   ss << "-" << std::setfill('0') << std::setw(5) << d << "-";
   sbuf = ss.str() + UniqueID();
   return sbuf.c_str();
}

// UniqueID:
//
// builds a string of the form cnt-time-clock-pid-tid, where
//   - cnt is a global counter
//   - time is the value returned by time(0)
//   - clock is the value returned by clock()
//   - pid is the value returned by getpid() (or "0" if getpid()
//        is not available)
//   - tid is the value returned by this_thread::get_id()
//        (or "0" if not using threads)
// each thread should have its own unique ID, which is guaranteed
//    to be unique across all threads in a process, and which
//    is hopefully unique across the entire system (but this
//    harder to guarantee)


const std::string& UniqueID()
{
   static AtomicCounter cnt; // a GLOBAL counter
   

   NTL_TLS_LOCAL(std::string, ID);

   NTL_TLS_LOCAL_INIT(bool, initialized, (false));
   NTL_TLS_LOCAL_INIT(unsigned long, local_cnt, (cnt.inc()));
   NTL_TLS_LOCAL_INIT(unsigned long, local_time, (time(0)));
   NTL_TLS_LOCAL_INIT(unsigned long, local_clock, (clock()));

   if (!initialized) {
      std::stringstream ss;
      ss << local_cnt << "-" << local_time << "-" 
         << local_clock << "-" << GetPID()  << "-" << CurrentThreadID();  
      ID = ss.str();
      initialized = true;
   }

   return ID;
}


NTL_END_IMPL

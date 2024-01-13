
#ifndef NTL_fileio__H
#define NTL_fileio__H

#include <NTL/tools.h>
#include <NTL/vector.h>
#include <fstream>                                                              
#include <string>


NTL_OPEN_NNS


class FileList {
private:
   Vec< Vec<char> > data;

   FileList(const FileList&); // disable
   void operator=(const FileList&); // disable

public:
   FileList() { }
   void AddFile(const char *name);
   void RemoveLast();

   ~FileList();


};



void OpenWrite(NTL_SNS ofstream& s, const char *name);
// opens file for writing...aborts if fails

void OpenWrite(NTL_SNS ofstream& s, const char *name, FileList& flist);
// opens file for writing and adds name to flist

void OpenRead(NTL_SNS ifstream& s, const char *name);
// opens file for reading

void CloseWrite(NTL_SNS ofstream& s);
// closes s, checks for failure



const char *FileName(const char* stem, long d);
// builds the name from stem-DDDDD, returns pointer to buffer

const NTL_SNS string& UniqueID();
// ideally, a unique ID (across all processes and threads),
// but it may not be perfect (useful for generating unique 
// file names and seeding PRG).

NTL_CLOSE_NNS

#endif



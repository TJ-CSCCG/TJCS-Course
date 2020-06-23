#ifndef NTL_LLL__H
#define NTL_LLL__H

#include <NTL/mat_ZZ.h>
#include <NTL/mat_RR.h>

NTL_OPEN_NNS

long LLL(ZZ& det, mat_ZZ& B, long verbose = 0);
long LLL(ZZ& det, mat_ZZ& B, mat_ZZ& U, long verbose = 0);

long LLL(ZZ& det, mat_ZZ& B, long a, long b, long verbose = 0);
long LLL(ZZ& det, mat_ZZ& B, mat_ZZ& U, long a, long b, long verbose = 0);

long LLL_plus(vec_ZZ& D, mat_ZZ& B, mat_ZZ& U, long verbose=0);
long LLL_plus(vec_ZZ& D, mat_ZZ& B, long verbose=0);
long LLL_plus(vec_ZZ& D, mat_ZZ& B, mat_ZZ& U, long a, long b, long verbose=0);
long LLL_plus(vec_ZZ& D, mat_ZZ& B, long a, long b, long verbose=0);

long image(ZZ& det, mat_ZZ& B, long verbose = 0);
long image(ZZ& det, mat_ZZ& B, mat_ZZ& U, long verbose = 0);

long LatticeSolve(vec_ZZ& x, const mat_ZZ& A, const vec_ZZ& y, long reduce=0);



typedef long (*LLLCheckFct)(const vec_ZZ&); 

extern NTL_CHEAP_THREAD_LOCAL double LLLStatusInterval;
extern NTL_CHEAP_THREAD_LOCAL char *LLLDumpFile;


// classical Gramm-Schmidt versions

long LLL_FP(mat_ZZ& B, double delta = 0.99,
	    long deep = 0, LLLCheckFct check = 0, long verbose = 0);

long LLL_FP(mat_ZZ& B, mat_ZZ& U, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);


long BKZ_FP(mat_ZZ& BB, double delta=0.99, long BlockSize=10, long prune=0,
         LLLCheckFct check = 0, long verbose = 0) ;
long BKZ_FP(mat_ZZ& BB, mat_ZZ& U, double delta=0.99, 
         long BlockSize=10, long prune=0, 
         LLLCheckFct check = 0, long verbose = 0);

long LLL_XD(mat_ZZ& B, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);
long LLL_XD(mat_ZZ& B, mat_ZZ& U, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);


long BKZ_XD(mat_ZZ& BB, double delta=0.99, long BlockSize=10, long prune=0,
         LLLCheckFct check = 0, long verbose = 0);
long BKZ_XD(mat_ZZ& BB, mat_ZZ& U, double delta=0.99,
         long BlockSize=10, long prune=0, LLLCheckFct check = 0, long verbose = 0);

long LLL_QP(mat_ZZ& B, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);
long LLL_QP(mat_ZZ& B, mat_ZZ& U, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);


long BKZ_QP(mat_ZZ& BB, double delta=0.99, long BlockSize=10, long prune=0,
         LLLCheckFct check = 0, long verbose = 0);
long BKZ_QP(mat_ZZ& BB, mat_ZZ& U, double delta=0.99,
         long BlockSize=10, long prune=0, LLLCheckFct check = 0, long verbose = 0);

long BKZ_QP1(mat_ZZ& BB, double delta=0.99, long BlockSize=10, long prune=0,
         LLLCheckFct check = 0, long verbose = 0);
long BKZ_QP1(mat_ZZ& BB, mat_ZZ& U, double delta=0.99,
         long BlockSize=10, long prune=0, LLLCheckFct check = 0, long verbose = 0);

long LLL_RR(mat_ZZ& B, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);
long LLL_RR(mat_ZZ& B, mat_ZZ& U, double delta = 0.99, 
            long deep = 0, LLLCheckFct check = 0, long verbose = 0);


long BKZ_RR(mat_ZZ& BB, double delta=0.99, long BlockSize=10, 
            long prune=0, LLLCheckFct check = 0, long verbose = 0);

long BKZ_RR(mat_ZZ& BB, mat_ZZ& U, double delta=0.99, 
            long BlockSize=10, long prune=0, LLLCheckFct check = 0, long verbose = 0);


// Givens rotations versions

long G_LLL_FP(mat_ZZ& B, double delta = 0.99,
	    long deep = 0, LLLCheckFct check = 0, long verbose = 0);

long G_LLL_FP(mat_ZZ& B, mat_ZZ& U, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);


long G_BKZ_FP(mat_ZZ& BB, double delta=0.99, long BlockSize=10, long prune=0,
         LLLCheckFct check = 0, long verbose = 0) ;
long G_BKZ_FP(mat_ZZ& BB, mat_ZZ& U, double delta=0.99, 
         long BlockSize=10, long prune=0, 
         LLLCheckFct check = 0, long verbose = 0);

long G_LLL_XD(mat_ZZ& B, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);
long G_LLL_XD(mat_ZZ& B, mat_ZZ& U, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);


long G_BKZ_XD(mat_ZZ& BB, double delta=0.99, long BlockSize=10, long prune=0,
         LLLCheckFct check = 0, long verbose = 0);
long G_BKZ_XD(mat_ZZ& BB, mat_ZZ& U, double delta=0.99,
         long BlockSize=10, long prune=0, LLLCheckFct check = 0, long verbose = 0);

long G_LLL_QP(mat_ZZ& B, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);
long G_LLL_QP(mat_ZZ& B, mat_ZZ& U, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);


long G_BKZ_QP(mat_ZZ& BB, double delta=0.99, long BlockSize=10, long prune=0,
         LLLCheckFct check = 0, long verbose = 0);
long G_BKZ_QP(mat_ZZ& BB, mat_ZZ& U, double delta=0.99,
         long BlockSize=10, long prune=0, LLLCheckFct check = 0, long verbose = 0);

long G_BKZ_QP1(mat_ZZ& BB, double delta=0.99, long BlockSize=10, long prune=0,
         LLLCheckFct check = 0, long verbose = 0);
long G_BKZ_QP1(mat_ZZ& BB, mat_ZZ& U, double delta=0.99,
         long BlockSize=10, long prune=0, LLLCheckFct check = 0, long verbose = 0);

long G_LLL_RR(mat_ZZ& B, double delta = 0.99, long deep = 0,
           LLLCheckFct check = 0, long verbose = 0);
long G_LLL_RR(mat_ZZ& B, mat_ZZ& U, double delta = 0.99, 
            long deep = 0, LLLCheckFct check = 0, long verbose = 0);


long G_BKZ_RR(mat_ZZ& BB, double delta=0.99, long BlockSize=10, 
            long prune=0, LLLCheckFct check = 0, long verbose = 0);

long G_BKZ_RR(mat_ZZ& BB, mat_ZZ& U, double delta=0.99, 
            long BlockSize=10, long prune=0, LLLCheckFct check = 0, long verbose = 0);

void ComputeGS(const mat_ZZ& B, mat_RR& mu, vec_RR& c);


void NearVector(vec_ZZ& ww, const mat_ZZ& BB, const vec_ZZ& a);

NTL_CLOSE_NNS

#endif

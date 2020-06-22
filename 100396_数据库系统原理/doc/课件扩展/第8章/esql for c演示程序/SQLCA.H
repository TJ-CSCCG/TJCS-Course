/***********************************************************************\
			    FILE SQLCA.H

	IMPORTANT
	=========
 This file is automatically included by sqlprep.exe.
 Do not include it manually.

		Copyright (c) Microsoft Corporation 1990, 1992
\***********************************************************************/


#ifndef _SQLPREP_
#pragma message( "Do not include sqlca.h manually. It will be included by sqlprep.exe. ")
#endif	// _SQLPREP_

#ifndef SQLCA_H

#define SQLCA_H


#ifdef WIN32
	#define ESQLAPI __stdcall
	#define far
#else
	#define ESQLAPI far pascal
#endif

#define SQLERRMC_SIZ    70
#define BETTER_ERRMC    999
#define EYECATCH_LEN    8

// SQL Communication Area - SQLCA

typedef struct tag_sqlca {
	unsigned char	sqlcaid[EYECATCH_LEN];	// Eyecatcher = 'SQLCA   '
	long		    sqlcabc;		        // SQLCA size in bytes = 136
	long		    sqlcode;		        // SQL return code
	short		    sqlerrml;		        // Length for SQLERRMC
	unsigned char	sqlerrmc[SQLERRMC_SIZ];	// Error message tokens
	unsigned char	sqlerrp[8];		        // Diagnostic information
	long		    sqlerrd[6];		        // Diagnostic information
	unsigned char	sqlwarn[8];		        // Warning flags
	unsigned char	sqlext[3];		        // Reserved
	unsigned char   sqlstate[5];            // SQLSTATE
//	unsigned char	sqlext[8];		        // Reserved
} SQLCA, *PSQLCA, far *LPSQLCA;

//#define SQLCODE sqlca->sqlcode
extern long SQLCODE;

#define SQLWARN0 sqlca->sqlwarn[0]
#define SQLWARN1 sqlca->sqlwarn[1]
#define SQLWARN2 sqlca->sqlwarn[2]
#define SQLWARN3 sqlca->sqlwarn[3]
#define SQLWARN4 sqlca->sqlwarn[4]
#define SQLWARN5 sqlca->sqlwarn[5]
#define SQLWARN6 sqlca->sqlwarn[6]
#define SQLWARN7 sqlca->sqlwarn[7]

#define SQLERRD1 sqlca->sqlerrd[0]
#define SQLERRD2 sqlca->sqlerrd[1]
#define SQLERRD3 sqlca->sqlerrd[2]
#define SQLERRD4 sqlca->sqlerrd[3]

#define SQLERRMC sqlca->sqlerrmc
#define SQLERRML sqlca->sqlerrml


#endif	// SQLCA_H

/* EOF: sqlca.h */


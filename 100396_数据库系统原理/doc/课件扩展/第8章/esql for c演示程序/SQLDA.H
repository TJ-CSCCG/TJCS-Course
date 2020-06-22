/**************************************************************************

   Module Name	  = SQLDA.H

   SQLDA External Include File

   Copyright = nnnnnnnn (C) Copyright IBM Corp. 1987
	       Licensed Material - Program Property of IBM
	       Refer to Copyright Instructions Form Number G120-3083

   Function = Include File defining SQLDA

		IMPORTANT
		=========
 This file is automatically included by sqlprep.exe.
 Do not include it manually.

**************************************************************************/

#ifndef _SQLPREP_
#pragma message( "Do not include sqlda.h manually. It will be included by sqlprep.exe. ")
#endif	// _SQLPREP_


/* SQL Descriptor Area - Variable descriptor */

#ifndef   SQLDASIZE

/* SQL Descriptor Area - SQLDA */
struct sqlda {
	
    unsigned char   sqldaid[8];	        // Eye catcher = 'SQLDA   ' 
	long            sqldabc;			// SQLDA size in bytes = 16+44*SQLN 
	short           sqln;			    // Number of SQLVAR elements
	short           sqld;			    // # of used SQLVAR elements
	struct          sqlvar 
    {
		short       sqltype;			// Variable data type
		short       sqllen;			    // Variable data length
		unsigned char far *sqldata;	    // Pointer to variable data value
		short       far *sqlind;		// Pointer to Null indicator
		struct      sqlname             // Variable Name
        {
			short   length;		        // Name length [1..30]
			unsigned char data[30];	    // Variable or Column name
		}sqlname;

	} sqlvar[1];
};

/* macro for allocating SQLDA */

#define   SQLDASIZE(n) (sizeof(struct sqlda) + (n-1)*sizeof(struct sqlvar))

#endif	// SQLDASIZE

/* EOF: sqlda.h */


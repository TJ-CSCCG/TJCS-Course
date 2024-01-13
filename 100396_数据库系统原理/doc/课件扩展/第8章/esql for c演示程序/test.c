/* ===== test.c =====*/


/* ===== NT doesn't need the following... */
#ifndef WIN32
#define WIN32
#endif
#define _loadds
#define _SQLPREP_
#include <sqlca.h>
#include <sqlda.h>
#include <string.h>
#define SQLLENMAX(x)      ( ((x) > 32767) ? 32767 : (x) )
short ESQLAPI _loadds sqlaaloc(
	unsigned short usSqlDaId,
	unsigned short sqld,
	unsigned short stmt_id,
	void far *spare);

short ESQLAPI _loadds sqlxcall(
	unsigned short usCallType,
	unsigned short usSection,
	unsigned short usSqldaInId,
	unsigned short usSqlDaOutId,
	unsigned short usSqlTextLen,
	char far *lpszSQLText);

short ESQLAPI _loadds sqlacall(
	unsigned short usCallType,
	unsigned short usSection,
	unsigned short usSqldaInId,
	unsigned short usSqlDaOutId,
	void far *spare);

short ESQLAPI _loadds sqladloc(
	unsigned short usSqldaInId,
	void far *spare);

short ESQLAPI _loadds sqlasets(
	unsigned short cbSqlText,
	void far *lpvSqlText,
	void far *spare);

short ESQLAPI _loadds sqlasetv(
	unsigned short usSqldaInId,
	unsigned short sqlvar_index,
	unsigned short sqltype,
	unsigned short sqllen,
	void far *sqldata,
	void far *sqlind,
	void far *spare);

short ESQLAPI _loadds sqlastop(
	void far *spare);

short ESQLAPI _loadds sqlastrt(
	void far *pid,
	void far *spare,
	void far *sqlca);

short ESQLAPI _loadds sqlausda(
	unsigned short sqldaId,
	void far *lpvSqlDa,
	void far *spare);

extern struct tag_sqlca far sql_sqlca;
extern struct tag_sqlca far *sqlca;
struct sqla_program_id2 { 
unsigned short length; 
unsigned short rp_rel_num; 
unsigned short db_rel_num; 
unsigned short bf_rel_num; 
unsigned char  sqluser[30]; 
unsigned char  sqlusername[30];
unsigned char  planname[256]; 
unsigned char  contoken[8]; 
unsigned char  buffer[8]; 
}; 
static struct sqla_program_id2 program_id = 
		{340,2,0,0,"                              ","","test","MMMNJdL0","        "};
static void far* pid = &program_id;
#line 1 "test.sqc"
#line 1 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"















#pragma once
#line 18 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"






#line 25 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"







#pragma pack(push,8)
#line 34 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"














#line 49 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"
#line 50 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"






#line 57 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"








#line 66 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"
#line 67 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"



typedef unsigned int size_t;

#line 73 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"




typedef unsigned short wchar_t;

#line 80 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"



typedef wchar_t wint_t;
typedef wchar_t wctype_t;

#line 87 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"
#line 88 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"









typedef char *  va_list;
#line 99 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"

#line 101 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"






#line 108 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"

#line 110 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"






























#line 141 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"





struct _iobuf {
        char *_ptr;
        int   _cnt;
        char *_base;
        int   _flag;
        int   _file;
        int   _charbuf;
        int   _bufsiz;
        char *_tmpfname;
        };
typedef struct _iobuf FILE;

#line 159 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"












#line 172 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"












#line 185 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"




























#line 214 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"
#line 215 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"





 extern FILE _iob[];
#line 222 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"









#line 232 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"


typedef __int64 fpos_t;







#line 243 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"
#line 244 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"


#line 247 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"




























 int __cdecl _filbuf(FILE *);
 int __cdecl _flsbuf(int, FILE *);




 FILE * __cdecl _fsopen(const char *, const char *, int);
#line 283 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"

 void __cdecl clearerr(FILE *);
 int __cdecl fclose(FILE *);
 int __cdecl _fcloseall(void);




 FILE * __cdecl _fdopen(int, const char *);
#line 293 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"

 int __cdecl feof(FILE *);
 int __cdecl ferror(FILE *);
 int __cdecl fflush(FILE *);
 int __cdecl fgetc(FILE *);
 int __cdecl _fgetchar(void);
 int __cdecl fgetpos(FILE *, fpos_t *);
 char * __cdecl fgets(char *, int, FILE *);




 int __cdecl _fileno(FILE *);
#line 307 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"

 int __cdecl _flushall(void);
 FILE * __cdecl fopen(const char *, const char *);
 int __cdecl fprintf(FILE *, const char *, ...);
 int __cdecl fputc(int, FILE *);
 int __cdecl _fputchar(int);
 int __cdecl fputs(const char *, FILE *);
 size_t __cdecl fread(void *, size_t, size_t, FILE *);
 FILE * __cdecl freopen(const char *, const char *, FILE *);
 int __cdecl fscanf(FILE *, const char *, ...);
 int __cdecl fsetpos(FILE *, const fpos_t *);
 int __cdecl fseek(FILE *, long, int);
 long __cdecl ftell(FILE *);
 size_t __cdecl fwrite(const void *, size_t, size_t, FILE *);
 int __cdecl getc(FILE *);
 int __cdecl getchar(void);
 int __cdecl _getmaxstdio(void);
 char * __cdecl gets(char *);
 int __cdecl _getw(FILE *);
 void __cdecl perror(const char *);
 int __cdecl _pclose(FILE *);
 FILE * __cdecl _popen(const char *, const char *);
 int __cdecl printf(const char *, ...);
 int __cdecl putc(int, FILE *);
 int __cdecl putchar(int);
 int __cdecl puts(const char *);
 int __cdecl _putw(int, FILE *);
 int __cdecl remove(const char *);
 int __cdecl rename(const char *, const char *);
 void __cdecl rewind(FILE *);
 int __cdecl _rmtmp(void);
 int __cdecl scanf(const char *, ...);
 void __cdecl setbuf(FILE *, char *);
 int __cdecl _setmaxstdio(int);
 int __cdecl setvbuf(FILE *, char *, int, size_t);
 int __cdecl _snprintf(char *, size_t, const char *, ...);
 int __cdecl sprintf(char *, const char *, ...);
 int __cdecl sscanf(const char *, const char *, ...);
 char * __cdecl _tempnam(const char *, const char *);
 FILE * __cdecl tmpfile(void);
 char * __cdecl tmpnam(char *);
 int __cdecl ungetc(int, FILE *);
 int __cdecl _unlink(const char *);
 int __cdecl vfprintf(FILE *, const char *, va_list);
 int __cdecl vprintf(const char *, va_list);
 int __cdecl _vsnprintf(char *, size_t, const char *, va_list);
 int __cdecl vsprintf(char *, const char *, va_list);








#line 363 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"




 FILE * __cdecl _wfsopen(const wchar_t *, const wchar_t *, int);
#line 369 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"

 wint_t __cdecl fgetwc(FILE *);
 wint_t __cdecl _fgetwchar(void);
 wint_t __cdecl fputwc(wint_t, FILE *);
 wint_t __cdecl _fputwchar(wint_t);
 wint_t __cdecl getwc(FILE *);
 wint_t __cdecl getwchar(void);
 wint_t __cdecl putwc(wint_t, FILE *);
 wint_t __cdecl putwchar(wint_t);
 wint_t __cdecl ungetwc(wint_t, FILE *);

 wchar_t * __cdecl fgetws(wchar_t *, int, FILE *);
 int __cdecl fputws(const wchar_t *, FILE *);
 wchar_t * __cdecl _getws(wchar_t *);
 int __cdecl _putws(const wchar_t *);

 int __cdecl fwprintf(FILE *, const wchar_t *, ...);
 int __cdecl wprintf(const wchar_t *, ...);
 int __cdecl _snwprintf(wchar_t *, size_t, const wchar_t *, ...);
 int __cdecl swprintf(wchar_t *, const wchar_t *, ...);
 int __cdecl vfwprintf(FILE *, const wchar_t *, va_list);
 int __cdecl vwprintf(const wchar_t *, va_list);
 int __cdecl _vsnwprintf(wchar_t *, size_t, const wchar_t *, va_list);
 int __cdecl vswprintf(wchar_t *, const wchar_t *, va_list);
 int __cdecl fwscanf(FILE *, const wchar_t *, ...);
 int __cdecl swscanf(const wchar_t *, const wchar_t *, ...);
 int __cdecl wscanf(const wchar_t *, ...);






 FILE * __cdecl _wfdopen(int, const wchar_t *);
 FILE * __cdecl _wfopen(const wchar_t *, const wchar_t *);
 FILE * __cdecl _wfreopen(const wchar_t *, const wchar_t *, FILE *);
 void __cdecl _wperror(const wchar_t *);
 FILE * __cdecl _wpopen(const wchar_t *, const wchar_t *);
 int __cdecl _wremove(const wchar_t *);
 wchar_t * __cdecl _wtempnam(const wchar_t *, const wchar_t *);
 wchar_t * __cdecl _wtmpnam(wchar_t *);



#line 414 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"
#line 415 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"


#line 418 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"
































 int __cdecl fcloseall(void);
 FILE * __cdecl fdopen(int, const char *);
 int __cdecl fgetchar(void);
 int __cdecl fileno(FILE *);
 int __cdecl flushall(void);
 int __cdecl fputchar(int);
 int __cdecl getw(FILE *);
 int __cdecl putw(int, FILE *);
 int __cdecl rmtmp(void);
 char * __cdecl tempnam(const char *, const char *);
 int __cdecl unlink(const char *);

#line 463 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"






#pragma pack(pop)
#line 471 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"

#line 473 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdio.h"
#line 2 "test.sqc"
#line 1 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
















#pragma once
#line 19 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"






#line 26 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"







#pragma pack(push,8)
#line 35 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"






















#line 58 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"










































typedef int (__cdecl * _onexit_t)(void);



#line 105 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

#line 107 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"






typedef struct _div_t {
        int quot;
        int rem;
} div_t;

typedef struct _ldiv_t {
        long quot;
        long rem;
} ldiv_t;


#line 125 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"












 extern int __mb_cur_max;
#line 139 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"





















#line 161 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

















#line 179 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
 extern int errno;               
 extern unsigned long _doserrno; 
#line 182 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"







 extern char * _sys_errlist[];   
 extern int _sys_nerr;           





























#line 221 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

 extern int __argc;          
 extern char ** __argv;      

 extern wchar_t ** __wargv;  
#line 227 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"




 extern char ** _environ;    

 extern wchar_t ** _wenviron;    
#line 235 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
#line 236 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

 extern char * _pgmptr;      

 extern wchar_t * _wpgmptr;  
#line 241 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

#line 243 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"


 extern int _fmode;          
 extern int _fileinfo;       




 extern unsigned int _osver;
 extern unsigned int _winver;
 extern unsigned int _winmajor;
 extern unsigned int _winminor;





 __declspec(noreturn) void   __cdecl abort(void);
 __declspec(noreturn) void   __cdecl exit(int);



#line 266 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"



#line 270 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
        int    __cdecl abs(int);
#line 272 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
        int    __cdecl atexit(void (__cdecl *)(void));
 double __cdecl atof(const char *);
 int    __cdecl atoi(const char *);
 long   __cdecl atol(const char *);



 void * __cdecl bsearch(const void *, const void *, size_t, size_t,
        int (__cdecl *)(const void *, const void *));
 void * __cdecl calloc(size_t, size_t);
 div_t  __cdecl div(int, int);
 void   __cdecl free(void *);
 char * __cdecl getenv(const char *);
 char * __cdecl _itoa(int, char *, int);

 char * __cdecl _i64toa(__int64, char *, int);
 char * __cdecl _ui64toa(unsigned __int64, char *, int);
 __int64 __cdecl _atoi64(const char *);
#line 291 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"


#line 294 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
        long __cdecl labs(long);
#line 296 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
 ldiv_t __cdecl ldiv(long, long);
 char * __cdecl _ltoa(long, char *, int);
 void * __cdecl malloc(size_t);
 int    __cdecl mblen(const char *, size_t);
 size_t __cdecl _mbstrlen(const char *s);
 int    __cdecl mbtowc(wchar_t *, const char *, size_t);
 size_t __cdecl mbstowcs(wchar_t *, const char *, size_t);
 void   __cdecl qsort(void *, size_t, size_t, int (__cdecl *)
        (const void *, const void *));
 int    __cdecl rand(void);
 void * __cdecl realloc(void *, size_t);
 int    __cdecl _set_error_mode(int);
 void   __cdecl srand(unsigned int);
 double __cdecl strtod(const char *, char **);
 long   __cdecl strtol(const char *, char **, int);



 unsigned long __cdecl strtoul(const char *, char **, int);

 int    __cdecl system(const char *);
#line 318 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
 char * __cdecl _ultoa(unsigned long, char *, int);
 int    __cdecl wctomb(char *, wchar_t);
 size_t __cdecl wcstombs(char *, const wchar_t *, size_t);







 wchar_t * __cdecl _itow (int, wchar_t *, int);
 wchar_t * __cdecl _ltow (long, wchar_t *, int);
 wchar_t * __cdecl _ultow (unsigned long, wchar_t *, int);
 double __cdecl wcstod(const wchar_t *, wchar_t **);
 long   __cdecl wcstol(const wchar_t *, wchar_t **, int);
 unsigned long __cdecl wcstoul(const wchar_t *, wchar_t **, int);
 wchar_t * __cdecl _wgetenv(const wchar_t *);
 int    __cdecl _wsystem(const wchar_t *);
 int __cdecl _wtoi(const wchar_t *);
 long __cdecl _wtol(const wchar_t *);

 wchar_t * __cdecl _i64tow(__int64, wchar_t *, int);
 wchar_t * __cdecl _ui64tow(unsigned __int64, wchar_t *, int);
 __int64   __cdecl _wtoi64(const wchar_t *);
#line 343 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"


#line 346 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
#line 347 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"




 char * __cdecl _ecvt(double, int, int *, int *);

 __declspec(noreturn) void   __cdecl _exit(int);


#line 357 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
 char * __cdecl _fcvt(double, int, int *, int *);
 char * __cdecl _fullpath(char *, const char *, size_t);
 char * __cdecl _gcvt(double, int, char *);
        unsigned long __cdecl _lrotl(unsigned long, int);
        unsigned long __cdecl _lrotr(unsigned long, int);

 void   __cdecl _makepath(char *, const char *, const char *, const char *,
        const char *);
#line 366 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
        _onexit_t __cdecl _onexit(_onexit_t);
 void   __cdecl perror(const char *);
 int    __cdecl _putenv(const char *);
        unsigned int __cdecl _rotl(unsigned int, int);
        unsigned int __cdecl _rotr(unsigned int, int);
 void   __cdecl _searchenv(const char *, const char *, char *);

 void   __cdecl _splitpath(const char *, char *, char *, char *, char *);
#line 375 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
 void   __cdecl _swab(char *, char *, int);






 wchar_t * __cdecl _wfullpath(wchar_t *, const wchar_t *, size_t);
 void   __cdecl _wmakepath(wchar_t *, const wchar_t *, const wchar_t *, const wchar_t *,
        const wchar_t *);
 void   __cdecl _wperror(const wchar_t *);
 int    __cdecl _wputenv(const wchar_t *);
 void   __cdecl _wsearchenv(const wchar_t *, const wchar_t *, wchar_t *);
 void   __cdecl _wsplitpath(const wchar_t *, wchar_t *, wchar_t *, wchar_t *, wchar_t *);


#line 392 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
#line 393 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"




 void __cdecl _seterrormode(int);
 void __cdecl _beep(unsigned, unsigned);
 void __cdecl _sleep(unsigned long);
#line 401 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"


#line 404 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"







 int __cdecl tolower(int);
#line 413 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

 int __cdecl toupper(int);
#line 416 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

#line 418 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"











#line 430 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"





 char * __cdecl ecvt(double, int, int *, int *);
 char * __cdecl fcvt(double, int, int *, int *);
 char * __cdecl gcvt(double, int, char *);
 char * __cdecl itoa(int, char *, int);
 char * __cdecl ltoa(long, char *, int);
        _onexit_t __cdecl onexit(_onexit_t);
 int    __cdecl putenv(const char *);
 void   __cdecl swab(char *, char *, int);
 char * __cdecl ultoa(unsigned long, char *, int);

#line 446 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

#line 448 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"







#pragma pack(pop)
#line 457 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"

#line 459 "D:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\include\\stdlib.h"
#line 3 "test.sqc"

#line 3
/*
EXEC SQL INCLUDE sqlca;
*/
#line 3
 

int main()
{
    
#line 7
/*
EXEC SQL BEGIN DECLARE SECTION;
*/
#line 7
 
		
		
                char serverName[20];
                char userName[10];
                char sno[5]; 
		char sname[10]; 
		int score; 
		char subject[10]; 
                int count;
               

    
#line 19
/*
EXEC SQL END DECLARE SECTION;
*/
#line 19
 
    printf("请输入服务器名称.数据库:  ");
    gets(serverName);
    printf("请输入登录用户名.密码:  ");
    gets(userName);
    
    
#line 25
/*
EXEC SQL CONNECT TO :serverName USER :userName;
*/
#line 25

#line 25
{
#line 25
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 25
	sqlaaloc(2, 2, 1, (void far *)0);
#line 25
	sqlasetv(2, 0, 462, (short) SQLLENMAX(sizeof(serverName)), (void far *)serverName, (void far *)0, (void far *)0L);
#line 25
	sqlasetv(2, 1, 462, (short) SQLLENMAX(sizeof(userName)), (void far *)userName, (void far *)0, (void far *)0L);
#line 25
	sqlxcall(30, 1, 2, 0, 42, (char far *)"  CONNECT TO @p1          USER @p2        ");
#line 25
	SQLCODE = sqlca->sqlcode;
#line 25
	sqlastop((void far *)0L);
#line 25
}
#line 26 
    if (SQLCODE == 0)
    {
        printf("SQL Server 连接成功!\n");
    }
    else
    {
        
        printf("ERROR: SQL Server连接失败!\n");
        return (1);
    }
        
#line 36
/*
EXEC SQL create table Score (
	sno 	char(5)  	primary key,
	sname 	char(10)  	not null,
	score int,
	subject	char(10)
	);
*/
#line 41

#line 36
{
#line 36
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 36
	sqlxcall(24, 2, 0, 0, 103, (char far *)"  create table Score ( sno char(5) primary key, sname char(10) not null, score int, subject char(10) ) ");
#line 36
	SQLCODE = sqlca->sqlcode;
#line 36
	sqlastop((void far *)0L);
#line 36
}
#line 42 
	
#line 42
/*
EXEC SQL insert into Score values('S1','学生A','70','数据库');
*/
#line 42

#line 42
{
#line 42
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 42
	sqlxcall(24, 3, 0, 0, 55, (char far *)"  insert into Score values('S1','学生A','70','数据库') ");
#line 42
	SQLCODE = sqlca->sqlcode;
#line 42
	sqlastop((void far *)0L);
#line 42
}
#line 43
	
#line 43
/*
EXEC SQL insert into Score values('S2','学生B','89','操作系统');
*/
#line 43

#line 43
{
#line 43
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 43
	sqlxcall(24, 4, 0, 0, 57, (char far *)"  insert into Score values('S2','学生B','89','操作系统') ");
#line 43
	SQLCODE = sqlca->sqlcode;
#line 43
	sqlastop((void far *)0L);
#line 43
}
#line 44
	
#line 44
/*
EXEC SQL insert into Score values('S3','学生C','90','数据库');
*/
#line 44

#line 44
{
#line 44
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 44
	sqlxcall(24, 5, 0, 0, 55, (char far *)"  insert into Score values('S3','学生C','90','数据库') ");
#line 44
	SQLCODE = sqlca->sqlcode;
#line 44
	sqlastop((void far *)0L);
#line 44
}
#line 45
	
#line 45
/*
EXEC SQL insert into Score values('S4','学生D','98','数据库');
*/
#line 45

#line 45
{
#line 45
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 45
	sqlxcall(24, 6, 0, 0, 55, (char far *)"  insert into Score values('S4','学生D','98','数据库') ");
#line 45
	SQLCODE = sqlca->sqlcode;
#line 45
	sqlastop((void far *)0L);
#line 45
}
#line 46
	
#line 46
/*
EXEC SQL insert into Score values('S5','学生E','77','操作系统');
*/
#line 46

#line 46
{
#line 46
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 46
	sqlxcall(24, 7, 0, 0, 57, (char far *)"  insert into Score values('S5','学生E','77','操作系统') ");
#line 46
	SQLCODE = sqlca->sqlcode;
#line 46
	sqlastop((void far *)0L);
#line 46
}
#line 47
	
#line 47
/*
EXEC SQL insert into Score values('S6','学生F','88','操作系统');
*/
#line 47

#line 47
{
#line 47
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 47
	sqlxcall(24, 8, 0, 0, 57, (char far *)"  insert into Score values('S6','学生F','88','操作系统') ");
#line 47
	SQLCODE = sqlca->sqlcode;
#line 47
	sqlastop((void far *)0L);
#line 47
}
#line 48
        
	
#line 49
/*
EXEC SQL DECLARE providerCursor CURSOR FOR
	SELECT 	sno,sname,subject,score
	FROM 	Score;
*/
#line 51

        
	
#line 53
/*
EXEC SQL OPEN providerCursor ;
*/
#line 53

#line 53
{
#line 53
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 53
	sqlxcall(26, 9, 0, 0, 74, (char far *)"/* providerCursor 9 nohold */ SELECT 	sno,sname,subject,score FROM 	Score ");
#line 53
	SQLCODE = sqlca->sqlcode;
#line 53
	sqlastop((void far *)0L);
#line 53
}
#line 54
        
	for(count=0;count<6;count++){ 
		
		
#line 57
/*
EXEC SQL FETCH providerCursor INTO :sno,:sname,:subject,:score;
*/
#line 57

#line 57
{
#line 57
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 57
	sqlaaloc(1, 4, 9, (void far *)0);
#line 57
	sqlasetv(1, 0, 462,(short) SQLLENMAX(sizeof(sno)),(void far *)&sno, (void far *)0,0L);
#line 57
	sqlasetv(1, 1, 462,(short) SQLLENMAX(sizeof(sname)),(void far *)&sname, (void far *)0,0L);
#line 57
	sqlasetv(1, 2, 462,(short) SQLLENMAX(sizeof(subject)),(void far *)&subject, (void far *)0,0L);
#line 57
	sqlasetv(1, 3, 496,(short) SQLLENMAX(sizeof(score)),(void far *)&score, (void far *)0,0L);
#line 57
	sqlxcall(25, 9, 0, 1, 60, (char far *)"  FETCH providerCursor INTO :    ,:      ,:        ,:       ");
#line 57
	SQLCODE = sqlca->sqlcode;
#line 57
	sqlastop((void far *)0L);
#line 57
}
#line 58
		
                printf ("学号:%s",sno);
		printf ("姓名:%s",sname);
		printf ("科目:%s",subject);
		printf ("成绩:%d\n",score);
	}
	
         
	
#line 66
/*
EXEC SQL CLOSE providerCursor;
*/
#line 66

#line 66
{
#line 66
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 66
	sqlxcall(20, 9, 0, 0, 23, (char far *)"  CLOSE providerCursor ");
#line 66
	SQLCODE = sqlca->sqlcode;
#line 66
	sqlastop((void far *)0L);
#line 66
}
#line 67 
	
	
        
#line 69
/*
EXEC SQL DISCONNECT ALL;
*/
#line 69

#line 69
{
#line 69
	sqlastrt((void far *)pid, (void far *)0, (struct tag_sqlca far *)sqlca);
#line 69
	sqlxcall(36, 10, 0, 0, 17, (char far *)"  DISCONNECT ALL ");
#line 69
	SQLCODE = sqlca->sqlcode;
#line 69
	sqlastop((void far *)0L);
#line 69
}
#line 70
	
	printf("按任意键退出程序\n");
        while((--((&_iob[0]))->_cnt >= 0 ? 0xff & *((&_iob[0]))->_ptr++ : _filbuf((&_iob[0]))))
        return 0;  
}

long SQLCODE;

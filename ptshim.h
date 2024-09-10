/*=======================================================================
 *	Support for shim libraries to intercept / log function calls
 *=======================================================================*/
#ifndef _ptshim_h
#define _ptshim_h

#include <stdlib.h>

typedef struct S_Ptshim *Ptshim;

/*--- Get handle to library and functions therein */
Ptshim
ptshim_redirect(const char *sLibName, /* sFuncName, ppfFunc */...);

/*--- Get handle to library */
Ptshim
ptshim_library(const char *sLibName);

/*--- Get a function */
void *
ptshim_function(Ptshim shim, const char *sFuncName);

/* Alternatively, get function on demand.  Define PTSHIM_LIBRARY to Ptshim instance */
#define PTSHIM_REALFUNC(s) real_ ## s
#define PTSHIM_INTERCEPT(r,s,a) static r (*PTSHIM_REALFUNC(s))a; r s a
#define PTSHIM_FINDFUNC(s) do {if (PTSHIM_REALFUNC(s)==0) {*(void**)&PTSHIM_REALFUNC(s) = ptshim_function(PTSHIM_LIBRARY, #s);}} while (0)

#endif /* _ptshim_h */

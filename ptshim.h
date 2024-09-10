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

#endif /* _ptshim_h */

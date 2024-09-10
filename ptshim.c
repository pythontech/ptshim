/*=======================================================================
 *	Support for shim library logging
 *=======================================================================*/
#include "ptshim.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dlfcn.h>

#if __sun__
#define STRDUP(s) strcpy(malloc(strlen(s)+1),s)
#else
#define STRDUP(s) strdup(s)
#endif

#define PTSHIM_DEFAULT_FD 9

/*-----------------------------------------------------------------------
 *	Private structure
 *-----------------------------------------------------------------------*/
struct S_Ptshim {
  char *sLibName;
  void *pDl;				/* handle returned by dlopen */
};

/*--- Get handle to library and functions therein */
Ptshim
ptshim_redirect(const char *sLibName, /* sFuncName, ppfFunc */...)
{
    Ptshim pSelf = ptshim_library(sLibName);

  {
      /*--- Look up functions */
      va_list ap;
      const char *sFuncName;
      va_start(ap, sLibName);
      while ((sFuncName = va_arg(ap, const char *)) != NULL) {
          void **ppFunc = va_arg(ap, void **);
          void *pFunc = ptshim_function(pSelf, sFuncName);
          /*--- Return function pointer to caller */
          *ppFunc = pFunc;
      }
  }
  return pSelf;
}

Ptshim
ptshim_library(const char *sLibName)
{
    Ptshim pShim;
    void *pDl = dlopen(sLibName,
                       RTLD_LAZY | RTLD_GLOBAL
#if __sun__
                       | RTLD_PARENT
#endif
                       );
    if (pDl == NULL) {
        fprintf(stderr, "Unable to dlopen %s: %s\n",
                sLibName, dlerror());
        abort();
    }

    pShim = (Ptshim)malloc(sizeof(*pShim));
    pShim->sLibName = STRDUP(sLibName);
    pShim->pDl = pDl;

    return pShim;
}

void *
ptshim_function(Ptshim pSelf, const char *sFuncName)
{
    void *pFunc = dlsym(pSelf->pDl, sFuncName);
    if (pFunc == NULL) {
        fprintf(stderr, "Could not find symbol %s in %s",
                sFuncName, pSelf->sLibName);
        abort();
    }
    return pFunc;
}

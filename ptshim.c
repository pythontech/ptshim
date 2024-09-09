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


/*--- Process-wide data */
static const char *sLibraryPath = NULL;
static FILE *pLog = NULL;
static int nNesting = 0;

static void
_ptshim_init(void)
{
  if (pLog == NULL) {
    const char *sLogName = getenv("PTSHIM_LOGFILE");
    if (sLogName != NULL) {
      pLog = fopen(sLogName, "a");
      if (pLog == NULL) {
	fprintf(stderr, "ptshim: Cannot open PTSHIM_LOGFILE %s for append: %s\n",
		sLogName, strerror(errno));
	abort();
      }
      fprintf(pLog, "--START--\n");
    } else {
      if (write(PTSHIM_DEFAULT_FD, "--START--\n", 10) < 0) {
	fprintf(stderr, "ptshim: cannot write to fd %d: %s\n",
		PTSHIM_DEFAULT_FD, strerror(errno));
	abort();
      } else {
	pLog = fdopen(PTSHIM_DEFAULT_FD, "a");
	if (pLog == NULL) {
	  fprintf(stderr, "ptshim: Cannot write log to fd %d: %s\n",
		  PTSHIM_DEFAULT_FD, strerror(errno));
	  abort();
	}
      }
    }

    sLibraryPath = getenv("LD_LIBRARY_PATH");
#if PTSHIM_REQUIRE_PATH
    if (sLibraryPath == NULL) {
      fprintf(stderr, "ptshim: LD_LIBRARY_PATH not set\n");
      abort();
    }
#endif
  }
}

/*--- Get handle to library and functions therein */
Ptshim
ptshim_redirect(const char *sLibName, /* sFuncName, ppfFunc */...)
{
  Ptshim pSelf = NULL;

  if (pLog == NULL) _ptshim_init();

  pSelf = ptshim_library(sLibName);
  fprintf(pLog, "--+ dlopen(\"%s\")\n", sLibName);
  fflush(pLog);

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

/*--- Start function call - push arguments */
void
ptshim_inputs(const char *sFuncName)
{
  if (nNesting > 0) {
    fprintf(pLog, "\n%*s", (nNesting+1)*2, "");
  }
  nNesting ++;
  fprintf(pLog, "%s(", sFuncName);
}

/*--- Result value follows */
void
ptshim_call(void)
{
  fprintf(pLog, ")");
}

/*--- Output values follow */
void
ptshim_outputs(void)
{
  fprintf(pLog, " ");
}

/*--- Call complete */
void
ptshim_done(void)
{
  fprintf(pLog, "\n");
  fflush(pLog);
  nNesting --;
}


/*--- Log inputs, outputs etc. */
void
ptshim_int(const char *sName, int iValue)
{
  fprintf(pLog, " %s=i4:%d", sName, iValue);
}

void
ptshim_short(const char *sName, short iValue)
{
  fprintf(pLog, " %s=i2:%d", sName, iValue);
}

void
ptshim_float(const char *sName, float rValue)
{
  fprintf(pLog, " %s=r4:%g", sName, rValue);
}

void
ptshim_double(const char *sName, double rValue)
{
  fprintf(pLog, " %s=r8:%g", sName, rValue);
}

void
ptshim_string(const char *sName, const char *sValue)
{
  fprintf(pLog, " %s=z:\"%s\"", sName, sValue);
}

void
ptshim_fstring(const char *sName, const char *sValue, size_t lSize)
{
  fprintf(pLog, " %s=s:\"%.*s\"", sName, (int)lSize, sValue);
}

void
ptshim_binary(const char *sName, const char *sType,
	   const void *pData, size_t lSize)
{
  ptshim_binaryn(sName, sType, pData, lSize, 1);
}

void
ptshim_binaryn(const char *sName, const char *sType,
	    const void *pData, size_t lSize, size_t lGroup)
{
  int i;
  if (pData == NULL) {
    fprintf(pLog, " %s=NULL", sName);
  } else {
    if (sType && sType[0]) {
      fprintf(pLog, " %s=b(%s):{", sName, sType);
    } else {
      fprintf(pLog, " %s=b:{", sName);
    }
    for (i=0; i<lSize; i++) {
      fprintf(pLog, "%s%02x", " "+(i==0 || i%lGroup != 0),
	      ((const unsigned char *)pData)[i]);
    }
    fprintf(pLog, "}");
  }
}

void
ptshim_ptr(const char *sName, const void *pPtr)
{
  fprintf(pLog, " %s=p:%p", sName, pPtr);
}

void
ptshim_voidfunc(const char *sName, void (*pfFunc)(void))
{
  fprintf(pLog, " %s=f:%p", sName, pfFunc);
}

void
ptshim_errorstack(void)
{
  /*--- FIXME */
}

void
ptshim_comment(const char *sFmt, ...)
{
  va_list ap;
  va_start(ap, sFmt);
  fprintf(pLog, "// ");
  vfprintf(pLog, sFmt, ap);
  fprintf(pLog, "\n");
  va_end(ap);
}

void
ptshim_sync(const char *sName, void *pRef, void *pA, void *pB, size_t lBytes)
{
  int i;
  char *pCRef = pRef, *pCA = pA, *pCB = pB;
  for (i=0; i<lBytes; i++) {
    if (pCA[i] != pCRef[i] || pCB[i] != pCRef[i]) {
      fprintf(pLog, "--sync: %s[%d] ref=%d a=%d b=%d\n",
	      sName, i, pCRef[i], pCA[i], pCB[i]);
    }
    if (pCA[i] != pCRef[i]) {
      pCB[i] = pCRef[i] = pCA[i];
    } else if (pCB[i] != pCRef[i]) {
      pCB[i] = pCRef[i] = pCA[i];
    }
  }
}

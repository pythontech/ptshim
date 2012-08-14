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

#define STRDUP(s) strcpy(malloc(strlen(s)+1),s)

#define PTSHIM_DEFAULT_FD 9

/*-----------------------------------------------------------------------
 *	Private structure
 *-----------------------------------------------------------------------*/
struct S_Ptshim {
  char *sLibName;
  char *sLibraryFilename;
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
    if (sLibraryPath == NULL) {
      fprintf(stderr, "ptshim: LD_LIBRARY_PATH not set\n");
      abort();
    }
  }
}

/*--- Get handle to library and functions therein */
Ptshim
ptshim_redirect(const char *sLibName, /* sFuncName, ppfFunc */...)
{
  Ptshim pSelf = NULL;
  int nFound;
  char *sDir, *sLasts;
  char acFilename[1024];
  char *sLibraryFilename = NULL;
  char *sPathCopy;

  if (pLog == NULL) _ptshim_init();

  /*--- Take a copy for strtok chopping */
  sPathCopy = STRDUP(sLibraryPath);

  for (sDir = strtok_r(sPathCopy, ":", &sLasts), nFound = 0;
       sDir && nFound < 2;
       sDir = strtok_r(NULL, ":", &sLasts)) {
    if (sDir[0] != '\0') {		/* Ignore xyz::abc */
      struct stat qStat;

      sprintf(acFilename, "%s/%s", sDir, sLibName);
      if (stat(acFilename, &qStat) == 0) {
	/*--- FIXME: check permissions? */
	++nFound;
	if (nFound == 2) {
	  sLibraryFilename = STRDUP(acFilename);
	}
      }
    }
  }
  if (nFound == 0) {
    fprintf(stderr, "%s not found in LD_LIBRARY_PATH\n",
	    sLibName);
    abort();
  } else if (nFound == 1) {
    fprintf(stderr, "Only one %s found in LD_LIBRARY_PATH\n",
	    sLibName);
    abort();
  } else {
    /*--- Open library but do not merge with global namespace */
    void *pDl = dlopen(sLibraryFilename,
#if 1
		       RTLD_LAZY | RTLD_GLOBAL | RTLD_PARENT
#else
		       RTLD_LAZY | RTLD_LOCAL | RTLD_PARENT
#endif
		       );
    fprintf(pLog, "--+ dlopen(\"%s\")\n", sLibraryFilename);
    fflush(pLog);
    if (pDl == NULL) {
      fprintf(stderr, "Unable to dlopen %s: %s\n",
	      sLibraryFilename, dlerror());
      abort();
    } else {
      /*--- Look up functions */
      va_list ap;
      const char *sFuncName;
      va_start(ap, sLibName);
      while ((sFuncName = va_arg(ap, const char *)) != NULL) {
	void **ppFunc = va_arg(ap, void **);
	void *pFunc = dlsym(pDl, sFuncName);
	if (pFunc == NULL) {
	  fprintf(stderr, "Could not find symbol %s in %s",
		  sFuncName, sLibraryFilename);
	  abort();
	} else {
	  /*--- Return function pointer to caller */
	  *ppFunc = pFunc;
	}
      }

      pSelf = (Ptshim)malloc(sizeof(*pSelf));
      pSelf->sLibName = STRDUP(sLibName);
      pSelf->sLibraryFilename = sLibraryFilename;
      pSelf->pDl = pDl;
    }
  }
  free(sPathCopy);
  return pSelf;
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

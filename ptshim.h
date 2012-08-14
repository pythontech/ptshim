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


/*--- Start function call - push arguments */
void
ptshim_inputs(const char */*sFuncName*/);

/*--- Call happens now */
void
ptshim_call(void);

/*--- Output values follow */
void
ptshim_outputs(void);

/*--- Call complete */
void
ptshim_done(void);


/*--- Log inputs, outputs etc. */
extern void
ptshim_int(const char */*sName*/, int /*iValue*/);

extern void
ptshim_short(const char */*sName*/, short /*iValue*/);

extern void
ptshim_float(const char */*sName*/, float /*rValue*/);

extern void
ptshim_double(const char */*sName*/, double /*rValue*/);

extern void
ptshim_string(const char */*sName*/, const char */*sValue*/);

extern void
ptshim_fstring(const char */*sName*/, const char */*sValue*/, size_t);

extern void
ptshim_binary(const char */*sName*/, const char */*sType*/,
	      const void */*pData*/, size_t);

extern void
ptshim_binaryn(const char */*sName*/, const char */*sType*/,
	       const void */*pData*/, size_t, size_t /*lGroup*/);

#define ptshim_binary2(n,t,p,l) ptshim_binaryn(n,t,p,l,2)
#define ptshim_binary4(n,t,p,l) ptshim_binaryn(n,t,p,l,4)

extern void
ptshim_ptr(const char */*sName*/, const void */*pPtr*/);

extern void
ptshim_voidfunc(const char */*sName*/, void (*/*pfFunc*/)(void));

#define ptshim_func(n,f) ptshim_voidfunc(n, (void(*)(void))(f))

extern void
ptshim_errorstack(void);

extern void
ptshim_comment(const char */*sFmt*/, ...);

extern void
ptshim_sync(const char *sName,
	 void */*pRef*/, void */*pA*/, void */*pB*/, size_t /*lBytes*/);

#endif /* _ptshim_h */

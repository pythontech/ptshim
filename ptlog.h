/*=======================================================================
 *      Support for logging from (e.g.) shim libraries
 *=======================================================================*/
#ifndef INC_ptlog_h
#define INC_ptlog_h

#include <stdlib.h>

void ptlog_init(void);
void ptlog_inputs(const char */*funcname*/);
void ptlog_call(void);
void ptlog_outputs(void);
void ptlog_done(void);

void ptlog_int(const char */*name*/, int /*value*/);
void ptlog_short(const char */*name*/, short /*value*/);
void ptlog_float(const char */*name*/, float /*value*/);
void ptlog_double(const char */*name*/, double /*value*/);
void ptlog_string(const char */*sname*/, const char */*value*/);
void ptlog_fstring(const char */*sname*/, const char */*value*/, size_t /*size*/);
void ptlog_binary(const char */*name*/, const char */*type*/,
                  const void */*data*/, size_t /*size*/);
void ptlog_binaryn(const char */*name*/, const char */*type*/,
                   const void */*data*/, size_t /*size*/, size_t /*group*/);
#define ptlog_binary2(_n,_t,_p,_l) ptlog_binaryn(_n,_t,_p,_l,2)
#define ptlog_binary4(_n,_t,_p,_l) ptlog_binaryn(_n,_t,_p,_l,4)
void ptlog_ptr(const char */*name*/, const void */*ptr*/);
void ptlog_voidfunc(const char */*name*/, void (*/*func*/)(void));
void ptlog_comment(const char *sFmt, ...);

#endif /* INC_ptlog_h */

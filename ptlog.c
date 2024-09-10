/*=======================================================================
 *      Logging form shim library - implementation
 *=======================================================================*/
#include "ptlog.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PTLOG_DEFAULT_FD 9

static struct {
    FILE *log;
    int nesting;
} ptlog = {
    .log = NULL,
    .nesting = 0,
};

/* Initialise */
void
ptlog_init(void)
{
    if (ptlog.log != NULL) {           /* Already set up */
        ;
    } else {
        const char *log_name = getenv("PTLOG_FILE");
        if (log_name != NULL) {
            ptlog.log = fopen(log_name, "a");
            if (ptlog.log == NULL) {
                fprintf(stderr, "ptlog: Cannot open PTLOG_FILE %s for append: %s\n",
                        log_name, strerror(errno));
                abort();
            }
            fprintf(ptlog.log, "--START--\n");
        } else {
            if (write(PTLOG_DEFAULT_FD, "--START--\n", 10) < 0) {
                fprintf(stderr, "ptshim: cannot write to fd %d: %s\n",
                        PTLOG_DEFAULT_FD, strerror(errno));
                abort();
            } else {
                ptlog.log = fdopen(PTLOG_DEFAULT_FD, "a");
                if (ptlog.log == NULL) {
                    fprintf(stderr, "ptlog: Cannot write log to fd %d: %s\n",
                            PTLOG_DEFAULT_FD, strerror(errno));
                    abort();
                }
            }
        }
    }
}

#define PTLOG_INIT() do {if (! ptlog.log) ptlog_init();} while (0)

/*--- Start function call - push arguments */
void
ptlog_inputs(const char *funcname)
{
    PTLOG_INIT();
    if (ptlog.nesting > 0) {
        fprintf(ptlog.log, "\n%*s", (ptlog.nesting+1)*2, "");
    }
    ptlog.nesting ++;
    fprintf(ptlog.log, "%s(", funcname);
}

/*--- Result value follows */
void
ptlog_call(void)
{
    PTLOG_INIT();
    fprintf(ptlog.log, ")");
}

/*--- Output values follow */
void
ptlog_outputs(void)
{
    PTLOG_INIT();
    fprintf(ptlog.log, " ");
}

/*--- Call complete */
void
ptlog_done(void)
{
    PTLOG_INIT();
    fprintf(ptlog.log, "\n");
    fflush(ptlog.log);
    ptlog.nesting --;
}


/*--- Log inputs, outputs etc. */
void
ptlog_int(const char *name, int value)
{
    fprintf(ptlog.log, " %s=i4:%d", name, value);
}

void
ptlog_short(const char *name, short value)
{
    fprintf(ptlog.log, " %s=i2:%d", name, value);
}

void
ptlog_float(const char *name, float value)
{
    fprintf(ptlog.log, " %s=r4:%g", name, value);
}

void
ptlog_double(const char *name, double value)
{
    fprintf(ptlog.log, " %s=r8:%g", name, value);
}

void
ptlog_string(const char *name, const char *value)
{
    fprintf(ptlog.log, " %s=z:\"%s\"", name, value);
}

void
ptlog_fstring(const char *name, const char *value, size_t size)
{
    fprintf(ptlog.log, " %s=s:\"%.*s\"", name, (int)size, value);
}

void
ptlog_binary(const char *name, const char *type,
             const void *data, size_t size)
{
    ptlog_binaryn(name, type, data, size, 1);
}

void
ptlog_binaryn(const char *name, const char *type,
              const void *data, size_t size, size_t group)
{
    int i;
    if (data == NULL) {
        fprintf(ptlog.log, " %s=NULL", name);
    } else {
        if (type && type[0]) {
            fprintf(ptlog.log, " %s=b(%s):{", name, type);
        } else {
            fprintf(ptlog.log, " %s=b:{", name);
        }
        for (i=0; i<size; i++) {
            fprintf(ptlog.log, "%s%02x", " "+(i==0 || i%group != 0),
                    ((const unsigned char *)data)[i]);
        }
        fprintf(ptlog.log, "}");
    }
}

void
ptlog_ptr(const char *name, const void *pPtr)
{
    fprintf(ptlog.log, " %s=p:%p", name, pPtr);
}

void
ptlog_voidfunc(const char *name, void (*pfFunc)(void))
{
    fprintf(ptlog.log, " %s=f:%p", name, pfFunc);
}

void
ptlog_comment(const char *fmt, ...)
{
    va_list ap;
    PTLOG_INIT();
    va_start(ap, fmt);
    fprintf(ptlog.log, "// ");
    vfprintf(ptlog.log, fmt, ap);
    fprintf(ptlog.log, "\n");
    va_end(ap);
}

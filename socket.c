#include "ptshim.h"
#include "ptlog.h"
#include <sys/socket.h>
#include <sys/errno.h>

static Ptshim socket_shim;
#define PTSHIM_LIBRARY socket_shim

static void netlog_init(void)
{
    socket_shim = ptshim_library("libc.so.6");
}

#define INIT() do {if (! socket_shim) netlog_init();} while (0);

PTSHIM_INTERCEPT(int, socket, (int, int, int));
int
socket(int domain, int type, int protocol)
{
    INIT();
    PTSHIM_FINDFUNC(socket);
    ptlog_inputs("socket");
    ptlog_int("domain", domain);
    ptlog_int("type", type);
    ptlog_int("protocol", protocol);
    ptlog_call();
    int ret = PTSHIM_REALFUNC(socket)(domain, type, protocol);
    int save_errno = errno;
    ptlog_outputs();
    ptlog_done();
    errno = save_errno;
    return ret;
}

PTSHIM_INTERCEPT(int, connect, (int, const struct sockaddr *, socklen_t));
int
connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    INIT();
    PTSHIM_FINDFUNC(connect);
    ptlog_inputs("connect");
    ptlog_int("sockfd", sockfd);
    ptlog_binary("addr", "sockaddr", addr, addrlen);
    //ptlog_int("addrlen", (int)addrlen);
    ptlog_call();
    int ret = PTSHIM_REALFUNC(connect)(sockfd, addr, addrlen);
    int save_errno = errno;
    ptlog_outputs();
    if (ret < 0) {
        ptlog_int("errno", save_errno);
    } else {
        ptlog_int("ret", ret);
    }
    ptlog_done();
    errno = save_errno;
    return ret;
}

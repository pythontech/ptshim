#include "ptshim.h"
#include "ptlog.h"
#include <sys/socket.h>
#include <sys/errno.h>

static struct {
    Ptshim shim;
    int (*p_socket)(int domain, int type, int protocol);
    int (*p_connect)(int, const struct sockaddr *, socklen_t);
} _data = {0};

static void _init(void);
#define INIT() do {if (! _data.shim) _init();} while (0);

int
socket(int domain, int type, int protocol)
{
    INIT();
    ptlog_inputs("socket");
    ptlog_int("domain", domain);
    ptlog_int("type", type);
    ptlog_int("protocol", protocol);
    ptlog_call();
    int ret = _data.p_socket(domain, type, protocol);
    int save_errno = errno;
    ptlog_outputs();
    ptlog_done();
    errno = save_errno;
    return ret;
}

int
connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    INIT();
    ptlog_inputs("connect");
    ptlog_int("sockfd", sockfd);
    ptlog_binary("addr", "sockaddr", addr, addrlen);
    //ptlog_int("addrlen", (int)addrlen);
    ptlog_call();
    int ret = _data.p_connect(sockfd, addr, addrlen);
    int save_errno = errno;
    ptlog_outputs();
    ptlog_int("ret", ret);
    if (ret < 0) ptlog_int("errno", save_errno);
    ptlog_done();
    errno = save_errno;
    return ret;
}

static void
_init(void)
{
    _data.shim = ptshim_redirect("libc.so.6",
                                 "socket", &_data.p_socket,
                                 "connect", &_data.p_connect,
                                 NULL);
}

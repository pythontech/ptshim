#include "ptshim.h"
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
    ptshim_inputs("socket");
    ptshim_int("domain", domain);
    ptshim_int("type", type);
    ptshim_int("protocol", protocol);
    ptshim_call();
    int ret = _data.p_socket(domain, type, protocol);
    int save_errno = errno;
    ptshim_outputs();
    ptshim_done();
    errno = save_errno;
    return ret;
}

int
connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    INIT();
    ptshim_inputs("connect");
    ptshim_int("sockfd", sockfd);
    ptshim_binary("addr", "sockaddr", addr, addrlen);
    //ptshim_int("addrlen", (int)addrlen);
    ptshim_call();
    int ret = _data.p_connect(sockfd, addr, addrlen);
    int save_errno = errno;
    ptshim_outputs();
    ptshim_int("ret", ret);
    if (ret < 0) ptshim_int("errno", save_errno);
    ptshim_done();
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

#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

int sendmsg(int s, const struct msghdr *msg, int flags)
{ struct SocketSettings *lss;
  StdFileDes *fp = _lx_fhfromfd(s);
  int rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      rc = SOCK_sendmsg(fp->lx_sock,msg,flags);
    break;

    case LX_AMITCP:
      rc = TCP_SendMsg(fp->lx_sock,msg,flags);
    break;

    default:
      rc = -1;
    break;
  }

  return rc;
}

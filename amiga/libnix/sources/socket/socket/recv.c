#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

int recv(int s, void *buf, size_t len, int flags)
{ struct SocketSettings *lss;
  StdFileDes *fp = _lx_fhfromfd(s);
  int rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      rc = SOCK_recv(fp->lx_sock,buf,len,flags);
    break;

    case LX_AMITCP:
      rc = TCP_Recv(fp->lx_sock,buf,len,flags);
    break;

    default:
      rc = -1;
    break;
  }

  return rc;
}

#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

int shutdown(int s, int how)
{ struct SocketSettings *lss;
  StdFileDes *fp = _lx_fhfromfd(s);
  int rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      rc = SOCK_shutdown(fp->lx_sock,how);
    break;

    case LX_AMITCP:
      rc = TCP_ShutDown(fp->lx_sock,how);
    break;

    default:
      rc = 0;
    break;
  }

  return rc;
}

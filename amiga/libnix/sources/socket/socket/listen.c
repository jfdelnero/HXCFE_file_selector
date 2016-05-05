#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

int listen(int s, int backlog)
{ struct SocketSettings *lss;
  register StdFileDes *fp;
  int rc;

  if ((fp=_lx_fhfromfd(s))) {
    switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
      case LX_AS225:
        rc = SOCK_listen(fp->lx_sock, backlog);
      break;

      case LX_AMITCP:
        rc = TCP_Listen(fp->lx_sock, backlog);
      break;

      default:
        rc = -1;
      break;
    }
  }
  else rc = -1;

  return rc;
}

#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

int setsockopt(int s, int level, int name, const void *val, int valsize)
{ struct SocketSettings *lss = _lx_get_socket_settings();
  StdFileDes *fp = _lx_fhfromfd(s);
  int rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      rc = SOCK_setsockopt(fp->lx_sock,level,name,val, valsize);
    break;

    case LX_AMITCP:
      rc = TCP_SetSockOpt(fp->lx_sock,level,name,val, valsize);
    break;

    default:
      rc = 0;
    break;
  }

  return rc;
}

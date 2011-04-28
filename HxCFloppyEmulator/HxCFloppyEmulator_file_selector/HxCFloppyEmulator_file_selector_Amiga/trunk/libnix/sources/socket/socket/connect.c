#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

int connect(int s, const struct sockaddr *name, int namelen)
{ struct SocketSettings *lss;
  StdFileDes *fp = _lx_fhfromfd(s);
  int oldlen,rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      oldlen = name->sa_len;
      ((struct sockaddr *)name)->sa_len = 0;
      rc = SOCK_connect(fp->lx_sock, name,namelen);
      ((struct sockaddr *)name)->sa_len = oldlen;
    break;

    case LX_AMITCP:
      rc = TCP_Connect(fp->lx_sock, name,namelen);
    break;

    default:
      rc = -1;
    break;
  }

  return rc;
}

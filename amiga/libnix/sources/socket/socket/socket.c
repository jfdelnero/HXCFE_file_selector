#include <sys/types.h>
#include <sys/socket.h>
//
#include "debuglib.h"
#include "socket.h"

int socket(int domain, int type, int protocol)
{ struct SocketSettings *lss;
  StdFileDes *fp;
  int rc;

  if ((fp=_create_socket(domain, type, protocol)) == NULL)
    return -1;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      rc = SOCK_socket(domain, type, protocol);
    break;

    case LX_AMITCP:
      rc = TCP_Socket(domain, type, protocol);
    break;

    default:
      rc = -1;
    break;
  }
  DB( BUG("Return from net_socket is %ld\n", rc); )

  if (rc >= 0) {
    fp->lx_sock = rc; rc = fp->lx_pos;
  }
  else {
    fp->lx_inuse = 0;
  }

  DB( BUG("Return fd from socket() is %ld, inuse is %ld\n", fp->lx_pos, fp->lx_inuse); )
  return rc;
}

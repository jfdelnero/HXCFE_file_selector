#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

int accept(int s, struct sockaddr *name, int *namelen)
{ struct SocketSettings *lss = _lx_get_socket_settings();
  StdFileDes *fp,*fp2;
  int rc;

  if ((fp=_lx_fhfromfd(s)) == NULL)
    return NULL;

  if ((fp2=_create_socket(fp->lx_family, fp->lx_domain, fp->lx_protocol)) == NULL)
    return -1;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      rc = SOCK_accept(fp->lx_sock, name, namelen);
    break;

    case LX_AMITCP:
      rc = TCP_Accept(fp->lx_sock, name, namelen);
    break;

    default:
      rc = -1;
    break;
  }

  if (rc >= 0) {
    fp2->lx_sock = rc; rc = fp2->lx_pos;
  }
  else {
    fp2->lx_inuse = 0;
  }

  return rc;
}

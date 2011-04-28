#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

int sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr *to, int tolen)
{ struct SocketSettings *lss;
  StdFileDes *fp = _lx_fhfromfd(s);
  int oldlen,rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      oldlen = to->sa_len;
      ((struct sockaddr *)to)->sa_len = 0;
      rc = SOCK_sendto(fp->lx_sock,buf,len,flags,to,tolen);
      ((struct sockaddr *)to)->sa_len = oldlen;
    break;

    case LX_AMITCP:
      rc = TCP_SendTo(fp->lx_sock,buf,len,flags,to,tolen);
    break;

    default:
      rc = -1;
    break;
  }

  return rc;
}

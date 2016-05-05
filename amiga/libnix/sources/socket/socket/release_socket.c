#include <unistd.h>
//
#include "socket.h"

/* TODO: fix me */
int release_socket(int s)
{ struct SocketSettings *lss;
  /* TODO: Finish me */
  /* dup the socket first, since for AmiTCP, we can only release once */
  int s2 = dup(s);
  /*StdFileDes *fp = _lx_fhfromfd(s);*/
  int rc = -1;

  if (s2 >= 0) {
    switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
      case LX_AS225:
        rc = (int)SOCK_release(s2);
        SOCK_close(s2);
      break;

      case LX_AMITCP:
        rc = TCP_ReleaseSocket(s2, -1);
        TCP_CloseSocket(s2);
      break;

      default:
        /* silence compiler */
      break;
    }
  }

  return rc;
}

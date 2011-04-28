#include "socket.h"

int obtain_socket(long id, int inet, int stream, int protocol)
{ struct SocketSettings *lss;
  int rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      rc = SOCK_inherit((void *)id);
    break;

    case LX_AMITCP:
      rc = TCP_ObtainSocket(id, inet, stream, protocol);
    break;

    default:
      rc = -1;
    break;
  }

  return rc;
}

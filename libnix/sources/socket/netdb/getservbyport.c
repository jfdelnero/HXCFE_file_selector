#include <netdb.h>
//
#include "socket.h"

struct servent *getservbyport(int port, const char *proto)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  switch (lss->lx_network_type) {
    case LX_AS225:
      return SOCK_getservbyport(port,proto);

    case LX_AMITCP:
      return TCP_GetServByPort(port,proto);

    default:
      return NULL;
  }
}

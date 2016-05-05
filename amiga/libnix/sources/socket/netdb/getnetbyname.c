#include <netdb.h>
//
#include "socket.h"

struct netent *getnetbyname(const char *name)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return SOCK_getnetbyname(name);

    case LX_AMITCP:
      return TCP_GetNetByName(name);

    default:
      return NULL;
  }
}

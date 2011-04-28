#include <netdb.h>
//
#include "socket.h"

struct servent *getservbyname(const char *name, const char *proto)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return SOCK_getservbyname(name,proto);

    case LX_AMITCP:
      return TCP_GetServByName(name,proto);

    default:
      return NULL;
  }
}

#include <netdb.h>
//
#include "socket.h"

struct protoent *getprotobyname(const char *name)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return SOCK_getprotobyname(name);

    case LX_AMITCP:
      return TCP_GetProtoByName(name);

    default:
      return NULL;
  }
}

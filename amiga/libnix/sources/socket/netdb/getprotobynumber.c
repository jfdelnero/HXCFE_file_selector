#include <netdb.h>
//
#include "socket.h"

struct protoent *getprotobynumber(int proto)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return SOCK_getprotobynumber(proto);

    case LX_AMITCP:
      return TCP_GetProtoByNumber(proto);

    default:
      return NULL;
  }
}

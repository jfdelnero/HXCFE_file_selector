#include <netdb.h>
//
#include "socket.h"

struct netent *getnetbyaddr(long net, int type)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return SOCK_getnetbyaddr(net,type);

    case LX_AMITCP:
      return TCP_GetNetByAddr(net,type);

    default:
      return NULL;
  }
}

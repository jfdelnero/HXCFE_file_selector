#include <netdb.h>
//
#include "socket.h"

struct hostent *gethostbyaddr(const char *addr, int len, int type)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return SOCK_gethostbyaddr(addr,len,type);

    case LX_AMITCP:
      return TCP_GetHostByAddr(addr,len,type);

    default:
      return NULL;
  }    
}

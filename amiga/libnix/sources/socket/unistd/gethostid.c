#include <unistd.h>
//
#include "socket.h"

long gethostid(void)
{ struct SocketSettings *lss;
  char hostname[MAXPATHLEN];
  struct hostent *ht;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      gethostname(hostname,MAXPATHLEN);
      if ((ht=SOCK_gethostbyname(hostname)))
        return (long)ht->h_addr;
      return 0;

    case LX_AMITCP:
      return (long)TCP_GetHostId();

    default:
      return -1;
  }
}

#include <string.h>
#include <unistd.h>
//
#include "socket.h"

static inline int tcp_getdomainname(char *domainname, int dsize)
{ char hname[MAXHOSTNAMELEN+1], *dn;

  gethostname(hname, MAXHOSTNAMELEN);
  strncpy(domainname, (dn=strchr(hname,'.')) ? dn+1 : hname, dsize-1);
  domainname[dsize-1] = '\0'; return 0;
}

int getdomainname(char *name, int namelen)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  switch (lss->lx_network_type) {
    case LX_AMITCP:
      return tcp_getdomainname(name, namelen);

    case LX_AS225:
      return SOCK_getdomainname(name, namelen);

    default:
      return -1;
  }
}

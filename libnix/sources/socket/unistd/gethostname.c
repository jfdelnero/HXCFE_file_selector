#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//
#include "socket.h"

int gethostname(char *name, int namelen)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP) {
    return TCP_GetHostName(name,namelen);
  }
  else { /* LX_AS225, default */
    char *host, domain[257+1];
    int len;
    if ((host=getenv("HOSTNAME")) || (host=getenv("hostname"))) {
      if (!strchr(host, '.')) {
        strcpy(domain, host);
        domain[len=strlen(domain)] = '.';
        if (++len,!getdomainname(domain+len,sizeof(domain)-len))
          domain[len]=0,host=domain;
      }
    }
    else
      host = "localhost";
    strncpy(name, host, namelen); return 0;
  }
}

int
sethostname (const char *name, int namelen)
{
#if 0 // XXX
  static char hostname[MAXHOSTNAMELEN] = "localhost";
  struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type != LX_AMITCP) {
    int len = namelen < sizeof (hostname) - 1 ? namelen : sizeof (hostname) - 1;
    strncpy (hostname, name, len);
    hostname[len] = 0;
  }
#endif
  return 0;
}

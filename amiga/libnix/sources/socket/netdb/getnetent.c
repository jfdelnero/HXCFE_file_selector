#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//
#include "socket.h"

#define MAXALIASES 35
#define TCP_PATH_NETWORKS "AmiTCP:db/networks"

void setnetent(int stayopen)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      SOCK_setnetent(stayopen);
    break;

    case LX_AMITCP:
      if (!lss->lx_net_fp)
        lss->lx_net_fp = fopen(TCP_PATH_NETWORKS, "r");
      else
        rewind(lss->lx_net_fp);
      lss->lx_net_stayopen = stayopen;
    break;

    default:
      /* silence compiler */
    break;
  }
}

void endnetent(void)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      SOCK_endnetent();
    break;

    default:
      lss->lx_net_stayopen = 0;
      if (lss->lx_net_fp) {
        fclose(lss->lx_net_fp); lss->lx_net_fp = NULL;
      }
#if 0
      if (lss->lx_net_aliases) {
        free(lss->lx_net_aliases); lss->lx_net_aliases = NULL;
      }
      if (lss->lx_net_line) {
        free(lss->lx_net_line); lss->lx_net_line = NULL;
      }
#endif
    break;
  }
}

struct netent *getnetent(void)
{ struct SocketSettings *lss;

  switch(lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return SOCK_getnetent();
    break;
	
    case LX_AMITCP:

      if (!lss->lx_net_line)
        lss->lx_net_line = malloc(BUFSIZ + 1);

      if (!lss->lx_net_aliases)
        lss->lx_net_aliases = malloc(MAXALIASES * sizeof(char *));

      if (!lss->lx_net_line || !lss->lx_net_aliases) {
        errno = ENOMEM; return NULL;
      }

      if (lss->lx_net_fp || (lss->lx_net_fp=fopen(TCP_PATH_NETWORKS, "r"))) {
        for(;;) {
          char *s, *cp, **q;

          if ((s=fgets(lss->lx_net_line, BUFSIZ, lss->lx_net_fp)) == NULL)
            break;

          if ((*s == '#') || ((cp=strpbrk(s, "#\n")) == NULL))
            continue;
          *cp = '\0';
          lss->lx_net.n_name = s;

          if ((cp=strpbrk(s, " \t")) == NULL)
            continue;
          *cp++ = '\0';
          while (*cp == ' ' || *cp == '\t')
            cp++;
          if ((s=strpbrk(cp, " \t")))
            *s++ = '\0';
          lss->lx_net.n_net = inet_network(cp);
          lss->lx_net.n_addrtype = AF_INET;

          q = lss->lx_net.n_aliases = lss->lx_net_aliases;
          if ((cp=s)) {
            while (cp && *cp) {
              if (*cp == ' ' || *cp == '\t') {
                cp++; continue;
              }
              if (q < &lss->lx_net_aliases[MAXALIASES - 1])
                *q++ = cp;
              if ((cp=strpbrk(cp, " \t")))
                *cp++ = '\0';
            }
          }
          *q = NULL; return &lss->lx_net;
        }
      }

      /* fall through */

    default:
      return NULL;
    break;
  }
}

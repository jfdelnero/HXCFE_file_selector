#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
//
#include "socket.h"

#define MAXALIASES 35
#define TCP_PATH_PROTOCOLS "AmiTCP:db/protocols"

void setprotoent(int stayopen)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      SOCK_setprotoent(stayopen);
    break;

    case LX_AMITCP:
      if (!lss->lx_proto_fp)
        lss->lx_proto_fp = fopen(TCP_PATH_PROTOCOLS, "r");
      else
        rewind(lss->lx_proto_fp);
      lss->lx_proto_stayopen = stayopen;
    break;

    default:
      /* silence compiler */
    break;
  }
}

void endprotoent(void)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      SOCK_endprotoent();
    break;

    default:
      lss->lx_proto_stayopen = 0;
      if (lss->lx_proto_fp) {
        fclose(lss->lx_proto_fp); lss->lx_proto_fp = NULL;
      }
#if 0
      if (lss->lx_proto_aliases) {
        free(lss->lx_proto_aliases); lss->lx_proto_aliases = NULL;
      }
      if (lss->lx_proto_line) {
        free(lss->lx_proto_line); lss->lx_proto_line = NULL;
      }
#endif
    break;
  }
}

struct protoent *getprotoent(void)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return SOCK_getprotoent();
    break;

    case LX_AMITCP:

      if (!lss->lx_proto_line)
        lss->lx_proto_line = malloc(BUFSIZ + 1);

      if (!lss->lx_proto_aliases)
        lss->lx_proto_aliases = malloc(MAXALIASES * sizeof(char *));

      if (!lss->lx_proto_line || !lss->lx_proto_aliases) {
        errno = ENOMEM; return NULL;
      }

      if (lss->lx_proto_fp || (lss->lx_proto_fp=fopen(TCP_PATH_PROTOCOLS, "r"))) {
        for(;;) {
          char *s, *cp, **q;

          if ((s=fgets(lss->lx_proto_line, BUFSIZ, lss->lx_proto_fp)) == NULL)
            break;

          if ((*s == '#') || ((cp=strpbrk(s, "#\n")) == NULL))
            continue;
          *cp = '\0';
          lss->lx_proto.p_name = s;

          if ((cp=strpbrk(s, " \t")) == NULL)
            continue;
          *cp++ = '\0';
          while (*cp == ' ' || *cp == '\t')
            cp++;
          if ((s=strpbrk(cp, " \t")))
            *s++ = '\0';
          lss->lx_proto.p_proto = atoi(cp);

          q = lss->lx_proto.p_aliases = lss->lx_proto_aliases;
          if ((cp=s)) {
            while (cp && *cp) {
              if (*cp == ' ' || *cp == '\t') {
                cp++; continue;
              }
              if (q < &lss->lx_proto_aliases[MAXALIASES - 1])
                *q++ = cp;

              if ((cp=strpbrk(cp, " \t")))
                *cp++ = '\0';
            }
          }
          *q = NULL; return &lss->lx_proto;
        }
      }

      /* fall through */

    default:
      return NULL;
    break;
  }
}

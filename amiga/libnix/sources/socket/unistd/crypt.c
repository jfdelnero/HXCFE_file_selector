#include <unistd.h>
//
#include "socket.h"

char *crypt(const char *key, const char *setting)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP)
    return UG_crypt(key,setting);
  return NULL;
}

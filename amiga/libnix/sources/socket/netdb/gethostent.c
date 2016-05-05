#include <unistd.h>
#include <sys/types.h>
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
//
#include "socket.h"

void sethostent(int stayopen)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AS225) {
    SOCK_sethostent(stayopen);
  }
  else if (stayopen && lss->lx_res) {
    lss->lx_res->options |= RES_STAYOPEN | RES_USEVC;
  }
}

void endhostent(void)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AS225) {
    SOCK_endhostent();
  }
  else if (lss->lx_res) {
    lss->lx_res->options &= ~(RES_STAYOPEN | RES_USEVC);
    if (*lss->lx_res_socket >= 0) {
      close(*lss->lx_res_socket); *lss->lx_res_socket = -1;
    }
  }
}

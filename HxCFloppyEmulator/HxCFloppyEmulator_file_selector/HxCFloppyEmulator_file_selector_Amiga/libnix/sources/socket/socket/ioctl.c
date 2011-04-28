#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <net/if.h>
//
#include "socket.h"

int ioctl(int s, unsigned long cmd, ...)
{ struct SocketSettings *lss;
  StdFileDes *fp = _lx_fhfromfd(s);
  int arglen,inout,rc;
  caddr_t data;
  va_list va;

  if (fp->lx_type == LX_FILE) {
    errno = EBADF; return -1;
  }
  
  va_start(va, cmd);
  inout = va_arg(va, int);
  arglen = va_arg(va, int);
  data = va_arg(va, caddr_t);
  va_end(va);

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {

    case LX_AS225:

      /* _SIGH_... they left almost everything neatly as it was in the BSD kernel
       *  code they used, but for whatever reason they decided they needed their
       *  own kind of ioctl encoding :-((
       *
       *  Well then, here we go, and map `normal' cmds into CBM cmds:
       */

      switch (cmd) {
        case SIOCADDRT      : cmd = ('r'<<8)|1; break;
        case SIOCDELRT      : cmd = ('r'<<8)|2; break;
        case SIOCSIFADDR    : cmd = ('i'<<8)|3; break;
        case SIOCGIFADDR    : cmd = ('i'<<8)|4; break;
        case SIOCSIFDSTADDR : cmd = ('i'<<8)|5; break;
        case SIOCGIFDSTADDR : cmd = ('i'<<8)|6; break;
        case SIOCSIFFLAGS   : cmd = ('i'<<8)|7; break;
        case SIOCGIFFLAGS   : cmd = ('i'<<8)|8; break;
        case SIOCGIFCONF    : cmd = ('i'<<8)|9; break;
        case SIOCSIFMTU     : cmd = ('i'<<8)|10; break;
        case SIOCGIFMTU     : cmd = ('i'<<8)|11; break;
        case SIOCGIFBRDADDR : cmd = ('i'<<8)|12; break;
        case SIOCSIFBRDADDR : cmd = ('i'<<8)|13; break;
        case SIOCGIFNETMASK : cmd = ('i'<<8)|14; break;
        case SIOCSIFNETMASK : cmd = ('i'<<8)|15; break;
        case SIOCGIFMETRIC  : cmd = ('i'<<8)|16; break;
        case SIOCSIFMETRIC  : cmd = ('i'<<8)|17; break;
        case SIOCSARP       : cmd = ('i'<<8)|18; break;
        case SIOCGARP       : cmd = ('i'<<8)|19; break;
        case SIOCDARP       : cmd = ('i'<<8)|20; break;
        case SIOCATMARK     : cmd = ('i'<<8)|21; break;
        case FIONBIO        : cmd = ('m'<<8)|22; break;
        case FIONREAD       : cmd = ('m'<<8)|23; break;
        case FIOASYNC       : cmd = ('m'<<8)|24; break;
        case SIOCSPGRP      : cmd = ('m'<<8)|25; break;
        case SIOCGPGRP      : cmd = ('m'<<8)|26; break;

        default:
        /* we really don't have to bother the library with cmds we can't even
         * map over...
         */
	break;
      }
      rc = SOCK_ioctl(fp->lx_sock,cmd,data);
    break;

    case LX_AMITCP:
      rc = TCP_IoctlSocket(fp->lx_sock,cmd,data);
    break;

    default:
      rc = 0;
    break;
  }

  return rc;
}

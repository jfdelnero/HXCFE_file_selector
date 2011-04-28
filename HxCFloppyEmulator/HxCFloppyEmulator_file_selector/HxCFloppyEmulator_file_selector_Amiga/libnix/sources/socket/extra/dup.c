#include <errno.h>
#include <unistd.h>
//
#include "stdio.h"

int dup(int d) 
{ StdFileDes *sfd = _lx_fhfromfd(d);

  if (sfd) {
    return sfd->lx_dup(sfd);
  }
  else {
    errno = EBADF;
    return -1;
  }
}

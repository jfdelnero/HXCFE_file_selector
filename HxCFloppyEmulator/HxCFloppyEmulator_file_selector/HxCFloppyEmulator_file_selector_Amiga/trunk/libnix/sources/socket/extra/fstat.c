#include <errno.h>
//
#include "stdio.h"

int fstat(int d, struct stat *buf)
{ StdFileDes *fp = _lx_fhfromfd(d);
  if (fp) {
    return fp->lx_fstat(fp, buf);
  }
  else {
    errno = EBADF;
    return -1;
  }
}

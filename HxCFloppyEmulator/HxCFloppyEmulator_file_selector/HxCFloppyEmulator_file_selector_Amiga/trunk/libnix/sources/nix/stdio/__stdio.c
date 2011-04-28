#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "stabs.h"

static FILE *__files[3];

FILE **__sF=__files; /* stdin, stdout, stderr */

void __initstdfio(void)
{ FILE **f=__sF, *err;

  if(((*f++=fdopen(STDIN_FILENO,"r"))==NULL)||
     ((*f++=fdopen(STDOUT_FILENO,"w"))==NULL)||
     ((*f=err=fdopen(STDERR_FILENO,"w"))==NULL))
    exit(20);
  free(err->buffer);
  err->flags&=~(__SMBF|__SLBF); err->flags|=__SNBF;
  err->buffer=NULL;
}

/* Call our private constructor */
//ADD2INIT(__initstdfio,-20);

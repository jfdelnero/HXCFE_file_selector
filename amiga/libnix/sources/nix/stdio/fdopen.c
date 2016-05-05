#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <proto/exec.h>

FILE *fdopen(int filedes,const char *mode)
{ extern int _lx_addflags(int,int);
  if (mode!=NULL)
  { struct filenode *node = (struct filenode *)calloc(1,sizeof(*node));
    if(node!=NULL)
    { if((node->FILE.buffer=(char *)malloc(BUFSIZ))!=NULL)
      { node->FILE.bufsize=BUFSIZ;
        node->FILE.file=filedes;
        node->FILE.flags|=__SMBF; /* Buffer is malloc'ed */
        if(isatty(filedes))
          node->FILE.flags|=__SLBF; /* set linebuffered flag */
        if(_lx_addflags(filedes,*mode=='a'?O_APPEND:0)&O_WRONLY)
          node->FILE.flags|=__SWO; /* set write-only flag */
        AddHead((struct List *)&__filelist,(struct Node *)&node->node);
        return &node->FILE;
      }
      else
        errno=ENOMEM;
      free(node);
    }
    else
      errno=ENOMEM;
  }
  return NULL;
}

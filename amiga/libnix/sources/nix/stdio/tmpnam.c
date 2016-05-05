#include <stdio.h>
#include <stdlib.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

static char namebuffer[34];

char *tmpnam(char *s)
{ BPTR filelock;
  static unsigned long filecount=0;
  if(s==NULL)
    s=namebuffer;
  do /* generate a filename that doesn't exist */
  { sprintf(s,"T:tempfile_2_%p_%lu",FindTask(NULL),filecount++);
    filelock=Lock(s,ACCESS_WRITE);
    if(filelock!=0)
      UnLock(filelock);
  }while(filelock!=0||IoErr()==ERROR_OBJECT_IN_USE);
  return s;
}

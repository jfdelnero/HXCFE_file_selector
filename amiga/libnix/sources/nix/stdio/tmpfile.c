#include <stdio.h>
#include <stdlib.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

FILE *tmpfile(void)
{ char *name;
  FILE *file;
  BPTR dirlock,filelock,cd;
  static unsigned long filecount=0;
  name=malloc(32); /* buffer for filename */
  if(name!=NULL)
  { dirlock=Lock("T:",ACCESS_READ); /* lock T: directory */
    if(dirlock!=0)
    { cd=CurrentDir(dirlock); /* cd T: */
      do /* generate a filename that doesn't exist */
      { sprintf(name,"tempfile_1_%p_%lu",FindTask(NULL),filecount++);
        filelock=Lock(name,ACCESS_WRITE);
        if(filelock!=0)
          UnLock(filelock);
      }while(filelock!=0||IoErr()==ERROR_OBJECT_IN_USE);
      file=fopen(name,"wb+");
      CurrentDir(cd);
      if(file!=NULL)
      { file->tmpdir=dirlock;
        file->name=name;
        return file; }
      UnLock(dirlock);
    }
    free(name);
  }
  return NULL;
}

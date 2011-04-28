#include <errno.h>
#include <stdio.h>
#include <proto/dos.h>

extern void __seterrno(void);
extern char *__amigapath(const char *path);

int remove(const char *filename)
{ 
#ifdef IXPATHS
  if((filename=__amigapath(filename))==NULL)
    return -1;
#endif

  if(DeleteFile((char *)filename))
    return 0;
  else
  { __seterrno(); return -1; }
}

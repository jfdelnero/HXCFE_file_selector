#include <proto/exec.h>
#include "stabs.h"

extern struct lib /* These are the elements pointed to by __LIB_LIST__ */
{ struct Library *base;
  char *name;
} *__LIB_LIST__[];

extern ULONG __oslibversion;
extern void __request(const char *text);
extern void exit(int returncode);

static void __openliberror(ULONG,...) __attribute__ ((noreturn));

static void __openliberror(ULONG version,...)
{ static const ULONG tricky=0x16c04e75; /* move.b d0,(a3)+ ; rts */
  char buf[60];

  RawDoFmt("Need version %.10ld of %.32s",(APTR)&version,(void (*)())&tricky,buf);
  __request(buf);
  exit(20);
}

void __initlibraries(void)
{
  ULONG version=__oslibversion;
  struct lib **list=__LIB_LIST__;
  ULONG numbases=(ULONG)*list++;

  if(numbases)
    do {
      struct lib *l=*list++;
      l->base=OpenLibrary(l->name,version);
/*    if(l->base==NULL)
       __openliberror(version,l->name);*/
    } while(--numbases);
}

void __exitlibraries(void)
{
  struct lib **list=__LIB_LIST__;
  ULONG numbases=(ULONG)*list++;

  if(numbases)
    do {
      struct lib *l=*list++;
      struct Library *lb=l->base;
      if(lb!=NULL) {
        /* l->base=NULL; */
        CloseLibrary(lb);
      }
    } while(--numbases);
}

ADD2INIT(__initlibraries,-60);
ADD2EXIT(__exitlibraries,-60);

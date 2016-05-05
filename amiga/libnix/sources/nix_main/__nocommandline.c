/*
 * This is a different calling convention for main (actually it's _main):
 *
 * int main(char *commandline);
 * void exit(int returncode);
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "stabs.h"

extern char *__argc; /* Defined in startup */
extern char *__commandline;
extern unsigned long __commandlen;
extern struct WBStartup *_WBenchMsg;

/* This guarantees that this module gets linked in.
   If you replace this by an own reference called
   __nocommandline you get no commandline arguments */
ALIAS(__nocommandline,__initcommandline);

void __initcommandline(void)
{ if(_WBenchMsg==NULL)
  { unsigned long len,i;
    char *a;

    for(len=__commandlen,i=256;;i+=256) /* try in steps of 256 bytes */
    { if(!(__argc=a=(char *)AllocVec(i+len+4,MEMF_ANY)))
        exit(20);
      GetProgramName(a+1,i); /* Who am I ? */
      if(IoErr()!=ERROR_LINE_TOO_LONG)
        break;
      FreeVec(a);
    }
    *a++='\"';
    while(*a++)
      ;
    a[-1]='\"';
    *a++=' ';
    a[len]=0;
    CopyMem(__commandline,a,len);
  }
}

void __exitcommandline(void)
{ char **ac;

  if(*(ac=&__argc)!=NULL)
    FreeVec(*ac);
}

/* Add these two functions to the lists */
//ADD2INIT(__initcommandline,-40);
//ADD2EXIT(__exitcommandline,-40);

#include <stdlib.h>
#include <exec/memory.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include "stabs.h"

extern int    __argc; /* Defined in startup */
extern char **__argv;
extern char  *__commandline;
extern unsigned long __commandlen;
extern struct WBStartup *_WBenchMsg;

extern char __stdiowin[];

static char *cline=NULL; /* Copy of commandline */
static BPTR cd=0l;       /* Lock for Current Directory */
static BPTR window=0l;   /* CLI-window for start from workbench */

/* This guarantees that this module gets linked in.
   If you replace this by an own reference called
   __nocommandline you get no commandline arguments */
ALIAS(__nocommandline,__initcommandline);

void __initcommandline(void)
{
  struct WBStartup *wbs=_WBenchMsg;

  if(wbs!=NULL)
  { if(__stdiowin[0])
    { BPTR win;

      if((window=win=Open(__stdiowin,MODE_OLDFILE))==0l)
        exit(RETURN_FAIL);
      SelectInput(win);
      SelectOutput(win);
    }
    if(wbs->sm_ArgList!=NULL) /* cd to icon */
      cd=CurrentDir(DupLock(wbs->sm_ArgList->wa_Lock));

    __argc=0;
    __argv=(char **)wbs;
  }else
  { 
    char **av,*a,*cl=__commandline;
    size_t i=__commandlen;
    int ac;

    if(!(cline=(char *)AllocVec(i+1,MEMF_ANY))) /* get buffer */
      exit(RETURN_FAIL);
  
    for(a=cline,ac=1;;) /* and parse commandline */
    {
      while(i&&(*cl==' '||*cl=='\t'||*cl=='\n'))
      { cl++;
        i--; }
      if(!i)
        break;
      if(*cl=='\"')
      {
        cl++;
        i--;
        while(i)
        {
          if(*cl=='\"')
          {
            cl++;
            i--;
            break;
          }
          if(*cl=='*')
          {
            cl++;
            i--;
            if(!i)
              break;
          }
          *a++=*cl++;
          i--;
        }
      }
      else
        while(i&&(*cl!=' '&&*cl!='\t'&&*cl!='\n'))
        { *a++=*cl++;
          i--; }
      *a++='\0';
      ac++;
    }
      /* NULL Terminated */
    if(!(__argv=av=(char **)AllocVec(((__argc=ac)+1)*sizeof(char *),MEMF_ANY|MEMF_CLEAR)))
      exit(RETURN_FAIL);

    for(a=cline,i=1;i<ac;i++)
    { 
      av[i]=a;
      while(*a++)
        ; 
    }
  
    for(i=256;;i+=256) /* try in steps of 256 bytes */
    { if(!(*av=(char *)AllocVec(i,MEMF_ANY)))
        break;
      GetProgramName(*av,i); /* Who am I ? */
      if(IoErr()!=ERROR_LINE_TOO_LONG)
        break;
      FreeVec(*av);
    }
  
    if(*av==NULL)
      exit(RETURN_FAIL);
  }
}
  
void __exitcommandline(void)
{ struct WBStartup *wbs=_WBenchMsg;

  if(wbs!=NULL)
  { BPTR file;
    if((file=window)!=0l)
      Close(file);
    if(wbs->sm_ArgList!=NULL) /* set original lock */
      UnLock(CurrentDir(cd));
  }else
  { char *cl=cline;

    if(cl!=NULL)
    { char **av=__argv;

      if(av!=NULL)
      { 
        if(*av!=NULL)
          FreeVec(*av);
        FreeVec(av);
      }
      FreeVec(cl);
    }
  }
}
  
/* Add these two functions to the lists */
//ADD2INIT(__initcommandline,-40);
//ADD2EXIT(__exitcommandline,-40);

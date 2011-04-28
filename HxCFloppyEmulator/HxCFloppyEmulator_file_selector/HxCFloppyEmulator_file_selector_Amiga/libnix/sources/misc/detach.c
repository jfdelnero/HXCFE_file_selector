#include <exec/memory.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "stabs.h"

/* Sorry, but the assembler code produced by gcc for this function is too bad.
 * This one is much better!
 */
#ifdef CreateNewProcTags
#undef CreateNewProcTags
static APTR createnewproctags(APTR DOSBase,ULONG Tag,...)
{ return CreateNewProc((struct TagItem *)&Tag); }
#define CreateNewProcTags(tag...) createnewproctags(DOSBase,tag)
#endif

extern struct WBStart *_WBenchMsg;
extern char *__commandline;
extern void *__SaveSP;
extern char __dosname[];

extern char *__procname;
extern long __priority;
extern unsigned long __stack;

/* I must close a library after my child is running -
 * and closing a library requires a working dispatcher (IMHO).
 * Also semaphores are much smarter ;). Therefore:
 */
static struct SignalSemaphore *sem = NULL;

void __initdetach(void)
{ struct Library *DOSBase,*SysBase = *(struct Library **)4;
  struct SignalSemaphore *sema;

  if (_WBenchMsg)
    return;

  if ((sema=sem)) {        /* I must be the child process */
    ObtainSemaphore(sema); /* Assert that my parent is already dead */
    ReleaseSemaphore(sema);
    FreeMem(sema,sizeof(*sema));
    return;
  }
                          /* I must be the parent */
  if ((sem=sema=(struct SignalSemaphore *)AllocMem(sizeof(*sema),MEMF_PUBLIC|MEMF_CLEAR))) {

    InitSemaphore(sema);

    if ((DOSBase=OpenLibrary(__dosname,30))) {

      struct CommandLineInterface *cli = Cli();
      APTR pr,stack = __SaveSP;

      ObtainSemaphore(sema); /* Assert that my child is suspended until I'm finished */

      pr = CreateNewProcTags(NP_Seglist,cli->cli_Module, /* child process gets my seglist */
                             NP_FreeSeglist,1,           /* and must free it */
                             NP_Cli,1,                   /* it must be a CLI process */
                             NP_StackSize,__stack,       /* it gets a stack */
                             NP_Name,(ULONG)__procname,  /* a name */
                             NP_Priority,__priority,     /* a priority */
                             NP_Arguments,(ULONG)__commandline,/* and my commandline Arguments */
                             TAG_END);
      CloseLibrary(DOSBase);

      if (pr) {

        cli->cli_Module = 0; /* I'm no longer owner of this */

        /* Adjust stack, release semaphore and return 0 in one.
         * Maybe the 3 movel are a bit too cautious, but they ARE working
         */
        asm("movel %0,sp;movel %1,a6;movel %2,a0;moveql #0,d0;jmp a6@(-570)"::
            "r"(stack),"r"(SysBase),"r"(sema):"sp","a6","a0");
      }

      ReleaseSemaphore(sema); /* Again only caution - you never know */
    }
    FreeMem(sema,sizeof(*sema)); /* Couldn't start child :( */
  }
  exit(20);
}

//ADD2INIT(__initdetach,-70); /* A very high priority */

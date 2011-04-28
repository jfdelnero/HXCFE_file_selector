#include <dos/dos.h>
#include <workbench/workbench.h>
#include <inline/exec.h>
#include <inline/dos.h>

int __nocommandline=1; /* Disable commandline parsing  */
int __initlibraries=0; /* Disable auto-library-opening */

struct DosLibrary *DOSBase=NULL;

extern struct WBStartup *_WBenchMsg;

int main(void)
{ if(_WBenchMsg==NULL)
  { if((DOSBase=(struct DosLibrary *)OpenLibrary("dos.library",33))!=NULL)
    { Write(Output(),"Hello world\n",12);
      CloseLibrary((struct Library *)DOSBase); } }
  return 0;
}


#include <exec/memory.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <libraries/commodities.h>
#include <proto/commodities.h>
#include <proto/icon.h>
#include <proto/exec.h>
#include <proto/dos.h>

#ifdef NEW_ARGARRAY
static UBYTE **argArray;
#endif
static struct DiskObject *dObj;

UBYTE **ArgArrayInit(long argc, UBYTE **argv)
{
  if (argc) {
    if (argc>1)
#ifdef NEW_ARGARRAY
      if ((argArray=AllocVec(argc*sizeof(char *),MEMF_ANY))) {
        UBYTE **p,**p1;

        p=p1=argArray;
        argv++;
        do
        {
         *p1++=*argv++;
        }
        while(--argc);
        return p;
      }
#else
    {
      argv++; return argv;
    }
#endif
  }
  else
    if ((dObj=GetDiskObject(((struct WBStartup *)argv)->sm_ArgList->wa_Name)))
      return (UBYTE **)dObj->do_ToolTypes;
  return NULL;
    
}

STRPTR ArgString(UBYTE **tt, STRPTR entry, STRPTR defstr)
{
  STRPTR str;

  if (!entry || !(str=FindToolType(tt,entry)))
    str=defstr;
  return str;
}

LONG ArgInt(UBYTE **tt, STRPTR entry, long defval)
{
  STRPTR str;

  if (entry && (str=FindToolType(tt,entry)))
    StrToLong(str,&defval);
  return defval;
}

#if 0 /* only a example ??? */

CxObj *UserFilter(UBYTE **tt, STRPTR entry, STRPTR defstr)
{
  STRPTR str;

  if (!entry || !(str=FindToolType(tt,entry)))
    str=defstr;
  return (CreateCxObj(CX_FILTER,(long)str,NULL));
}

#endif

void ArgArrayDone(void)
{
  struct DiskObject **d;
#ifdef NEW_ARGARRAY
  UBYTE ***a;

  if (*(a=&argArray)) {
    FreeVec(*a); *a=NULL;
  }
#endif
  if (*(d=&dObj)) {
    FreeDiskObject(*d); *d=NULL;
  }
}

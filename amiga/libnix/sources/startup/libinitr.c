/******************************************************************************/
/*                                                                            */
/* our include                                                                */
/*                                                                            */
/******************************************************************************/

#define EXTENDED /* multibase library! */
#include "libinit.h"

/******************************************************************************/
/*                                                                            */
/* *** FIRST *** function - prevents a crash when called from CLI!            */
/*                                                                            */
/******************************************************************************/

LONG safefail(VOID)
{
  return -1;
}

/******************************************************************************/
/*                                                                            */
/* a do nothing stub (required!)                                              */
/*                                                                            */
/******************************************************************************/

LONG
LibExtFunc(VOID)
{
  return 0;
}

/******************************************************************************/
/*                                                                            */
/* remove library from memory if possible                                     */
/*                                                                            */
/* !!! CAUTION: This function runs in a forbidden state !!!                   */
/*                                                                            */
/******************************************************************************/

LONG
LibExpunge(REG(a6,__LIB lib))
{
  LONG SegList = 0;
  
  /* get 'real' base */

  lib = lib->Parent;

  /* set delayed expunge flag */

  lib->LibNode.lib_Flags |= LIBF_DELEXP;

  /* still in use? */

  if (!lib->LibNode.lib_OpenCnt) {

    APTR SysBase = lib->SysBase;

    /* remove library from SysBase->LibList */

    Remove((struct Node *)lib);

    /* return the seglist for UnLoadSeg() */

    SegList = lib->SegList;

    /* free library */

    FreeMem((UBYTE *)lib-lib->LibNode.lib_NegSize,lib->LibNode.lib_NegSize+lib->LibNode.lib_PosSize);
  }

  return SegList;
}

/******************************************************************************/
/*                                                                            */
/* LibClose() will be called for every CloseLibrary()                         */
/*                                                                            */
/* !!! CAUTION: This function runs in a forbidden state !!!                   */
/*                                                                            */
/******************************************************************************/

LONG
LibClose(REG(a6,__LIB lib))
{
  APTR SysBase = lib->SysBase;
  LONG SegList = 0;

  if (!--lib->Parent->LibNode.lib_OpenCnt && (lib->LibNode.lib_Flags & LIBF_DELEXP))
    SegList = LibExpunge(lib);

  /* call user-exit */

  __UserLibCleanup(lib->DataSeg);

  /* free library */

  FreeMem((UBYTE *)lib-lib->LibNode.lib_NegSize,lib->LibNode.lib_NegSize+lib->LibNode.lib_PosSize);

  /* SegList or NULL (still in use) */

  return SegList;
}

/******************************************************************************/
/*                                                                            */
/* LibOpen() will be called for every OpenLibrary()                           */
/*                                                                            */
/* !!! CAUTION: This function runs in a forbidden state !!!                   */
/*                                                                            */
/******************************************************************************/

static __inline ULONG __DSize(void)
{ ULONG res;

  __asm("movel #___data_size,%0" : "=d" (res)); return res;
}

APTR
LibOpen(REG(a6,__LIB lib))
{
  APTR dataseg, SysBase = lib->SysBase;
  __LIB child;

  /* any memory allocation can cause a call of THIS library expunge vector.
     if OpenCnt is zero the library might go away ... So fake a user :-) */

  lib->LibNode.lib_OpenCnt++;

  /* create new library base */

  if ((child=(__LIB)MakeLibrary(&__FuncTable__[1],NULL,(ULONG (*)())LibInit,sizeof(*lib)+lib->DataSize,0))) {

    LONG *relocs,numrel;

    /* one user */

    child->LibNode.lib_OpenCnt++;

    /* copy dataseg */

    CopyMem(child->DataSeg,dataseg=(UBYTE *)child+sizeof(*lib),__DSize());

    /* relocate */

    relocs = __datadata_relocs;
    if ((numrel=*relocs++)) {
      LONG dist = (LONG)child->DataSeg - (LONG)dataseg;

      do { 
        *(LONG *)((LONG)dataseg + *relocs++) -= dist;
      } while (--numrel);
    }
    child->DataSeg = (dataseg=(UBYTE *)dataseg+0x7ffe);

    /* our 'real' parent */

    child->Parent = lib;

    /* assume user-init won't fail */

    lib->LibNode.lib_Flags &= LIBF_DELEXP;
    lib->LibNode.lib_OpenCnt++;

    /* now call user-init */

    if (!__UserLibInit(&child->LibNode,dataseg)) {
      FreeMem((UBYTE *)child-child->LibNode.lib_NegSize,child->LibNode.lib_NegSize+sizeof(*child)+child->DataSize);
      --lib->LibNode.lib_OpenCnt;
      child = NULL;
    }
  }

  /* end of expunge protection */

  --lib->LibNode.lib_OpenCnt;

  return child;
}

/******************************************************************************/
/*                                                                            */
/* initialization function called by MakeLibrary()                            */
/*                                                                            */
/******************************************************************************/

static __inline APTR __GetDataSeg(void)
{ APTR res;

  __asm("lea ___a4_init-0x7ffe,%0" : "=a" (res)); return res;
}

static __inline ULONG __GetDBSize(void)
{ ULONG res;

  __asm("movel #___data_size,%0; addl #___bss_size,%0" : "=d" (res)); return res;
}

APTR
LibInit(REG(a0,LONG SegList),REG(d0,__LIB lib),REG(a6,struct Library *SysBase))
{
  /* set up header data */

  lib->LibNode.lib_Node.ln_Type = NT_LIBRARY;
  lib->LibNode.lib_Node.ln_Name = (char *)LibName;
  lib->LibNode.lib_Flags        = LIBF_CHANGED | LIBF_SUMUSED;
  lib->LibNode.lib_Version      = (UWORD)LibVersion;
  lib->LibNode.lib_Revision     = (UWORD)LibRevision;
  lib->LibNode.lib_IdString     = (char *)LibIdString;

  /* setup private data */

  lib->SegList  = SegList;
  lib->SysBase  = SysBase;
  lib->DataSeg  = __GetDataSeg();
  lib->DataSize = __GetDBSize();
  lib->Parent   = lib;

  /* this will be added to SysBase->LibList */

  return lib;
}
/******************************************************************************/
/*                                                                            */
/* autoinit table for use with initial MakeLibrary()                          */
/*                                                                            */
/******************************************************************************/

static const APTR InitTab[] = {
  (APTR)sizeof(struct libBase),
  (APTR)&__LibTable__[1],
  (APTR)NULL,
  (APTR)&LibInit
};

/******************************************************************************/
/*                                                                            */
/* resident structure                                                         */
/*                                                                            */
/******************************************************************************/

static const struct Resident RomTag = {
  RTC_MATCHWORD,
  (struct Resident *)&RomTag,
  (struct Resident *)&RomTag+1,
  RTF_AUTOINIT,
  0,
  NT_LIBRARY,
  0,
  (char *)LibName,
  (char *)LibIdString,
  (APTR)&InitTab
};

/******************************************************************************/
/*                                                                            */
/* add these functions to 'liblist'                                           */
/*                                                                            */
/******************************************************************************/

ADD2LIST(LibOpen,   __LibTable__,22);
ADD2LIST(LibExtFunc,__LibTable__,22);
ADD2LIST(LibExpunge,__LibTable__,22);
ADD2LIST(LibExtFunc,__LibTable__,22);
asm(".stabs \"___LibTable__\",20,0,0,-1");

/******************************************************************************/
/*                                                                            */
/* add these functions to 'funclist'                                          */
/*                                                                            */
/******************************************************************************/

ADD2LIST(LibExtFunc,__FuncTable__,22);
ADD2LIST(LibClose,  __FuncTable__,22);
ADD2LIST(LibExpunge,__FuncTable__,22);
ADD2LIST(LibExtFunc,__FuncTable__,22);

/******************************************************************************/
/*                                                                            */
/* end of libinitr.c                                                          */
/*                                                                            */
/******************************************************************************/

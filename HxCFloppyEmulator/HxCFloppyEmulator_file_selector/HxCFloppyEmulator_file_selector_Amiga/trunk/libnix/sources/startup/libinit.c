/******************************************************************************/
/*                                                                            */
/* our include                                                                */
/*                                                                            */
/******************************************************************************/

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

  /* set delayed expunge flag */

  lib->LibNode.lib_Flags |= LIBF_DELEXP;

  /* still in use? */

  if (!lib->LibNode.lib_OpenCnt) {

    APTR SysBase = lib->SysBase;

    /* remove library from SysBase->LibList */

    Remove((struct Node *)lib);

    /* now call user-exit */

    __UserLibCleanup(lib->DataSeg);

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
  LONG SegList = 0;

  /* one less user */

  if (!--lib->LibNode.lib_OpenCnt && (lib->LibNode.lib_Flags & LIBF_DELEXP))
    SegList = LibExpunge(lib);

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

APTR
LibOpen(REG(a6,__LIB lib))
{
  /* clear delayed expunge flag */

  lib->LibNode.lib_Flags &= ~LIBF_DELEXP;

  /* one new user */

  lib->LibNode.lib_OpenCnt++;

  /* return basePtr */

  return lib;
}

/******************************************************************************/
/*                                                                            */
/* initialization function called by MakeLibrary()                            */
/*                                                                            */
/******************************************************************************/

static __inline APTR __GetDataSeg(void)
{ APTR res;

  __asm("lea ___a4_init,%0" : "=a" (res)); return res;
}

static __inline ULONG __BSize(void)
{ ULONG res;

  __asm("movel #___bss_size,%0" : "=d" (res)); return res;
}

static __inline APTR __GetBssStart(void)
{ APTR res;

  __asm("lea __edata,%0" : "=a" (res)); return res;
}

APTR
LibInit(REG(a0,LONG SegList),REG(d0,__LIB lib),REG(a6,struct Library *SysBase))
{ ULONG size;

  /* set up header data */

  lib->LibNode.lib_Node.ln_Type = NT_LIBRARY;
  lib->LibNode.lib_Node.ln_Name = (char *)LibName;
  lib->LibNode.lib_Flags        = LIBF_CHANGED | LIBF_SUMUSED;
  lib->LibNode.lib_Version      = (UWORD)LibVersion;
  lib->LibNode.lib_Revision     = (UWORD)LibRevision;
  lib->LibNode.lib_IdString     = (char *)LibIdString;

  /* setup private data */

  lib->SegList = SegList;
  lib->SysBase = SysBase;

#if 1

  lib->DataSeg = __GetDataSeg();

  /* clear the bss part */

  if ((size=__BSize())) {

    ULONG *p = __GetBssStart();

    do {
      *p++ = 0;
    } while ((size-=sizeof(*p)));
  }

#endif

  /* now call user-init */

  if (!__UserLibInit(&lib->LibNode,lib->DataSeg)) {
    FreeMem((UBYTE *)lib-lib->LibNode.lib_NegSize,lib->LibNode.lib_NegSize+lib->LibNode.lib_PosSize);
    lib = NULL;
  }

  /* this will be added to SysBase->LibList or NULL (init error) */

  return lib;
}

/******************************************************************************/
/*                                                                            */
/* autoinit table for use with initial MakeLibrary()                          */
/*                                                                            */
/******************************************************************************/

static const APTR InitTab[] = {
  (APTR)sizeof(struct libBase),
  (APTR)&__FuncTable__[1],
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
/* add these functions to 'funclist'                                          */
/*                                                                            */
/******************************************************************************/

ADD2LIST(LibOpen,__FuncTable__,22);
ADD2LIST(LibClose,__FuncTable__,22);
ADD2LIST(LibExpunge,__FuncTable__,22);
ADD2LIST(LibExtFunc,__FuncTable__,22);

/******************************************************************************/
/*                                                                            */
/* end of libinit.c                                                           */
/*                                                                            */
/******************************************************************************/

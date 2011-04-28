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
DevExtFunc(VOID)
{
  return 0;
}

/******************************************************************************/
/*                                                                            */
/* remove device from memory if possible                                      */
/*                                                                            */
/* !!! CAUTION: This function runs in a forbidden state !!!                   */
/*                                                                            */
/******************************************************************************/

LONG
DevExpunge(REG(a6,__DEV dev))
{
  LONG SegList = 0;

  /* set delayed expunge flag */

  dev->LibNode.lib_Flags |= LIBF_DELEXP;

  /* still in use? */

  if (!dev->LibNode.lib_OpenCnt) {

    APTR SysBase = dev->SysBase;

    /* remove device from SysBase->DevList */

    Remove((struct Node *)dev);

    /* now call user-exit */

    __UserDevCleanup(dev->DataSeg);

    /* return the seglist for UnLoadSeg() */

    SegList = dev->SegList;

    /* free device */

    FreeMem((UBYTE *)dev-dev->LibNode.lib_NegSize,dev->LibNode.lib_NegSize+dev->LibNode.lib_PosSize);
  }

  return SegList;
}

/******************************************************************************/
/*                                                                            */
/* DevClose() will be called for every CloseDevice()                          */
/*                                                                            */
/* !!! CAUTION: This function runs in a forbidden state !!!                   */
/*                                                                            */
/******************************************************************************/

LONG
DevClose(REG(a1,APTR iorq),REG(a6,__DEV dev))
{
  LONG SegList = 0;

    /* call user-close */

  __UserDevClose(iorq,dev->DataSeg);

  /* one less user */

  if (!--dev->LibNode.lib_OpenCnt && (dev->LibNode.lib_Flags&LIBF_DELEXP))
    SegList = DevExpunge(dev);

  /* SegList or NULL (still in use) */

  return SegList;
}

/******************************************************************************/
/*                                                                            */
/* DevOpen() will be called for every OpenDevice()                            */
/*                                                                            */
/* !!! CAUTION: This function runs in a forbidden state !!!                   */
/*                                                                            */
/******************************************************************************/

VOID
DevOpen(REG(d0,ULONG unit),REG(a1,APTR iorq),REG(d1,ULONG flags),REG(a6,__DEV dev))
{
  /* any memory allocation can cause a call of THIS device expunge vector.
     if OpenCnt is zero the library might go away ... so fake a user :-) */

  ++dev->LibNode.lib_OpenCnt;

  /* call user-open */

  if (!__UserDevOpen(iorq,unit,flags,dev->DataSeg)) {

    /* clear delayed expunge flag */

    dev->LibNode.lib_Flags &= ~LIBF_DELEXP;

    /* one new user */

    ++dev->LibNode.lib_OpenCnt;
  }

  /* end of expunge protection */

  --dev->LibNode.lib_OpenCnt;

  /* exec returns io_error later */
}

/******************************************************************************/
/*                                                                            */
/* initialization function called by MakeLibrary()                            */
/*                                                                            */
/******************************************************************************/

static inline APTR __GetDataSeg(void)
{ APTR res;

  __asm("lea ___a4_init,%0" : "=a" (res)); return res;
}

static inline ULONG __BSize(void)
{ ULONG res;

  __asm("movel #___bss_size,%0" : "=d" (res)); return res;
}

static inline APTR __GetBssStart(void)
{ APTR res;

  __asm("lea __edata,%0" : "=a" (res)); return res;
}

APTR
DevInit(REG(a0,LONG SegList),REG(d0,__DEV dev),REG(a6,struct Library *SysBase))
{ ULONG size;

  /* set up header data */

  dev->LibNode.lib_Node.ln_Type = NT_DEVICE;
  dev->LibNode.lib_Node.ln_Name = (UBYTE *)DevName;
  dev->LibNode.lib_Flags        = LIBF_CHANGED | LIBF_SUMUSED;
  dev->LibNode.lib_Version      = (UWORD)DevVersion;
  dev->LibNode.lib_Revision     = (UWORD)DevRevision;
  dev->LibNode.lib_IdString     = (char *)DevIdString;

  /* setup private data */

  dev->SegList = SegList;
  dev->SysBase = SysBase;

#if 1

  dev->DataSeg = __GetDataSeg();

  /* clear the bss part */

  if ((size=__BSize())) {

    ULONG *p = __GetBssStart();

    do {
      *p++=0;
    } while ((size-=sizeof(*p)));
  }

#endif

  /* now call user-init */

  if (!__UserDevInit(&dev->LibNode,dev->DataSeg)) {
    FreeMem((UBYTE *)dev-dev->LibNode.lib_NegSize,dev->LibNode.lib_NegSize+dev->LibNode.lib_PosSize);
    dev = NULL;
  }

  /* this will be added to SysBase->DevList or NULL (init error) */

  return dev;
}

/******************************************************************************/
/*                                                                            */
/* autoinit table for use with initial MakeLibrary()                          */
/*                                                                            */
/******************************************************************************/

static const APTR InitTab[4] = {
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
  NT_DEVICE,
  0,
  (char *)DevName,
  (char *)DevIdString,
  (APTR)&InitTab
};

/******************************************************************************/
/*                                                                            */
/* add these functions to 'funclist'                                          */
/*                                                                            */
/******************************************************************************/

ADD2LIST(DevOpen,__FuncTable__,22);
ADD2LIST(DevClose,__FuncTable__,22);
ADD2LIST(DevExpunge,__FuncTable__,22);
ADD2LIST(DevExtFunc,__FuncTable__,22);

/******************************************************************************/
/*                                                                            */
/* end of devinit.c                                                           */
/*                                                                            */
/******************************************************************************/

#include <exec/io.h>
#include <exec/memory.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include "stabs.h"

struct IORequest *CreateExtIO(CONST struct MsgPort *port,LONG iosize)
{ APTR SysBase = *(APTR *)4L;
  struct IORequest *ioreq = NULL;

  if (port && (ioreq=AllocMem(iosize,MEMF_CLEAR|MEMF_PUBLIC))) {
    ioreq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
    ioreq->io_Message.mn_ReplyPort    = port;
    ioreq->io_Message.mn_Length       = iosize;
  }
  return ioreq;
}

struct IOStdReq *CreateStdIO(CONST struct MsgPort *port)
{
  return (struct IOStdReq *)CreateExtIO(port,sizeof(struct IOStdReq));
}

VOID DeleteExtIO(struct IORequest *ioreq)
{ APTR SysBase = *(APTR *)4L;
  LONG i;

  i = -1;
  ioreq->io_Message.mn_Node.ln_Type = i;
  ioreq->io_Device                  = (struct Device *)i;
  ioreq->io_Unit                    = (struct Unit *)i;
  FreeMem(ioreq,ioreq->io_Message.mn_Length);
}

ALIAS(DeleteStdIO,DeleteExtIO);

#include <exec/ports.h>
#include <exec/memory.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>

#define NEWLIST(l) ((l)->lh_Head = (struct Node *)&(l)->lh_Tail, \
                    /*(l)->lh_Tail = NULL,*/ \
                    (l)->lh_TailPred = (struct Node *)&(l)->lh_Head)

struct MsgPort *CreatePort(CONST_STRPTR name,LONG pri)
{ APTR SysBase = *(APTR *)4L;
  struct MsgPort *port = NULL;
  UBYTE portsig;

  if ((BYTE)(portsig=AllocSignal(-1)) >= 0) {
    if (!(port=AllocMem(sizeof(*port),MEMF_CLEAR|MEMF_PUBLIC)))
      FreeSignal(portsig);
    else {
      port->mp_Node.ln_Type = NT_MSGPORT;
      port->mp_Node.ln_Pri  = pri;
      port->mp_Node.ln_Name = name;
      /* done via AllocMem
      port->mp_Flags        = PA_SIGNAL;
      */
      port->mp_SigBit       = portsig;
      port->mp_SigTask      = FindTask(NULL);
      NEWLIST(&port->mp_MsgList);
      if (port->mp_Node.ln_Name)
        AddPort(port);
    }
  }
  return port;
}

VOID DeletePort(struct MsgPort *port)
{ APTR SysBase = *(APTR *)4L;

  if (port->mp_Node.ln_Name)
    RemPort(port);
  FreeSignal(port->mp_SigBit); FreeMem(port,sizeof(*port));
}

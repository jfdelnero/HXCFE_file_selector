#include <exec/memory.h>
#include <devices/timer.h>
#include <proto/exec.h>

#define NEWLIST(l) ((l)->lh_Head = (struct Node *)&(l)->lh_Tail, \
                    /*(l)->lh_Tail = NULL,*/ \
                    (l)->lh_TailPred = (struct Node *)&(l)->lh_Head)

LONG TimeDelay(LONG unit,ULONG secs,ULONG microsecs)
{ APTR SysBase = *(APTR *)4L;
  struct PortIO {
    struct timerequest treq;
    struct MsgPort     port;
  } *portio;
  LONG ret=-1;

  if ((portio=(struct PortIO *)AllocMem(sizeof(*portio),MEMF_CLEAR|MEMF_PUBLIC))) {
    portio->port.mp_Node.ln_Type=NT_MSGPORT;
    if ((BYTE)(portio->port.mp_SigBit=AllocSignal(-1))>=0) {
      portio->port.mp_SigTask=FindTask(NULL);
      NEWLIST(&portio->port.mp_MsgList);
      portio->treq.tr_node.io_Message.mn_Node.ln_Type=NT_REPLYMSG;
      portio->treq.tr_node.io_Message.mn_ReplyPort=&portio->port;
      if (!(OpenDevice(TIMERNAME,unit,&portio->treq.tr_node,0))) {
        portio->treq.tr_node.io_Command=TR_ADDREQUEST;
        portio->treq.tr_time.tv_secs=secs;
        portio->treq.tr_time.tv_micro=microsecs;
        if (!DoIO(&portio->treq.tr_node))
          ret=0;
        CloseDevice(&portio->treq.tr_node);
      }
      FreeSignal(portio->port.mp_SigBit);
    }
    FreeMem(portio,sizeof(*portio));
  }

  return ret;
}

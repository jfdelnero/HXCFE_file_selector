#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <devices/timer.h>
#include <proto/exec.h>
#include "debuglib.h"
#include "select.h"
#include "socket.h"
#include "stabs.h"

/*
**
*/
static struct MsgPort *_tport=NULL;
static struct timerequest *_treq=NULL;

/*
**
*/
void __setupselect(void)
{
  if (((_tport=CreateMsgPort())==NULL) ||
      ((_treq=CreateIORequest(_tport,sizeof(*_treq)))==NULL))
    exit(20);
}
//ADD2INIT(__setupselect,-10);

void __cleanupselect(void)
{
  if (_treq) {
    DeleteIORequest(_treq); _treq=NULL;
  }

  if (_tport) {
    DeleteMsgPort(_tport); _tport=NULL;
  }
}
//ADD2EXIT(__cleanupselect,-10);

/*
**
*/
static inline void handle_select_port(void)
{ extern struct MsgPort *__selport;
  struct StandardPacket *pkt;

  while((pkt=GetPacket(__selport))) {
    DB( BUG("handle_select_port got packet %lx\n", pkt); )
    pkt->sp_Pkt.dp_Port = NULL;
  }
}

static inline void setcopy(int nfd, u_int *ifd, u_int *ofd)
{
  /* this procedure is here, because it's "normal" that if you only
   * want to select on fd 0,1,2 eg. you only pass a long to select,
   * not a whole fd_set, so we can't simply copy over results in the
   * full size of an fd_set.. */
  
  /* we have to copy that many longs... */
  for (nfd=(nfd+31)>>5; nfd; *ofd++=*ifd++,--nfd);
}

static inline int net_select(int s, fd_set *in, fd_set *out, fd_set *exc, struct timeval *timeout, u_long *sigs)
{ struct SocketSettings *lss;
  int rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      rc = SOCK_selectwait(s, in, out, exc, timeout, sigs);
    break;

    case LX_AMITCP:
      rc = TCP_WaitSelect(s, in, out, exc, timeout, sigs);
    break;

    default:
      rc = -1;
    break;
  }

  return rc;
}

static inline int lx_select(int nfd, fd_set *ifd, fd_set *ofd, fd_set *efd, struct timeval *timeout, u_long *mask)
{ int i, waitin, waitout, waitexc, dotout, result, skipped_wait;
  u_long net_nfds, wait_sigs, recv_wait_sigs=0, origmask = mask ? *mask : 0;
  StdFileDes *f;

  /* as long as I don't support anything similar to a network, I surely
   * won't get any exceptional conditions, so *efd is mapped into
   * *ifd, if it's set.
   */

  /* first check, that all included descriptors are valid and support
   * the requested operation. If the user included a request to wait
   * for a descriptor to be ready to read, while the descriptor was only
   * opened for writing, the requested bit is immediately cleared
   */
  waitin = waitout = waitexc = 0;
  /*if (nfd > NOFILE) nfd = NOFILE;*/

  for (i = 0; i < nfd; i++) {
    if (ifd && FD_ISSET(i, ifd) && (f = _lx_fhfromfd(i))) {
      if (!f->lx_read || !f->lx_select)
        FD_CLR(i, ifd);
      else
        ++waitin;
    }

    if (ofd && FD_ISSET(i, ofd) && (f = _lx_fhfromfd(i))) {
      if (!f->lx_write || !f->lx_select)
        FD_CLR(i, ofd);
      else
        ++waitout;
    }

    if (efd && FD_ISSET(i, efd) && (f = _lx_fhfromfd(i))) {
      /* question: can an exceptional condition also occur on a 
       * write-only fd?? */
      if (!f->lx_read || !f->lx_select)
        FD_CLR(i, efd);
      else
        ++waitexc;
    }
  }

  dotout = (timeout && timerisset(timeout));

  for (skipped_wait = 0; ; skipped_wait=1) {
    fd_set readyin, readyout, readyexc;
    fd_set netin, netout, netexc;
    extern int network_installed;
    int tout, readydesc, cmd;

    FD_ZERO(&readyin);
    FD_ZERO(&readyout);
    FD_ZERO(&readyexc);
      
    if (network_installed) {
      FD_ZERO(&netin);
      FD_ZERO(&netout);
      FD_ZERO(&netexc);
      net_nfds = 0;
    }

    handle_select_port();

    tout = readydesc = 0;

    wait_sigs = SIGBREAKF_CTRL_C  | origmask;

    if (cmd=SELCMD_POLL,skipped_wait) {

      if (dotout)  {
        _treq->tr_node.io_Command = TR_ADDREQUEST;
        _treq->tr_time.tv_sec     = timeout->tv_sec;
        _treq->tr_time.tv_usec    = timeout->tv_usec;
        SendIO(&_treq->tr_node);
        /* clear the bit, it's used for sync packets too, and might be set */
        SetSignal (0, 1 << _tport->mp_SigBit);
        wait_sigs |= 1 << _tport->mp_SigBit;
      }

      /* have all watched files get prepared for selecting */
      for (i = 0; i < nfd; i++) {
        if (ifd && FD_ISSET (i, ifd) && (f = _lx_fhfromfd(i)))
          wait_sigs |= f->lx_select (f, SELCMD_PREPARE, SELMODE_IN, &netin, &net_nfds);
        if (ofd && FD_ISSET (i, ofd) && (f = _lx_fhfromfd(i)))
          wait_sigs |= f->lx_select (f, SELCMD_PREPARE, SELMODE_OUT, &netout, &net_nfds);
        if (efd && FD_ISSET (i, efd) && (f = _lx_fhfromfd(i)))
          wait_sigs |= f->lx_select (f, SELCMD_PREPARE, SELMODE_EXC, &netexc, &net_nfds);
      }

      /* now wait for all legally possible signals, this includes BSD
       * signals (but want at least one signal set!)
       */
      if (network_installed) {
        int res;
        DB( BUG("Calling _net_select, signals are %lx\n", wait_sigs); )
        res = net_select(net_nfds, &netin, &netout, &netexc, NULL, &wait_sigs);
        recv_wait_sigs = wait_sigs;
        DB( BUG("Net select returns %ld, wait_sigs are %lx\n", res, wait_sigs); )
      }
      else
         while (!(recv_wait_sigs = Wait (wait_sigs))) ;

      if (mask)
        *mask = recv_wait_sigs & origmask;

      if (dotout) {
        /* IMPORTANT: unqueue the timer request BEFORE polling the fd's,
         *            or __wait_packet() will treat the timer request
         *            as a packet...
         */
        if (!CheckIO(&_treq->tr_node))
          AbortIO(&_treq->tr_node);
        else
          recv_wait_sigs |= 1 << _tport->mp_SigBit;
        WaitIO(&_treq->tr_node);
      }

      handle_select_port();

      cmd = SELCMD_CHECK;
    }

    /* no matter what caused Wait() to return, wait for all requests
     * to complete (we CAN'T abort a DOS packet, sigh...)
     */

    /* collect information from the file descriptors */
    for (i = 0; i < nfd; i++) {    
      if (ifd && FD_ISSET (i, ifd) && (f = _lx_fhfromfd(i))
          && f->lx_select (f, cmd, SELMODE_IN, &netin, NULL)) {
        DB( BUG("Select: fd %ld, has data ready for reading\n", i); )
        FD_SET (i, &readyin);
        ++readydesc;
      }

      if (ofd && FD_ISSET (i, ofd) && (f = _lx_fhfromfd(i))
          && f->lx_select (f, cmd, SELMODE_OUT, &netout, NULL)) {
        DB( BUG("Select: fd %ld, has data ready for writing\n", i); )
        FD_SET (i, &readyout);
        ++readydesc;
      }

      if (efd && FD_ISSET (i, efd) && (f = _lx_fhfromfd(i))
          && f->lx_select (f, cmd, SELMODE_EXC, &netexc, NULL)) {
        DB( BUG("Select: fd %ld, has an exception\n", i); )
        FD_SET (i, &readyexc);
        ++readydesc;
      }
    }

    /* we have a timeout condition, if readydesc == 0, dotout == 1 and
     * end_time < current time
     */

    DB( BUG("After loop, readydesc is %ld\n", readydesc); )

    if (!readydesc && dotout)
      tout = recv_wait_sigs & (1 << _tport->mp_SigBit);

    DB( BUG("readydesc is %ld, tout is %ld, timeout is %lx\n", readydesc, tout, timeout); )

    if (readydesc || tout || (timeout && !timerisset(timeout)))  {
      if (ifd) setcopy(nfd, (u_int *)&readyin,  (u_int *)ifd);
      if (ofd) setcopy(nfd, (u_int *)&readyout, (u_int *)ofd);
      if (efd) setcopy(nfd, (u_int *)&readyexc, (u_int *)efd);
      result = readydesc; /* ok for tout, since then readydesc is already 0 */
      DB( BUG("Breaking out of loop, result is %ld\n", result); )
      break;
    }

    if (recv_wait_sigs & SIGBREAKF_CTRL_C) {
      DB( BUG("Found CTRL-C\n"); )
      /* clear it and resend it so the outer stuff can catch it */
      SetSignal(0, SIGBREAKF_CTRL_C);
      Signal(FindTask(NULL), SIGBREAKF_CTRL_C);
      result = -1;
      break;
    }
  }

  if (recv_wait_sigs == (u_long)-1)
    return -1;

  /* have to set this here, since errno can be changed in signal handlers */
  if (result == -1)
    errno = EINTR;

  DB( BUG("SELECT: returning %ld\n", result); )
  return result;
}

int select(int nfd, fd_set *ifd, fd_set *ofd, fd_set *efd, struct timeval *timeout)
{ return lx_select(nfd, ifd, ofd, efd, timeout, NULL); }

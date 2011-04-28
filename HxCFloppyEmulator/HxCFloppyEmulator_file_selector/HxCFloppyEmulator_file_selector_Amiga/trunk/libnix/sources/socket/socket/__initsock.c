#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <inetd.h>
#include "debuglib.h"
#include "select.h"
#include "socket.h"
#include "stabs.h"

/*
**
*/
extern int    __argc;
extern char **__argv;

/*
**
*/
static ssize_t _sock_read(StdFileDes *fp,void *buf,size_t len)
{
  return recv(fp->lx_pos,buf,len,0);
}

static ssize_t _sock_write(StdFileDes *fp,const void *buf,size_t len)
{
  return send(fp->lx_pos,buf,len,0);
}

static int _sock_close(StdFileDes *fp)
{ struct SocketSettings *lss;
  int rc;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {

    case LX_AS225:
      rc = SOCK_close(fp->lx_sock);
    break;

    case LX_AMITCP:
      rc = TCP_CloseSocket(fp->lx_sock);
    break;

    default:
      rc = 0;
    break;
  }

  return rc;
}

static int _sock_dup(StdFileDes *fp)
{ struct SocketSettings *lss;
  StdFileDes *fp2;
  int rc;

  if ((fp2=_create_socket(fp->lx_family, fp->lx_domain, fp->lx_protocol)) == NULL)
    return -1;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {

    case LX_AS225:
      /* only INET-225 has dup */
      if (lss->lx_SocketBase->lib_Version >= 8)
        rc = SOCK_dup(fp->lx_sock);
      else
        rc = fp->lx_sock;
    break;

    case LX_AMITCP:
      rc = TCP_Dup2Socket(fp->lx_sock, -1);
    break;

    default:
      rc = -1;
    break;
  }

  if (rc >= 0) {
    fp2->lx_sock = rc; rc = fp2->lx_pos;
  }
  else {
    fp2->lx_inuse = 0;
  }

  return rc;
}

static int _sock_fstat(StdFileDes *fp,struct stat *sb)
{ long value;
  int size = sizeof(value);

  memset(sb, 0, sizeof(*sb));

//sb->st_uid = 0;
//sb->st_gid = 0;

  if (getsockopt(fp->lx_sock, SOL_SOCKET, SO_SNDBUF,(void *)&value, &size) == 0)
    sb->st_blksize = value;

  return 0;
}

static int _sock_poll(StdFileDes *fp,int io_mode,struct SocketSettings *lss)
{ struct timeval tv = {0, 0};
  fd_set in, out, exc;
  int rc;

  FD_ZERO(&in);
  FD_ZERO(&out);
  FD_ZERO(&exc);

  switch (io_mode) {
    case SELMODE_IN:
      FD_SET(fp->lx_sock,&in);
    break;

    case SELMODE_OUT:
      FD_SET(fp->lx_sock,&out);
    break;

    case SELMODE_EXC:
      FD_SET(fp->lx_sock,&exc);
    break;
  }

  switch (lss->lx_network_type) {

    case LX_AS225:
      rc = SOCK_selectwait(fp->lx_sock, &in, &out, &exc, &tv, NULL);
    break;

    case LX_AMITCP:
      rc = TCP_WaitSelect(fp->lx_sock, &in, &out, &exc, &tv, NULL);
    break;

    default:
      rc = -1;
    break;
  }

  return ((rc == 1) ? 1 : 0);
}

static int _sock_select(StdFileDes *fp,int select_cmd,int io_mode,fd_set *set,u_long *nfds)
{ struct SocketSettings *lss = _lx_get_socket_settings();
  
  if (select_cmd == SELCMD_PREPARE) {
    FD_SET(fp->lx_sock, set);
    if (fp->lx_sock > *nfds)
      *nfds = fp->lx_sock;
    return (1L << lss->lx_sigurg | 1L << lss->lx_sigio);
  }

  if (select_cmd == SELCMD_CHECK)
    return FD_ISSET(fp->lx_sock, set);

  if (select_cmd == SELCMD_POLL)
    return _sock_poll(fp, io_mode, lss);

  return 0;
}

StdFileDes *_create_socket(int family,int type,int protocol)
{ extern StdFileDes *_allocfd(void);
  StdFileDes *sfd = _allocfd();

  if (sfd) {
    sfd->lx_type     = LX_SOCKET;
    sfd->lx_inuse    = 1;
    sfd->lx_family   = family;
    sfd->lx_domain   = type;
    sfd->lx_protocol = protocol;
    sfd->lx_read     = _sock_read;
    sfd->lx_write    = _sock_write;
    sfd->lx_close    = _sock_close;
    sfd->lx_dup      = _sock_dup;
    sfd->lx_fstat    = _sock_fstat;
    sfd->lx_select   = _sock_select;
  }

  return sfd;
}

/*
** Redirect stdio (stdin, stdout and stderr) to/from socket 'sock'.
** This is done by dup2()'ing 'sock' to the level 1 files underneath
** the stdin, stdout and stderr.
*/
static int set_socket_stdio(int sock)
{
  if (sock>=0 && dup2(sock,0)>=0 && dup2(sock,1)>=0 && dup2(sock,2)>=0) {
    /*
     * Close the original reference to the sock if necessary
     */
    if (sock > 2)
      close(sock);

    return 0;
  }

  return -1;
}

/*
 *  init_inet_daemon.c - obtain socket accepted by the inetd
 *
 *  Copyright © 1994 AmiTCP/IP Group,
 *       Network Solutions Development Inc.
 *       All rights reserved.
 *  Portions Copyright © 1995 by Jeff Shepherd
 */

static int init_inet_daemon(int *argc,char ***argv,struct SocketSettings *lss)
{ StdFileDes *fp = NULL;
  int sock, err, i;

  lss->lx_isdaemon = 0;

  if (lss->lx_network_type == LX_AS225) {

    /* code loosely derived from timed.c from AS225r2 */
    /* this program was called from inetd if :
     * 1> the first arg is a valid protocol(call getprotobyname)
     * 2> inetd is started - FindPort("inetd") returns non-NULL
     * NOT 3> argv[0] is the program found in inetd.conf for the program (scan inetd.conf)
     */

    /* save a little time with this comparison */
    if (err=-1,*argc >= 4) {

      struct servent *serv1 = SOCK_getservbyname((*argv)[1],"tcp");
      struct servent *serv2 = SOCK_getservbyname((*argv)[1],"udp");

      if ((serv1 || serv2) && FindPort("inetd")) {

        long sock_arg;

        sock_arg = atol((*argv)[2]);
        lss->lx_sockid = atoi((*argv)[3]);
        sock = SOCK_inherit((void *)sock_arg);
        if (sock >= 0) {
          lss->lx_isdaemon = 1; /* I was started from inetd */
          do {
            if ((fp=_create_socket(AF_INET, SOCK_STREAM, 0)) == NULL)
              break;
            fp->lx_sock = sock;
            /* get rid of the args that AS225 put in */
            for (i=1; i < (*argc)-3; i++)
              (*argv)[i] = (*argv)[i+3];
            (*argc) -= 3;
            err = 0;
          } while (0);
        }
      }
    }

    return ((errno=err) ? -1 : fp->lx_pos);
  }

  if (lss->lx_network_type == LX_AMITCP) {
    struct Process *me = (struct Process *)FindTask(NULL);
    struct DaemonMessage *dm = (struct DaemonMessage *)me->pr_ExitData;

    DB( BUG("daemon message is %lx\n", dm); )
    if (dm == NULL) {
      /*
      * No DaemonMessage, return error code - probably not an inet daemon
      */
      return -1;
    }

    /*
     * Obtain the server socket
     */
    sock = TCP_ObtainSocket(dm->dm_Id, dm->dm_Family, dm->dm_Type, 0);
    DB( BUG("Obtained socket is %ld\n", sock); )

    if (sock < 0) {
      /*
      * If ObtainSocket fails we need to exit with this specific exit code
      * so that the inetd knows to clean things up
      */
      exit(DERR_OBTAIN);
    }

    err = -1;

    do {

      if ((fp=_create_socket(dm->dm_Family, dm->dm_Type, 0)) == NULL)
        break;
      fp->lx_sock = sock;

      DB( BUG("FD for daemon socket is %ld\n", fp->lx_pos); )

      err = 0;
    } while (0);

    return ((errno=err) ? -1 : fp->lx_pos);
  }

  return -1;
}

/* This is only needed for AS225 */
static void shutdown_inet_daemon(struct SocketSettings *lss)
{
  if (lss->lx_network_type == LX_AS225 && lss->lx_isdaemon) {

    /* AS225 inet daemon stuff */
    struct inetmsg { struct Message msg; ULONG id; } inet_message;

    if ((inet_message.id=lss->lx_sockid)) {

      struct MsgPort *msgport,*replyport;

      if ((replyport=CreateMsgPort())) {

        inet_message.msg.mn_Node.ln_Type = NT_MESSAGE;
        inet_message.msg.mn_ReplyPort    = replyport;
        inet_message.msg.mn_Length       = sizeof(struct inetmsg);

        Forbid();
        if ((msgport=FindPort("inetd"))) {
          PutMsg(msgport,&inet_message.msg);
          /* we can't exit until we received a reply */
          WaitPort(replyport);
        }
        Permit();
        DeleteMsgPort(replyport);
      }
    }
  }
}

/*
**
*/
struct SocketSettings *_lx_get_socket_settings(void) 
{ static struct SocketSettings lx_socket_settings;
  return &lx_socket_settings;
}

/*
**
*/
extern int network_installed;

/*
**
*/
void __initsocket(void)
{ struct SocketSettings *lss = _lx_get_socket_settings();
  int sock;

  lss->lx_sigurg = AllocSignal(-1l);
  lss->lx_sigio  = AllocSignal(-1l);
  DB( BUG("sigurg is %ld\nsigio is %ld\n", lss->lx_sigurg, lss->lx_sigio); )

  /* We could check for the existance of the AMITCP port here
     to make sure we're using the AmiTCP bsdsocket.library,
     and not the bsdsocket.library emulation for AS225.
     But in that case, the Miami package isn't recognized,
     because they don't open an AMITCP port. Oh well... */
  lss->lx_network_type = LX_AMITCP;
  if ((lss->lx_BsdSocketBase=OpenLibrary("bsdsocket.library",3))) {
    struct Process *pr = (struct Process *)FindTask(NULL);
    struct CommandLineInterface *cli = BADDR(pr->pr_CLI);
    char *progname = (cli ? BADDR(cli->cli_CommandName) : pr->pr_Task.tc_Node.ln_Name);
    struct TagItem list[] = {
      { SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(int))), (ULONG)&errno    },
      { SBTM_SETVAL(SBTC_HERRNOLONGPTR),         (ULONG)&h_errno  },
      { SBTM_SETVAL(SBTC_LOGTAGPTR),             (ULONG)progname  },
      { SBTM_SETVAL(SBTC_SIGURGMASK),            0                },
      { SBTM_SETVAL(SBTC_SIGIOMASK),             0                },
      { SBTM_SETVAL(SBTC_BREAKMASK),             SIGBREAKF_CTRL_C },
      { TAG_END,TAG_END }
    };

    DB( BUG("lss->lx_BsdSocketBase is %lx\n", lss->lx_BsdSocketBase); )
    DB( BUG("progname is %s\n", progname); )

    list[3].ti_Data = (1L << lss->lx_sigurg);
    list[4].ti_Data = (1L << lss->lx_sigio);

    /* I will assume this always is successful */
    TCP_SocketBaseTagList(list);

    /* only use usergroup stuff when AmiTCP is started only */
    /* I call OpenLibrary() with the full path since
     * usergroup.library might open yet - some people bypass the
     * "login" command which loads usergroup.library
     */
    lss->lx_UserGroupBase = OpenLibrary("AmiTCP:libs/usergroup.library",1);
    if (lss->lx_UserGroupBase) {
      struct TagItem ug_list[] = {
        { UGT_ERRNOPTR(sizeof(int)), (ULONG)&errno    },
        { UGT_INTRMASK,              SIGBREAKB_CTRL_C },
        { TAG_END,TAG_END }
      };
      ug_SetupContextTagList(progname, ug_list);

      sock = init_inet_daemon(&__argc, &__argv, lss);
      if (sock >= 0) {
        set_socket_stdio(sock);
      }

      network_installed = 1;
      return;
    }
    CloseLibrary(lss->lx_BsdSocketBase);
  }

  lss->lx_network_type = LX_AS225;
  if ((lss->lx_SocketBase=OpenLibrary("socket.library",3))) {

    SOCK_setup_sockets(128, &errno);

    sock = init_inet_daemon(&__argc, &__argv, lss);
    if (sock >= 0) {
      set_socket_stdio(sock);
    }

    network_installed = 1;
    return;
  }

  FreeSignal(lss->lx_sigio);
  FreeSignal(lss->lx_sigurg);

  lss->lx_network_type = LX_NONE;
}

void __exitsocket(void)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  DB( BUG("__exitsocket start\n"); )

  shutdown_inet_daemon(lss);

  switch (lss->lx_network_type) {
    case LX_AS225:
      DB( BUG("lss->lx_SocketBase is %lx\n", lss->lx_SocketBase); )
      if (lss->lx_SocketBase) {
        SOCK_cleanup_sockets();
        CloseLibrary(lss->lx_SocketBase);
        FreeSignal(lss->lx_sigio);
        FreeSignal(lss->lx_sigurg);
      }
    break;

    case LX_AMITCP:
      DB( BUG("lss->lx_UserGroupBase is %lx\n", lss->lx_UserGroupBase); )
      if (lss->lx_UserGroupBase)
        CloseLibrary(lss->lx_UserGroupBase);
      DB( BUG("lss->lx_BsdSocketBase is %lx\n", lss->lx_BsdSocketBase); )
      if (lss->lx_BsdSocketBase) {
        CloseLibrary(lss->lx_BsdSocketBase);
        FreeSignal(lss->lx_sigio);
        FreeSignal(lss->lx_sigurg);
      }
    break;

    default:
      /* silence compiler */
    break;
  }

  DB( BUG("__exitsocket done\n"); )
}

//ADD2INIT(__initsocket, -35); // used to be -10
//ADD2EXIT(__exitsocket, -35); // need to clean up FD's before closing socket library

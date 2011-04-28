#include <netdb.h>
#include <string.h>
#include <unistd.h>
//
#include "socket.h"

char *hstrerror(int err)
{ static char *const h_errlist[] = {
    "Resolver Error 0 (no error)",
    "Unknown host",			/* 1 HOST_NOT_FOUND */
    "Host name lookup failure",		/* 2 TRY_AGAIN */
    "Unknown server error",		/* 3 NO_RECOVERY */
    "No address associated with name",	/* 4 NO_ADDRESS */
  };
  struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP) {
    struct TagItem list[] = {
      {SBTM_GETREF(SBTC_HERRNOSTRPTR),0}, {TAG_END,TAG_END}
    };
    list[0].ti_Data=(ULONG)&err; return (char *)TCP_SocketBaseTagList(list);
  }
  else if (err < 0)
    return "Resolver internal error";
  else if (err < (int)(sizeof(h_errlist)/sizeof(h_errlist[0])))
    return h_errlist[err];
  return "Unknown resolver error";
}

void herror(const char *s)
{
  if (s && *s)
    write(STDERR_FILENO,s,strlen(s)),write(STDERR_FILENO,": ",2);
  s = hstrerror(h_errno);
  write(STDERR_FILENO,s,strlen(s)),write(STDERR_FILENO,"\n",1);
}

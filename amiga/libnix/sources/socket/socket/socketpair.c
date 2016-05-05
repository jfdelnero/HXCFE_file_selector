#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

int socketpair(int d, int type, int protocol, int sv[2])
{
  errno = ENOSYS; return -1;
}

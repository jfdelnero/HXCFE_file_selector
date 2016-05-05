#include <unistd.h>
#include <sys/param.h>

char *getwd(char *buf)
{
  return getcwd(buf,MAXPATHLEN);
}

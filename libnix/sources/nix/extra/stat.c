#include <sys/types.h>
#include <sys/stat.h>
#include "stabs.h"


int stat(const char *name,struct stat *buf)
{
  return -1;
}

ALIAS(lstat,stat);

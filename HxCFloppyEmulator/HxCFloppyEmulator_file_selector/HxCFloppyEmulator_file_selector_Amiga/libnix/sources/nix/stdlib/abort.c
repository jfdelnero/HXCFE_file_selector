#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void abort(void)
{
  raise(SIGABRT);
  fputs("Program aborted\n",stderr);
  exit(20);
}

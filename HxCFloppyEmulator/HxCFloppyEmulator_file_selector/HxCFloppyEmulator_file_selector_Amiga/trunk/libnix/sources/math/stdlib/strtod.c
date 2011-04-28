#include <stdio.h>

double strtod(const char *nptr,char **endptr)
{ double a;
  unsigned long n=0;
  sscanf(nptr,"%lf%ln",&a,&n);
  if(endptr)
    *endptr=(char *)nptr+n;
  return a;
}

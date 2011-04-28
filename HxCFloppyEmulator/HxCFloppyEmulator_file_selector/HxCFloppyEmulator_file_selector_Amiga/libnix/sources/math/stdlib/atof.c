#include <stdio.h>

double atof(const char *nptr)
{ double a;
  sscanf(nptr,"%lf",&a);
  return a;
}

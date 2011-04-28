#include <proto/mathieeedoubbas.h>

static inline double floor(double x)
{ return IEEEDPFloor(x); }

static inline double ceil(double x)
{ return IEEEDPCeil(x); }

double fmod(double x,double y)
{
  double a=x/y;
  if(a>=0)
    return x-y*floor(a);
  else
    return x-y*ceil(a);
}

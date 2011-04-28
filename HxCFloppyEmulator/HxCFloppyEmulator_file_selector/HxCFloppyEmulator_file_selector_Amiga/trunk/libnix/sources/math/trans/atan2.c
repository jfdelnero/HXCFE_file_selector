#include <proto/mathieeedoubtrans.h>

#define PI 3.14159265358979323846

static inline double atan(double x)
{ return IEEEDPAtan(x); }

double atan2(double y,double x)
{ return x>=y?(x>=-y?      atan(y/x):     -PI/2-atan(x/y)):
              (x>=-y? PI/2-atan(x/y):y>=0? PI  +atan(y/x):
                                          -PI  +atan(y/x));
}

#undef __HAVE_68881__
#include <math.h>

double modf(double x,double *p)
{
    if(x<0){
        *p=ceil(x);
        return(*p-x);
    }else{
        *p=floor(x);
        return(x-*p);
    }
}

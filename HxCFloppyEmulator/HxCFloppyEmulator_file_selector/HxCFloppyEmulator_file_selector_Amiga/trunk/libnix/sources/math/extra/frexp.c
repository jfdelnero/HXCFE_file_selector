#undef __HAVE_68881__
#include <math.h>

double frexp(double x,int *p)
{
  int neg,j;
  j=neg=0;

  if(x<0){
      x=-x;
      neg=1;
  }
  if(x>=1){
    while(x>=1){
        j++;
        x/=2;
      }
  }else if(x<0.5&&x!=0){
    while(x<0.5){
        j--;
        x*=2;
      }
  }
  *p = j;
  if(neg) x=-x;
  return x;
}

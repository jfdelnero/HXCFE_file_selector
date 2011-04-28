#include <stdlib.h>

void *calloc(size_t nmemb,size_t size)
{
  size_t l;
  size_t *a;
  void *b;
  l=(nmemb*size+(sizeof(size_t)-1))&~(sizeof(size_t)-1);
  a=(size_t *)(b=malloc(l));
  if(b!=NULL)
  {
    do
      *a++=0;
    while((l-=sizeof(size_t))!=0);
  }
  return b;
}

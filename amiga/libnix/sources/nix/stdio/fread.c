#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int fread(void *ptr,size_t size,size_t nmemb,FILE *stream)
{
  signed long subsize;
  unsigned long total;
  unsigned char *b=(unsigned char *)ptr;
  if(!(total=size*nmemb)) /* Just in case size==0 */
    return total;
  do
  {
    if(stream->incount>0)
    {
      subsize=total>stream->incount?stream->incount:total;
      memcpy(b,stream->p,subsize);
      stream->p+=subsize;
      stream->incount-=subsize;
      b+=subsize;
      total-=subsize;
    }else
    {
      int c;
      if((c=__srget(stream))==EOF)
        break;
      *b++=c;
      total--;
    }
  }while(total);
  return (b-(unsigned char *)ptr)/size;
}

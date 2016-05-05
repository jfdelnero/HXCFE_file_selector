#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int fwrite(const void *ptr,size_t size,size_t nmemb,FILE *stream)
{
  signed long subsize;
  unsigned long total;
  unsigned char *b=(unsigned char *)ptr;
  if(!(total=size*nmemb)) /* Just in case size==0 */
    return total;
  do
  {
    if(stream->outcount>0)
    {
      subsize=total>stream->outcount?stream->outcount:total;
      memcpy(stream->p,b,subsize);
      stream->p+=subsize;
      stream->outcount-=subsize;
      b+=subsize;
      total-=subsize;
    }else
    {
      int c;
      c=*b++;
      if(putc(c,stream)==EOF)
        break;
      total--;
    }
  }while(total);
  return (b-(unsigned char *)ptr)/size;
}

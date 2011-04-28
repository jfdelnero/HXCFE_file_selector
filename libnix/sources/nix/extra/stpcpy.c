#include <string.h>

char *stpcpy(char *dest,const char *source)
{
  while((*dest++=*source++)); return(dest-1);
}

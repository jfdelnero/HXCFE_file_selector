#include <stdio.h>

char *fgets(char *s,int n,FILE *stream)
{ char *s2=s;

  while(--n)
  { int c=fgetc(stream);
    if(c==EOF)
    { if(s2==s)
        return NULL;
      break;
    }
    *s2++=c;
    if(c=='\n')
      break;
  }
  *s2++='\0';
  return s;
}

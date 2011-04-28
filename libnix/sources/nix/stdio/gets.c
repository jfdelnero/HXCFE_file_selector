#include <stdio.h>

char *gets(char *s)
{ char *s2=s;

  for(;;)
  { int c=fgetc(stdin);
    if(c==EOF)
    { if(s2==s)
        return NULL;
      break;
    }
    if(c=='\n')
      break;
    *s2++=c;
  }
  *s2++='\0';
  return s;
}

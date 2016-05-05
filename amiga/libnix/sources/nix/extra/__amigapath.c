#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "stabs.h"

static int __pathconv;

/* Convert Un*x style pathnames to Amiga OS ones */

char *__amigapath(const char *path)
{ static char *s1=NULL;
  char *s2,*s3;
  int c;

  if(!__pathconv)
    return (char *)path;

  if(s1!=NULL)
    free(s1);

  s1=strdup(path);
  if(s1==NULL)
  { errno=ENOMEM;
    return NULL; }
  
  s3=s2=s1; /* Replace multiple following '/' by single ones */
  do
  { if(*s2=='/')
      while(s2[1]=='/')
        s2++;
    *s3++=*s2;
  }while(*s2++);

  s3=s2=s1; /* Remove single dots '.' as directory names */
  c=1;
  do
  { while(c&&s2[0]=='.'&&(s2[1]=='/'||s2[1]=='\0'))
    { s2++;
      if(*s2=='/')
        s2++;
    }
    *s3++=*s2;
    c=0;
    if(*s2=='/')
      c=1;
  }while(*s2++);
  
  s3=s2=s1; /* Remove double dots '..' as directory names */
  c=1;
  do
  { if(c&&s2[0]=='.'&&s2[1]=='.')
    { if(s2[2]=='/')
        s2+=2; 
      else if(s2[2]=='\0')
      { *s3++='/';
        s2+=2; }
    }
    *s3++=*s2;
    c=0;
    if(*s2=='/')
      c=1;
  }while(*s2++);

  if(*s1=='/') /* Convert names beginning with '/' */
  { s3=s2=s1;
    s2++;
    if(*s2=='/'||*s2=='\0') /* The root directory */
      return "SYS:";
    while(*s2!='/'&&*s2!='\0')
      *s3++=*s2++;
    *s3++=':';
    if(*s2=='/')
      s2++;
    do
      *s3++=*s2;
    while(*s2++);
  }

  return s1;
}

void __initamigapath(void)
{ char *s;
  s=getenv("NOIXPATHS"); /* Check explicitly for "1", so we can override it locally */
  if(s&&s[0]=='1'&&s[1]=='\0') /* with 'set NOIXPATHS 0' */
    __pathconv=1;
}

//ADD2INIT(__initamigapath,-10);

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "stabs.h"

struct atexitnode
{ 
  struct atexitnode *next;
  void (*func)(void);
};

static struct atexitnode *__funclist=NULL; /* List of functions to call at exit */

int atexit(void (*func)(void))
{ 
  struct atexitnode *node;
  if((node=(struct atexitnode *)malloc(sizeof(struct atexitnode)))==NULL)
  { errno=ENOMEM;
    return -1; }
  else
  { node->next=__funclist;
    node->func=func;
    __funclist=node;
    return 0; }
}

void __exitatexit(void)
{
  struct atexitnode *thisf=__funclist;
  while(thisf!=NULL)
  { (*thisf->func)();
    thisf=thisf->next; }
}

ADD2EXIT(__exitatexit,0);

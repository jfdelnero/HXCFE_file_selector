#if 1

#include <exec/tasks.h>
#include <exec/memory.h>
#include <proto/exec.h>

struct newMemList {
  struct Node     nml_Node;
  UWORD           nml_NumEntries;
  struct MemEntry nml_ME[2];
};

const struct newMemList MemTemplate = {
  {0,},
  2,
  { {MEMF_CLEAR|MEMF_PUBLIC, sizeof(struct Task)},
    {MEMF_CLEAR, 0} }
};

#define NEWLIST(l) ((l)->lh_Head = (struct Node *)&(l)->lh_Tail, \
                    /*(l)->lh_Tail = NULL,*/ \
                    (l)->lh_TailPred = (struct Node *)&(l)->lh_Head)

struct Task *CreateTask(STRPTR name, LONG pri, APTR initpc, ULONG stacksize)
{ struct Library *SysBase = *(struct Library **)4L;
  struct newMemList nml;
  struct MemList *ml;
  struct Task *newtask;
  APTR task2;

  stacksize=(stacksize+3)&~3;

  { long *p1,*p2;
    int i;

    for (p1=(long *)&nml,p2=(long*)&MemTemplate,i=7; i; *p1++=*p2++,i--) ;
    *p1=stacksize;
  }

  if (!(((unsigned int)ml=AllocEntry((struct MemList *)&nml)) & (1<<31))) {
    newtask=ml->ml_ME[0].me_Addr;
    newtask->tc_Node.ln_Type = NT_TASK;
    newtask->tc_Node.ln_Pri  = pri;
    newtask->tc_Node.ln_Name = name;
    newtask->tc_SPReg        = (APTR)((ULONG)ml->ml_ME[1].me_Addr+stacksize);
    newtask->tc_SPLower      = ml->ml_ME[1].me_Addr;
    newtask->tc_SPUpper      = newtask->tc_SPReg;
    NEWLIST(&newtask->tc_MemEntry);
    AddHead(&newtask->tc_MemEntry,&ml->ml_Node);
    task2=AddTask(newtask,initpc,0);
    if (SysBase->lib_Version>36 && !task2) {
      FreeEntry(ml); newtask = NULL;
    }
  }
  else
    newtask = NULL;

  return newtask;
}

#else

asm("
		.globl	_CreateTask

_CreateTask:	moveml	d2/d3/a2/a3/a6,sp@-
		movel	sp@(12+6*4:W),d0
		addql	#3,d0
		moveq	#-4,d2
		andl	d0,d2			| stack adjusted
		movel	d2,sp@-
		lea	pc@(Lmemlist-.+2+7*4),a0
		moveq	#6,d0
L3:		movel	a0@-,sp@-		| copy memlist
		dbra	d0,L3
		movel	sp,a0
		movel	4:W,a6
		jsr	a6@(-222:W)		| AllocEntry()
		movel	d0,a2
		movel	a2,d0
		lea	sp@(32:W),sp
		blts	L2
		movel	a2@(16:W),a3
		moveb	#1,a3@(8:W)		| ln_Type
		moveb	sp@(7+6*4:W),a3@(9:W)	| ln_Pri
		movel	sp@(0+6*4:W),a3@(10:W)	| ln_Name
		movel	a2@(24:W),d0
		addl	d0,d2
		movel	d2,a3@(54:W)		| tc_SPReg
		movel	d0,a3@(58:W)		| tc_SPLower
		movel	d2,a3@(62:W)		| tc_SPUpper
		lea	a3@(74:W),a0		| tc_MemList
		movel	a0,a0@(8:W)
		addqw	#4,a0
		movel	a0,a0@-
		movel	a2,a1
		jsr	a6@(-240:W)		| AddHead()
		movel	a3,d3
		movel	d3,a1
		movel	sp@(8+6*4:W),d2
		exg	d2,a2
		subal	a3,a3
		jsr	a6@(-282:W)		| AddTask()
		cmpw	#30,a6@(20:W)
		bcss	L1			| kick2.0+ ?
		tstl	d0
		bnes	L1			| task added
		movel	d2,a0
		jsr	a6@(-228:W)		| FreeEntry()
L2:		moveq	#0,d3
L1:		movel	d3,d0			| new task
		moveml	sp@+,d2/d3/a2/a3/a6
		rts

		.align	2

Lmemlist:	.long	0,0			| Succ,Pred
		.byte	0,0			| Pri,Type
		.long	0			| Name
		.word	2			| NumEntries
		.long	0x10001			| MemType
		.long	92			| Length
		.long	0x10000			| MemType
|		.long	0			| Length
");

#endif

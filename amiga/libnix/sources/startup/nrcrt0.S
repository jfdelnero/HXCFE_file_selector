| residentable baserelative startup code for gcc v2.3.3+
| (c) by M.Fleischer and G.Nikl Wed Apr 13 17:46 1994
| No bugs known
|
| Problem: geta4()
| I really don´t know how to implement it with multiple starts
|
| You should use the non-resident startup-code if you need it !

| some specific defines

_LVOForbid	=	-132
_LVOAllocMem	=	-198
_LVOFindTask	=	-294
_LVOFreeMem	=	-210
_LVOGetMsg	=	-372
_LVOReplyMsg	=	-378
_LVOWaitPort	=	-384
_LVOCopyMemQuick =	-630

pr_MsgPort	=	  92
pr_CLI		=	 172

ERROR_NO_FREE_STORE =	 103

| public symbols

		.globl	__exit
		.text

| first entry - check for cli or wb start

start:		movel	a0,d3
		movel	d0,d4

		movel	4:W,a6

		subal	a1,a1
		jsr	a6@(_LVOFindTask:W)
		movel	d0,a3
		tstl	a3@(pr_CLI:W)
		subal	a2,a2
		bne	fromCLI

| wb start - get our message

fromWB: 	lea	a3@(pr_MsgPort:W),a0
		jsr	a6@(_LVOWaitPort:W)
		lea	a3@(pr_MsgPort:W),a0
		jsr	a6@(_LVOGetMsg:W)
		movel	d0,a2

| resident setup - allocate memory and relocate

fromCLI:	movql	#ERROR_NO_FREE_STORE,d7
		movel	#___data_size,d5
		movel	#___bss_size,d2
		addl	d5,d2
		movel	d2,d0
		movql	#1,d1
		swap	d1
		jsr	a6@(_LVOAllocMem:W)
		movel	d0,a4
		movel	a4,d0
		beq	cleanup
		lea	___a4_init-0x7ffe,a5
		movel	d5,d0
		movel	a5,a0
		movel	a4,a1
		jsr	a6@(_LVOCopyMemQuick:W)
		lea	___datadata_relocs,a0
		movel	a0@+,d0
		beq	no_reloc
		movel	a5,d1
		subl	a4,d1
do_reloc:	movel	a0@+,a1
		subl	d1,a4@(a1:l)
		subql	#1,d0
		bne	do_reloc
no_reloc:	lea	a4@(0x7ffe:W),a4

		movel	sp,a4@(___SaveSP:W)
		movel	a6,a4@(_SysBase:W)
		movel	a2,a4@(__WBenchMsg:W)
		movel	d3,a4@(___commandline:W)
		movel	d4,a4@(___commandlen:W)
		movel	d2,a4@(___MemLen:W)

| call main() - push args onto stack

		lea	a4@(___INIT_LIST__:W),a2
		addqw	#4,a2
		moveql	#-1,d2
		jbsr	callfuncs
		movel	a4@(___env:W),sp@-
		movel	a4@(___argv:W),sp@-
		movel	a4@(___argc:W),sp@-
		jbsr	_main
		movel	d0,sp@(4:W)

| exit() entry - get return val, restore stackptr

__exit: 	lea	a4@(___EXIT_LIST__:W),a2
		addqw	#4,a2
		moveql	#0,d2
		jbsr	callfuncs

		movel	sp@(4:W),d7
		movel	a4@(___SaveSP:W),sp

| prepare cleanup - setup regs, free resident memory

		movel	a4@(_SysBase:W),a6

		movel	a4@(__WBenchMsg:W),a2
		movel	a4@(___MemLen:W),d0
		lea	a4@(-0x7ffe:W),a1
		jsr	a6@(_LVOFreeMem:W)

| cleanup - reply wbmsg and exit

cleanup:	movel	a2,d0
		beqs	todos
		jsr	a6@(_LVOForbid:W)
		movel	a2,a1
		jsr	a6@(_LVOReplyMsg:W)
todos:		movel	d7,d0
		rts

| call all functions in the NULL terminated list pointed to by a2
| d2 ascending or descending priority mode

callfuncs:	lea	a4@(cleanupflag:W),a5
		movel	a2,a3
		moveql	#0,d3
		jra	oldpri
stabloop:	movel	a3@+,d4
		movel	a5@,d5
		cmpl	d4,d5
		jne	notnow
		movel	d0,a0
		jsr	a0@
notnow: 	eorl	d2,d4
		eorl	d2,d5
		cmpl	d5,d4
		jcc	oldpri
		cmpl	d3,d4
		jls	oldpri
		movel	d4,d3
oldpri: 	movel	a3@+,d0
		jne	stabloop
		eorl	d2,d3
		movel	d3,a5@
		cmpl	d2,d3
		jne	callfuncs
		rts

| redirection of _exit

		.stabs	"_exit",11,0,0,0
		.stabs	"__exit",1,0,0,0

| data area

		.data

		.long ___nocommandline
		.long ___initlibraries
		.long ___cpucheck

.comm		_SysBase,4
.comm		___SaveSP,4
.comm		__WBenchMsg,4
.comm		___commandline,4
.comm		___commandlen,4
.comm		___MemLen,4
.comm		___argc,4
.comm		___argv,4
.comm		___env,4
.lcomm		cleanupflag,4

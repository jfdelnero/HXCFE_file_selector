#include <time.h>
#include <limits.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include "stabs.h"

static struct DateStamp ds;

void __initclock(void)
{ DateStamp(&ds); }

//ADD2INIT(__initclock,-10);

clock_t clock(void)
{ struct DateStamp ds2;
  DateStamp(&ds2);
  return (((ds2.ds_Days-ds.ds_Days)*(24*60)+
           ds2.ds_Minute-ds.ds_Minute)*(60*TICKS_PER_SECOND)+
           ds2.ds_Tick-ds.ds_Tick)*CLOCKS_PER_SEC/TICKS_PER_SECOND;
}

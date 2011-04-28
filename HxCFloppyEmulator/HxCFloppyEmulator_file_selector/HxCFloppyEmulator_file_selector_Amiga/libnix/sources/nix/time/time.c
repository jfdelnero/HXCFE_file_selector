#include <time.h>
#include <dos/dos.h>
#include <proto/dos.h>

extern long __gmtoffset;

time_t time(time_t *tloc)
{ struct DateStamp t;
  time_t ti;
  DateStamp(&t); /* Get timestamp */
  ti=((t.ds_Days+2922)*1440+t.ds_Minute+__gmtoffset)*60+
     t.ds_Tick/TICKS_PER_SECOND;
  if(tloc!=NULL)
    *tloc=ti;
  return ti;
}

/*
 * 2922 is the number of days between 1.1.1970 and 1.1.1978 (2 leap years and 6 normal)
 * 1440 is the number of minutes per day
 *   60 is the number of seconds per minute 
 */ 

#include <stdio.h>

void (*__signalfunc[6])(int)={ NULL }; /* array of signalhandlers */
int __signalpending=0,__signalmask=0;

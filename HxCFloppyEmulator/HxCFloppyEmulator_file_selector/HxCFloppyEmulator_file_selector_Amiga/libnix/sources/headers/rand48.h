#ifndef _HEADERS_RAND48_H
#define _HEADERS_RAND48_H

void srand48(long);
unsigned short *seed48(unsigned short *);
void lcong48(unsigned short *);
long lrand48(void);
long nrand48(unsigned short *);
long mrand48(void);
long jrand48(unsigned short *);
double drand48(void);
double erand48(unsigned short *seed);

#endif /* _HEADERS_RAND48_H */

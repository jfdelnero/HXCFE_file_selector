/* Random number generation using the linear congruential algorithm
   X(n+1) = (a * X(n) + c) mod m
   with a precision of 48 bits.

   Author: Kriton Kyrimis (kyrimis@theseas.softlab.ece.ntua.gr)
   Code status: Public Domain.
*/

#include <limits.h>
#include <float.h>

/* Parameters for the linear congruential algorithm:
   parm[0..2] is the current value of Xn (internal seed, m.s.word last)
   parm[3..5] is the value of a (m.s.word last)
   parm[6] is the value of c.
*/
#define X0 0x1234 /* MSB * Initial value for Xn, obtained using seed48() */
#define X1 0xABCD	/* on SunOS 4.1.3 */
#define X2 0x330E

#define A0 0x0005 /* MSB * Default value for a, taken from the man page */
#define A1 0xDEEC
#define A2 0xE66D

#define C0 0x000B	/* Default value for c, taken from the man page */

static unsigned short parm[7] = {
  X2, X1, X0,
  A2, A1, A0,
  C0
};

/* To produce a double random number in [0,1) we get a 32-bit unsigned long
   random number, convert it to double, and divide it by ULONG_MAX + EPSILON.
   (We add the EPSILON to exclude 1.0 from the set of possible results.) 

   We derive EPSILON by noting that for a random value of ULONG_MAX,
   we want to return the smallest double that is less than 1.0.
   Therefore:

        ULONG_MAX
   --------------------- = (1.0 - DBL_EPSILON)
   (ULONG_MAX + EPSILON)

   (This is probably overkill.)

*/

#define EPSILON	(double)ULONG_MAX*(1.0/(1.0-DBL_EPSILON)-1.0)


/*--------------------------------------------------------------------------*
 * Parameter initialization functions                                       *
 *--------------------------------------------------------------------------*/

/* This function sets the two m.s.words of the internal seed to the value
   supplied by the caller, and the l.s.word of the internal seed to 0x330E.
*/
void
srand48(long seed)
{
  parm[0] = 0x330E;
  parm[1] = ((unsigned long)seed) & 0xFFFF;
  parm[2] = ((unsigned long)seed >> 16) & 0xFFFF;
  parm[3] = A2;
  parm[4] = A1;
  parm[5] = A0;
  parm[6] = C0;
}

/* This function sets all three words of the internal seed to the value
   supplied by the caller. It returns a pointer to an array containing
   a copy of the previous value of the internal seed.
*/
unsigned short *
seed48(unsigned short *seed)
{
  static unsigned short oldparm[3];
  unsigned short tmpparm[3];

  /* Can't assign oldparm[] = parm[] directly, because seed[] may be a pointer
     to oldparm[], obtained from a previous call to seed48 , in which case
     we would destroy the contents of seed[] */
  tmpparm[0] = parm[0];
  tmpparm[1] = parm[1];
  tmpparm[2] = parm[2];
  parm[0] = seed[0];
  parm[1] = seed[1];
  parm[2] = seed[2];
  oldparm[0] = tmpparm[0];
  oldparm[1] = tmpparm[1];
  oldparm[2] = tmpparm[2];
  parm[3] = A2;
  parm[4] = A1;
  parm[5] = A0;
  parm[6] = C0;

  return oldparm;
}

/* This function sets all seven words of the internal parameters array to the
   values supplied by the caller.
*/
void
lcong48(unsigned short *new_parm)
{
  parm[0] = new_parm[0];
  parm[1] = new_parm[1];
  parm[2] = new_parm[2];
  parm[3] = new_parm[3];
  parm[4] = new_parm[4];
  parm[5] = new_parm[5];
  parm[6] = new_parm[6];
}


/*--------------------------------------------------------------------------*
 * Random number generator                                                  *
 *--------------------------------------------------------------------------*/

/* This function implements the linear congruential algorithm.  Thanks to
   gcc's long long ints, implementing 48-bit arithmetic (actually 64-bit,
   truncating the result) is trivial.  Limitations of long long int
   implementation in (amiga?) gcc 2.7.0, made me use the kludge with the union
   to convert from short[3] to long long int. (It's probably faster though!)

   This function takes an array of three shorts (a 48-bit seed, m.s.word
   last) and returns a long between -2**31 and 2**31-1, updating the seed
   (the result is the two m.s.words of the updated seed).
*/

static long
rng(unsigned short *seed)
{
  long long int Xn, Xn1, a, c;
  union {
    long long int l;
    unsigned short s[4];
  } i;

  i.s[0] = 0;
  i.s[1] = seed[2];
  i.s[2] = seed[1];
  i.s[3] = seed[0];
  Xn = i.l;

  i.s[0] = 0;
  i.s[1] = parm[5];
  i.s[2] = parm[4];
  i.s[3] = parm[3];
  a = i.l;

  c =  (long long int)(parm[6]);

  Xn1 = a * Xn + c;

  i.l = Xn1;
  seed[0] = i.s[3];
  seed[1] = i.s[2];
  seed[2] = i.s[1];

  return (long)((((unsigned long)seed[2]) << 16) + seed[1]);
}


/*--------------------------------------------------------------------------*
 * Interface functions to the random number generator                       *
 *--------------------------------------------------------------------------*/

/* This function returns a long between 0 and 2**31-1 by calling rng
   with the internal seed, returning the 15 most significant bits of the
   result shifted by one position to the right.
*/
long
lrand48(void)
{
  return (rng(parm) >> 1) & 0x7FFFFFFF;
}

/* Same as lrand48(), but using an external seed. */

long
nrand48(unsigned short seed[3])
{
  return (rng(seed) >> 1) & 0x7FFFFFFF;
}

/* This function returns a long between -2**31 and 2**31-1 by calling rng
   with the internal seed.
*/
long
mrand48(void)
{
  return rng(parm);
}

/* Same as mrand48(), but using an external seed. */

long
jrand48(unsigned short seed[3])
{
  return rng(seed);
}

/* This function returns a double in the interval [0,1) by calling mrand48()
   and dividing the result by ULONG_MAX + EPSILON. */

double
drand48(void)
{
  union {
    long l;
    unsigned long u;
  } x;

  x.l = mrand48();
  return (double)x.u / ((double)(ULONG_MAX) + EPSILON);
}

/* Same as drand48(), but using an external seed. */

double
erand48(unsigned short seed[3])
{
  union {
    long l;
    unsigned long u;
  } x;

  x.l = nrand48(seed);
  return (double)x.u / ((double)(ULONG_MAX) + EPSILON);
}

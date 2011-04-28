#include "bases.h"


unsigned long __mulsi3(unsigned long a, unsigned long b)
{
 unsigned long r = 0;

  while (a)
    {
      if (a & 1)
        r += b;
      a >>= 1;
      b <<= 1;
    }

  return r;
}

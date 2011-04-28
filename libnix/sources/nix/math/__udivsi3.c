#include "bases.h"

static unsigned int _udiv_Normalize(unsigned int dividend, unsigned int divisor)
{
    unsigned int count;

    count=0;
    if (divisor <= dividend>>(count+16)) count+=16;
    if (divisor <= dividend>>(count+8))  count+=8;
    if (divisor <= dividend>>(count+4))  count+=4;
    if (divisor <= dividend>>(count+2))  count+=2;
    if (divisor <= dividend>>(count+1))  count+=1;
    return count;
}


unsigned int __umodsi3(unsigned int dividend, unsigned int divisor)
{
    unsigned int count;

    count = _udiv_Normalize(dividend,divisor);
    divisor <<= count;

    /* now do the division */
    do
    {
        if (dividend >= divisor)
            dividend -= divisor;
        divisor >>= 1;
        if (!count--)
            break;

        if (dividend >= divisor)
            dividend -= divisor;
        divisor >>= 1;
     } while (count--);

    return dividend;
}





unsigned int __udivsi3(unsigned int dividend, unsigned int divisor)
{
    unsigned int result,count;

    count = _udiv_Normalize(dividend,divisor);
    divisor <<= count;

    /* now do the division */
    result = 0;
    do
    {
        result = (result<<1) | (dividend >= divisor);
        if (dividend >= divisor)
            dividend -= divisor;
        divisor >>= 1;
        if (!count--) return result;

        result = (result<<1) | (dividend >= divisor);
        if (dividend >= divisor)
            dividend -= divisor;
        divisor >>= 1;
     } while (count--);

    return result;
}

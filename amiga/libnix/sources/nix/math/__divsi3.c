#include "bases.h"

unsigned int __udivsi3(unsigned int dividend, unsigned int divisor);
unsigned int __umodsi3(unsigned int dividend, unsigned int divisor);



int __modsi3(int dividend, int divisor)
{
    int DividendNegSign,DivisorNegSign;
    int result;

    DividendNegSign = dividend & 0x80000000;
    DivisorNegSign  = divisor  & 0x80000000;

    if (DividendNegSign)
        dividend = -dividend;
    if (DivisorNegSign)
        divisor = -divisor;

    result = (int)__umodsi3(dividend,divisor);
    if (DividendNegSign)
        result = -result;

    return result;
}



int __divsi3(int dividend, int divisor)
{
    int DividendNegSign,DivisorNegSign;
    int result;

    DividendNegSign = dividend & 0x80000000;
    DivisorNegSign  = divisor  & 0x80000000;

    if (DividendNegSign)
        dividend = -dividend;
    if (DivisorNegSign)
        divisor = -divisor;

    result = (int)__udivsi3(dividend,divisor);
    if (DividendNegSign ^ DivisorNegSign)
        result = -result;

    return result;
}


unsigned long __divsi4(unsigned long num, unsigned long den)
{
 return __divsi3(num,den);
}

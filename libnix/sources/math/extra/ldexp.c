#undef __HAVE_68881__
#include <math.h>

#define MANT_MASK 0x800FFFFF    /* Mantissa extraction mask     */
#define ZPOS_MASK 0x3FF00000    /* Positive # mask for exp = 0  */
#define ZNEG_MASK 0x3FF00000    /* Negative # mask for exp = 0  */

#define EXP_MASK 0x7FF00000     /* Mask for exponent            */
#define EXP_SHIFTS 20           /* Shifts to get into LSB's     */
#define EXP_BIAS 1023           /* Exponent bias                */


union dtol
{
  double dval;
  int ival[2];
};

double ldexp (double x,int n)
{
  union dtol number;
  int *iptr, cn;

  if (x == 0.0)
    return (0.0);
  else
    {
      number.dval = x;
      iptr = &number.ival[0];
      cn = (((*iptr) & EXP_MASK) >> EXP_SHIFTS) - EXP_BIAS;
      *iptr &= ~EXP_MASK;
      n += EXP_BIAS;
      *iptr |= ((n + cn) << EXP_SHIFTS) & EXP_MASK;
      return (number.dval);
    }
}

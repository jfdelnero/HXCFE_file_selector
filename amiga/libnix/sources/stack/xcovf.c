#include <dos/dos.h>
#include <stdlib.h>

void __request(const char *text);

void _XCOVF(void) /* stack overflow handler */
{
  __request("Stack overflow");
  exit(RETURN_FAIL);
}

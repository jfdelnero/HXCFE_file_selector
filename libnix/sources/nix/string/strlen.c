#include <string.h>

size_t strlen(const char *string)
{ const char *s=string;

  do;while(*s++); return ~(string-s);
}

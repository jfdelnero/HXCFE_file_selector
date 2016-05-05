#include <locale.h>

extern struct lconv __lconv;

struct lconv *localeconv(void)
{ return &__lconv; }

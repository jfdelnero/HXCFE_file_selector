#include "stabs.h"
extern char __dosname[];
void *__DOSBase[2]={ 0l,__dosname };
ADD2LIB(__DOSBase);

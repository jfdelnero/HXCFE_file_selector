#include "stabs.h"
extern char __mathieeesingbasname[];
void *__MathIeeeSingBasBase[2]={ 0l,__mathieeesingbasname };
ADD2LIB(__MathIeeeSingBasBase);

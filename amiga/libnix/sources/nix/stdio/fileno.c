#include <stdio.h>

int fileno(FILE *file)
{ return file->file; }

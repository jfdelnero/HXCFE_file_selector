int isnan(double a)
{ 
  unsigned long *b=(unsigned long *)&a;
  return (b[0]&0x7ff00000)==0x7ff00000&&((b[0]&0xfffff)||b[1]);
}

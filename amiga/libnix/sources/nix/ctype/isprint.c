extern char *_ctype_;

int isprint(int c)
{ return _ctype_[1+c]&151; }

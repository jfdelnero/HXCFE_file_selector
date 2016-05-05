extern char *_ctype_;

int isxdigit(int c)
{ return _ctype_[1+c]&68; }

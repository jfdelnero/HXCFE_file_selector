extern char *_ctype_;

int ispunct(int c)
{ return _ctype_[1+c]&16; }

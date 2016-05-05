extern char *_ctype_;

int islower(int c)
{ return _ctype_[1+c]&2; }

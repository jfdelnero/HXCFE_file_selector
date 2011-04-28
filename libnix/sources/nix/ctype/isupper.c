extern char *_ctype_;

int isupper(int c)
{ return _ctype_[1+c]&1; }

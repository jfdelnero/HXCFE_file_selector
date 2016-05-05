extern char *_ctype_;

int isgraph(int c)
{ return _ctype_[1+c]&23; }

extern char *_ctype_;

int tolower(int c)
{ return _ctype_[1+c]&1?c+'a'-'A':c; }

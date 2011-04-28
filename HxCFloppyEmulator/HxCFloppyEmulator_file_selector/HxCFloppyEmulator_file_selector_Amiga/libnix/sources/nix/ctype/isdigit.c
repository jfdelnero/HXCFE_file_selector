extern char *_ctype_;

int isdigit(int c)
{ return _ctype_[1+c]&4; }

extern char *_ctype_;

int isspace(int c)
{ return _ctype_[1+c]&8; }

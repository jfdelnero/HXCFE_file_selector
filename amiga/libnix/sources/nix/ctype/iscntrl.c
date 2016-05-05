extern char *_ctype_;

int iscntrl(int c)
{ return _ctype_[1+c]&32; }

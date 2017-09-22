#define L_INDIAN(var) (var)
#define B_INDIAN(var) (((var&0x000000FF)<<24) |((var&0x0000FF00)<<8) |((var&0x00FF0000)>>8) |((var&0xFF000000)>>24))

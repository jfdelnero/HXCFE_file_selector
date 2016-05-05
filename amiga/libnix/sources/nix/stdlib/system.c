#include <dos/dos.h>
#include <utility/tagitem.h>
#include <proto/dos.h>

int system(const char *string)
{ static struct TagItem notags[]={ { TAG_END,0 } };

  if(!string)
    return 1;
  else
    return SystemTagList((char *)string,notags); 
}

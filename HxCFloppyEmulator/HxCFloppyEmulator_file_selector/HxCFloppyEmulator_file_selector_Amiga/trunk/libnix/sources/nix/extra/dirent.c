#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <strsup.h>

DIR *opendir(const char *dirname)
{ DIR *dirp;

  if ((dirp=(DIR *)AllocVec(sizeof(DIR),MEMF_PUBLIC)) != NULL) {
    if ((dirp->d_lock=(Lock((STRPTR)dirname,SHARED_LOCK))) != 0ul) {
      dirp->d_count=0; dirp->d_more = DOSTRUE;
      if ((dirp->d_eac=AllocDosObject(DOS_EXALLCONTROL,NULL)) != NULL) {
#if 0
        dirp->d_eac->eac_LastKey=0;
        dirp->d_eac->eac_MatchString=NULL;
        dirp->d_eac->eac_MatchFunc=NULL;
#endif
        if (Examine(dirp->d_lock,&dirp->d_info)) {
          if (dirp->d_info.fib_EntryType>=0)
            return dirp;
        }
        FreeDosObject(DOS_EXALLCONTROL,dirp->d_eac);
      }
      UnLock(dirp->d_lock);
    }
    FreeVec(dirp); dirp=NULL;
  }
  return dirp;
}

struct dirent *readdir(DIR *dirp)
{ struct dirent *result;

  if (!dirp->d_count && dirp->d_more!=DOSFALSE) {
    dirp->d_more=ExAll(dirp->d_lock,(APTR)&dirp->d_ead[0],sizeof(dirp->d_ead),ED_NAME,dirp->d_eac);
    dirp->current=(struct ExAllData *)&dirp->d_ead[0];
    dirp->d_count=dirp->d_eac->eac_Entries;
  }

  if (result=NULL,dirp->d_count) {
    dirp->dd_ent.d_fileno = dirp->dd_ent.d_reclen = 1;
    strcpy(dirp->dd_ent.d_name,dirp->current->ed_Name);
    dirp->dd_ent.d_namlen = strlen(dirp->dd_ent.d_name);
    dirp->current=dirp->current->ed_Next;
    dirp->d_count--;
    result=&dirp->dd_ent;
  }

  return result;
}

void rewinddir(DIR *dirp)
{
  if (dirp->d_more!=DOSFALSE)
    do {
      dirp->d_more=ExAll(dirp->d_lock,(APTR)&dirp->d_ead[0],sizeof(dirp->d_ead),ED_NAME,dirp->d_eac);
    }
    while(dirp->d_more!=DOSFALSE);
  dirp->d_count=0; dirp->d_more = DOSTRUE; dirp->d_eac->eac_LastKey=0;
}

int closedir(DIR *dirp)
{
  rewinddir(dirp);
  FreeDosObject(DOS_EXALLCONTROL,dirp->d_eac);
  UnLock(dirp->d_lock);
  FreeVec(dirp);
  return 0;
}

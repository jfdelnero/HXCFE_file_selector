#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket.h"
#include "stabs.h"

/*
**
*/
static int logname_valid = 1;
static char logname[MAXLOGNAME];
static uid_t ruid, euid;
static gid_t rgid, egid;
static int ngroups;
static gid_t grouplist[NGROUPS_MAX];

/*
**
*/
uid_t getuid(void)
{
  return ruid;
}

uid_t geteuid(void)
{
  return euid;
}

int setuid(uid_t uid)
{
  return setreuid((int)uid, (int)uid);
}

int seteuid(uid_t uid)
{
  return setreuid(-1, (int)uid);
}

int setreuid(int truid, int teuid)
{
  if (truid != -1)
    ruid = (uid_t)truid;

  if (teuid != -1)
    euid = (uid_t)teuid;

  /* just always succeed... */
  return 0;
}

/*
**
*/
gid_t getgid(void)
{
  return rgid;
}

gid_t getegid(void)
{
  return egid;
}

int setgid(gid_t tgid)
{
  return setregid((int)tgid, (int)tgid);
}

int setegid(gid_t tgid)
{
  return setregid(-1, (int)tgid);
}

int setregid(int trgid, int tegid)
{
  if (trgid != -1)
    rgid = (gid_t)trgid;

  if (tegid != -1) {
    egid = (gid_t)tegid;
    grouplist[0] = egid;
  }

  /* just always succeed... */
  return 0;
}

/*
**
*/
char *getlogin(void)
{
  return (logname_valid && *logname ? logname : 0);
}

int setlogin(const char *name)
{
  strncpy (logname, name, MAXLOGNAME);
  logname[MAXLOGNAME] = '\0';
  logname_valid = 1;

  return 0;
}

/*
**
*/
int getgroups(int gidsetlen, int *gidset)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP)
    return UG_getgroups(gidsetlen,gidset);

  /* parameter check */
  if (!gidset || gidsetlen < 0)  {
    errno = EFAULT;
    return -1;
  }

  if ((gidsetlen < 1) || (gidsetlen < ngroups)) {
    errno = EINVAL;
    return -1;
  }

  for (gidsetlen = 0; gidsetlen < ngroups; gidsetlen++)
    gidset[gidsetlen] = grouplist[gidsetlen];

  return ngroups;
}

int setgroups(int gidsetlen, const int *gidset)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP)
    return UG_setgroups(gidsetlen,gidset);

  /* parameter check */
  if (!gidset || gidsetlen < 0) {
    errno = EFAULT;
    return -1;
  }

  if (!gidsetlen || gidsetlen > NGROUPS_MAX) {
    errno = EINVAL;
    return -1;
  }

  ngroups = gidsetlen;

  for (gidsetlen = 0; gidsetlen < ngroups; gidsetlen++) {
    grouplist[gidsetlen] = gidset[gidsetlen];
  }

  egid = (gid_t)(grouplist[0]);

  return 0;
}

int initgroups(const char *name, int basegid)
{ struct SocketSettings *lss = _lx_get_socket_settings();
  int gidset[NGROUPS_MAX], ngroups = 2;
  struct group *gr;
  char **grm;

  if (lss->lx_network_type == LX_AMITCP)
    return UG_initgroups(name,basegid);

  /* parameter check */
  if (!name || basegid < 0) {
    errno = EFAULT; return -1;
  }

  gidset[0] = gidset[1] = (gid_t)basegid;

  setgrent();

  while ((gr = getgrent())) {
    if (gr->gr_gid != (gid_t)basegid) {
      for (grm = gr->gr_mem; grm && *grm; grm++)  {
        if (!strcmp(name,*grm))  {
          gidset[ngroups++] = (int)(gr->gr_gid);
          break;
        }
      }
    }
    if (ngroups == NGROUPS_MAX) break;
  }

  endgrent();

  return setgroups(ngroups, gidset);
}

/*
**
*/
void __inituser(void)
{ struct passwd *pw;
  char *var;

  if (((var=getenv("LOGNAME")) == NULL) && ((var=getenv("USER")) == NULL))
    var = "nobody";
  strncpy(logname,var,MAXLOGNAME);
  logname[MAXLOGNAME] = '\0';
  logname_valid = 1;

  if ((pw=getpwnam(logname))) {
    ruid = euid = pw->pw_uid;
    rgid = egid = pw->pw_gid;
  }
  else  {
    ruid = euid = (uid_t)(-2);
    rgid = egid = (uid_t)(-2);
  }

  if (initgroups(logname, (int)rgid)) {
    grouplist[1] = (int)rgid;
    ngroups = 2;
  }

  if ((var = getenv("UID")))
    ruid = euid = (uid_t)atoi(var);

  if ((var = getenv("GID")))
    rgid = egid = (gid_t)atoi(var);

  if ((var = getenv("EUID")))
    euid = (uid_t)atoi(var);

  if ((var = getenv("EGID")))
    egid = (gid_t)atoi(var);

  grouplist[0] = (int)egid;
}
//ADD2INIT(__inituser, -9);

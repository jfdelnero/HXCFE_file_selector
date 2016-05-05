#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
//
#define _KERNEL
#include "socket.h"

#define PATH_PASSWD "etc:passwd"

#define MAXLINELENGTH  1024

#define UNIX_STRSEP    ":\n"
#define AMIGAOS_STRSEP "|\n"

static uid_t nextuid;

/*
** builtin passwd info
*/
static const struct passwd builtin_root =
{ "root", "", 0, 0, 0, "", "The Master of All Things", "SYS:", "sh", 0 };

static const struct passwd builtin_nobody =
{ "nobody", "", -2, -2, 0, "", "The Anonymous One", "T:", "sh", 0 };

/*
**
*/
static int pw_start(struct SocketSettings *lss)
{
  if (lss->lx_pwd_fp) {
    rewind(lss->lx_pwd_fp); return 1;
  }
  return ((lss->lx_pwd_fp = fopen(PATH_PASSWD, "r")) ? 1 : 0);
}

static int pw_scan(int search, uid_t uid, const char *name, struct SocketSettings *lss)
{
  if (!lss->lx_pwd_line)
    if ((lss->lx_pwd_line=malloc(MAXLINELENGTH + 1)) == NULL) {
      errno = ENOMEM; return 0;
    }

  for (;;) {
    char *sep, *bp, *cp;

    if (!fgets(lss->lx_pwd_line, MAXLINELENGTH, lss->lx_pwd_fp))
      return 0;

    /* skip lines that are too big */
    if (!strchr(lss->lx_pwd_line, '\n')) {
      int ch;

      while ((ch = getc(lss->lx_pwd_fp)) != '\n' && ch != EOF)
        ;

      continue;
    }

    sep = strchr(bp=lss->lx_pwd_line,'|') ? AMIGAOS_STRSEP : UNIX_STRSEP;

    lss->lx_pwd.pw_name = strsep(&bp, sep);

    if (search && name && strcmp(lss->lx_pwd.pw_name, name))
      continue;

    lss->lx_pwd.pw_passwd = strsep(&bp, sep);

    if (!(cp = strsep(&bp, sep)))
      continue;

    lss->lx_pwd.pw_uid = (uid_t)atoi(cp);

    if (search && !name && lss->lx_pwd.pw_uid != uid)
      continue;

    if (!(cp = strsep(&bp, sep))) 
      continue;

    lss->lx_pwd.pw_gid = (gid_t)atoi(cp);

    lss->lx_pwd.pw_gecos = strsep(&bp, sep);
    if (!lss->lx_pwd.pw_gecos) 
      continue;

    lss->lx_pwd.pw_dir = strsep(&bp, sep);
    if (!lss->lx_pwd.pw_dir) 
      continue;

    lss->lx_pwd.pw_shell = strsep(&bp, "\n");
    if (!lss->lx_pwd.pw_shell) 
      continue;

    return 1;
  }
  /* NOTREACHED */
}

/*
** change the AS225/INet225's password structure to the global format
*/
static struct passwd *__AS225InetPwd(struct AS225_passwd *pwd,struct SocketSettings *lss)
{
  if (pwd) {
    lss->lx_pwd.pw_name   = pwd->pw_name;
    lss->lx_pwd.pw_passwd = pwd->pw_passwd;
    lss->lx_pwd.pw_uid    = pwd->pw_uid;
    lss->lx_pwd.pw_gid    = pwd->pw_gid;
    lss->lx_pwd.pw_change = time((time_t *)NULL);
    lss->lx_pwd.pw_class  = NULL;
    lss->lx_pwd.pw_gecos  = pwd->pw_gecos;
    lss->lx_pwd.pw_dir    = pwd->pw_dir;
    lss->lx_pwd.pw_shell  = pwd->pw_shell;
    lss->lx_pwd.pw_expire = (time_t)-1;
    lss->lx_pwd.pw_change = (time_t)-1;
    return &lss->lx_pwd;
  }
  return NULL;
}

/*
** change the AmiTCP password structure to the global format
*/
static struct passwd *__TCP2InetPwd(struct TCP_passwd *pwd,struct SocketSettings *lss)
{
  if (pwd) {
    lss->lx_pwd.pw_name   = pwd->pw_name;
    lss->lx_pwd.pw_passwd = pwd->pw_passwd;
    lss->lx_pwd.pw_uid    = pwd->pw_uid;
    lss->lx_pwd.pw_gid    = pwd->pw_gid;
    lss->lx_pwd.pw_change = time((time_t *)NULL);
    lss->lx_pwd.pw_class  = NULL;
    lss->lx_pwd.pw_gecos  = pwd->pw_gecos;
    lss->lx_pwd.pw_dir    = pwd->pw_dir;
    lss->lx_pwd.pw_shell  = pwd->pw_shell;
    lss->lx_pwd.pw_expire = (time_t)-1;
    return &lss->lx_pwd;
  }
  return NULL;
}

/*
**
*/

int setpwent(void)
{
  return setpassent(0);
}

int setpassent(int stayopen)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      SOCK_setpwent(stayopen);
    break;

    case LX_AMITCP:
      UG_setpwent();
    break;

    default:
      if (nextuid=0,pw_start(lss))
        lss->lx_pwd_stayopen = stayopen;
    break;
  }

  return 1;
}

void endpwent(void)
{ struct SocketSettings *lss;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      SOCK_endpwent();
    break;

    case LX_AMITCP:
      UG_endpwent();
    break;

    default:
      if (nextuid=0,lss->lx_pwd_fp) {
        fclose(lss->lx_pwd_fp); lss->lx_pwd_fp = NULL;
      }
    break;
  }
}

struct passwd *getpwent(void)
{ struct SocketSettings *lss = _lx_get_socket_settings();
  struct passwd *pwd;
  char *name;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return __AS225InetPwd(SOCK_getpwent(),lss);
    break;

    case LX_AMITCP:
      if ((pwd=__TCP2InetPwd(UG_getpwent(),lss)) == NULL)
        errno = ug_GetErr();
      return pwd;
    break;

    default:
      if ((lss->lx_pwd_fp || pw_start(lss)) && pw_scan(0, 0, NULL, lss))
        return &lss->lx_pwd;
      switch (nextuid) {
        case 0:
          nextuid = 1;
          return (struct passwd *)&builtin_root;
        default:
          nextuid = -2;
          if ((name = (char *)getlogin()))
            return getpwnam(name);
        case (uid_t)(-2):
          nextuid = -1;
          return (struct passwd *)&builtin_nobody;
        case (uid_t)(-1):
          return 0;        
      }
    break;
  }
}

struct passwd *getpwuid(uid_t uid)
{ struct SocketSettings *lss;
  struct passwd *pwd;

  if (uid == (uid_t)(-2))
    return (struct passwd *)&builtin_nobody; /* always handle nobody */

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return __AS225InetPwd(SOCK_getpwuid(uid),lss);
    break;

    case LX_AMITCP:
      /* Don't do this if uid == -2 (nobody2) */
      /* This happens when someone doesn't use AmiTCP's login */
      if (uid != (uid_t)-2) {
        if ((pwd=__TCP2InetPwd(UG_getpwuid(uid),lss)) == NULL)
          errno = ug_GetErr();
        return pwd;
      }
      else {
        return getpwnam(getenv("USER"));
      }
    break;

    default:
      if (pw_start(lss))  {
        int rval = pw_scan(1, uid, NULL, lss);
        if (!lss->lx_pwd_stayopen)
          endpwent();
        return (rval ? &lss->lx_pwd : NULL);
      }
      if (uid == 0)
        return (struct passwd *)&builtin_root;
      if ((uid == getuid()) && (lss->lx_pwd.pw_name=getlogin()))
        if ((lss->lx_pwd.pw_dir=getenv("HOME")) == NULL)
          lss->lx_pwd.pw_dir = "SYS:";
      if ((lss->lx_pwd.pw_gecos=getenv("REALNAME")) == NULL)
        lss->lx_pwd.pw_gecos = "Amiga User";
      lss->lx_pwd.pw_uid    = uid;
      lss->lx_pwd.pw_gid    = getgid();
      lss->lx_pwd.pw_change = 0;
      lss->lx_pwd.pw_expire = 0;
      lss->lx_pwd.pw_class  = "";
      lss->lx_pwd.pw_passwd = "*";
      lss->lx_pwd.pw_shell  = "sh";
      return &lss->lx_pwd;
    break;
  }
}

struct passwd *getpwnam(const char *name)
{ struct SocketSettings *lss;
  struct passwd *pwd;

  if (!strcmp(name,"nobody"))
    return (struct passwd *)&builtin_nobody;

  switch (lss=_lx_get_socket_settings(),lss->lx_network_type) {
    case LX_AS225:
      return __AS225InetPwd(SOCK_getpwnam((char *)name),lss);
    break;

    case LX_AMITCP:
      if ((pwd=__TCP2InetPwd(UG_getpwnam(name),lss)) == NULL)
        errno = ug_GetErr();
      return pwd;
    break;

    default:
      if (pw_start(lss)) {
        int rval = pw_scan(1, 0, name, lss);
        if (!lss->lx_pwd_stayopen)
          endpwent();
        return (rval ? &lss->lx_pwd : NULL);
      }
      if (!strcmp(name,"root"))
        return (struct passwd *)&builtin_root;
      if ((lss->lx_pwd.pw_name=getlogin()) && !strcmp(lss->lx_pwd.pw_name,name))
        if ((lss->lx_pwd.pw_dir=getenv("HOME")) == NULL)
          lss->lx_pwd.pw_dir = "SYS:";
      if ((lss->lx_pwd.pw_gecos=getenv("REALNAME")) == NULL)
        lss->lx_pwd.pw_gecos = "Amiga User";
      lss->lx_pwd.pw_uid    = getuid();
      lss->lx_pwd.pw_gid    = getgid();
      lss->lx_pwd.pw_change = 0;
      lss->lx_pwd.pw_expire = 0;
      lss->lx_pwd.pw_class  = "";
      lss->lx_pwd.pw_passwd = "*";
      lss->lx_pwd.pw_shell  = "sh";
      return &lss->lx_pwd;
    break;
  }
}

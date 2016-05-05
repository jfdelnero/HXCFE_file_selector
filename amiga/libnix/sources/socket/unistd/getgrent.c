#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
//
#include "socket.h"

/* for AS225 */
#define PATH_GROUP "inet:db/group"

#define MAXGRP          200
#define MAXLINELENGTH  1024

#define UNIX_STRSEP    ":\n"
#define AMIGAOS_STRSEP "|\n"

static gid_t nextgid;

/*
** builtin group info
*/
static const char *dummy_members[] =
{ 0 };

static const struct group builtin_wheel =
{ "wheel", "*", 0, (char **)dummy_members };

static const struct group builtin_nogroup =
{ "nogroup", "*", -2, (char **)dummy_members };

/*
**
*/
static int gr_start(struct SocketSettings *lss)
{
  if (lss->lx_grp_fp) {
    rewind(lss->lx_grp_fp); return 1;
  }
  return ((lss->lx_grp_fp = fopen(PATH_GROUP, "r")) ? 1 : 0);
}

static int gr_scan(int search, gid_t gid, const char *name, struct SocketSettings *lss)
{
  if (!lss->lx_grp_line)
    lss->lx_grp_line = malloc(MAXLINELENGTH + 1);

  if (!lss->lx_members)
    lss->lx_members = malloc(MAXGRP * sizeof(char *));

  if (!lss->lx_grp_line || !lss->lx_members) {
    errno = ENOMEM; return 0;
  }

  for (;;) {
    char *sep, *bp, *cp, **m;

    if (!fgets(lss->lx_grp_line, MAXLINELENGTH, lss->lx_grp_fp))
      return 0;

    /* skip lines that are too big */
    if (!strchr(lss->lx_grp_line, '\n')) {
      int ch;

      while ((ch = getc(lss->lx_grp_fp)) != '\n' && ch != EOF)
        ;

      continue;
    }

    sep = strchr(bp=lss->lx_grp_line,'|') ? AMIGAOS_STRSEP : UNIX_STRSEP;

    lss->lx_grp.gr_name = strsep(&bp, sep);

    if (search && name && strcmp(lss->lx_grp.gr_name, name))
      continue;

    lss->lx_grp.gr_passwd = strsep(&bp, sep);

    if (!(cp = strsep(&bp, sep)))
      continue;

    lss->lx_grp.gr_gid = atoi(cp);

    if (search && !name && lss->lx_grp.gr_gid != gid)
      continue;

    for (m = lss->lx_grp.gr_mem = lss->lx_members;; ++m) {
      if (m == &lss->lx_members[MAXGRP - 1]) {
        *m = NULL; break;
      }

      if ((*m = strsep(&bp, ", \n")) == NULL)
        break;
    }
    return 1;
  }
  /* NOTREACHED */
}

/*
**
*/

int setgrent(void)
{
  return setgroupent(0);
}

int setgroupent(int stayopen)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP) {
    UG_setgrent();
  }
  else if (nextgid=0,gr_start(lss)) {
    lss->lx_grp_stayopen = stayopen;
  }

  return 1;
}

void endgrent(void)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP) {
    UG_endgrent();
  }
  else if (nextgid=0,lss->lx_grp_fp) {
    fclose(lss->lx_grp_fp); lss->lx_grp_fp = NULL;
  }
}

struct group *getgrent(void)
{ struct SocketSettings *lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP) {
    return UG_getgrent();
  }
  else if ((lss->lx_grp_fp || gr_start(lss)) && gr_scan(0, 0, NULL, lss)) {
    return &lss->lx_grp;
  }
  else if (lss->lx_network_type == LX_NONE) {
    char *name;
    switch (nextgid) {
      case 0:
        nextgid = 1;
        return (struct group *)&builtin_wheel;
      default:
        nextgid = -2;
        if ((name = getenv("GROUP")))
          return getgrnam(name);
      case (gid_t)(-2):
        nextgid = -1;
        return (struct group *)&builtin_nogroup;
      case (gid_t)(-1):
        //return NULL;
	break;
    }
  }

  return NULL;
}

struct group *getgrgid(gid_t gid)
{ struct SocketSettings *lss;

  if (gid == (gid_t)(-2))
    return (struct group *)&builtin_nogroup; /* always handle nogroup */

  lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP) {
    struct group *grp = UG_getgrgid(gid);
    if (!grp)
      errno = ug_GetErr();
    return grp;
  }
  else if (gr_start(lss)) {
    int rval = gr_scan(1, gid, NULL, lss);
    if (!lss->lx_grp_stayopen)
      endgrent();
    return(rval ? &lss->lx_grp : NULL);
  }
  else if (lss->lx_network_type == LX_NONE) {
    if (gid == 0)
      return (struct group *)&builtin_wheel;
    if ((gid == getgid()) && (lss->lx_grp.gr_name = getenv("GROUP"))) {
      lss->lx_grp.gr_gid    = gid;
      lss->lx_grp.gr_passwd = "*";
      lss->lx_grp.gr_mem    = (char **)dummy_members;
      return &lss->lx_grp;
    }
  }

  return NULL;
}

struct group *getgrnam(const char *name)
{ struct SocketSettings *lss;

  if (!strcmp(name,"nogroup"))
    return (struct group *)&builtin_nogroup; /* always handle nogroup */

  lss = _lx_get_socket_settings();

  if (lss->lx_network_type == LX_AMITCP) {
    struct group *grp = UG_getgrnam(name);
    if (!grp)
      errno = ug_GetErr();
    return grp;
  }
  else if (gr_start(lss)) {
    int rval = gr_scan(1, 0, name, lss);
    if (!lss->lx_grp_stayopen)
      endgrent();
    return(rval ? &lss->lx_grp : NULL);
  }
  else if (lss->lx_network_type == LX_NONE) {
    if (!strcmp(name,"wheel"))
      return (struct group *)&builtin_wheel;
    if ((lss->lx_grp.gr_name = getenv("GROUP")) && !strcmp(lss->lx_grp.gr_name,name)) {
      lss->lx_grp.gr_gid    = getgid();
      lss->lx_grp.gr_passwd = "*";
      lss->lx_grp.gr_mem    = (char **)dummy_members;
      return &lss->lx_grp;
    }
  }

  return NULL;
}

#include <sys/file.h>
#include "his.h"

#define DEBUG 1
#undef DEBUG

static datum content, inputkey;

enum
{
  SUBJECT, FROM, NAME
};


char *
DBfetch(key)
  char *key;
{
static char dboutput[1025];

#ifdef	MapleBBS

  inputkey.dptr = key;
  inputkey.dsize = strlen(key);

#else

static char dbinput[512];

  int i;
  if (key == NULL)
    return NULL;
  i = strlen(key);
  if (i > 510)
    i = 510;
  memcpy(dbinput, key, i);
  dbinput[i] = '\0';
  inputkey.dptr = dbinput;
  inputkey.dsize = i;
#endif

  content.dptr = dboutput;
  return ((char *) HISfilesfor(&inputkey, &content));
}


#ifndef	MapleBBS


static int
DBstore(key, paths)
  char *key;
  char *paths;
{
  int i;

  if (key == NULL)
    return -1;

  i = strlen(key);
  if (i > 510)
    i = 510;
  memcpy(dbinput, key, i);
  dbinput[i] = '\0';
  inputkey.dptr = dbinput;
  inputkey.dsize = i;

  time(&now);
  if (HISwrite(&inputkey, now, paths) == FALSE)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}


int 
storeDB(mid, paths)
  char *mid;
  char *paths;
{
  char *key, *ptr;
  int rel;
  ptr = DBfetch(mid);
  if (ptr != NULL)
  {
    return 0;
  }
  else
  {
    return DBstore(mid, paths);
  }
}


my_mkdir(idir, mode)
  char *idir;
  int mode;
{
  char buffer[LEN];
  char *ptr, *dir = buffer;
  struct stat st;
  strncpy(dir, idir, LEN - 1);
  for (; dir != NULL && *dir;)
  {
    ptr = (char *) strchr(dir, '/');
    if (ptr != NULL)
    {
      *ptr = '\0';
    }
    if (stat(dir, &st) != 0)
    {
      if (mkdir(dir, mode) != 0)
	return -1;
    }
    chdir(dir);
    if (ptr != NULL)
      dir = ptr + 1;
    else
      dir = ptr;
  }
  return 0;
}

#endif

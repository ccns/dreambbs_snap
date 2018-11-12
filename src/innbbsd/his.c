/*
 * $Revision: 1.1.1.1 $ *
 * 
 * History file routines.
 */
#include "innbbsconf.h"
#include "bbslib.h"
#include "his.h"

#define STATIC static
/* STATIC char	HIShistpath[] = _PATH_HISTORY; */
STATIC FILE *HISwritefp;
STATIC int HISreadfd;
STATIC int HISdirty;
STATIC int HISincore = XINDEX_DBZINCORE;

STATIC char *LogName = "xindexchan";

#ifndef EXPIREDAYS
#define EXPIREDAYS 4
#endif

#ifndef DEFAULT_HIST_SIZE
#define DEFAULT_HIST_SIZE 100000
#endif


void
hisincore(flag)
  int flag;
{
  HISincore = flag;
}

static void
makedbz(histpath, entry)
  char *histpath;
  long entry;
{
  dbzfresh(histpath, dbzsize(entry), '\t', 0, 1);
  dbmclose();
}


/*
 * *  Set up the history files.
 */


static void
myHISsetup(histpath)
  char *histpath;
{
  if (HISwritefp == NULL)
  {
    /* Open the history file for appending formatted I/O. */
    if ((HISwritefp = fopen(histpath, "a")) == NULL)
    {
      syslog(LOG_CRIT, "%s cant fopen %s %m", LogName, histpath);
      exit(1);
    }
    CloseOnExec((int) fileno(HISwritefp), TRUE);

    /* Open the history file for reading. */
    if ((HISreadfd = open(histpath, O_RDONLY)) < 0)
    {
      syslog(LOG_CRIT, "%s cant open %s %m", LogName, histpath);
      exit(1);
    }
    CloseOnExec(HISreadfd, TRUE);

    /* Open the DBZ file. */
    (void) dbzincore(HISincore);

#ifndef	MMAP
    (void) dbzwritethrough(1);
#endif

    if (dbminit(histpath) < 0)
    {
      syslog(LOG_CRIT, "%s cant dbminit %s %m", histpath, LogName);
      exit(1);
    }
  }
}


void
HISsetup()
{
  myHISsetup(HISTORY);
}


/*
 * *  Synchronize the in-core history file (flush it).
 */
static void
HISsync()
{
  if (HISdirty)
  {
    if (dbzsync())
    {
      syslog(LOG_CRIT, "%s cant dbzsync %m", LogName);
      exit(1);
    }
    HISdirty = 0;
  }
}


/*
 * *  Close the history files.
 */
void
HISclose()
{
  if (HISwritefp != NULL)
  {
    /*
     * Since dbmclose calls dbzsync we could replace this line with "HISdirty
     * = 0;".  Oh well, it keeps the abstraction clean.
     */
    HISsync();
    if (dbmclose() < 0)
      syslog(LOG_ERR, "%s cant dbmclose %m", LogName);
    if (fclose(HISwritefp) == EOF)
      syslog(LOG_ERR, "%s cant fclose history %m", LogName);
    HISwritefp = NULL;
    if (close(HISreadfd) < 0)
      syslog(LOG_ERR, "%s cant close history %m", LogName);
    HISreadfd = -1;
  }
}


#ifdef HISset
/*
 * *  File in the DBZ datum for a Message-ID, making sure not to copy any *
 * illegal characters.
 */
STATIC void
HISsetkey(p, keyp)
  register char *p;
  datum *keyp;
{
  static BUFFER MessageID;
  register char *dest;
  register int i;

  /* Get space to hold the ID. */
  i = strlen(p);
  if (MessageID.Data == NULL)
  {
    MessageID.Data = NEW(char, i + 1);
    MessageID.Size = i;
  }
  else if (MessageID.Size < i)
  {
    RENEW(MessageID.Data, char, i + 1);
    MessageID.Size = i;
  }

  for (keyp->dptr = dest = MessageID.Data; i = *p; p++)
  {
    *dest++ = (i == HIS_FIELDSEP || i == '\n') ? HIS_BADCHAR : i;
  }
  *dest = '\0';

  keyp->dsize = dest - MessageID.Data + 1;
}
#endif

/*
 * *  Get the list of files under which a Message-ID is stored.
 */
char *
HISfilesfor(key, output)
  datum *key;
  datum *output;
{
  register char *dest;
  datum val;
  long offset;
  register char *p;
  register int i;

  /* Get the seek value into the history file. */
  val = dbzfetch(*key);
  if (val.dptr == NULL || val.dsize != sizeof offset)
  {
    /* printf("fail here val.dptr %d\n",val.dptr); */
    return NULL;
  }

#ifdef	MapleBBS
  p = output->dptr;
#else
  /* Get space. */
  if ( output->dptr == NULL)
  {
    printf("fail here output->dptr null\n");
    return NULL;
  }
#endif

  /* Copy the value to an aligned spot. */

#ifdef	MapleBBS
  memcpy(&offset, val.dptr, sizeof(offset));
#else
  for (p = val.dptr, dest = (char *) &offset, i = sizeof offset; --i >= 0;)
    *dest++ = *p++;
#endif
  if (lseek(HISreadfd, offset, SEEK_SET) == -1)
  {
    printf("fail here lseek %d\n", offset);
    return NULL;
  }

  /* Read the text until \n or EOF. */

      output->dsize = 0;
        i = read(HISreadfd, p, LEN - 1);
        if (i <= 0) {
            printf("fail here i %d\n",i);
            return NULL;
        }
        p[i] = '\0';
        if ((dest = (char*)strchr(p, '\n')) != NULL) {
            *dest = '\0';
        }

  /* Move past the first two fields -- Message-ID and date info. */
  if ((p = (char *) strchr(p, HIS_FIELDSEP)) == NULL)
  {
    printf("fail here no HIS_FILE\n");
    return NULL;
  }
  return p + 1;
  /*
   * if ((p = (char*)strchr(p + 1, HIS_FIELDSEP)) == NULL) return NULL;
   */

  /* Translate newsgroup separators to slashes, return the fieldstart. */
}


#ifndef	MapleBBS


/*
 * *  Have we already seen an article?
 */

#ifdef HISh
BOOL
HIShavearticle(MessageID)
  char *MessageID;
{
  datum key;
  datum val;

  HISsetkey(MessageID, &key);
  val = dbzfetch(key);
  return val.dptr != NULL;
}
#endif


/*
 * *  Turn a history filename entry from slashes to dots.  It's a pity *  we
 * have to do this.
 */
STATIC void
HISslashify(p)
  register char *p;
{
  register char *last;

  for (last = NULL; *p; p++)
  {
    if (*p == '/')
    {
      *p = '.';
      last = p;
    }
    else if (*p == ' ' && last != NULL)
      *last = '/';
  }
  if (last)
    *last = '/';
}


static				/* BOOL */
myHISwrite(key, remain)
  datum *key;
  char *remain;
{
  long offset;
  datum val;
  int i;

  val = dbzfetch(*key);
  if (val.dptr != NULL)
  {
    return FALSE;
  }

  flock(fileno(HISwritefp), LOCK_EX);

  offset = ftell(HISwritefp);
  i = fprintf(HISwritefp, "%s%c%s",
    key->dptr, HIS_FIELDSEP, remain);
  if (i == EOF || fflush(HISwritefp) == EOF)
  {
    /* The history line is now an orphan... */
    syslog(LOG_ERR, "%s cant write history %m", LogName);
    flock(fileno(HISwritefp), LOCK_UN);
    return FALSE;
  }

  /* Set up the database values and write them. */
  val.dptr = (char *) &offset;
  val.dsize = sizeof offset;
  if (dbzstore(*key, val) < 0)
  {
    syslog(LOG_ERR, "%s cant dbzstore %m", LogName);
    flock(fileno(HISwritefp), LOCK_UN);
    return FALSE;
  }

  if (++HISdirty >= ICD_SYNC_COUNT)
  {
    fflush(HISwritefp);
    HISsync();
  }

  flock(fileno(HISwritefp), LOCK_UN);
  return TRUE;
}

#endif





#ifndef	MapleBBS


/*
 * *  Have we already seen an article?
 */

#ifdef HISh
BOOL
HIShavearticle(MessageID)
  char *MessageID;
{
  datum key;
  datum val;

  HISsetkey(MessageID, &key);
  val = dbzfetch(key);
  return val.dptr != NULL;
}
#endif


/*
 * *  Turn a history filename entry from slashes to dots.  It's a pity *  we
 * have to do this.
 */
STATIC void
HISslashify(p)
  register char *p;
{
  register char *last;

  for (last = NULL; *p; p++)
  {
    if (*p == '/')
    {
      *p = '.';
      last = p;
    }
    else if (*p == ' ' && last != NULL)
      *last = '/';
  }
  if (last)
    *last = '/';
}


static				/* BOOL */
myHISwrite(key, remain)
  datum *key;
  char *remain;
{
  long offset;
  datum val;
  int i;

  val = dbzfetch(*key);
  if (val.dptr != NULL)
  {
    return FALSE;
  }

  flock(fileno(HISwritefp), LOCK_EX);

  offset = ftell(HISwritefp);
  i = fprintf(HISwritefp, "%s%c%s",
    key->dptr, HIS_FIELDSEP, remain);
  if (i == EOF || fflush(HISwritefp) == EOF)
  {
    /* The history line is now an orphan... */
    syslog(LOG_ERR, "%s cant write history %m", LogName);
    flock(fileno(HISwritefp), LOCK_UN);
    return FALSE;
  }

  /* Set up the database values and write them. */
  val.dptr = (char *) &offset;
  val.dsize = sizeof offset;
  if (dbzstore(*key, val) < 0)
  {
    syslog(LOG_ERR, "%s cant dbzstore %m", LogName);
    flock(fileno(HISwritefp), LOCK_UN);
    return FALSE;
  }

  if (++HISdirty >= ICD_SYNC_COUNT)
  {
    fflush(HISwritefp);
    HISsync();
  }

  flock(fileno(HISwritefp), LOCK_UN);
  return TRUE;
}

#endif


/*
 * *  Write a history entry.
 */

#ifdef	MapleBBS


void
HISflush()
{
  fflush(HISwritefp);
  HISsync();
}


/* HERwrite : simple HISwrite, because "paths" indicate "date" */


BOOL
HERwrite(key, paths)
  char *key;			/* messageID */
  char *paths;			/* board + file */
{
  FILE *fp;
  long offset;
  datum val, data;

  offset = ftell(fp = HISwritefp);
  fprintf(fp, "%s%c%s\n", key, HIS_FIELDSEP, paths);

  data.dptr = key;
  data.dsize = strlen(key);

  /* Set up the database values and write them. */
  val.dptr = (char *) &offset;
  val.dsize = sizeof offset;

  if (dbzstore(data, val) < 0)
  {
    syslog(LOG_ERR, "%s cant dbzstore %m", LogName);
    return FALSE;
  }

  if (++HISdirty >= ICD_SYNC_COUNT)
  {
    fflush(fp);
    HISsync();
  }

  return TRUE;
}

#else

BOOL
HISwrite(key, date, paths)
  datum *key;
  char *paths;
  long date;
{
  long offset;
  datum val;
  int i;

  flock(fileno(HISwritefp), LOCK_EX);
  offset = ftell(HISwritefp);
  i = fprintf(HISwritefp, "%s%c%ld%c%s\n",
    key->dptr, HIS_FIELDSEP, (long) date, HIS_FIELDSEP,
    paths);
  if (i == EOF || fflush(HISwritefp) == EOF)
  {
    /* The history line is now an orphan... */
    syslog(LOG_ERR, "%s cant write history %m", LogName);
    flock(fileno(HISwritefp), LOCK_UN);
    return FALSE;
  }

  /* Set up the database values and write them. */
  val.dptr = (char *) &offset;
  val.dsize = sizeof offset;
  if (dbzstore(*key, val) < 0)
  {
    syslog(LOG_ERR, "%s cant dbzstore %m", LogName);
    flock(fileno(HISwritefp), LOCK_UN);
    return FALSE;
  }

  if (++HISdirty >= ICD_SYNC_COUNT)
  {
    fflush(HISwritefp);
    HISsync();
  }

  flock(fileno(HISwritefp), LOCK_UN);
  return TRUE;
}


void
mkhistory(srchist)
  char *srchist;
{
  FILE *hismaint;

  if (hismaint = fopen(srchist, "r"))
  {
    char newhistpath[256];
    char newhistdirpath[256];
    char newhistpagpath[256];
    char maintbuff[256];
    register char *ptr;

    sprintf(newhistpath, "%s.n", srchist);
    sprintf(newhistdirpath, "%s.dir", newhistpath);
    sprintf(newhistpagpath, "%s.pag", newhistpath);
    if (!isfile(newhistdirpath) || !isfile(newhistpagpath))
    {
      makedbz(newhistpath, DEFAULT_HIST_SIZE);
    }
    myHISsetup(newhistpath);
    while (fgets(maintbuff, sizeof(maintbuff), hismaint) != NULL)
    {
      datum key;

      if (ptr = (char *) strchr(maintbuff, '\t'))
      {
	*ptr = '\0';
	ptr++;
      }
      key.dptr = maintbuff;
      key.dsize = strlen(maintbuff);
      myHISwrite(&key, ptr);
    }
    (void) HISclose();

#ifdef	abc
    rename(newhistpath, srchist);
    rename(newhistdirpath,
      fileglue("%s.dir", srchist));
    rename(newhistpagpath,
      fileglue("%s.pag", srchist));
#endif

    fclose(hismaint);
  }
}

#endif


#ifdef	MapleBBS


time_t
mychrono32(str)
  char *str;
{
  time_t chrono;
  int ch;

  chrono = 0;

  if (str = strrchr(str, '/'))	/* "board/0123456\n" */
  {
  for (;;)
  {
    ch = *++str;
    if (ch == ' ' || ch == '\n' || ch == '\0')
      break;
    ch -= '0';
    if (ch >= 10)
      ch -= 'A' - '0' - 10;
    chrono = (chrono << 5) + ch;
  }
  *str = '\0';
  }

  return chrono;
}


#endif


time_t
gethisinfo()
{
  FILE *hismaint;

  if (hismaint = fopen(HISTORY, "r"))
  {
    char maintbuff[4096];
    register char *ptr;

    fgets(ptr = maintbuff, sizeof(maintbuff), hismaint);
    fclose(hismaint);

#ifdef	MapleBBS
    return mychrono32(ptr);
#else
    if (ptr = (char *) strchr(ptr, '\t'))
    {
      return atol(++ptr);
    }
#endif

  }
  return 0;
}


void
HISmaint()
{
  FILE *hismaint;
  time_t lasthist, now;
  char maintbuff[4096];
  register char *ptr;

  if (!isfile(HISTORY))
  {
    makedbz(HISTORY, DEFAULT_HIST_SIZE);
  }

#ifdef	_MapleBBS_	/* comment out for init daemon */
  else
  {
    HISflush();
  }
#endif

  hismaint = fopen(HISTORY, "r");
  if (hismaint == NULL)
  {
    return;
  }
  fgets(maintbuff, sizeof(maintbuff), hismaint);
  if (ptr = (char *) strchr(maintbuff, '\t'))
  {
#ifdef	MapleBBS
    lasthist = mychrono32(ptr);
#else
    lasthist = atol(++ptr);
#endif
    now = time(0);
    if (lasthist + 86400 * Expiredays * 2 < now)
    {
      char newhistpath[256];
      char newhistdirpath[256];
      char newhistpagpath[256];
      (void) HISclose();
      sprintf(newhistpath, "%s.n", HISTORY);
      sprintf(newhistdirpath, "%s.dir", newhistpath);
      sprintf(newhistpagpath, "%s.pag", newhistpath);
      if (!isfile(newhistdirpath))
      {
	makedbz(newhistpath, DEFAULT_HIST_SIZE);
      }
      myHISsetup(newhistpath);
      now -= 99600 * Expiredays;
      while (fgets(maintbuff, sizeof(maintbuff), hismaint))
      {
	if (ptr = (char *) strchr(maintbuff, '\t'))
	{
	  *ptr++ = '\0';
#ifdef	MapleBBS
	  lasthist = mychrono32(ptr);
#else
	  lasthist = atol(ptr);
#endif
	  if (lasthist >= now)
	  {
#ifdef	MapleBBS
	    HERwrite(maintbuff, ptr);
#else
	    datum key;

	    key.dptr = maintbuff;
	    key.dsize = strlen(maintbuff);
	    myHISwrite(&key, ptr);
#endif
	  }
	}
      }
      (void) HISclose();
      rename(HISTORY, (char *) fileglue("%s.o", HISTORY));
      rename(newhistpath, HISTORY);
      rename(newhistdirpath, (char *) fileglue("%s.dir", HISTORY));
      rename(newhistpagpath, (char *) fileglue("%s.pag", HISTORY));
      (void) HISsetup();
    }
  }
  fclose(hismaint);
}

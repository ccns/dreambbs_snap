#if defined( LINUX )
# include "innbbsconf.h"
# include "bbslib.h"
# include <varargs.h>
#else
# include <varargs.h>
# include "innbbsconf.h"
# include "bbslib.h"
#endif
#include <stdlib.h>

char BBSHOME[] = _PATH_BBSHOME;
char MYBBSID[32];
char INNBBSCONF[]	= "innd/innbbs.conf";
char INNDHOME[]		= "innd";
char HISTORY[]		= "innd/history";
char LOGFILE[]		= "innd/innbbs.log";
char ECHOMAIL[]		= "innd/echomail.log";
char BBSFEEDS[]		= "innd/bbsfeeds.log";
char LOCALDAEMON[]	= "innd/.innbbsd";

int His_Maint_Min = HIS_MAINT_MIN;
int His_Maint_Hour = HIS_MAINT_HOUR;
int Expiredays = EXPIREDAYS;

nodelist_t *NODELIST = NULL, **NODELIST_BYNODE = NULL;
newsfeeds_t *NEWSFEEDS = NULL, **NEWSFEEDS_BYBOARD = NULL;
static char *NODELIST_BUF, *NEWSFEEDS_BUF;
int NFCOUNT, NLCOUNT;
int LOCALNODELIST = 0, NONENEWSFEEDS = 0;


static FILE *bbslogfp;

static int verboseFlag = 0;

static char * verboseFilename = NULL;
static char verbosename[MAXPATHLEN];


void
verboseon(filename)
  char *filename;
{
  verboseFlag = 1;
  if (filename != NULL)
  {
    if (strchr(filename, '/') == NULL)
    {
      sprintf(verbosename, "%s/innd/%s", BBSHOME, filename);
      filename = verbosename;
    }
  }
  verboseFilename = filename;
}


void
verboseoff()
{
  verboseFlag = 0;
}


void
setverboseon()
{
  verboseFlag = 1;
}


int
isverboselog()
{
  return verboseFlag;
}


void
setverboseoff()
{
  verboseoff();
  if (bbslogfp != NULL)
  {
    fclose(bbslogfp);
    bbslogfp = NULL;
  }
}


void
verboselog(va_alist)
va_dcl
{
  va_list ap;
  register char *fmt;
  char datebuf[40];
  time_t now;

  if (verboseFlag == 0)
    return;

  va_start(ap);

  time(&now);
  strftime(datebuf, sizeof(datebuf), "%b %d %X ", localtime(&now));

  if (bbslogfp == NULL)
  {
    if (verboseFilename != NULL)
      bbslogfp = fopen(verboseFilename, "a");
    else
      bbslogfp = fdopen(1, "a");
  }
  if (bbslogfp == NULL)
  {
    va_end(ap);
    return;
  }
  fmt = va_arg(ap, char *);
  fprintf(bbslogfp, "%s[%d] ", datebuf, getpid());
  vfprintf(bbslogfp, fmt, ap);
  fflush(bbslogfp);
  va_end(ap);
}


void

#ifdef PalmBBS
xbbslog(va_alist)
#else
bbslog(va_alist)
#endif

va_dcl
{
  va_list ap;
  register char *fmt;
  char datebuf[40];
  time_t now;

  va_start(ap);

  time(&now);
  strftime(datebuf, sizeof(datebuf), "%b %d %X ", localtime(&now));

  if (bbslogfp == NULL)
  {
    bbslogfp = fopen(LOGFILE, "a");
  }
  if (bbslogfp == NULL)
  {
    va_end(ap);
    return;
  }
  fmt = va_arg(ap, char *);
  fprintf(bbslogfp, "%s[%d] ", datebuf, getpid());
  vfprintf(bbslogfp, fmt, ap);
#if 1
  /* Thor.981118: 暴力強制log不打結, 有空再好好改:P */
  fclose(bbslogfp);
  bbslogfp = NULL;
#else
  fflush(bbslogfp);
#endif
  va_end(ap);
}



static int
nf_byboardcmp(a, b)
  newsfeeds_t **a, **b;
{
  /*
   * if (!a || !*a || !(*a)->board) return -1; if (!b || !*b || !(*b)->board)
   * return 1;
   */
  return strcasecmp((*a)->board, (*b)->board);
}

static int
nfcmp(a, b)
  newsfeeds_t *a, *b;
{
  /*
   * if (!a || !a->newsgroups) return -1; if (!b || !b->newsgroups) return 1;
   */
  return strcasecmp(a->newsgroups, b->newsgroups);
}

static int
nlcmp(a, b)
  nodelist_t *a, *b;
{
  /*
   * if (!a || !a->host) return -1; if (!b || !b->host) return 1;
   */
  return strcasecmp(a->host, b->host);
}

static int
nl_bynodecmp(a, b)
  nodelist_t **a, **b;
{
  /*
   * if (!a || !*a || !(*a)->node) return -1; if (!b || !*b || !(*b)->node)
   * return 1;
   */
  return strcasecmp((*a)->node, (*b)->node);
}

/* read in newsfeeds.bbs and nodelist.bbs */

static int
readnlfile(inndhome, outgoing)
  char *inndhome;
  char *outgoing;
{
  FILE *fp;
  char buff[1024];
  struct stat st;
  register int i, count;
  register char *ptr, *nodelistptr;
  static lastcount = 0;

  sprintf(buff, "%s/nodelist.bbs", inndhome);
  fp = fopen(buff, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "open fail %s", buff);
    return -1;
  }
  if (fstat(fileno(fp), &st) != 0)
  {
    fprintf(stderr, "stat fail %s", buff);
    return -1;
  }
  i = st.st_size + 1;
  if (NODELIST_BUF == NULL)
  {
    NODELIST_BUF = (char *) mymalloc(i);
  }
  else
  {
    NODELIST_BUF = (char *) myrealloc(NODELIST_BUF, i);
  }
  i = count = 0;
  while (fgets(buff, sizeof buff, fp))
  {
    if (buff[0] != '#' && buff[0] != '\n')
    {
      strcpy(NODELIST_BUF + i, buff);
      i += strlen(buff);
      count++;
    }
  }
  fclose(fp);

  i = sizeof(nodelist_t) * (count + 1);
  if (NODELIST == NULL)
  {
    NODELIST = (nodelist_t *) mymalloc(i);
    NODELIST_BYNODE = (nodelist_t **) mymalloc(i);
  }
  else
  {
    NODELIST = (nodelist_t *) myrealloc(NODELIST, i);
    NODELIST_BYNODE = (nodelist_t **) myrealloc(NODELIST_BYNODE, i);
  }
  for (i = lastcount; i < count; i++)
  {
    NODELIST[i].feedfp = NULL;
  }
  lastcount = count;
  NLCOUNT = 0;
  for (ptr = NODELIST_BUF; nodelistptr = (char *) strchr(ptr, '\n'); ptr = nodelistptr + 1, NLCOUNT++)
  {
    register char *nptr, *bptr, *pptr, *tptr, *nodeptr;

    *nodelistptr = '\0';
    NODELIST[NLCOUNT].host = "";
    NODELIST[NLCOUNT].exclusion = "";
    NODELIST[NLCOUNT].node = "";
    NODELIST[NLCOUNT].protocol = "IHAVE(119)";
    NODELIST[NLCOUNT].comments = "";
    NODELIST_BYNODE[NLCOUNT] = NODELIST + NLCOUNT;
    for (nptr = ptr; (i = *nptr) && isspace(i);)
      nptr++;
    if (*nptr == '\0')
    {
      bbslog("nodelist.bbs %d entry read error\n", NLCOUNT);
      return -1;
    }
    /* NODELIST[NLCOUNT].id = nptr; */
    NODELIST[NLCOUNT].node = nodeptr = nptr;
    for (nptr++; (i = *nptr) && !isspace(i);)
      nptr++;
    if (*nptr == '\0')
    {
      bbslog("nodelist.bbs node %d entry read error\n", NLCOUNT);
      return -1;
    }
    *nptr = '\0';
    if ((tptr = strchr(nodeptr, '/')))
    {
      *tptr = '\0';
      NODELIST[NLCOUNT].exclusion = tptr + 1;
    }
    else
    {
      NODELIST[NLCOUNT].exclusion = "";
    }
    for (nptr++; (i = *nptr) && isspace(i);)
      nptr++;
    if (!(i = *nptr))
      continue;
    if (i == '+' || i == '-')
    {
      NODELIST[NLCOUNT].feedtype = i;
      if (NODELIST[NLCOUNT].feedfp != NULL)
      {
	fclose(NODELIST[NLCOUNT].feedfp);
      }
      if (i == '+')
	if (outgoing)
	{
	  NODELIST[NLCOUNT].feedfp = fopen((char *) fileglue("innd/%s.%s", nodeptr, outgoing), "a");
	}
      nptr++;
    }
    else
    {
      NODELIST[NLCOUNT].feedtype = ' ';
    }
    NODELIST[NLCOUNT].host = nptr;
    for (nptr++; (i = *nptr) && !isspace(i);)
      nptr++;
    if (*nptr == '\0')
    {
      continue;
    }
    *nptr = '\0';
    for (nptr++; (i = *nptr) && isspace(i);)
      nptr++;
    if (*nptr == '\0')
      continue;
    NODELIST[NLCOUNT].protocol = nptr;
    for (nptr++; (i = *nptr) && !isspace(i);)
      nptr++;
    if (*nptr == '\0')
      continue;
    *nptr = '\0';
    for (nptr++; (i = *nptr) && strchr(" \t\r\n", i);)
      nptr++;
    if (*nptr == '\0')
      continue;
    NODELIST[NLCOUNT].comments = nptr;
  }
  qsort(NODELIST, NLCOUNT, sizeof(nodelist_t), nlcmp);
  qsort(NODELIST_BYNODE, NLCOUNT, sizeof(nodelist_t *), nl_bynodecmp);
  return 0;
}


static int
readnffile(inndhome)
  char *inndhome;
{
  FILE *fp;
  char buff[1024];
  struct stat st;
  register int i, count;
  register char *ptr, *newsfeedsptr;

  sprintf(buff, "%s/newsfeeds.bbs", inndhome);
  fp = fopen(buff, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "open fail %s", buff);
    return -1;
  }
  if (fstat(fileno(fp), &st) != 0)
  {
    fprintf(stderr, "stat fail %s", buff);
    return -1;
  }
  i = st.st_size + 1;
  if (NEWSFEEDS_BUF == NULL)
  {
    NEWSFEEDS_BUF = (char *) mymalloc(i);
  }
  else
  {
    NEWSFEEDS_BUF = (char *) myrealloc(NEWSFEEDS_BUF, i);
  }
  i = count = 0;
  while (fgets(buff, sizeof buff, fp) != NULL)
  {
    if (buff[0] != '#' && buff[0] != '\n')
    {
      strcpy(NEWSFEEDS_BUF + i, buff);
      i += strlen(buff);
      count++;
    }
  }
  fclose(fp);

  i = sizeof(newsfeeds_t) * (count + 1);
  if (NEWSFEEDS == NULL)
  {
    NEWSFEEDS = (newsfeeds_t *) mymalloc(i);
    NEWSFEEDS_BYBOARD = (newsfeeds_t **) mymalloc(i);
  }
  else
  {
    NEWSFEEDS = (newsfeeds_t *) myrealloc(NEWSFEEDS, i);
    NEWSFEEDS_BYBOARD = (newsfeeds_t **) myrealloc(NEWSFEEDS_BYBOARD, i);
  }
  NFCOUNT = 0;
  for (ptr = NEWSFEEDS_BUF; newsfeedsptr = (char *) strchr(ptr, '\n'); ptr = newsfeedsptr + 1, NFCOUNT++)
  {
    register char *nptr, *bptr, *pptr;

    *newsfeedsptr = '\0';
    NEWSFEEDS[NFCOUNT].newsgroups = "";
    NEWSFEEDS[NFCOUNT].board = "";
    NEWSFEEDS[NFCOUNT].path = NULL;
    NEWSFEEDS_BYBOARD[NFCOUNT] = NEWSFEEDS + NFCOUNT;
    for (nptr = ptr; (i = *nptr) && isspace(i);)
      nptr++;
    if (*nptr == '\0')
      continue;
    NEWSFEEDS[NFCOUNT].newsgroups = nptr;
    for (nptr++; (i = *nptr) && !isspace(i);)
      nptr++;
    if (*nptr == '\0')
      continue;
    *nptr = '\0';
    for (nptr++; (i = *nptr) && isspace(i);)
      nptr++;
    if (*nptr == '\0')
      continue;
    NEWSFEEDS[NFCOUNT].board = nptr;
    for (nptr++; (i = *nptr) && !isspace(i);)
      nptr++;
    if (*nptr == '\0')
      continue;
    *nptr = '\0';
    for (nptr++; (i = *nptr) && isspace(i);)
      nptr++;
    if (*nptr == '\0')
      continue;
    NEWSFEEDS[NFCOUNT].path = nptr;
    for (nptr++; (i = *nptr) && !strchr("\r\n", i);)
      nptr++;
    /* if (*nptr == '\0') continue; */
    *nptr = '\0';
  }
  qsort(NEWSFEEDS, NFCOUNT, sizeof(newsfeeds_t), nfcmp);
  qsort(NEWSFEEDS_BYBOARD, NFCOUNT, sizeof(newsfeeds_t *), nf_byboardcmp);
}


int
initial_bbs(outgoing)
  char *outgoing;
{
  FILE *FN;
  struct stat st;
  int fd, i;
  char *bbsnameptr = NULL;

#ifdef	MapleBBS
  chdir(BBSHOME);		/* chdir to bbs_home first */
#endif

  /* reopen bbslog */

  if (bbslogfp != NULL)
  {
    fclose(bbslogfp);
    bbslogfp = NULL;
  }

#ifdef WITH_ECHOMAIL

#ifndef	MapleBBS
  init_echomailfp();
  init_bbsfeedsfp();
#endif

#endif

  LOCALNODELIST = NONENEWSFEEDS = 0;

  if (isfile(INNBBSCONF))
  {
    FILE *conf;
    char buffer[MAXPATHLEN];
    conf = fopen(INNBBSCONF, "r");
    if (conf != NULL)
    {
      while (fgets(buffer, sizeof buffer, conf) != NULL)
      {
	char *ptr, *front = NULL, *value = NULL, *value2 = NULL, *value3 = NULL;
	if (buffer[0] == '#' || buffer[0] == '\n')
	  continue;
	for (front = buffer; *front && isspace(*front); front++);
	for (ptr = front; *ptr && !isspace(*ptr); ptr++);
	if (*ptr == '\0')
	  continue;
	*ptr++ = '\0';
	for (; *ptr && isspace(*ptr); ptr++);
	if (*ptr == '\0')
	  continue;
	value = ptr++;
	for (; *ptr && !isspace(*ptr); ptr++);
	if (*ptr)
	{
	  *ptr++ = '\0';
	  for (; *ptr && isspace(*ptr); ptr++);
	  value2 = ptr++;
	  for (; *ptr && !isspace(*ptr); ptr++);
	  if (*ptr)
	  {
	    *ptr++ = '\0';
	    for (; *ptr && isspace(*ptr); ptr++);
	    value3 = ptr++;
	    for (; *ptr && !isspace(*ptr); ptr++);
	    if (*ptr)
	    {
	      *ptr++ = '\0';
	    }
	  }
	}
	if (strcasecmp(front, "expiredays") == 0)
	{
	  Expiredays = atoi(value);
	  if (Expiredays < 0)
	  {
	    Expiredays = EXPIREDAYS;
	  }
	}
	else if (strcasecmp(front, "expiretime") == 0)
	{
	  ptr = strchr(value, ':');
	  if (ptr == NULL)
	  {
	    fprintf(stderr, "Syntax error in innbbs.conf\n");
	  }
	  else
	  {
	    *ptr++ = '\0';
	    His_Maint_Hour = atoi(value);
	    His_Maint_Min = atoi(ptr);
	    if (His_Maint_Hour < 0)
	      His_Maint_Hour = HIS_MAINT_HOUR;
	    if (His_Maint_Min < 0)
	      His_Maint_Min = HIS_MAINT_MIN;
	  }
	}
	else if (strcasecmp(front, "newsfeeds") == 0)
	{
	  if (strcmp(value, "none") == 0)
	    NONENEWSFEEDS = 1;
	}
	else if (strcasecmp(front, "nodelist") == 0)
	{
	  if (strcmp(value, "local") == 0)
	    LOCALNODELIST = 1;
	}
#if 0
			 else if ( strcasecmp(front,"newsfeeds") ==
				 0) { printf("newsfeeds %s\n", value); }
				 else if ( strcasecmp(front,"nodelist") ==
				 0) { printf("nodelist %s\n", value); }
				 else if ( strcasecmp(front,"bbsname") ==
				 0) { printf("bbsname %s\n", value); }
#endif
      }
      fclose(conf);
    }
  }

#ifdef WITH_ECHOMAIL
  bbsnameptr = (char *) fileglue("%s/bbsname.bbs", INNDHOME);
  if ((FN = fopen(bbsnameptr, "r")) == NULL)
  {
    fprintf(stderr, "can't open file %s\n", bbsnameptr);
    return 0;
  }
  while (fscanf(FN, "%s", MYBBSID) != EOF);
  fclose(FN);

#ifndef	MapleBBS
  if (!isdir(fileglue("%s/out.going", BBSHOME)))
  {
    mkdir((char *) fileglue("%s/out.going", BBSHOME), 0750);
  }
#endif

  if (NONENEWSFEEDS == 0)
    readnffile(INNDHOME);
  if (LOCALNODELIST == 0)
  {
    if (readnlfile(INNDHOME, outgoing) != 0)
      return 0;
  }
#endif

  return 1;
}


newsfeeds_t *
search_board(board)
  char *board;
{
  if (!NONENEWSFEEDS)
  {
    newsfeeds_t nft, *nftptr, **find;
    nft.board = board;
    nftptr = &nft;
    find = (newsfeeds_t **) bsearch((char *) &nftptr, NEWSFEEDS_BYBOARD, NFCOUNT, sizeof(newsfeeds_t *), nf_byboardcmp);
    if (find)
      return *find;
  }
  return NULL;
}


nodelist_t *
search_nodelist_bynode(node)
  char *node;
{
  if (!LOCALNODELIST)
  {
    nodelist_t nlt, *nltptr, **find;
    nlt.node = node;
    nltptr = &nlt;
    find = (nodelist_t **) bsearch((char *) &nltptr, NODELIST_BYNODE, NLCOUNT, sizeof(nodelist_t *), nl_bynodecmp);
    if (find)
      return *find;
  }
  return NULL;
}


nodelist_t *
search_nodelist(site, identuser)
  char *site;
  char *identuser;
{
  nodelist_t nlt, *find;
  char buffer[1024];
  if (LOCALNODELIST)
    return NULL;
  nlt.host = site;
  find = (nodelist_t *) bsearch((char *) &nlt, NODELIST, NLCOUNT, sizeof(nodelist_t), nlcmp);
  if (find == NULL && identuser)
  {
    sprintf(buffer, "%s@%s", identuser, site);
    nlt.host = buffer;
    find = (nodelist_t *) bsearch((char *) &nlt, NODELIST, NLCOUNT, sizeof(nodelist_t), nlcmp);
  }
  return find;
}


newsfeeds_t *
search_group(newsgroup)
  char *newsgroup;
{
  newsfeeds_t nft, *find;
  if (NONENEWSFEEDS)
    return NULL;
  nft.newsgroups = newsgroup;
  find = (newsfeeds_t *) bsearch((char *) &nft, NEWSFEEDS, NFCOUNT, sizeof(newsfeeds_t), nfcmp);
  return find;
}


char *
ascii_date(now)
  time_t now;
{
  static char datebuf[40];
  strftime(datebuf, sizeof(datebuf), "%d %b %Y %X GMT", gmtime(&now));
  return datebuf;
}


char *
restrdup(ptr, string)
  char *ptr;
  char *string;
{
  int len;
  if (string == NULL)
  {
    if (ptr != NULL)
      *ptr = '\0';
    return ptr;
  }
  len = strlen(string) + 1;
  if (ptr != NULL)
  {
    ptr = (char *) myrealloc(ptr, len);
  }
  else
    ptr = (char *) mymalloc(len);
  strcpy(ptr, string);
  return ptr;
}


void *
mymalloc(size)
  int size;
{
  char *ptr = (char *) malloc(size);
  if (ptr == NULL)
  {
    syslog(LOG_ERR, "cant allocate memory %m");
    exit(1);
  }
  return ptr;
}

void *
myrealloc(optr, size)
  void *optr;
  int size;
{
  char *ptr = (char *) realloc(optr, size);
  if (ptr == NULL)
  {
    syslog(LOG_ERR, "cant allocate memory %m");
    exit(1);
  }
  return ptr;
}


#ifndef	MapleBBS
void
testandmkdir(dir)
  char *dir;
{
  if (!isdir(dir))
  {
    char path[MAXPATHLEN + 12];
    sprintf(path, "mkdir -p %s", dir);
    system(path);
  }
}
#endif


static char splitbuf[1024];
static char joinbuf[1024];
#define MAXTOK 50
static char *Splitptr[MAXTOK];
char **
split(line, pat)
  char *line, *pat;
{
  register char *p;
  register int i;

  for (i = 0; i < MAXTOK; ++i)
    Splitptr[i] = NULL;
  strncpy(splitbuf, line, sizeof splitbuf - 1);
  /* printf("%d %d\n",strlen(line),strlen(splitbuf)); */
  splitbuf[sizeof splitbuf - 1] = '\0';
  for (i = 0, p = splitbuf; *p && i < MAXTOK - 1;)
  {
    for (Splitptr[i++] = p; *p && !strchr(pat, *p); p++);
    if (*p == '\0')
      break;
    for (*p++ = '\0'; *p && strchr(pat, *p); p++);
  }
  return Splitptr;
}


char **
BNGsplit(line)
  char *line;
{
  register char **ptr = split(line, ",");
  register newsfeeds_t *nf1, *nf2;
  register char *n11, *n12, *n21, *n22;
  register char *str, *key;
  register int i, j;

  for (i = 0; str = ptr[i]; i++)
  {
    nf1 = (newsfeeds_t *) search_group(str);
    for (j = i + 1; key = ptr[j]; j++)
    {
      if (strcmp(str, key) == 0)
      {
	*key = '\0';
	continue;
      }
      nf2 = (newsfeeds_t *) search_group(key);
      if (nf1 && nf2)
      {
	if (strcmp(nf1->board, nf2->board) == 0)
	{
	  *key = '\0';
	  continue;
	}
	for (n11 = nf1->board, n12 = (char *) strchr(n11, ',');
	  n11 && *n11; n12 = (char *) strchr(n11, ','))
	{
	  if (n12)
	    *n12 = '\0';
	  for (n21 = nf2->board, n22 = (char *) strchr(n21, ',');
	    n21 && *n21; n22 = (char *) strchr(n21, ','))
	  {
	    if (n22)
	      *n22 = '\0';
	    if (strcmp(n11, n21) == 0)
	    {
	      *n21 = '\t';
	    }
	    if (n22)
	    {
	      *n22 = ',';
	      n21 = n22 + 1;
	    }
	    else
	      break;
	  }
	  if (n12)
	  {
	    *n12 = ',';
	    n11 = n12 + 1;
	  }
	  else
	    break;
	}
      }
    }
  }
  return ptr;
}


char **
ssplit(line, pat)
  char *line, *pat;
{
  char *p;
  int i;
  for (i = 0; i < MAXTOK; ++i)
    Splitptr[i] = NULL;
  strncpy(splitbuf, line, 1024);
  for (i = 0, p = splitbuf; *p && i < MAXTOK;)
  {
    for (Splitptr[i++] = p; *p && !strchr(pat, *p); p++);
    if (*p == '\0')
      break;
    *p++ = 0;
    /* for (*p='\0'; strchr(pat,*p);p++); */
  }
  return Splitptr;
}


char *
join(lineptr, pat, num)
  char **lineptr, *pat;
  int num;
{
  int i;
  joinbuf[0] = '\0';
  if (lineptr[0] != NULL)
    strncpy(joinbuf, lineptr[0], 1024);
  else
  {
    joinbuf[0] = '\0';
    return joinbuf;
  }
  for (i = 1; i < num; i++)
  {
    strcat(joinbuf, pat);
    if (lineptr[i] != NULL)
      strcat(joinbuf, lineptr[i]);
    else
      break;
  }
  return joinbuf;
}

#ifdef BBSLIB
main()
{
  initial_bbs("feed");
  printf("%s\n", ascii_date());
}
#endif

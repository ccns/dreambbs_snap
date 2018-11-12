/*
 * BBS implementation dependendent part
 * 
 * The only two interfaces you must provide
 * 
 * #include "inntobbs.h" int receive_article(); 0 success not 0 fail
 * 
 * if (storeDB(HEADER[MID_H], hispaths) < 0) { .... fail }
 * 
 * int cancel_article_front( char *msgid );	     0 success not 0 fail
 * 
 * char *ptr = (char*)DBfetch(msgid);
 * 
 * 收到之文章內容 (body)在 char *BODY, 檔頭 (header)在 char *HEADER[] SUBJECT_H,
 * FROM_H, DATE_H, MID_H, NEWSGROUPS_H, NNTPPOSTINGHOST_H, NNTPHOST_H,
 * CONTROL_H, PATH_H, ORGANIZATION_H
 */

/*
 * Sample Implementation
 * 
 * receive_article()         --> post_article()   --> bbspost_write_post();
 * cacnel_article_front(mid) --> cancel_article() --> bbspost_write_cancel();
 */


#define	storeDB	HERwrite


#ifndef PowerBBS
#include "innbbsconf.h"
#include "daemon.h"
#include "bbslib.h"
#include "inntobbs.h"

extern int Junkhistory;


#ifdef	MapleBBS
#define storeDB	HERwrite
static char myfrom[256], mysubject[128];

#else

static char *post_article ARG((char *, char *, char *, char *));
static int cancel_article ARG((char *, char *));

report()
{
  /* Function called from record.o */
  /* Please leave this function empty */
}
#endif


#if defined(PalmBBS)

#ifndef PATH
#define PATH XPATH
#endif

#ifndef HEADER
#define HEADER XHEADER
#endif

#endif

#define XLOG	bbslog

static void
stripn_ansi(buf, str, len)
  char *buf, *str;
  int len;
{
  register int ch, ansi;

  /* chuan: check null */

  if (str != NULL)
  {
    for (ansi = 0; ch = *str; str++)
    {
      if (ch == '\n')
      {
	break;
      }
      else if (ch == 27)
      {
	ansi = 1;
      }
      else if (ansi)
      {
	if (!strchr("[01234567;", ch))
	  ansi = 0;
      }
      else
      {
	*buf++ = ch;
	if (--len <= 0)
	  break;
      }
    }
  }
  *buf = '\0';
}


/* process post write */


static int
bbspost_write_post(fh, board)
  int fh;
  char *board;
{
  register FILE *fhfd = fdopen(fh, "w");
  register char *fptr, *ptr;
  register int mode, ch;
  char buf[128];

  if (fhfd == NULL)
  {
    close(fh);
    bbslog("can't fdopen, maybe disk full\n");
    return -1;
  }

#if 0
  stripn_ansi(buf, FROM, 57);
#endif

  fprintf(fhfd, "發信人: %.57s, 看板: %s\n", FROM, board);

#if 0
  str_decode(SUBJECT);

  stripn_ansi(buf, SUBJECT, 128);
  str_decode(buf);
  str_ncpy(SUBJECT = mysubject, buf, 70);
  stripn_ansi(SUBJECT, SUBJECT, 70);
#endif

  fprintf(fhfd, "標  題: %s\n", SUBJECT);

  stripn_ansi(buf, SITE, 43);
  fprintf(fhfd, "發信站: %s (%s)\n", buf, DATE);

  fprintf(fhfd, "轉信站: %s\n", PATH);

  if (POSTHOST != NULL)
  {
    fprintf(fhfd, "Origin: %.70s\n", POSTHOST);
  }

  fprintf(fhfd, "\n%s", BODY);	/* chuan: header 跟 body 要空行格開 */
  fclose(fhfd);
  close(fh);
  return 0;
}


#ifdef	KEEP_NETWORK_CANCEL
/* process cancel write */
bbspost_write_cancel(fh, board, filename)
  int fh;
  char *board, *filename;
{
  char *fptr, *ptr;
  FILE *fhfd = fdopen(fh, "w"), *fp;
  char buffer[256];

  if (fhfd == NULL)
  {
    bbslog("can't fdopen, maybe disk full\n");
    return -1;
  }

  fprintf(fhfd, "發信人: %s, 信區: %s\n", FROM, board);
  fprintf(fhfd, "標  題: %s\n", SUBJECT);
  fprintf(fhfd, "發信站: %.43s (%s)\n", SITE, DATE);
  fprintf(fhfd, "轉信站: %.70s\n", PATH);
  if (HEADER[CONTROL_H] != NULL)
  {
    fprintf(fhfd, "Control: %s\n", HEADER[CONTROL_H]);
  }
  if (POSTHOST != NULL)
  {
    fprintf(fhfd, "Origin: %s\n", POSTHOST);
  }
  fprintf(fhfd, "\n");
  for (fptr = BODY, ptr = strchr(fptr, '\r'); ptr != NULL && *ptr != '\0'; fptr = ptr + 1, ptr = strchr(fptr, '\r'))
  {
    int ch = *ptr;

    *ptr = '\0';
    fputs(fptr, fhfd);
    *ptr = ch;
  }
  fputs(fptr, fhfd);
  if (POSTHOST != NULL)
  {
    fprintf(fhfd, "\n * Origin: ● %.26s ● From: %.40s\n", SITE, POSTHOST);
  }
  fprintf(fhfd, "\n---------------------\n");
  fp = fopen(filename, "r");
  if (fp == NULL)
  {
    bbslog("can't open %s\n", filename);
    return -1;
  }
  while (fgets(buffer, sizeof buffer, fp) != NULL)
  {
    fputs(buffer, fhfd);
  }
  fclose(fp);
  fflush(fhfd);
  fclose(fhfd);

  {
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
      bbslog("can't write %s\n", filename);
      return -1;
    }
    fprintf(fp, "發信人: %s, 信區: %s\n", FROM, board);
    fprintf(fp, "標  題: %.70s\n", SUBJECT);
    fprintf(fp, "發信站: %.43s (%s)\n", SITE, DATE);
    fprintf(fp, "轉信站: %.70s\n", PATH);
    if (POSTHOST != NULL)
    {
      fprintf(fhfd, "Origin: %s\n", POSTHOST);
    }
    if (HEADER[CONTROL_H] != NULL)
    {
      fprintf(fhfd, "Control: %s\n", HEADER[CONTROL_H]);
    }
    fprintf(fp, "\n");
    for (fptr = BODY, ptr = strchr(fptr, '\r'); ptr != NULL && *ptr != '\0'; fptr = ptr + 1, ptr = strchr(fptr, '\r'))
    {
      *ptr = '\0';
      fputs(fptr, fp);
    }
    fputs(fptr, fp);
    if (POSTHOST != NULL)
    {
      fprintf(fp, "\n * Origin: ● %.26s ● From: %.40s\n", SITE, POSTHOST);
    }
    fclose(fp);
  }
  return 0;
}
#endif				/* KEEP_NETWORK_CANCEL */


#ifndef	MapleBBS
bbspost_write_control(fh, board, filename)
  int fh;
  char *board;
  char *filename;
{
  char *fptr, *ptr;
  FILE *fhfd = fdopen(fh, "w");

  if (fhfd == NULL)
  {
    bbslog("can't fdopen, maybe disk full\n");
    return -1;
  }

  fprintf(fhfd, "Path: %s!%s\n", MYBBSID, HEADER[PATH_H]);
  fprintf(fhfd, "From: %s\n", FROM);
  fprintf(fhfd, "Newsgroups: %s\n", GROUPS);
  fprintf(fhfd, "Subject: %s\n", SUBJECT);
  fprintf(fhfd, "Date: %s\n", DATE);
  fprintf(fhfd, "Organization: %s\n", SITE);
  if (POSTHOST != NULL)
  {
    fprintf(fhfd, "NNTP-Posting-Host: %.70s\n", POSTHOST);
  }
  if (HEADER[CONTROL_H] != NULL)
  {
    fprintf(fhfd, "Control: %s\n", HEADER[CONTROL_H]);
  }
  if (HEADER[APPROVED_H] != NULL)
  {
    fprintf(fhfd, "Approved: %s\n", HEADER[APPROVED_H]);
  }
  if (HEADER[DISTRIBUTION_H] != NULL)
  {
    fprintf(fhfd, "Distribution: %s\n", HEADER[DISTRIBUTION_H]);
  }
  fprintf(fhfd, "\n");
  for (fptr = BODY, ptr = strchr(fptr, '\r'); ptr != NULL && *ptr != '\0'; fptr = ptr + 1, ptr = strchr(fptr, '\r'))
  {
    int ch = *ptr;

    *ptr = '\0';
    fputs(fptr, fhfd);
    *ptr = ch;
  }
  fputs(fptr, fhfd);


  fflush(fhfd);
  fclose(fhfd);
  return 0;
}
#endif				/* MapleBBS */


time_t datevalue;


#if defined(PhoenixBBS) || defined(SecretBBS) || defined(PivotBBS) || defined(MapleBBS)
/* for PhoenixBBS's post article and cancel article */
#ifdef MapleBBS
/* Thor.990318: 跳過HEADER的定義, 不然 HEADER的定義會衝到 */
#define _DNS_H_ 
#endif
#include "bbs.h"

#include <sys/shm.h>

FWCACHE *fwshm;
FWOCACHE *fwoshm;

#ifdef	HAVE_COUNT_BOARD
BCACHE *bshm;
#endif
/* ----------------------------------------------------- */
/* chrono ==> file name (32-based)			 */
/* 0123456789ABCDEFGHIJKLMNOPQRSTUV			 */
/* ----------------------------------------------------- */


char radix32[32] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
  'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
};


void
archiv32(chrono, fname)
  register time_t chrono;	/* 32 bits */
  register char *fname;		/* 7 chars */
{
  register char *str;

  str = fname + 7;
  *str = '\0';
  for (;;)
  {
    *(--str) = radix32[chrono & 31];
    if (str == fname)
      return;
    chrono >>= 5;
  }
}


time_t
chrono32(str)
  register char *str;		/* 0123456 */
{
  register time_t chrono;
  register int ch;

  chrono = 0;
  while (ch = *str++)
  {
    ch -= '0';
    if (ch >= 10)
      ch -= 'A' - '0' - 10;
    chrono = (chrono << 5) + ch;
  }
  return chrono;
}

static void *
attach_shm(shmkey, shmsize)
  register int shmkey, shmsize;
{
  register void *shmptr;
  register int shmid;

  shmid = shmget(shmkey, shmsize, 0);
  if (shmid < 0)
  {
    shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
  }
  else
  {
    shmsize = 0;
  }

  shmptr = (void *) shmat(shmid, NULL, 0);

  if (shmsize)
    memset(shmptr, 0, shmsize);

  return shmptr;
}

static void
log_match(match)
  char *match;
{
  FILE *file;
  file = fopen(FN_BANMAIL_LOG,"a+");
  if(file)
  {
    fprintf(file,"MATCH : %s\n",match);
    fclose(file);
  }
}

static int
seek_log1(title,brd,mode,flag)
  char *title;
  char *brd;
  int mode;
  int flag;
{
  BANMAIL *head,*tail;
  FW *fhead,*ftail;
  head = fwshm->fwcache;
  tail = head + fwshm->number;
  fhead = fwoshm->fwocache;
  ftail = fhead + MAXOFILEWALL;
  
  if(!title)
    return 0;

  while(mode && fwshm->number && head < tail )
  {
     if( (head->mode & flag)&& strstr(title,head->data))
     {
       log_match(head->data);
       if (head->usage < 500000)
         head->usage++;
       head->time = time(0);
       return 1;
     }
     head++;
  }

  while(fhead < ftail && *(fhead->name))
  {
     if((fhead->mode & flag) && !strcmp(brd,fhead->name) && strstr(title,fhead->data))
     {
       log_match(fhead->data);
       if(fhead->usage < 500000)
         fhead->usage++;
       fhead->time = time(0);
       return 1;
     }
     fhead++;
  }

  return 0;
}

static void
log_it(board,form,subject,host,date,path)
  char *board;
  char *form;
  char *subject;
  char *host;
  char *date;
  char *path;
{
  FILE *file;
  file = fopen(FN_BANMAIL_LOG,"a+");
  if(file)
  {
    fprintf(file,"%-12s%-50s\n%-80s%-80s\n%-30.30s%-50.50s\n",board,form,subject,host ? host : "無來源",
            date?date:"無時間",path?path:"無路徑");
    fclose(file);
  }
}

#ifdef HAVE_COUNT_BOARD
static BRD *
find_brd(bname)
  char *bname;
{
  BRD *head, *tail;

  head = bshm->bcache;
  tail = head + bshm->number;
  do
  {
    if (!str_cmp(bname, head->brdname))
      return head;
  } while (++head < tail);

  return NULL;
}
#endif

static char *
post_article(board, userid, usernick, firstpath)
  char *board, *userid, *usernick;
  char *firstpath;
{
  static char name[8];
  char *fname, fpath[128],buf[128];
  HDR header;
  time_t now;
  int fh, linkflag,tag,check_ok;

#ifdef HAVE_COUNT_BOARD
  BRD *brd;
#endif
  
  if(!fwoshm)
    fwoshm = attach_shm(FWOSHM_KEY,sizeof(FWOCACHE));
  if(!fwshm)
    fwshm = attach_shm(FWSHM_KEY, sizeof(FWCACHE));

#ifdef HAVE_COUNT_BOARD
  if(!bshm)
    bshm = attach_shm(BRDSHM_KEY, sizeof(BCACHE));

  brd = find_brd(board);
#endif  

  check_ok = 0;
  sprintf(buf,"brd/%s/check_ban",board);
  tag = access(buf,0);

  if(seek_log1(FROM,board,tag,FW_OWNER) || seek_log1(SUBJECT,board,tag,FW_TITLE) || 
     seek_log1(SITE,board,tag,FW_TIME) || seek_log1(PATH,board,tag,FW_PATH)||
     seek_log1(POSTHOST,board,tag,FW_ORIGIN))
  {
    log_it(board,FROM,SUBJECT,POSTHOST,SITE,PATH);
    check_ok = 1;
#ifdef	HAVE_COUNT_BOARD
    if(brd) // && !strcmp(brd->brdname, "WindTop"))
      brd->n_bans++;
#endif

  }

  sprintf(fpath, "brd/%s/@/A", check_ok ? BRD_TRASHCAN : board);
  fname = strrchr(fpath, 'A') + 1;

  now = time(NULL);
  while (1)
  {
    archiv32(now, fname);
    fname[-3] = fname[6];

    fh = open(fpath, O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (fh >= 0)
      break;

    if (errno != EEXIST)
    {
      bbslog(" Err: can't write or other errors\n");
      return NULL;
    }
    now++;
  }

  strcpy(name, fname);

  linkflag = 1;

  if (firstpath && *firstpath)
  {
    close(fh);
    unlink(fpath);

#ifdef DEBUGLINK
    bbslog("try to link %s to %s", firstpath, fpath);
#endif

    linkflag = link(firstpath, fpath);
    if (linkflag)
    {
      fh = open(fpath, O_CREAT | O_EXCL | O_WRONLY, 0644);
    }
  }
  if (linkflag)
  {
    if (bbspost_write_post(fh, board) < 0)
      return NULL;

    if (firstpath)
      strcpy(firstpath, fpath);	/* opus : write back */
  }

  bzero((void *) &header, sizeof(header));
  header.chrono = now;
  header.xmode = POST_INCOME;
  strcpy(header.xname, --fname);
#if 0
  strcpy(header.owner, userid);
  strcpy(header.nick, usernick);
#endif
  /* Thor.980825: 防止字串太長蓋過頭 */
  strncpy(header.owner, userid, sizeof(header.owner)-1);
  strncpy(header.nick, usernick, sizeof(header.nick)-1);

  {
    struct tm *ptime;

    ptime = localtime(&datevalue);
    /* Thor.990329: y2k */
    sprintf(header.date, "%02d/%02d/%02d",
      ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  }
  /* Thor.980825: 防止字串太長蓋過頭 */
#if 0
  strcpy(header.title, SUBJECT);
#endif
  strncpy(header.title, SUBJECT, sizeof(header.title)-1);

  strcpy(fname - 2, ".DIR");
  /* Thor.980825: 特別注意 fpath是否會寫入 bbs home */
  if ((fh = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
  {
    return NULL;
  }

  write(fh, &header, sizeof(header));
  close(fh);

#ifdef	HAVE_COUNT_BOARD
    if(brd) // && !strcmp(brd->brdname, "WindTop"))
      brd->n_news++;
#endif


  return name;
}


static int
cancel_article(board, time)
  char *board;
  register time_t time;
{
  HDR header;
  struct stat state;
  register long size;
  register int fd, ent;
  char buf[128];

  /* XLOG("cancel [%s] %d\n", board, time); */

  sprintf(buf, "brd/%s/.DIR", board);
  if ((fd = open(buf, O_RDWR)) == -1)
    return 0;

  /* XLOG("cancel folder [%s]\n", buf); */

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
  f_exlock(fd);

  fstat(fd, &state);
  size = sizeof(header);
  ent = ((long) state.st_size) / size;
  while (1)
  {
    ent -= 16;
    if (ent <= 0)
      break;
    lseek(fd, size * ent, SEEK_SET);
    if (read(fd, &header, size) != size)
      break;

    if (header.chrono <= time)
    {
      /* XLOG("cancel 1: %s\n", header.xname); */

      do
      {
	if (header.chrono == time)
	/* if (header.chrono == time && !(header.xmode & POST_MARKED)) */
	{ /* Thor.980824: 禁止 mark的文章被cancel */
          /* Thor.981014: 想想算了, cancel就cancel吧... */
	  header.xmode |= POST_CANCEL;
	  if (POSTHOST)
	  {
	    sprintf(board = buf, "cancel by: %s", POSTHOST);
	    board[TTLEN] = '\0';
	  }
	  else
	  {
	    board = "<< article canceled >>";
	  }
	  strcpy(header.title, board);
	  lseek(fd, (off_t) - size, SEEK_CUR);
	  write(fd, &header, size);
	  break;
	}
	if (header.chrono > time)
	  break;
      } while (read(fd, &header, size) == size);
      break;
    }
  }

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
  f_unlock(fd);

  close(fd);
  return 0;
}


#elif defined(PalmBBS)
#undef PATH XPATH
#undef HEADER XHEADER
#include "server.h"


char *
post_article(homepath, userid, board, writebody, pathname, firstpath)
  char *homepath;
  char *userid, *board;
  int (*writebody) ();
char *pathname, *firstpath;

{
  PATH msgdir, msgfile;
  static PATH name;

  READINFO readinfo;
  SHORT fileid;
  char buf[MAXPATHLEN];
  struct stat stbuf;
  int fh;

  strcpy(msgdir, homepath);
  if (stat(msgdir, &stbuf) == -1 || !S_ISDIR(stbuf.st_mode))
  {
    /* A directory is missing! */
    bbslog(":Err: Unable to post in %s.\n", msgdir);
    return NULL;
  }
  get_filelist_ids(msgdir, &readinfo);

  for (fileid = 1; fileid <= BBS_MAX_FILES; fileid++)
  {
    int oumask;

    if (test_readbit(&readinfo, fileid))
      continue;
    fileid_to_fname(msgdir, fileid, msgfile);
    sprintf(name, "%04x", fileid);

#ifdef DEBUG
    printf("post to %s\n", msgfile);
#endif

    if (firstpath && *firstpath)
    {

#ifdef DEBUGLINK
      bbslog("try to link %s to %s", firstpath, msgfile);
#endif

      if (link(firstpath, msgfile) == 0)
	break;
    }
    oumask = umask(0);
    fh = open(msgfile, O_CREAT | O_EXCL | O_WRONLY, 0664);
    umask(oumask);
    if (writebody)
    {
      if ((*writebody) (fh, board, pathname) < 0)
	return NULL;
    }
    else
    {
      if (bbspost_write_post(fh, board, pathname) < 0)
	return NULL;
    }
    close(fh);
    break;
  }

#ifdef CACHED_OPENBOARD
  {
    char *bname;

    bname = strrchr(msgdir, '/');
    if (bname)
      notify_new_post(++bname, 1, fileid, stbuf.st_mtime);
  }
#endif

  return name;
}

cancel_article(homepath, board, file)
  char *homepath;
  char *board, *file;
{
  PATH fname;

#ifdef  CACHED_OPENBOARD
  PATH bdir;
  struct stat stbuf;

  sprintf(bdir, "%s/boards/%s", homepath, board);
  stat(bdir, &stbuf);
#endif

  sprintf(fname, "%s/boards/%s/%s", homepath, board, file);
  unlink(fname);
  /* kill it now! the function is far small then original..  :) */
  /* because it won't make system load heavy like before */

#ifdef CACHED_OPENBOARD
  notify_new_post(board, -1, hex2SHORT(file), stbuf.st_mtime);
#endif
}

#else
error("You should choose one of the systems: PhoenixBBS, PowerBBS, or PalmBBS")
#endif

#else

receive_article()
{
}

cancel_article_front(msgid)
  char *msgid;
{
}
#endif


/* process cancel write */


/* opus : a lot of modification here */


#if 0
static void
unify_from(addr, userid, usernick)
  register char *addr, *userid, *usernick;
{
  register char *ptr, *lesssym;

  *usernick = 0;

  ptr = (char *) strchr(addr, '@');
  if (ptr == NULL)
  {
    sprintf(userid, "err@%s", addr);
    return;
  }

  lesssym = (char *) strchr(addr, '<');
  if (lesssym == NULL || (*addr != '"' && lesssym >= ptr))
  {
    /* mail_addr (name) */
    /* mail_addr */

    if (ptr = strchr(addr, '('))
    {
      ptr[-1] = 0;
      if (lesssym = strrchr(++ptr, ')'))
      {
	*lesssym = 0;
	strcpy(usernick, ptr);
	str_decode(usernick);
      }
    }
    strcpy(userid, addr);
  }
  else
  {
    /* name <mail_addr> */

    ptr = lesssym - 2;
    if (*addr == '"')
    {
      addr++;
      if (*ptr == '"')
	ptr--;
    }
    if (*addr == '(')
    {
      addr++;
      if (*ptr == ')')
	ptr--;
    }
    ptr[1] = '\0';
    strcpy(usernick, addr);
    str_decode(usernick);

    if (ptr = strchr(++lesssym, '>'))
      *ptr = '\0';
    strcpy(userid, lesssym);
  }
}
#endif


#ifdef	_ANTI_SPAM_
/* ---------------------------------------------------- */
/* anti-spam						 */
/* ---------------------------------------------------- */


#define SPAM_HASH_SIZE  2048


typedef struct SpamHeader
{
  unsigned int zauthor;
  unsigned int ztitle;
  unsigned int zbody;

  char *author;
  char *title;
  newsfeeds_t *nf;		/* 第一次出現在哪個 newsfeeds */

  int count;			/* 有幾篇已發現 */
  int cspam;			/* 有幾篇被攔截 */
  time_t uptime;		/* 最近出現時間 */

  struct SpamHeader *next;
}          SpamHeader;


static SpamHeader *SpamBucket[SPAM_HASH_SIZE];


static unsigned int
str_zcode(str, len)
  unsigned char *str;
  int len;
{
  unsigned int code, cc;

  code = 32713;
  while (cc = *str++)
  {
    code = (code << 3) ^ cc ^ len;
    if (--len <= 0)
      break;
  }

  return code;
}


static SpamHeader *
spam_find(spam)
  SpamHeader *spam;
{
  SpamHeader *hash;
  unsigned int zauthor, ztitle, zbody;
  char *author, *title;

  zauthor = spam->zauthor;

  if (hash = SpamBucket[zauthor & (SPAM_HASH_SIZE - 1)])
  {
    ztitle = spam->ztitle;
    zbody = spam->zbody;
    author = spam->author;
    title = spam->title;

    do
    {
      if (zbody == spam->zbody && zauthor == hash->zauthor &&
	ztitle == hash->ztitle && !strcmp(hash->author, author) &&
	!strcmp(hash->title, title))
	break;
    } while (hash = hash->next);
  }

  return hash;
}


static void
spam_add(spam)
  SpamHeader *spam;
{
  SpamHeader *hash;
  int zcode;

  zcode = spam->zauthor & (SPAM_HASH_SIZE - 1);
  spam->author = strdup(spam->author);
  spam->title = strdup(spam->title);

  hash = (SpamHeader *) malloc(sizeof(SpamHeader));
  memcpy(hash, spam, sizeof(SpamHeader));

  time(&hash->uptime);
  hash->next = SpamBucket[zcode];
  SpamBucket[zcode] = hash;
}


void
spam_expire()
{
  time_t now, due;
  int i;
  SpamHeader *spam, **prev, *next;
  FILE *fp;

  due = time(&now) - 60 * 60;
  fp = fopen("innd/spam.log", "a");
  fprintf(fp, "[%d] %s\n", now, ctime(&now));

  for (i = 0; i < SPAM_HASH_SIZE; i++)
  {
    if (spam = SpamBucket[i])
    {
      prev = &SpamBucket[i];
      do
      {
	next = spam->next;
	fprintf(fp, "[%d] %d %d\n  %s\n  %s\n",
	  spam->uptime, spam->count, spam->cspam,
	  spam->author, spam->title);

	if (spam->uptime < due)
	{
	  free(spam->author);
	  free(spam->title);
	  free(spam);
	  *prev = next;
	}
	else
	{
	  prev = &(spam->next);
	}
      } while (spam = next);
    }
  }

  fclose(fp);
}
#endif				/* _ANTI_SPAM_ */


/* ---------------------------------------------------- */


int
receive_article()
{
  register char *ngptr, *nngptr, *pathptr;
  register char **splitptr;
  register char *fname;
  register newsfeeds_t *nf;
  char xdate[32];
  char xpath[180];
  char hispaths[2048];
  char firstpath[128], *firstpathbase;
  register int hislen;
  char boardhome[80];
  char myaddr[128], mynick[128];

  /* Thor.980825:
        gc patch:
            原因
             a.lib/str_decode 只能接受 decode 完 strlen < 256 ...
             b.盡量不修改 lib , 而且太長的 from 和 subject 似乎沒什麼意義. */
 
#if 0
  stripn_ansi(hispaths, FROM, 512);
#endif
  stripn_ansi(hispaths, FROM, 255);


  str_decode(hispaths);
  str_ncpy(myfrom, hispaths, sizeof(myfrom));
  FROM = myfrom;

#if 0
  stripn_ansi(hispaths, SUBJECT, 512);
#endif
  stripn_ansi(hispaths, SUBJECT, 255);


  str_decode(hispaths);
  str_ncpy(mysubject, hispaths, 70);
  SUBJECT = mysubject;

#ifdef _ANTI_SPAM_
  SpamHeader zart, *spam;
#endif

#if 0
  strcpy(hispaths, FROM);
  str_from(hispaths, myaddr, mynick);
#endif

#ifdef _ANTI_SPAM_
  memset(&zart, 0, sizeof(zart));
  zart.zauthor = str_zcode(zart.author = myaddr, 256);
  zart.ztitle = str_zcode(zart.title = str_ttl(SUBJECT), 128);
  zart.zbody = str_zcode(BODY, 512);

  if (spam = spam_find(&zart))
  {
    time(&spam->uptime);
    if ((++spam->count > 6) || spam->cspam)
    {
      ++spam->cspam;
      return 0;
    }
  }
#endif

#if 0
  if (FROM == NULL)
  {
    bbslog(":Err: article without usrid %s\n", MSGID);
    return 0;
  }
#endif

  firstpath[0] = *hispaths = hislen = 0;
  firstpathbase = firstpath;

  /* try to split newsgroups into separate group and check if any duplicated */

  splitptr = (char **) BNGsplit(GROUPS);

  /* try to use hardlink */

  /*
   * for ( ngptr = GROUPS, nngptr = (char*) strchr(ngptr,','); ngptr != NULL
   * && *ngptr != '\0'; nngptr = (char*)strchr(ngptr,',')) {
   */

  /* for (ngptr = *splitptr; ngptr; ngptr = *(++splitptr)) /* opus */
  while (ngptr = *splitptr++)
  {
    char *boardptr, *nboardptr;

    /* if (nngptr != NULL) { nngptr = '\0'; } */

    if (!*ngptr)
      continue;

    nf = (newsfeeds_t *) search_group(ngptr);
    if (nf == NULL)
    {
      /* bbslog( "unwanted \'%s\'\n", ngptr ); */
      /* if( strstr( ngptr, "tw.bbs" ) != NULL ) { } */
      continue;
    }

#ifdef	_ANTI_SPAM_
    if (spam)
    {
      if (spam->nf == nf)	/* 同一個 newsgroup 重複 post ==> 惡性重大 */
      {
	spam->count++;
	spam->cspam++;
	break;
      }
    }
    else
    {
      zart.count++;

      if (!zart.nf)
	zart.nf = nf;
    }
#endif

    if (!(boardptr = nf->board) || !*boardptr)
      continue;
    if (nf->path == NULL || !*nf->path)
      continue;

    for (nboardptr = (char *) strchr(boardptr, ','); boardptr && *boardptr; nboardptr = (char *) strchr(boardptr, ','))
    {
      if (nboardptr)
      {
	*nboardptr = '\0';
      }
      if (*boardptr != '\t')
      {

#ifndef	MapleBBS
	boardhome = (char *) fileglue("%s/boards/%s", BBSHOME, boardptr);

	if (!isdir(boardhome))
	{
	  bbslog(":Err: unable to write %s\n", boardhome);
	  continue;
	}
#endif

	if (!hislen)		/* opus : 第一個 */
	{
	  char poolx[256];

	  str_ncpy(poolx, FROM, 255);
	  str_from(poolx, myaddr, mynick);

#if 000
	  if (mynick[0])
	  {
	    sprintf(FROM = myfrom, "%s (%s)", myaddr, mynick);
	  }
#endif

	  datevalue = parsedate(DATE, NULL);

	  if (datevalue > 0)
	  {
	    memcpy(xdate, ctime(&datevalue), 24);
	    xdate[24] = '\0';
	    DATE = xdate;
	  }

#ifndef	MapleBBS
	  if (SITE && strcasecmp("Computer Science & Information Engineering NCTU", SITE) == 0)
	  {
	    SITE = "交大資工 News Server";
	  }
	  else if (SITE && strcasecmp("Dep. Computer Sci. & Information Eng., Chiao Tung Univ., Taiwan, R.O.C", SITE) == 0)
	  {
	    SITE = "交大資工 News Server";
	  }
	  else if (SITE == NULL || *SITE == '\0')
	  {
	    if (nameptrleft != NULL && nameptrright != NULL)
	    {
	      static char sitebuf[80];
	      char savech = *nameptrright;

	      *nameptrright = '\0';
	      strncpy(sitebuf, nameptrleft, sizeof sitebuf);
	      *nameptrright = savech;
	      SITE = sitebuf;
	    }
	    else
	      /* SITE = "(Unknown)"; */
	      SITE = "";
	  }
	  if (strlen(MYBBSID) > 70)
	  {
	    bbslog(" :Err: your bbsid %s too long\n", MYBBSID);
	    return 0;
	  }
#endif

	  sprintf(xpath, "%s!%.*s", MYBBSID, sizeof(xpath) - strlen(MYBBSID) - 2, PATH);
	  for (pathptr = PATH = xpath; pathptr = strstr(pathptr, ".edu.tw");)
	  {
	    strcpy(pathptr, pathptr + 7);
	  }
	  xpath[71] = '\0';

#ifndef	MapleBBS
	  echomaillog();
#endif
	}

	/*
	 * if ( !isdir( boardhome )) { bbslog( ":Err: unable to write
	 * %s\n",boardhome); testandmkdir(boardhome); }
	 */

	if (fname = (char *) post_article(boardptr, myaddr, mynick, firstpath))
	{
	  register int flen;

	  sprintf(xpath, "%s/%s", boardptr, fname);
	  flen = strlen(fname = xpath);

#ifndef	MapleBBS
	  if (firstpath[0] == '\0')
	  {
	    sprintf(firstpath, "%s/boards/%s", BBSHOME, fname);
	    firstpathbase = firstpath + strlen(BBSHOME) + strlen("/boards/");
	  }
#endif

	  if (flen + hislen < sizeof(hispaths) - 2)
	  {
	    if (hislen)
	    {
	      hispaths[hislen++] = ' ';
	    }

	    strcpy(hispaths + hislen, fname);
	    hislen += flen;

#if 000
	    strcpy(hispaths + hislen, fname);
	    hislen += flen;
	    strcpy(hispaths + hislen - 1, " ");
#endif
	  }
	}
	else
	{
	  bbslog(":Err: board %s\n", boardptr);
	  return 0;		/* opus : -1 */
	}
      }

      /* boardcont: */

      if (nboardptr)
      {
	*nboardptr = ',';
	boardptr = nboardptr + 1;
      }
      else
	break;

    }				/* for board1,board2,... */
    /* if (nngptr != NULL) ngptr = nngptr + 1; else break; */

#ifndef	MapleBBS
    if (*firstpathbase)
      feedfplog(nf, firstpathbase, 'P');
#endif
  }

#ifndef	MapleBBS
  if (*hispaths)
    bbsfeedslog(hispaths, 'P');
#endif

  if (hislen || Junkhistory)
  {
    if (storeDB(HEADER[MID_H], hispaths) < 0)
    {
      bbslog("store DB fail\n");
      /* I suspect here will introduce duplicated articles */
      /* return -1; */
    }
  }

#ifdef	_ANTI_SPAM_
  if (!spam)
  {
    spam_add(&zart);
  }
#endif

  return 0;
}


#ifndef	MapleBBS
receive_control()
{
  char *boardhome, *fname;
  char firstpath[MAXPATHLEN], *firstpathbase;
  char **splitptr, *ngptr;
  newsfeeds_t *nf;

  bbslog("control post %s\n", HEADER[CONTROL_H]);
  boardhome = (char *) fileglue("%s/boards/control", BBSHOME);
  testandmkdir(boardhome);
  *firstpath = '\0';
  if (isdir(boardhome))
  {
    fname = (char *) post_article(boardhome, FROM, "control", bbspost_write_control, NULL, firstpath);
    if (fname != NULL)
    {
      if (firstpath[0] == '\0')
	sprintf(firstpath, "%s/boards/control/%s", BBSHOME, fname);
      if (storeDB(HEADER[MID_H], (char *) fileglue("control/%s", fname)) < 0)
      {
      }
      bbsfeedslog(fileglue("control/%s", fname), 'C');
      firstpathbase = firstpath + strlen(BBSHOME) + strlen("/boards/");
      splitptr = (char **) BNGsplit(GROUPS);
      for (ngptr = *splitptr; ngptr != NULL; ngptr = *(++splitptr))
      {
	if (*ngptr == '\0')
	  continue;
	nf = (newsfeeds_t *) search_group(ngptr);
	if (nf == NULL)
	  continue;
	if (nf->board == NULL)
	  continue;
	if (nf->path == NULL)
	  continue;
	feedfplog(nf, firstpathbase, 'C');
      }
    }
  }
  return 0;
}
#endif				/* MapleBBS */


#define XLOG	bbslog


static int
cancel_article_front(msgid)
  char *msgid;
{
  register char *ptr = (char *) DBfetch(msgid);
  register char *filelist;
  char histent[1024], filename[80], myfrom[128];
  char buffer[256];
  register char *token;

#ifndef	MapleBBS
  char firstpath[MAXPATHLEN], *firstpathbase;
#endif

  /* XLOG("cancel %s <%s>\n", HEADER[FROM_H], msgid);  */

  if (ptr == NULL)
    return 0;

  str_from(HEADER[FROM_H], myfrom, buffer);

  /* XLOG("cancel %s (%s)\n", myfrom, buffer); */

  str_ncpy(filelist = histent, ptr, sizeof histent);

#if 000
  if (filelist = strchr(filelist, '\t'))
  {
    filelist++;
  }

  XLOG("cancel list (%s)\n", filelist);
#endif

  for (ptr = filelist; ptr && *ptr;)
  {
    register char *file;
    register int fd;

    for (; *ptr && isspace(*ptr); ptr++);
    if (*ptr == '\0')
      break;
    file = ptr;

    /* XLOG("cancel file (%s)\n", file); */

    for (ptr++; *ptr && !isspace(*ptr); ptr++);
    if (*ptr != '\0')
    {
      *ptr++ = '\0';
    }

    /* XLOG("cancel file-2 (%s)\n", file);  */

    token = strchr(file, '/');
    if (!token)
      return 0;

    /* XLOG("cancel token (%s)\n", token);  */

    *token++ = '\0';
    sprintf(filename, "brd/%s/%c/A%s", file, token[6], token);

    /* XLOG("cancel path (%s)\n", filename);  */

#ifdef	MapleBBS
    if ((fd = open(filename, O_RDONLY)) >= 0)
    {
      register char *xfrom;
      register int len;


      len = read(fd, buffer, 128);
      close(fd);

#if 0
      if ((len < 8) || memcmp(buffer, "發信人: ", 8))
	return 0;
#endif

      /* SoC: get local artilce headers for cancel back */

      if ((len > 8) && (memcmp(buffer, "發信人: ", 8) == 0))
      {
        /* Thor.981221: 註解: 外來文章 */
	char *str;

	buffer[len] = '\0';
	xfrom = buffer + 8;
	if (str = strchr(xfrom, '\n'))
	  *str = '\0';
	if (str = strrchr(xfrom, ','))
	  *str = '\0';

	if (strstr(xfrom, myfrom))
	  fd = -1;
      }
      else if ((len > 6) && (memcmp(buffer, "作者: ", 6) == 0))
      {
	char XFROM[128];
        /* Thor.981221: 註解: 接受 CS-cancel 砍local文章 */

	/* The cancel msg will cancel back bbs article */
	if (!strstr(PATH, "cs-cancel"))
	{
          /* Thor.981221: xfrom 還沒跑出來:P */
	  /* bbslog("Fake cancel %s, %s/%s, path: %s\n", xfrom, file, token, PATH); */
	  bbslog("Fake cancel %s/%s, sender: %s, path: %s\n", file, token, FROM, PATH);
	  return 0;
	}
        /* Thor.981221: xfrom 還沒跑出來:P */
	/* bbslog("CS-cancel %s, %s/%s, path: %s\n", xfrom, file, token, PATH); */
        

	buffer[len] = '\0';
	if (xfrom = strchr(buffer + 6, ' '))
	  *xfrom = '\0';
	sprintf(XFROM, "%s.bbs@%s", buffer + 6, BBSADDR);
	xfrom = XFROM;

	bbslog("CS-cancel %s, %s/%s, path: %s\n", xfrom, file, token, PATH);

	if (!strncmp(myfrom, xfrom, 80))
	  fd = -1;
      }

      if (fd >= 0)
      {
	bbslog("Invalid cancel %s, sender: %s, path: %s\n", xfrom, FROM, PATH);
	return 0;
      }

      /* SoC - */

#else

    if (fp = fopen(filename, "r"))
    {
      register char *xfrom;

      while (fgets(buffer, sizeof buffer, fp))
      {
	if (strncmp(buffer, "發信人: ", 8) == 0)
	{
	  if (xfrom = strchr(boardhome, ' '))
	    *xfrom = '\0';
	  xfrom = boardhome;
	  break;
	}
	if (buffer[0] == '\n')
	  break;
	if (xfrom = strchr(buffer, '\n'))
	  *xfrom = '\0';
      }
      fclose(fp);
#endif

#ifdef	KEEP_NETWORK_CANCEL
      *firstpath = '\0';
      bbslog("cancel post %s\n", filename);
      boardhome = (char *) fileglue("%s/brd/deleted", BBSHOME);
      testandmkdir(boardhome);
      if (isdir(boardhome))
      {
	char subject[1024];
	char *fname;

	if (POSTHOST)
	{
	  sprintf(subject, "cancel by: %.1000s", POSTHOST);
	}
	else
	{
	  char *body, *body2;

	  body = strchr(BODY, '\r');
	  if (body != NULL)
	    *body = '\0';
	  body2 = strchr(BODY, '\n');
	  if (body2 != NULL)
	    *body = '\0';
	  sprintf(subject, "%.1000s", BODY);
	  if (body != NULL)
	    *body = '\r';
	  if (body2 != NULL)
	    *body = '\n';
	}
	if (*subject)
	  SUBJECT = subject;
	fname = (char *) post_article(boardhome, FROM, "deleted", bbspost_write_cancel, filename, firstpath);
	if (fname != NULL)
	{
	  if (firstpath[0] == '\0')
	  {
	    sprintf(firstpath, "%s/boards/deleted/%s", BBSHOME, fname);
	    firstpathbase = firstpath + strlen(BBSHOME) + strlen("/boards/");
	  }
	  if (storeDB(HEADER[MID_H], (char *) fileglue("deleted/%s", fname)) < 0)
	  {
	    /* should do something */
	    bbslog("store DB fail\n");
	    /* return -1; */
	  }
	  bbsfeedslog(fileglue("deleted/%s", fname), 'D');

#ifdef OLDDISPATCH
	  {
	    char board[256];
	    newsfeeds_t *nf;
	    char *filebase = filename + strlen(BBSHOME) + strlen("/boards/");
	    char *filetail = strrchr(filename, '/');

	    if (filetail != NULL)
	    {
	      strncpy(board, filebase, filetail - filebase);
	      nf = (newsfeeds_t *) search_board(board);
	      if (nf != NULL && nf->board && nf->path)
	      {
		feedfplog(nf, firstpathbase, 'D');
	      }
	    }
	  }
#endif
	}
	else
	{
	  bbslog(" fname is null %s %s\n", boardhome, filename);
	  return -1;
	}
      }
#else
      /* bbslog("**** %s should be removed\n", filename); */

/*      unlink(filename);*/   /* by visor 20000814 */
      /* Thor.981014: unlink 其實是可以不用加的, 因為 cancel_article
                      會標示 POST_CANCEL, 而 expire 會unlink 
                      若要保留mark文章不被砍, 則此項須commented */
#endif

      cancel_article(file, chrono32(token));

    }
  }

#ifndef	MapleBBS
  if (*firstpath)
  {
    char **splitptr, *ngptr;
    newsfeeds_t *nf;

    splitptr = (char **) BNGsplit(GROUPS);
    for (ngptr = *splitptr; ngptr != NULL; ngptr = *(++splitptr))
    {
      if (*ngptr == '\0')
	continue;
      nf = (newsfeeds_t *) search_group(ngptr);
      if (nf == NULL)
	continue;
      if (nf->board == NULL)
	continue;
      if (nf->path == NULL)
	continue;
      feedfplog(nf, firstpathbase, 'D');
    }
  }
#endif

  return 0;
}


static inline int
is_loopback(path, token, len)
  char *path, *token;
  int len;
{
  int cc;

  for (;;)
  {
    cc = path[len];
    if ((!cc || cc == '!') && !memcmp(path, token, len))
      return 1;

    for (;;)
    {
      cc = *path;
      if (!cc)
	return 0;
      path++;
      if (cc == '!')
	break;
    }
  }
}


int
my_recv(client)
  ClientType *client;
{
  char *data;
  FILE *out;
  int rel;

  out = client->Argv.out;
  rel = 0;

  readlines(client);

#if 0
  if (HEADER[SUBJECT_H] && HEADER[FROM_H] && HEADER[DATE_H] &&
    HEADER[MID_H] && HEADER[NEWSGROUPS_H])
#endif
  /* Thor.980825: 
        gc patch: 原因
          a.null path (不論之前是什麼原因) 到判斷 is_lookback 時會當掉.
          b.後面還有一段是處理 null path 的(1596), 如此應較合理. */
  if (HEADER[SUBJECT_H] && HEADER[FROM_H] && HEADER[DATE_H] &&
    HEADER[MID_H] && HEADER[NEWSGROUPS_H] && HEADER[PATH_H])
  {
    if (data = HEADER[CONTROL_H])
    {
      /* chuan: the information seems useless */
      /* bbslog("Control: %s\n", HEADER[CONTROL_H]); */

      if (strncasecmp(data, "cancel ", 7) == 0)
      {
	rel = cancel_article_front(data + 7);
      }

#ifndef	MapleBBS
      else
      {
	rel = receive_control();
      }
#endif
    }

#ifndef	MapleBBS
    else if (FROM == NULL)	/* it should have */
    {
      bbslog(":Err: article without usrid %s\n", MSGID);
    }
#endif

    else
    {
/*#define	BBS_TOKEN	"bbs.yzu"*/	/* opus : custom */

      data = HEADER[PATH_H];
      /* if (is_loopback(data, BBS_TOKEN, sizeof(BBS_TOKEN) - 1)) */
      /* Thor.990318: 直接以 MYBBSID 取代, 所需修改比較少 */
      if (is_loopback(data, MYBBSID, strlen(MYBBSID)) && HEADER[PATH_H])
                                              /* lkchu:981201: patch by gc */
      {
	bbslog(":Warn: Loop back article: %s\n", data);
      }
      else
      {

#if 1
	/* opus : anti-spam */
	int cc;

	data = GROUPS;
	for (;;)
	{
	  cc = *data++;
	  if (cc == 0)
	  {
	    rel = receive_article();
	    break;
	  }
	  if (cc == ',')
	  {
	    if (++rel > 10)	/* 超過 10 個 news groups */
	    {
	      rel = 0;
	      break;
	    }
	  }
	}

#else

	rel = receive_article();
#endif

      }
    }

    if (rel == -1)
    {
      fprintf(out, "400 server side failed\r\n");
      fflush(out);
      fclose(out);
      verboselog("Ihave Put: 400\n");
      clearfdset(client->fd);
      fclose(client->Argv.in);
      close(client->fd);
      client->fd = -1;
      client->mode = 0;
      client->ihavefail++;
      return -1;
    }
    else
    {
      fprintf(out, "235\r\n");
      /* verboselog("Ihave Put: 235\n"); */
    }
    /* fflush(out); */
  }
  else if (!HEADER[PATH_H])
  {
   /* Thor.981224: lkchu patch: 
          此情況在 Solaris 易發生, 尤其是有上游 news server 主動餵信時, innbbsd
          掛掉的情況更常發生.

          在 HEADER[PATH_H] 為 null path 時, 只要去 access HEADER[MID_H] 的話,
          innbbsd 就死了 :(   */

    fprintf(out, "437 empty article\r\n"); /* patch by tsaoyc. */
#if 0
    fprintf(out, "437 No Path in \"ihave %s\" header\r\n", HEADER[MID_H]);
    /* fflush(out); */
    verboselog("Put: 437 No Path in \"ihave %s\" header\n", HEADER[MID_H]);
#endif
    client->ihavefail++;
  }
  else
  {
    fprintf(out, "437 empty article\r\n");
#if 0
    fprintf(out, "437 No colon-space in \"ihave %s\" header\r\n", HEADER[MID_H]);
    /* fflush(out); */
    verboselog("Ihave Put: 437 No colon-space in \"ihave %s\" header\n", HEADER[MID_H]);
#endif
    client->ihavefail++;
  }

  fflush(out);

  return 0;
}

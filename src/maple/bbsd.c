/*-------------------------------------------------------*/
/* bbsd.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw		 	 */
/* target : BBS daemon/main/login/top-menu routines 	 */
/* create : 95/03/29				 	 */
/* update : 96/10/10				 	 */
/*-------------------------------------------------------*/



#define	_MAIN_C_


#include "bbs.h"
#include "dns.h"


#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/telnet.h>
#include <sys/resource.h>


#define	QLEN		3
#define	PID_FILE	"run/bbs.pid"
#define	LOG_FILE	"run/bbs.log"
#undef	SERVER_USAGE


#define MAXPORTS        3
static int myports[MAXPORTS] = {23, 3456, 3001 /* , 3002, 3003 */ };


extern UCACHE *ushm;

/* Thor.990113: exports for anonymous log */
/* static */ char rusername[40];

/* static int mport; */ /* Thor.990325: ¤£»Ý­n¤F:P */
static u_long tn_addr;


#ifdef CHAT_SECURE
char passbuf[9];
#endif

#ifdef	TREAT
int treat=0;
#endif


#ifdef MODE_STAT
extern UMODELOG modelog;
extern time_t mode_lastchange;
#endif

#if defined(LINUX)
#ifndef REDHAT7
int
getloadavg(cpu_load,mode)
  float *cpu_load;
  int mode;
{
  FILE *fp;

  fp = fopen("/proc/loadavg", "r");
  if (!fp)
    cpu_load[0] = cpu_load[1] = cpu_load[2] = 0;
  else
  {
    float av[3];

    fscanf(fp, "%g %g %g", av, av + 1, av + 2);
    fclose(fp);
    cpu_load[0] = av[0];
    cpu_load[1] = av[1];
    cpu_load[2] = av[2];
  }
  return 0;
}
#endif
#endif

/* ----------------------------------------------------- */
/* Â÷¶} BBS µ{¦¡					 */
/* ----------------------------------------------------- */

void
blog(mode, msg)
  char *mode, *msg;
{
  char buf[512], data[256];
  time_t now;

  time(&now);
  if (!msg)
  {
    msg = data;
    sprintf(data, "Stay: %d (%d)", (int)(now - ap_start) / 60, currpid);
  }

  sprintf(buf, "%s %s %-13s%s\n", Etime(&now), mode, cuser.userid, msg);
  f_cat(FN_USIES, buf);
}


#ifdef MODE_STAT
void
log_modes()
{
  time(&modelog.logtime);
  rec_add(FN_MODE_CUR, &modelog, sizeof(UMODELOG));
}
#endif

#ifdef	TRANUFO
typedef struct
{
  usint old;
  usint new;
}       TABLE;

TABLE table[] = {
//  {UFO_COLOR,UFO2_COLOR},
//  {UFO_MOVIE,UFO2_MOVIE},
//  {UFO_BRDNEW,UFO2_BRDNEW},
//  {UFO_BNOTE,UFO2_BNOTE},
//  {UFO_VEDIT,UFO2_VEDIT},
//  {UFO_PAL,UFO2_PAL},
//  {UFO_MOTD,UFO2_MOTD},
//  {UFO_MIME,UFO2_MIME},
//  {UFO_SIGN,UFO2_SIGN},
//  {UFO_SHOWUSER,UFO2_SHOWUSER},
//  {UFO_REALNAME,UFO2_REALNAME},
//  {UFO_SHIP,UFO2_SHIP},
//  {UFO_NWLOG,UFO2_NWLOG},
//  {UFO_NTLOG,UFO2_NTLOG},
  {UFO_ACL,UFO2_ACL},
  {0,0}
};

#endif


void
u_exit(mode)
  char *mode;
{
  int fd, delta;
  ACCT tuser;
  char fpath[80];

  utmp_free();			/* ÄÀ©ñ UTMP shm */
  blog(mode, NULL);

  if (cuser.userlevel)
  {
    ve_backup();		/* ½s¿è¾¹¦Û°Ê³Æ¥÷ */
    brh_save();			/* Àx¦s¾\Åª°O¿ýÀÉ */
  }

  /* Thor.980405: Â÷¯¸®É¥²©w§R°£¼ö°T */
#ifndef LOG_BMW        /* lkchu.981201: ¥i¦b config.h ³]©w */

  usr_fpath(fpath, cuser.userid, FN_BMW);
  unlink(fpath);

#endif

#ifdef MODE_STAT
  log_modes();
#endif

  usr_fpath(fpath, cuser.userid, FN_ACCT);
  fd = open(fpath, O_RDWR);
  if (fd >= 0)
  {
    if (read(fd, &tuser, sizeof(ACCT)) == sizeof(ACCT))
    {
      delta = time(&cuser.lastlogin) - ap_start;
      cuser.staytime += delta;
      /* lkchu.981201: ¥Î delta, ¨C¦¸¤W¯¸³£­n¶W¹L¤T¤ÀÄÁ¤~ºâ */
      if (delta > 3 * 60)
	cuser.numlogins++;
#ifdef HAVE_SONG	
      cuser.request = tuser.request;
      if(cuser.request>500)
        cuser.request = 500;
      else if (cuser.request<=0)
        cuser.request = 0;
#endif
      cuser.userlevel = tuser.userlevel;
      cuser.tvalid = tuser.tvalid;
      cuser.vtime = tuser.vtime;
      cuser.deny = tuser.deny;
      if(tuser.numlogins > cuser.numlogins)
        cuser.numlogins = tuser.numlogins;
      if(tuser.numposts > cuser.numposts)
        cuser.numposts = tuser.numposts;
      if(tuser.staytime > cuser.staytime)
        cuser.staytime = tuser.staytime;
      strcpy(cuser.justify, tuser.justify);
      strcpy(cuser.vmail, tuser.vmail);
#ifdef	TRANUFO
      {
        TABLE *ptr;
        cuser.ufo2 = 0;
        for(ptr = table;ptr->old;ptr++)
        {
          if(cuser.ufo & ptr->old)
            cuser.ufo2 |= ptr->new;
          else
            cuser.ufo2 &= ~ptr->new;
        }
      }
#endif
#ifdef	HAVE_CLASSTABLEALERT
      if(!utmp_find(cuser.userno))
        classtable_free();
//	  vmsg("test");
#endif
      lseek(fd, (off_t) 0, SEEK_SET);
      write(fd, &cuser, sizeof(ACCT));
    }
    close(fd);
  }
}


void
abort_bbs()
{
  if (bbstate)
    u_exit("AXXED");
  
  exit(0);
}


static void
login_abort(msg)
  char *msg;
{
  outs(msg);
  refresh();
  blog("LOGIN", currtitle);
  exit(0);
}


/* Thor.980903: lkchu patch: ¤£¨Ï¥Î¤W¯¸¥Ó½Ð±b¸¹®É, «h¤U¦C function§¡¤£¥Î */

#ifdef LOGINASNEW

/* ----------------------------------------------------- */
/* ÀË¬d user µù¥U±¡ªp					 */
/* ----------------------------------------------------- */


static int
belong(flist, key)
  char *flist;
  char *key;
{
  int fd, rc;

  rc = 0;
  fd = open(flist, O_RDONLY);
  if (fd >= 0)
  {
    mgets(-1);

    while (flist = mgets(fd))
    {
      str_lower(flist, flist);
      if (str_str(key, flist))
      {
	rc = 1;
	break;
      }
    }

    close(fd);
  }
  return rc;
}


static int
is_badid(userid)
  char *userid;
{
  int ch;
  char *str;

  if (strlen(userid) < 2)
    return 1;

  if (!is_alpha(*userid))
    return 1;

  if (!str_cmp(userid, "new"))
    return 1;

  str = userid;
  while (ch = *(++str))
  {
    if (!is_alnum(ch))
      return 1;
  }
  return (belong(FN_ETC_BADID, userid));
}


static int
uniq_userno(fd)
  int fd;
{
  char buf[4096];
  int userno, size;
  SCHEMA *sp;			/* record length 16 ¥i¾ã°£ 4096 */

  userno = 1;

  while ((size = read(fd, buf, sizeof(buf))) > 0)
  {
    sp = (SCHEMA *) buf;
    do
    {
      if (sp->userid[0] == '\0')
      {
	lseek(fd, -size, SEEK_CUR);
	return userno;
      }
      userno++;
      size -= sizeof(SCHEMA);
      sp++;
    } while (size);
  }

  return userno;
}


static void
acct_apply()
{
  SCHEMA slot;
  char buf[80];
  char *userid;
  int try, fd;

  film_out(FILM_APPLY, 0);

  memset(&cuser, 0, sizeof(cuser));
  userid = cuser.userid;
  try = 0;
  for (;;)
  {
    if (!vget(17, 0, msg_uid, userid, IDLEN + 1, DOECHO))
      login_abort("\n¦A¨£ ...");

    if (is_badid(userid))
    {
      vmsg("µLªk±µ¨ü³o­Ó¥N¸¹¡A½Ð¨Ï¥Î­^¤å¦r¥À¡A¨Ã¥B¤£­n¥]§tªÅ®æ");
    }
    else
    {
      usr_fpath(buf, userid, NULL);
      if (dashd(buf))
	vmsg("¦¹¥N¸¹¤w¸g¦³¤H¨Ï¥Î");
      else
	break;
    }

    if (++try >= 10)
      login_abort("\n±z¹Á¸Õ¿ù»~ªº¿é¤J¤Ó¦h¡A½Ð¤U¦¸¦A¨Ó§a");
  }

  for (;;)
  {
    vget(18, 0, "½Ð³]©w±K½X¡G", buf, 9, NOECHO);
    if ((strlen(buf) < 3) || !strcmp(buf, userid))
    {
      vmsg("±K½X¤ÓÂ²³æ¡A©ö¾D¤J«I¡A¦Ü¤Ö­n 4 ­Ó¦r¡A½Ð­«·s¿é¤J");
      continue;
    }

    vget(19, 0, "½ÐÀË¬d±K½X¡G", buf + 10, 9, NOECHO);
    if (!strcmp(buf, buf + 10))
      break;

    vmsg("±K½X¿é¤J¿ù»~, ½Ð­«·s¿é¤J±K½X");
  }

  str_ncpy(cuser.passwd, genpasswd(buf), PASSLEN);

  do
  {
    vget(19, 0, "¼Ê    ºÙ¡G", cuser.username, sizeof(cuser.username), DOECHO);
  } while (strlen(cuser.username) < 2);

  do
  {
    vget(20, 0, "¯u¹ê©m¦W¡G", cuser.realname, sizeof(cuser.realname), DOECHO);
  } while (strlen(cuser.realname) < 4);

  do
  {
    vget(21, 0, "©~¦í¦a§}¡G", cuser.address, sizeof(cuser.address), DOECHO);
  } while (strlen(cuser.address) < 12);

 
  cuser.userlevel = PERM_DEFAULT;
  cuser.ufo2 = UFO2_COLOR | UFO2_MOVIE | UFO2_BNOTE;
  /* Thor.980805: µù¸Ñ, ¹w³]ºX¼Ðufo */
  cuser.numlogins = 1;

  /* dispatch unique userno */

  cuser.firstlogin = cuser.lastlogin = cuser.tcheck = slot.uptime = time(0)/*ap_start*/;
  memcpy(slot.userid, userid, IDLEN);

  fd = open(FN_SCHEMA, O_RDWR | O_CREAT, 0600);
  {

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: ¥Î fcntl ¨ú¥Nflock, POSIX¼Ð·Ç¥Îªk */
    f_exlock(fd);

    cuser.userno = try = uniq_userno(fd);
    write(fd, &slot, sizeof(slot));
    /* flock(fd, LOCK_UN); */
    /* Thor.981205: ¥Î fcntl ¨ú¥Nflock, POSIX¼Ð·Ç¥Îªk */
    f_unlock(fd);
  }
  close(fd);

  /* create directory */

  usr_fpath(buf, userid, NULL);
  mkdir(buf, 0755);
  strcat(buf, "/@");
  mkdir(buf, 0755);

  usr_fpath(buf, userid, FN_ACCT);
  fd = open(buf, O_WRONLY | O_CREAT, 0600);
  write(fd, &cuser, sizeof(cuser));
  close(fd);
  /* Thor.990416: ª`·N: «ç»ò·|¦³ .ACCTªø«×¬O0ªº, ¦Ó¥B¥u¦³ @¥Ø¿ý, «ùÄòÆ[¹î¤¤ */

  sprintf(buf, "%d", try);
  blog("APPLY", buf);
}

#endif /* LOGINASNEW */


/* ----------------------------------------------------- */
/* bad login						 */
/* ----------------------------------------------------- */
static void
logattempt(type)
  int type;			/* '-' login failure   ' ' success */
{
  char buf[128], fpath[80];

//  time_t now = time(0);
  struct tm *p;
  p = localtime(&ap_start);
  sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d %cBBS\t%s\n",
    p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, type, currtitle);

#if 0
  sprintf(buf, "%c%-12s[%s] %s\n", type, cuser.userid,
    Etime(&ap_start), currtitle);
  f_cat(FN_LOGIN_LOG, buf);
#endif

//  str_stamp(fpath, &ap_start);
//  sprintf(buf, "%s %cBBS\t%s\n", fpath, type, currtitle); 
  /* Thor.990415: currtitle¤w¤º§tip */
  /* sprintf(buf, "%s %cBBS\t%s ip:%08x\n", fpath, type, currtitle,tn_addr); */
  /* Thor.980803: °lÂÜ ip address */
  usr_fpath(fpath, cuser.userid, FN_LOG);
  f_cat(fpath, buf);

  if (type != ' ')
  {
    usr_fpath(fpath, cuser.userid, FN_LOGINS_BAD);
    sprintf(buf, "[%s] BBS %s\n", Ctime(&ap_start),
      (type == '*' ? currtitle : fromhost));
    f_cat(fpath, buf);
  }
}


/* ----------------------------------------------------- */
/* µn¿ý BBS µ{¦¡					 */
/* ----------------------------------------------------- */


extern void talk_rqst();
extern void bmw_rqst();


static void
utmp_setup(mode)
  int mode;
{
  UTMP utmp;
  char *guestname[GUESTNAME]={GUEST_NAMES};
  
  cutmp = NULL; /* Thor.980805: pal_cache¤¤·| check cutmp  */
  /*pal_cache();*/  /* by visor */
  /* cutmp = NULL; */
  memset(&utmp, 0, sizeof(utmp));
  utmp.pid = currpid;
  utmp.userno = cuser.userno;
  utmp.mode = bbsmode = mode;
  utmp.in_addr = tn_addr;
  utmp.ufo = cuser.ufo;
  utmp.userlevel = cuser.userlevel;
#ifdef	HAVE_SHOWNUMMSG
  utmp.num_msg = 0;
#endif 
#ifdef HAVE_BOARD_PAL
  utmp.board_pal = -1;
#endif

  strcpy(utmp.userid, cuser.userid);
  srand(time(0));
  strcpy(utmp.username, ((!str_cmp(cuser.userid, "guest")||!HAS_PERM(PERM_VALID)||HAS_PERM(PERM_DENYNICK))&&!HAS_PERM(PERM_SYSOP)) ? guestname[rand()%GUESTNAME] : cuser.username);
  strcpy(utmp.realname, cuser.realname);
  /* str_ncpy(utmp.from, fromhost, sizeof(utmp.from) - 1); */
  str_ncpy(utmp.from, fromhost, sizeof(utmp.from));
  /* Thor.980921: str_ncpy §t 0 */

  /* Thor: §i¶DUser¤w¸gº¡¤F©ñ¤£¤U... */

  if (!utmp_new(&utmp))
  {
    login_abort("\n±z­è­è¿ïªº¦ì¤l¤w¸g³Q¤H±¶¨¬¥ýµn¤F¡A½Ð¤U¦¸¦A¨Ó§a");
  }
  pal_cache();
#ifdef	HAVE_BANMSG
  banmsg_cache();
#endif  
}


/* ----------------------------------------------------- */
/* user login						 */
/* ----------------------------------------------------- */


static void
tn_login()
{
  int fd, attempts;
  usint level, ufo;
  time_t start,check_deny;
  char fpath[80], uid[IDLEN + 1];

#ifndef CHAT_SECURE
  char passbuf[9];
#endif

  /* ­É currtitle ¤@¥Î */

  /* sprintf(currtitle, "%s@%s", rusername, fromhost); */
  /* Thor.990415: ¬ö¿ýip, ©È¥¿¬d¤£¨ì */
  sprintf(currtitle, "%s@%s ip:%08x", rusername, fromhost,(int) tn_addr);
  
  
/* by visor */
#if 0
  move(20,0);
  outs("Ãö¯¸¤¤½Ð¨£½Ì~~~~");
  vkey();
  sleep(10);
  exit(0);
#endif    
/* by visor */ 


  move(20, 0);
  prints("\033[m°ÑÆ[¥Î±b¸¹¡Gguest¡A¥Ó½Ð·s±b¸¹¡Gnew¡C¥Ø«e½u¤W¤H¼Æ [%d/%d] ¤H¡C",ushm->count,MAXACTIVE);

  /*move(b_lines, 0);
  outs("¡° µLªk³s½u®É¡A½Ð§Q¥Î port 3456 ¤W¯¸");*/

  attempts = 0;
  for (;;)
  {
    if (++attempts > LOGINATTEMPTS)
    {
      film_out(FILM_TRYOUT, 0);
      login_abort("\n¦A¨£ ....");
    }

    vget(21, 0, msg_uid, uid, IDLEN + 1, DOECHO);

    if (str_cmp(uid, "new") == 0)
    {

#ifdef LOGINASNEW
      acct_apply(); /* Thor.980917: µù¸Ñ: setup cuser ok */
      level = cuser.userlevel;
      ufo = cuser.ufo;
      /* cuser.userlevel = PERM_DEFAULT; */
      /* cuser.ufo = UFO_COLOR | UFO_MOVIE | UFO_BNOTE; */
      break;
#else
      outs("\n¥»¨t²Î¥Ø«e¼È°±½u¤Wµù¥U, ½Ð¥Î guest ¶i¤J");
      continue;
#endif
    }
    else if (!*uid || (acct_load(&cuser, uid) < 0))
    {
      vmsg(err_uid);
    }
    else if (str_cmp(uid, STR_GUEST))
    {
      if (!vget(22, 0, MSG_PASSWD, passbuf, 9, NOECHO))
      {
	continue;	/* µo²{ userid ¿é¤J¿ù»~¡A¦b¿é¤J passwd ®Éª½±µ¸õ¹L */
      }

      passbuf[8] = '\0';

      if (chkpasswd(cuser.passwd, passbuf))
      {
	logattempt('-');
	vmsg(ERR_PASSWD);
      }
      else
      {
	/* SYSOP gets all permission bits */

	if (!str_cmp(cuser.userid, str_sysop))
	  cuser.userlevel = ~0 ^ (PERM_DENYPOST | PERM_DENYTALK |
	    PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYSTOP | PERM_DENYNICK |
	    PERM_DENYLOGIN | PERM_PURGE);
	/* else */  /* Thor.980521: everyone should have level */

        level = cuser.userlevel;

	ufo = cuser.ufo & ~(HAS_PERM(PERM_LOGINCLOAK) ?
        /* lkchu.981201: ¨C¦¸¤W¯¸³£­n 'c' »á³Â·Ð :p */
          (UFO_BIFF | UFO_BIFFN | UFO_SOCKET | UFO_NET) :
          (UFO_BIFF | UFO_BIFFN | UFO_SOCKET | UFO_NET | UFO_CLOAK));
                            

	if ((level & PERM_ADMIN) && (cuser.ufo2 & UFO2_ACL))
	{
	  char buf[40]; /* check ip */
	  uschar *addr = (uschar*) &tn_addr;
	  sprintf(buf, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
	  usr_fpath(fpath, cuser.userid, "acl");
	  str_lower(rusername, rusername);      /* lkchu.981201: ´«¤p¼g */
	  str_lower(fromhost, fromhost);
	  if (acl_has(fpath, rusername, fromhost) == 0
	      || acl_has(fpath, rusername, buf) == 0)
	  {  /* Thor.980728: ª`·N aclÀÉ, ©M rusername,fromhost ­n¥þ³¡¤p¼g */
	    logattempt('*');
	    login_abort("\n§Aªº¤W¯¸¦aÂI¤£¤Ó¹ï«l¡A½Ð®Ö¹ï [¤W¯¸¦aÂI³]©wÀÉ]");
	  }
	}

	logattempt(' ');

	/* check for multi-session */

	if (!(level & PERM_SYSOP))
	{
	  UTMP *ui;
	  pid_t pid;

	  if (level & (PERM_DENYLOGIN | PERM_PURGE))
	    login_abort("\n³o­Ó±b¸¹¼È°±ªA°È¡A¸Ô±¡½Ð¦V¯¸ªø¬¢¸ß¡C");


	  if (!(ui = (UTMP *) utmp_find(cuser.userno)))
	    break;		/* user isn't logged in */


	  pid = ui->pid;
	  if (!pid || (kill(pid, 0) == -1))
	  {
	    memset(ui, 0, sizeof(UTMP));
	    break;		/* stale entry in utmp file */
	  }

	  if (vans("±z·Q§R°£¨ä¥L­«½Æªº login (Y/N)¶Ü¡H[N] ") == 'y')
	  {
	    kill(pid, SIGTERM);
	    blog("MULTI", cuser.username);
	    break;
	  }

	  if (utmp_count(cuser.userno, 0) > 2)
	    login_abort("\n¦A¨£ .....");
	}
	break;
      }
    }
    else
    {				/* guest */
      logattempt(' ');
      cuser.userlevel = level = 0; 
      /* Thor.981207: ©È¤H¶Ãª±, ±j¨î¼g¦^cuser.userlevel */
      ufo = UFO_PAGER | UFO_QUIET | UFO_MESSAGE | UFO_PAGER1 |UFO_HIDEDN;
      cuser.ufo = ufo;
      cuser.ufo2 = UFO2_COLOR | UFO2_BNOTE | UFO2_MOVIE;
      if (utmp_count(cuser.userno, 0) > MAXGUEST)
        login_abort("\n¦A¨£ ......");      
      break; 
    }
  }

  /* --------------------------------------------------- */
  /* µn¿ý¨t²Î						 */
  /* --------------------------------------------------- */

  start = ap_start;


//  setproctitle("%s@%s",cuser.userid,fromhost);

  sprintf(fpath, "%s (%d)", currtitle, currpid);


  blog("ENTER", fpath);
  if (rusername[0])
    strcpy(cuser.ident, currtitle);

  /* --------------------------------------------------- */
  /* ªì©l¤Æ utmp¡Bflag¡Bmode				 */
  /* --------------------------------------------------- */

  cuser.ufo = ufo & (~UFO_WEB);
  ufo = cuser.ufo;
  utmp_setup(M_LOGIN); /* Thor.980917: µù¸Ñ: cutmp, cutmp-> setup ok */
  bbstate = STAT_STARTED;
  bbsothermode = 0;
  mbox_main();

  film_out(FILM_WELCOME, 0);

        
#ifdef MODE_STAT
  memset(&modelog, 0, sizeof(UMODELOG));
  mode_lastchange = ap_start;
#endif

  if (level)			/* not guest */
  {
    /*
     * Thor: ¤º³¡§ïÅÜ¹L userlevel, ©Ò¥H substitute_record ¥²¶·­n¥i¼g¤J
     * userlevel. ¥u¦³¦b userlogin ®É, ¤£¥i¥H§ï userlevel, ¨ä¥L®É­Ô§¡¥i¥H.
     */
    /* Thor.990318: ²{¦b¥u¦³¦b"¨Ï¥ÎªÌ¸ê®Æ½]®Ö¤¤"®É¤£¯à§ïuserlevel, ¨ä¥L³£¥i */

    /* ------------------------------------------------- */
    /* ®Ö¹ï user level					 */
    /* ------------------------------------------------- */

#ifdef JUSTIFY_PERIODICAL
    /* if (level & PERM_VALID) */
    if ((level & PERM_VALID) && !(level & (PERM_SYSOP|PERM_XEMPT)))
    {  /* Thor.980819: ¯¸ªø¤£§@©w´Á¨­¥÷»{ÃÒ, ex. SysOp  */
      if (cuser.tvalid + VALID_PERIOD < start)
      {
	level ^= PERM_VALID;
	find_same_email(cuser.email,3);
      }
    }
#endif


    time(&check_deny);

    level |= (PERM_POST | PERM_PAGE | PERM_CHAT);


    if (!(level & PERM_SYSOP))
    {
#ifdef NEWUSER_LIMIT
      /* Thor.980825: lkchu patch: ¬JµM¦³ NEWUSER_LIMIT, ÁÙ¬O¥[¤@¤U¦n¤F, 
                                   §_«h¤@ª½¦^µª FAQ ®¼²Öªº. :) */
      if (start - cuser.firstlogin < 3 * 86400)
      {
	/* if (!(level & PERM_VALID)) */
	level &= ~PERM_POST;	/* §Y¨Ï¤w¸g³q¹L»{ÃÒ¡AÁÙ¬O­n¨£²ß¤T¤Ñ */
      }
#endif

      /* Thor.980629: ¥¼¸g¨­¤À»{ÃÒ, ¸T¤î chat/talk/write */
      if(!(level & PERM_VALID))
      {
        level &= ~(PERM_CHAT | PERM_PAGE | PERM_POST);
      }
      
      if(level & (PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYNICK))
      {
        if(cuser.deny < check_deny && !(level & PERM_DENYSTOP))
        {
          usr_fpath(fpath, cuser.userid, FN_STOPPERM_LOG);
          unlink(fpath);
          level &= ~(PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYNICK);
        }
      }
      else if(!(level & PERM_DENYSTOP) && cuser.deny > check_deny) cuser.deny = check_deny;
      
      if (!(level & PERM_CLOAK))
        ufo &= ~(UFO_CLOAK|UFO_HIDEDN);

      if (level & PERM_DENYPOST)
	level &= ~PERM_POST;

      if (level & PERM_DENYTALK)
      {
        ufo |= UFO_MESSAGE|UFO_PAGER1;
	level &= ~PERM_PAGE;
      }

      if (level & PERM_DENYCHAT)
	level &= ~PERM_CHAT;

/*
      if ((cuser.numemail >> 4) > (cuser.numlogins + cuser.numposts))
	level |= PERM_DENYMAIL;*/
    }

    supervisor = check_admin(cuser.userid);

    /* ------------------------------------------------- */
    /* ¦n¤Í¦W³æ¦P¨B¡B²M²z¹L´Á«H¥ó			 */
    /* ------------------------------------------------- */

    if (start > cuser.tcheck + CHECK_PERIOD)
    {
#ifdef	HAVE_MAILGEM
      if(cuser.userlevel & PERM_MBOX)
      {
        int (*p)();
        char fpath[128];
        usr_fpath(fpath,cuser.userid,"gem");
      
        p = DL_get("bin/mailgem.so:gcheck");
        if(p)
          (*p)(0,fpath);
      }
#endif

      pal_sync(NULL); /* Thor.990318: ¦b³o¦^»{ÃÒ«Hªº¾÷²v¤£¤j:P */
      aloha_sync();
#ifdef	HAVE_BANMSG
      banmsg_sync(NULL);	/* ©Ú¦¬°T®§ */
#endif      
      ufo |= m_quota(); /* Thor.980804: µù¸Ñ, ¸ê®Æ¾ã²z½]®Ö¦³¥]§t BIFF check */
      cuser.ufo = ufo;
      cuser.tcheck = start;
    }
    else
    { 
      if(m_query(cuser.userid)>0)
        ufo |= UFO_BIFF;
    }
    if(check_personal_note(1,cuser.userid))
      ufo |= UFO_BIFFN;

    cutmp->ufo = cuser.ufo = ufo; /* Thor.980805: ¸Ñ¨M ufo ¦P¨B°ÝÃD */

    /* ------------------------------------------------- */
    /* ±N .ACCT ¼g¦^					 */
    /* ------------------------------------------------- */

#if 1
    /* Thor.990318: ¬°¨¾¤î¦³¤j¾÷²v ¦³¤H¦bwelcomeµe­±¦^»{ÃÒ«H, ¬G²¾¦Ü¦¹ */
    move(b_lines - 2, 0);
    prints("     Åwªï±z²Ä [1;33m%d[m «×«ô³X¥»¯¸¡A\
¤W¦¸±z¨Ó¦Û [1;33m%s[m¡A\n§Ú°O±o¨º¤Ñ¬O [1;33m%s[m¡C",
      cuser.numlogins, cuser.lasthost, Ctime(&cuser.lastlogin));
    /* Thor.990321: ±Nvmsg²¾¦Ü«á¤è, ¨¾¤î¦³¤H¦b¦¹®É¦^»{ÃÒ«H */
#endif

    cuser.lastlogin = start;
    cuser.userlevel = level;
    str_ncpy(cuser.lasthost, fromhost, sizeof(cuser.lasthost));
    usr_fpath(fpath, cuser.userid, FN_ACCT);
    fd = open(fpath, O_WRONLY);
    write(fd, &cuser, sizeof(ACCT));
    close(fd);
#if 1
    vmsg(NULL);
#endif

#if 1
    usr_fpath(fpath, cuser.userid, FN_LOGINS_BAD);
    /* Thor.990204: ¬°¦Ò¼{more ¶Ç¦^­È */
    if (more(fpath, (char *)-1) >= 0 && vans("±z­n§R°£¤W­z°O¿ý¶Ü(Y/N)?[Y]") != 'n')
      unlink(fpath);
#endif

    usr_fpath(fpath, cuser.userid, FN_STOPPERM_LOG);
    if(more(fpath, (char *)-1)>= 0)
      vmsg(NULL);


#if 1
    if (!(level & PERM_VALID))
    {
      bell();
      more(FN_ETC_NOTIFY, NULL);
    }
    /* ¦³®Ä®É¶¡¹O´Á 10 ¤Ñ«e´£¥XÄµ§i */
#ifdef JUSTIFY_PERIODICAL
    else if(!(level & (PERM_ADMIN|PERM_XEMPT)))
    {
      if (cuser.tvalid + VALID_PERIOD - 10 * 86400 < start)
      {
        bell();
        more(FN_ETC_REREG, NULL);
      }
    }
#endif
#endif

#ifdef NEWUSER_LIMIT
      /* Thor.980825: lkchu patch: ¬JµM¦³ NEWUSER_LIMIT, ÁÙ¬O¥[¤@¤U¦n¤F, 
                                   §_«h¤@ª½¦^µª FAQ ®¼²Öªº. :) */
      if (start - cuser.firstlogin < 3 * 86400)
      {
	/* if (!(level & PERM_VALID)) */
	/* §Y¨Ï¤w¸g³q¹L»{ÃÒ¡AÁÙ¬O­n¨£²ß¤T¤Ñ */
	more(FN_ETC_NEWUSER, NULL);
      }
#endif

#if 0
    if (ufo & UFO_MQUOTA)
    {
      bell();
      more(FN_ETC_MQUOTA, NULL);
    }
#endif

    ve_recover();

#ifdef LOGIN_NOTIFY
    if (!(ufo & UFO_CLOAK))     /* lkchu.981201: ¦Û¤vÁô¨­´N¤£¥Î notify */
    {
      void loginNotify();
      loginNotify();
    }
    else
    {
      /* lkchu.981201: ²M±¼ benz, ¤£³qª¾¤F */
      usr_fpath(fpath, cuser.userid, FN_BENZ);
      unlink(fpath);
    }
#endif

#ifdef HAVE_ALOHA 
    if (!(ufo & UFO_CLOAK))     /* lkchu.981201: ¦Û¤vÁô¨­´N¤£¥Î notify */
    {
      void aloha();
      aloha();
    }

#endif
  }
  else
  {
    vmsg(NULL);			/* Thor: for guest look welcome */
  }




  /* lkchu.990510: ­«­n¤½§i©ñ¨ì announce, ¤£¸ò welcome À½ :p */
  more(FN_ETC_ANNOUNCE, NULL);
#if 0
  bell();
  sleep(1);
  bell();
#endif

  /* Thor.980917: µù¸Ñ: ¨ì¦¹¬°¤îÀ³¸Ó³]©w¦n cuser. level? ufo? rusername bbstate cutmp cutmp-> */

  if (!(cuser.ufo2 & UFO2_MOTD))
  {
    more("gem/@/@-day", NULL);	/* ¤µ¤é¼öªù¸ÜÃD */
    clear();
    pad_view();
  }
#ifdef	HAVE_CLASSTABLEALERT
  classtable_free();
  classtable_main();
#endif
  showansi = cuser.ufo2 & UFO2_COLOR;
}


/* ----------------------------------------------------- */
/* trap signals						 */
/* ----------------------------------------------------- */


static void
tn_signals()
{
  struct sigaction act;

  /* act.sa_mask = 0; */ /* Thor.981105: ¼Ð·Ç¥Îªk */
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  act.sa_handler = (void *) abort_bbs;
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGXCPU, &act, NULL);
#ifdef SIGSYS
  /* Thor.981221: easy for porting */
  sigaction(SIGSYS, &act, NULL);/* bad argument to system call */
#endif

  act.sa_handler = (void *) talk_rqst;
  sigaction(SIGUSR1, &act, NULL);

  act.sa_handler = (void *) bmw_rqst;
  sigaction(SIGUSR2, &act, NULL);

  /* sigblock(sigmask(SIGPIPE)); */
  /* Thor.981206: lkchu patch: ²Î¤@ POSIX ¼Ð·Ç¥Îªk  */
  /* ¦b¦¹­É¥Î sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


static inline void
tn_main()
{
  double load[3];
  char buf[128], buf2[40];
  uschar *addr = (uschar*) &tn_addr;
  

#if 0
/*  ·M¤H¸`±M¥Î*/
  time_t test;
  test = time(0);
  if(test >= 1017590400 && test <= 1017676800)
  {
    clear();
    more("etc/@/repair",(char *)-1);
    bell();
    vkey();
//    more("etc/@/virus",(char *)-1);
//    bell();
//    vkey();
    more("etc/@/disconnect",(char *)-1);    
    bell();
    vkey();    
    bell();
    vkey();    
    bell();
    vkey();
    more("etc/@/systembusy",(char *)-1);
    bell();
    vkey();
    bell();
    vkey();
    bell();
    vkey();
    more("etc/@/sorry",(char *)-1);    
    bell();
    vkey();      
  } 
#endif

  sprintf(buf2, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);  
  str_lower(buf,fromhost);
  if(acl_has(FN_ETC_BANIP_ACL, "", buf) > 0
     || acl_has(FN_ETC_BANIP_ACL, "", buf2) > 0)
    exit(0);
  
  /* ±N login ªº³¡¤À²MªÅ */
  add_io(0, 0); 
  while(igetch() != I_TIMEOUT); 
  add_io(0, 60);
  
  
  clear();

  getloadavg(load,3);
  prints( MYHOSTNAME " ¡ó " BOARDNAME " ¡ó " BBSIP " " BBSNAME "\n\
Åwªï¥úÁ{¡i\033[1;33;46m %s \033[m¡j¡C¨t²Î­t¸ü¡G%.2f %.2f %.2f [%s]",
    str_site, load[0],load[1],load[2],load[0]>10?"­t¸ü¹L°ª":load[0]>5?"­t¸ü°¾°ª":"­t¸ü¥¿±`");

  film_out(FILM_INCOME , 2);
  
  total_num = ushm->count+1;
  currpid = getpid();
  cutmp = NULL;


  tn_signals();  /* Thor.980806: ©ñ©ó tn_login«e, ¥H«K call in¤£·|³Q½ð */
  tn_login();

  /*tn_signals(); */

#ifdef	HAVE_FAVORITE  
  if(HAS_PERM(PERM_VALID))
    favorite_main();
#endif
  board_main();  
  gem_main();
  talk_main();

#ifdef	HAVE_FORCE_BOARD
  force_board();  /* CdChen.990417: ±j¨î¾\Åª¤½§iªO */
#endif
  count_update();
  time(&ap_start);  

#ifdef	HAVE_CHK_MAILSIZE
 if(!HAS_PERM(PERM_DENYMAIL) && HAS_PERM(PERM_BASIC))
 {
   if(mail_stat(CHK_MAIL_VALID))
   {
     chk_mailstat = 1;
     vmsg("±zªº«H½c¤w¶W¥X®e¶q¡AµLªk¨Ï¥Î¥»¥\\¯à¡A½Ð²M²z±zªº«H½c¡I");
     xover(XZ_MBOX);
   }
 }  
#endif

  menu();

  abort_bbs();			/* to make sure it will terminate */
}


/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol	 */
/* ----------------------------------------------------- */


static void
telnet_init()
{
  static char svr[] = {
    IAC, DO, TELOPT_TTYPE,
    IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
    IAC, WILL, TELOPT_ECHO,
    IAC, WILL, TELOPT_SGA
  };

  int n, len;
  char *cmd;
  int rset;
  struct timeval to;
  char buf[64];

  /* --------------------------------------------------- */
  /* init telnet protocol				 */
  /* --------------------------------------------------- */

#if 0
  to.tv_sec = 1;
  to.tv_usec = 1;
#endif
  cmd = svr;

  for (n = 0; n < 4; n++)
  {
    len = (n == 1 ? 6 : 3);
    send(0, cmd, len, 0);
    cmd += len;

    rset = 1;
    /* Thor.981221: for future reservation bug */
    to.tv_sec = 1;
    to.tv_usec = 1;
    if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
      recv(0, buf, sizeof(buf), 0);
  }
}


/* ----------------------------------------------------- */
/* stand-alone daemon					 */
/* ----------------------------------------------------- */


static void
start_daemon(port)
  int port; /* Thor.981206: ¨ú 0 ¥Nªí *¨S¦³°Ñ¼Æ* , -1 ¥Nªí -i (inetd) */
{
  int n;
  struct linger ld;
  struct sockaddr_in sin;
  struct rlimit rl;
  char buf[80], data[80];
  time_t val;

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time(&val);
  strftime(buf, 80, "%d/%b/%Y %H:%M:%S", localtime(&val));

  /* --------------------------------------------------- */
  /* adjust resource : 16 mega is enough		 */
  /* --------------------------------------------------- */

  rl.rlim_cur = rl.rlim_max = 16 * 1024 * 1024;
  /* setrlimit(RLIMIT_FSIZE, &rl); */
  setrlimit(RLIMIT_DATA, &rl);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS
  /* Thor.981206: port for solaris 2.6 */
#endif

  setrlimit(RLIMIT_RSS, &rl);

  rl.rlim_cur = rl.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rl);

  rl.rlim_cur = rl.rlim_max = 60 * 20;
  setrlimit(RLIMIT_CPU, &rl);

  /* --------------------------------------------------- */
  /* speed-hacking DNS resolve				 */
  /* --------------------------------------------------- */

  dns_init();

  /* --------------------------------------------------- */
  /* change directory to bbshome       			 */
  /* --------------------------------------------------- */

  chdir(BBSHOME);
  umask(077);

  /* --------------------------------------------------- */
  /* detach daemon process				 */
  /* --------------------------------------------------- */

  close(1);
  close(2);

  if(port == -1) /* Thor.981206: inetd -i */
  {
    /* Give up root privileges: no way back from here	 */
    setgid(BBSGID);
    setuid(BBSUID);
#if 1
    n = sizeof(sin);
    if (getsockname(0, (struct sockaddr *) &sin, &n) >= 0)
      /* mport = */ port = ntohs(sin.sin_port);
#endif
    /* mport = port; */ /* Thor.990325: ¤£»Ý­n¤F:P */

    sprintf(data, "%d\t%s\t%d\tinetd -i\n", getpid(), buf, port);
    f_cat(PID_FILE, data);
    return;
  }

  close(0);

  if (fork())
    exit(0);

  setsid();

  if (fork())
    exit(0);

  /* --------------------------------------------------- */
  /* fork daemon process				 */
  /* --------------------------------------------------- */

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;

  if (port == 0) /* Thor.981206: port 0 ¥Nªí¨S¦³°Ñ¼Æ */
  {
    n = MAXPORTS - 1;
    while (n)
    {
      if (fork() == 0)
	break;

      sleep(1);
      n--;
    }
    port = myports[n];
  }

  n = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  val = 1;
  setsockopt(n, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

#if 0
  setsockopt(n, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, sizeof(val));

  setsockopt(n, IPPROTO_TCP, TCP_NODELAY, (char *) &val, sizeof(val));
#endif

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(n, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  /* mport = port; */ /* Thor.990325: ¤£»Ý­n¤F:P */
  sin.sin_port = htons(port);
  if ((bind(n, (struct sockaddr *) &sin, sizeof(sin)) < 0) || (listen(n, QLEN) < 0))
    exit(1);

  /* --------------------------------------------------- */
  /* Give up root privileges: no way back from here	 */
  /* --------------------------------------------------- */

  setgid(BBSGID);
  setuid(BBSUID);

  sprintf(data, "%d\t%s\t%d\n", getpid(), buf, port);
  f_cat(PID_FILE, data);
}


/* ----------------------------------------------------- */
/* reaper - clean up zombie children			 */
/* ----------------------------------------------------- */


static inline void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


#ifdef	SERVER_USAGE
static void
servo_usage()
{
  struct rusage ru;
  FILE *fp;

  fp = fopen("run/bbs.usage", "a");

  if (!getrusage(RUSAGE_CHILDREN, &ru))
  {
    fprintf(fp, "\n[Server Usage] %d: %d\n\n"
      "user time: %.6f\n"
      "system time: %.6f\n"
      "maximum resident set size: %lu P\n"
      "integral resident set size: %lu\n"
      "page faults not requiring physical I/O: %d\n"
      "page faults requiring physical I/O: %d\n"
      "swaps: %d\n"
      "block input operations: %d\n"
      "block output operations: %d\n"
      "messages sent: %d\n"
      "messages received: %d\n"
      "signals received: %d\n"
      "voluntary context switches: %d\n"
      "involuntary context switches: %d\n\n",

      getpid(), ap_start,
      (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
      (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
      ru.ru_maxrss,
      ru.ru_idrss,
      ru.ru_minflt,
      ru.ru_majflt,
      ru.ru_nswap,
      ru.ru_inblock,
      ru.ru_oublock,
      ru.ru_msgsnd,
      ru.ru_msgrcv,
      ru.ru_nsignals,
      ru.ru_nvcsw,
      ru.ru_nivcsw);
  }

  fclose(fp);
}
#endif


static void
main_term()
{
#ifdef	SERVER_USAGE
  servo_usage();
#endif
  exit(0);
}


static inline void
main_signals()
{
  struct sigaction act;

  /* act.sa_mask = 0; */ /* Thor.981105: ¼Ð·Ç¥Îªk */
  sigemptyset(&act.sa_mask);      
  act.sa_flags = 0;

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

  act.sa_handler = main_term;
  sigaction(SIGTERM, &act, NULL);

#ifdef	SERVER_USAGE
  act.sa_handler = servo_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* sigblock(sigmask(SIGPIPE)); */
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int csock;			/* socket for Master and Child */
  int *totaluser;
  int value;
  struct sockaddr_in sin;

  /* --------------------------------------------------- */
  /* setup standalone daemon				 */
  /* --------------------------------------------------- */

  /* Thor.990325: usage, bbsd, or bbsd -i, or bbsd 1234 */
  /* Thor.981206: ¨ú 0 ¥Nªí *¨S¦³°Ñ¼Æ*, -1 ¥Nªí -i */
  start_daemon(argc > 1 ? strcmp("-i",argv[1]) ? atoi(argv[1]) : -1 : 0);

  main_signals();
      
  /* --------------------------------------------------- */
  /* attach shared memory & semaphore			 */
  /* --------------------------------------------------- */
#ifdef	HAVE_SEM
  sem_init(); 
#endif
  ushm_init();
  bshm_init();
  fshm_init();
  fwshm_init();
  count_init();
#ifdef	HAVE_OBSERVE_LIST
  observeshm_init();
#endif

  /* --------------------------------------------------- */
  /* main loop						 */
  /* --------------------------------------------------- */

  totaluser = &ushm->count;
  /* avgload = &ushm->avgload; */

  for (;;)
  {
    value = 1;
    if (select(1, (fd_set *) & value, NULL, NULL, NULL) < 0)
      continue;

    value = sizeof(sin);
    csock = accept(0, (struct sockaddr *) &sin, &value);
    if (csock < 0)
    {
      reaper();
      continue;
    }

    time(&ap_start);
    argc = *totaluser; 
    if (argc >= MAXACTIVE - 5 /* || *avgload > THRESHOLD */ )
    {
      sprintf(currtitle,
	"¥Ø«e½u¤W¤H¼Æ [%d] ¤H¡A¨t²Î¹¡©M¡A½Ðµy«á¦A¨Ó\n", argc);
      send(csock, currtitle, strlen(currtitle), 0);
      close(csock);
      continue;
    }

    if (fork())
    {
      close(csock);
      continue;
    }

    dup2(csock, 0);
    close(csock);

#if 0
    /* Thor.990121: §K±o¤Ï¬d®ÉÅý¤H¤[µ¥ */

    telnet_init(); 

    sprintf(currtitle, "¥¿¶i¤J%s...\n", str_site);
    send(0, currtitle, strlen(currtitle), 0);
#endif

    /* ------------------------------------------------- */
    /* ident remote host / user name via RFC931		 */
    /* ------------------------------------------------- */

    /* rfc931(&sin, fromhost, rusername); */

    tn_addr = sin.sin_addr.s_addr;
    /* Thor.990325: ­×§ïdns_ident©w¸q, ¨Ó¦Û­þif³s¨º */
    /* dns_ident(mport, &sin, fromhost, rusername); */
    dns_ident(0, &sin, fromhost, rusername);

    telnet_init(); 
    tn_main();
  }
}

#include "innbbsconf.h"
#include "daemon.h"
#include "bbslib.h"

#undef DEBUG

#ifndef MAXCLIENT
#define MAXCLIENT 10
#endif

#ifndef ChannelSize
#define ChannelSize	4096
#endif

#ifndef ReadSize
#define ReadSize	4096
#endif

#ifndef DefaultINNBBSPort
#define DefaultINNBBSPort "7777"
#endif

#ifndef HIS_MAINT
#define HIS_MAINT
#define HIS_MAINT_HOUR 5
#define HIS_MAINT_MIN  30
#endif

int Maxclient = MAXCLIENT;
ClientType *Channel = NULL;
ClientType INNBBSD_STAT;

int Max_Art_Size = MAX_ART_SIZE;

int inetdstart = 0;

int Junkhistory = 0;

char *REMOTEUSERNAME, *REMOTEHOSTNAME;

static fd_set rfd, wfd, efd, orfd, owfd, oefd;


void
clearfdset(fd)
  int fd;
{
  FD_CLR(fd, &rfd);
}


static void
channelcreate(client, sock)
  ClientType *client;
  int sock;
{
  register buffer_t *in, *out;

  client->fd = sock;
  FD_SET(sock, &rfd);		/* FD_SET(sock,&wfd); */
  client->Argv.in = fdopen(sock, "r");
  client->Argv.out = fdopen(sock, "w");

  client->buffer[0] = '\0';
  client->mode = 0;
  client->midcheck = 1;
  time(&client->begin);

  in = &client->in;
  out = &client->out;
  if (in->data != NULL)
    free(in->data);
  in->data = (char *) mymalloc(ChannelSize * 4);
  in->left = ChannelSize * 4;

  if (out->data != NULL)
    free(out->data);
  out->data = (char *) mymalloc(ChannelSize);
  out->left = ChannelSize;

  in->used =
    out->used =
    client->ihavecount =
    client->ihaveduplicate =
    client->ihavefail =
    client->ihavesize =
    client->statcount =
    client->statfail = 0;
}


void
channeldestroy(client)
  ClientType *client;
{
  if (client->in.data != NULL)
  {
    free(client->in.data);
    client->in.data = NULL;
  }
  if (client->out.data != NULL)
  {
    free(client->out.data);
    client->out.data = NULL;
  }

#if !defined(PowerBBS) && !defined(DBZSERVER)
  if (client->ihavecount > 0 || client->statcount > 0)
  {
    bbslog("%s@%s T%d S%d R%d dup:%d fail:%d, stat rec:%d fail:%d\n",
      client->username, client->hostname, time(NULL) - client->begin,
      client->ihavesize, client->ihavecount, client->ihaveduplicate,
      client->ihavefail, client->statcount, client->statfail);
    INNBBSD_STAT.ihavecount += client->ihavecount;
    INNBBSD_STAT.ihaveduplicate += client->ihaveduplicate;
    INNBBSD_STAT.ihavefail += client->ihavefail;
    INNBBSD_STAT.ihavesize += client->ihavesize;
    INNBBSD_STAT.statcount += client->statcount;
    INNBBSD_STAT.statfail += client->statfail;

#ifdef	MapleBBS
  HISflush();			/* flush & sync histroy */
#endif
  }
  else
  {
    bbslog("%s@%s T%d stat rec:%d fail:%d\n",
      client->username, client->hostname, time(NULL) - client->begin,
      client->statcount, client->statfail);
  }
#endif
}


#if 0
static inline void
commandparse(client)
  ClientType *client;
{
  register argv_t *Argv = &client->Argv;
  register buffer_t *in = &client->in;
  register char *buffer = in->data;
  register int used = in->used;
  register int fd = client->fd;
  register char *ptr, *lastend;
  register int dataused;
  register int dataleft;
  int (*Main) ();

#ifdef DEBUG
  printf("%s %s buffer %s", client->username, client->hostname, buffer);
#endif

  dataleft = in->lastread;
  if (client->mode == 0 && client->in.data != NULL) /* lkchu.981201: patch by gc */
  {
    ptr = (char *) strchr(buffer + used, '\n');

    if (ptr == NULL)
    {
      in->used += dataleft;
      in->left -= dataleft;
      return;
    }
    else
    {
      dataused = ptr - (buffer + used) + 1;
      dataleft -= dataused;
      lastend = ptr + 1;
    }

  }
  else
  {
    /* data mode */

    if (used >= 5)
    {
      ptr = (char *) strstr(buffer + used - 5, "\r\n.\r\n");
    }
    else if (buffer[0] == '.' && buffer[1] == '\r' && buffer[2] == '\n')
    {
      ptr = buffer;
    }
    else
    {
      ptr = (char *) strstr(buffer + used, "\r\n.\r\n");
    }

    if (buffer[0] == '.' && buffer[1] == '\r' && buffer[2] == '\n')
      dataused = 3;
    else
      dataused = ptr - (buffer + used) + 5;
    ptr[2] = '\0';
    dataleft -= dataused;
    lastend = ptr + 5;

#ifndef	MapleBBS
    verboselog("Get: %s@%s end of data . size %d\n", client->username, client->hostname, in->used + dataused);
#endif

    client->ihavesize += used + dataused;
  }

  if (client->mode == 0)
  {
    register struct Daemoncmd *dp;

    Argv->argc = 0;
    Argv->argv = NULL;
    Argv->inputline = buffer;
    if (ptr != NULL)
      *ptr = '\0';
    verboselog("Get: %s\n", Argv->inputline);
    Argv->argc = argify(buffer + used, &Argv->argv);
    if (ptr != NULL)
      *ptr = '\n';
    Argv->dc = dp = (struct Daemoncmd *) searchcmd(Argv->argv[0]);
    if (dp)
    {

#ifdef DEBUG
      printf("enter command %s\n", Argv->argv[0]);
#endif

      if (Argv->argc < dp->argc)
      {
	fprintf(Argv->out, "%d Usage: %s\r\n", dp->errorcode, dp->usage);
	fflush(Argv->out);
	verboselog("Put: %d Usage: %s\n", dp->errorcode, dp->usage);
      }
      else if (dp->argno && Argv->argc > dp->argno)
      {
	fprintf(Argv->out, "%d Usage: %s\r\n", dp->errorcode, dp->usage);
	fflush(Argv->out);
	verboselog("Put: %d Usage: %s\n", dp->errorcode, dp->usage);
      }
      else
      {
	if (Main = Argv->dc->main)
	{
	  /* fflush(stdout); */
	  (*Main) (client);
	}
      }
    }
    else
    {
      fprintf(Argv->out, "500 Syntax error or bad command\r\n");
      fflush(Argv->out);
      verboselog("Put: 500 Syntax error or bad command\r\n");
    }
    /* deargify(&Argv->argv); */
  }
  else
  {
    if (Argv->dc)
    {

#ifdef DEBUG
      printf("enter data mode\n");
#endif

      if (Main = Argv->dc->main)
      {

#ifndef	MapleBBS
	fflush(stdout);
#endif

	(*Main) (client);
      }
    }
  }

  if (client->mode == 0)
  {
    if (dataleft > 0)
    {
      strncpy(buffer, lastend, dataleft);

#ifdef INNBBSDEBUG
      printf("***** try to copy %x %x %d bytes\n", in->data, lastend, dataleft);
#endif
    }
    else
    {
      dataleft = 0;
    }
    in->left += used - dataleft;
    in->used = dataleft;
  }
}
#endif


static int
channelreader(client)
  ClientType *client;
{
  register int len, used;
  register char *data, *head;
  register buffer_t *in = &client->in;

  len = in->left;
  used = in->used;
  data = in->data;
  if (len < ReadSize + 3)
  {

#ifdef	MapleBBS
    len += (used + ReadSize);
    len += (len >> 3);
#else
    len += (used + ReadSize + 3);
    len += len / 5;
    verboselog("channelreader realloc %d\n", len);
#endif

    in->data = data = (char *) myrealloc(data, len);
    len -= used;
    in->left = len;
  }

  head = data + used;
  len = recv(client->fd, head, ReadSize, 0);

  if (len <= 0)
    return len;

  in->lastread = len;
  head[len] = '\0';
  head[len + 1] = '\0';

  if (client->mode)		/* data mode */
  {
    char *dest;
    int cc;

    dest = head - 1;
    client->ihavesize += len;

    for (;;)
    {
      cc = *head;

      if (!cc)
      {
	used = dest - data + 1;
	in->left -= (used - in->used);
	in->used = used;
	return len;
      }

      head++;

      if (cc == '\r')
	continue;

      if (cc == '\n')
      {
	used = *dest;

	if (used == '.')
	{
	  used = dest[-1];
	  if (used == '\n')
	    break;		/* end of article body */
	  if (used == '.' && dest[-2] == '\n')
	    *dest = ' ';	/* strip leading ".." to ". " */
	}
	else
	{
#if 0     
	  /* strip the trailing space */

	  while (used == ' ' || used == '\t')
	    used = *--dest;
#endif
          /* Thor.990110: 不砍 空行\n 的 space, for multi-line merge */
	  /* strip the trailing space */

	  while (dest[-1] != '\n' && (used == ' ' || used == '\t'))
	    used = *--dest;
	}
      }

      *++dest = cc;
    }

    /* strip the trailing empty lines */

    *data = '\0';
    while (*--dest == '\n')
      ;
    dest += 2;
    *dest = '\0';

    my_recv(client);
    client->mode = 0;
  }
  else
  {
    /* command mode */

    register argv_t *Argv;
    register struct Daemoncmd *dp;

    head = (char *) strchr(head, '\n');

    if (head == NULL)
    {
      in->used += len;
      in->left -= len;
      return len;
    }

    *head++ = '\0';

    REMOTEHOSTNAME = client->hostname;
    REMOTEUSERNAME = client->username;

    Argv = &client->Argv;
    /* Argv->inputline = data; */
    Argv->argc = argify(data, &Argv->argv);
    Argv->dc = dp = (struct Daemoncmd *) searchcmd(Argv->argv[0]);
    if (dp)
    {
      if (Argv->argc < dp->argc)
      {
	fprintf(Argv->out, "%d Usage: %s\r\n", dp->errorcode, dp->usage);
	fflush(Argv->out);
	verboselog("Put: %d Usage: %s\n", dp->errorcode, dp->usage);
      }
      else if (dp->argno && Argv->argc > dp->argno)
      {
	fprintf(Argv->out, "%d Usage: %s\r\n", dp->errorcode, dp->usage);
	fflush(Argv->out);
	verboselog("Put: %d Usage: %s\n", dp->errorcode, dp->usage);
      }
      else
      {
	int (*Main) ();

	if (Main = Argv->dc->main)
	{
	  (*Main) (client);
	}
      }
    }
    else
    {
      fprintf(Argv->out, "500 Syntax error or bad command\r\n");
      fflush(Argv->out);
      verboselog("Put: 500 Syntax error or bad command\r\n");
    }
  }

   /* Thor.980825:
         gc patch: 
         原因
         a.如果已經執行過 CMDquit, 下面的動作都不用做了(cleint destroied) :) */
#if 0
  if (client->mode == 0)
#endif
  if (client->mode == 0 && client->in.data != NULL)
  {
    int left;

    left = in->left + in->used;

    if (used = *head)
    {
      char *str;

      bbslog("CARRY: %s\n", head);

      str = data;
      while (*str++ = *head++)
	;

      used = str - data;
    }

    in->left = left - used;
    in->used = used;
  }

  return len;
}


#ifdef	_ANTI_SPAM_
#define	SPAM_PERIOD	(60 * 45)
#endif


static int
inndchannel(port, path)
  char *port, *path;
{
  time_t uptime;		/* time to maintain */

#ifdef	_ANTI_SPAM_
  time_t spam_due;		/* time to expire anti-spam-hash-table */
#endif

  int i;
  int bbsinnd;
  int localbbsinnd;
  int localdaemonready = 0;

  Channel = (ClientType *) mymalloc(sizeof(ClientType) * Maxclient);

  bbsinnd = pmain(port);
  if (bbsinnd < 0)
  {
    perror("pmain, existing");
    docompletehalt();
    return (-1);
  }

  FD_ZERO(&rfd);
  FD_ZERO(&wfd);
  FD_ZERO(&efd);

  localbbsinnd = p_unix_main(path);
  if (localbbsinnd < 0)
  {
    perror("local pmain, existing");
    if (!inetdstart)
      fprintf(stderr, "if no other innbbsd running, try to remove %s\n", path);
    close(bbsinnd);
    return (-1);
  }
  else
  {
    FD_SET(localbbsinnd, &rfd);
    localdaemonready = 1;
  }

  FD_SET(bbsinnd, &rfd);

  uptime = time((time_t *) 0);

#ifdef	_ANTI_SPAM_
  spam_due = uptime + SPAM_PERIOD;
#endif

  {
    struct tm *local;

    local = localtime(&uptime);
    i = (His_Maint_Hour - local->tm_hour) * 3600 + (His_Maint_Min - local->tm_min) * 60;
    if (i <= 100)		/* 保留時間差 100 秒 */
      i += 86400;
    uptime += i;
  }

  for (i = 0; i < Maxclient; ++i)
  {
    register ClientType *clientp = &Channel[i];

    clientp->fd = -1;
    clientp->buffer[0] = '\0';
    clientp->access = 0;
      clientp->mode = 0;
      clientp->in.left = 0;
      clientp->in.used = 0;
      clientp->in.data = NULL;
      clientp->out.left = 0;
      clientp->out.used = 0;
      clientp->out.data = NULL;
    clientp->midcheck = 1;
  }

#ifdef	_ANTI_SPAM_
  /* spam_init(); */
#endif

  for (;;)
  {
    static char msg_no_desc[] = "502 no free descriptors\r\n";
    int nsel;
    time_t now;
    struct timeval tout;

    /*
     * When to maintain history files.
     */
    if (INNBBSDshutdown())
    {
      HISclose();
      bbslog("Shutdown Complete\n");

#ifdef	_ANTI_SPAM_
      /* spam_exit(); */
#endif

      docompletehalt();
      exit(0);
    }

    now = time(NULL);
    if (now > uptime)
    {
      bbslog(":Maint: start\n");
      HISmaint();
      bbslog(":Maint: end\n");
      uptime += 86400;
    }

#ifdef	_ANTI_SPAM_
    if (now > spam_due)
    {
      spam_expire();
      spam_due += SPAM_PERIOD;
    }
#endif

    /*
     * in order to maintain history, timeout every 20 minutes in case no
     * connections
     */

    tout.tv_sec = 60 * 20;
    tout.tv_usec = 0;
    orfd = rfd;
    if ((nsel = select(FD_SETSIZE, &orfd, NULL, NULL, &tout)) <= 0)
    {
      continue;
    }

    if (localdaemonready && FD_ISSET(localbbsinnd, &orfd))
    {
      int ns;
      ClientType *clientp;

      ns = tryaccept(localbbsinnd);
      if (ns < 0)
	continue;
      for (i = 0, clientp = Channel; i < Maxclient; ++i, clientp++)
      {
	if (clientp->fd == -1)
	  break;
      }
      if (i == Maxclient)
      {
	write(ns, msg_no_desc, sizeof(msg_no_desc) - 1);
	close(ns);
	continue;
      }

      channelcreate(clientp, ns);
      strcpy(clientp->username, "localuser");
      strcpy(clientp->hostname, "localhost");

#if !defined(PowerBBS) && !defined(DBZSERVER)
      bbslog("connected from (%s@%s)\n", clientp->username, clientp->hostname);
#endif

#ifdef INNBBSDEBUG
      printf("connected from (%s@%s)\n", clientp->username, clientp->hostname);
#endif

#ifdef DBZSERVER
      fprintf(clientp->Argv.out, "200 %s InterNetNews DBZSERVER server %s (%s@%s).\r\n", MYBBSID, VERSION, clientp->username, clientp->hostname);
#else
      fprintf(clientp->Argv.out, "200 %s INNBBSD %s (%s@%s)\r\n", MYBBSID, VERSION, clientp->username, clientp->hostname);
#endif

      fflush(clientp->Argv.out);

#ifndef	MapleBBS
      verboselog("UNIX Connect from %s@%s\n", clientp->username, clientp->hostname);
#endif
    }

    if (FD_ISSET(bbsinnd, &orfd))
    {
      int ns = tryaccept(bbsinnd);
      char *username, *hostname;
      ClientType *clientp;
      struct hostent *hp;
      struct sockaddr_in there;
      int length;

      if (ns < 0)
	continue;

      length = sizeof(there);
      getpeername(ns, (struct sockaddr *) & there, &length);
      username = (char *) my_rfc931_name(ns, &there);
      hp = (struct hostent *) gethostbyaddr((char *) &there.sin_addr, sizeof(struct in_addr), there.sin_family);
      hostname = hp ? hp->h_name : (char *) inet_ntoa(there.sin_addr);

      if ((char *) search_nodelist(hostname, username) == NULL)
      {
	char buf[256];

	bbslog(":Err: invalid connection (%s@%s)\n", username, hostname);
	sprintf(buf, "502 You are not in my access file (%s@%s).\r\n", username, hostname);
	write(ns, buf, strlen(buf));
	close(ns);
	continue;
      }

      for (i = 0, clientp = Channel; i < Maxclient; ++i, clientp++)
      {
	if (clientp->fd == -1)
	  break;
      }
      if (i == Maxclient)
      {
	write(ns, msg_no_desc, sizeof(msg_no_desc) - 1);
	close(ns);
	continue;
      }

      strncpy(clientp->username, username, 20);
      strncpy(clientp->hostname, hostname, 128);
      channelcreate(clientp, ns);

      bbslog("connected from (%s@%s)\n", username, hostname);

#ifdef DBZSERVER
      fprintf(clientp->Argv.out, "200 %s InterNetNews DBZSERVER server %s (%s@%s).\r\n", MYBBSID, VERSION, username, hostname);
#else
      fprintf(clientp->Argv.out, "200 %s INNBBSD %s (%s@%s).\r\n", MYBBSID, VERSION, username, hostname);
#endif

      fflush(clientp->Argv.out);

#ifndef	MapleBBS
      verboselog("INET Connect from %s@%s\n", username, hostname);
#endif
    }

    for (i = 0; i < Maxclient; ++i)
    {
      ClientType *clientp = &Channel[i];
      int fd = clientp->fd;

      if (fd < 0)
      {
	continue;
      }
      if (FD_ISSET(fd, &orfd))
      {
	register int nr;

#ifdef DEBUG
	printf("before read i %d in.used %d in.left %d\n", i, clientp->in.used, clientp->in.left);
#endif

	nr = channelreader(clientp);

#ifdef DEBUG
	printf("after read i %d in.used %d in.left %d\n", i, clientp->in.used, clientp->in.left);
#endif

	if (nr <= 0)
	{
	  FD_CLR(fd, &rfd);
	  fclose(clientp->Argv.in);
	  fclose(clientp->Argv.out);
	  close(fd);
	  clientp->fd = -1;
	  channeldestroy(clientp);
	  continue;
	}

#ifdef DEBUG
	printf("nr %d %.*s", nr, nr, clientp->buffer);
#endif

	if (clientp->access == 0)
	{
	  continue;
	}
      }
    }
  }
}


static void
dopipesig(s)
  int s;
{
  printf("catch sigpipe\n");
  signal(SIGPIPE, dopipesig);
}


static void
standaloneinit(port)
  char *port;
{
  int ndescriptors;
  FILE *pf;
  char pidfile[24];
  ndescriptors = getdtablesize();
  /* #ifndef NOFORK */
  if (!inetdstart)
    if (fork())
      exit(0);
  /* #endif */

  sprintf(pidfile, "run/innbbsd-%s.pid", port);
  if (!inetdstart)
    fprintf(stderr, "PID file is in %s\n", pidfile);

  {
    int s;
    for (s = 3; s < ndescriptors; s++)
      (void) close(s);
  }
  if (pf = fopen(pidfile, "w"))
  {
    fprintf(pf, "%d\n", getpid());
    fclose(pf);
  }
}


static void
innbbsusage(name)
  char *name;
{
  fprintf(stderr, "Usage: %s\t[options] [port [path]]\n"
    "  -v\t(verbose log)\n"
    "  -h|?\t(help)\n"
    "  -n\t(not to use in core dbz)\n"
    "  -i\t(start from inetd with wait option)\n"
    "  -c\tconnections  (maximum number of connections accepted)\n"
    "\tdefault=%d\n"
    "  -j\t(keep history of junk article, default=none)\n", name, Maxclient);
}


#ifdef DEBUGNGSPLIT
main()
{
  char **ngptr;
  char buf[1024];
  gets(buf);
  ngptr = (char **) BNGsplit(buf);
  printf("line %s\n", buf);
  while (*ngptr != NULL)
  {
    printf("%s\n", *ngptr);
    ngptr++;
  }
}
#endif

static time_t INNBBSDstartup;

int
innbbsdstartup()
{
  return INNBBSDstartup;
}


extern char *optarg;
extern int opterr, optind;


main(argc, argv)
  int argc;
  char **argv;
{

  char *port, *path;
  int c, errflag = 0;
  extern INNBBSDhalt();

  port = DefaultINNBBSPort;
  path = LOCALDAEMON;

  umask(077);
  time(&INNBBSDstartup);
  openlog("innbbsd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
  while ((c = getopt(argc, argv, "c:f:s:vhidn?j")) != -1)
  {
    switch (c)
    {
    case 'j':
      Junkhistory = 1;
      break;
    case 'v':
      verboseon("innbbsd.log");
      break;
    case 'n':
      hisincore(0);
      break;
    case 'c':
      Maxclient = atoi(optarg);
      if (Maxclient < 0)
	Maxclient = 0;
      break;

    case 'i':
      {
	struct sockaddr_in there;
	int len = sizeof(there);
	int rel;
	if ((rel = getsockname(0, (struct sockaddr *) & there, &len)) < 0)
	{
	  fprintf(stdout, "You must run -i from inetd with inetd.conf line: \n"
	    "service-port stream  tcp wait  bbs  /home/bbs/innbbsd innbbsd -i port\n");
	  exit(5);
	}
	inetdstart = 1;
	startfrominetd(1);
      }
      break;

#ifdef	DBZDEBUG
    case 'd':
      dbzdebug(1);
      break;
#endif

    case 's':
      Max_Art_Size = atol(optarg);
      if (Max_Art_Size < 0)
	Max_Art_Size = 0;
      break;
    case 'h':
    case '?':
    default:
      errflag++;
    }
  }

  if (errflag > 0)
  {
    innbbsusage(argv[0]);
    exit(1);
  }
  if (argc - optind >= 1)
  {
    port = argv[optind];
  }
  if (argc - optind >= 2)
  {
    path = argv[optind + 1];
  }

#ifdef	MapleBBS
  chdir(BBSHOME);
#endif

  standaloneinit(port);

  initial_bbs("feed");

  if (!inetdstart)
    fprintf(stderr, "Try to listen in port %s and path %s\n", port, path);

  HISmaint();
  HISsetup();
  installinnbbsd();
  sethaltfunction(INNBBSDhalt);

  signal(SIGPIPE, dopipesig);
  inndchannel(port, path);
  HISclose();
}

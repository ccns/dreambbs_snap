
#include "bbs.h"

#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFSIZE         1024

struct fileheader currentmail;
struct userec currentuser;

char LowUserid[20];
char genbuf[BUFSIZE];

#define	QLEN		5

#define S_CONNECT       1
#define S_LOGIN         2


#define STRN_CPY(d,s,l) { strncpy((d),(s),(l)); (d)[(l)-1] = 0; }
#define ANY_PORT        0
#define RFC931_PORT     113
#define RFC931_TIMEOUT         5
#define POP3_TIMEOUT         60

static jmp_buf timebuf;

int State;
int msock, sock;		/* master server socket */
int reaper();
char fromhost[STRLEN];
char inbuf[BUFSIZE];
char remote_userid[STRLEN];
FILE *cfp;
char *msg, *cmd;
int fd;
struct fileheader *fcache;
int totalnum, totalbyte, markdel, idletime;
int *postlen;

void log_usies();
int Quit(), User(), Pass(), Noop(), Stat(), List(), Retr(), Rset();
int Last(), Dele(), Uidl();

struct commandlist
{
  char *name;
  int (*fptr) ();
}           cmdlist[] =
{
  "retr", Retr,
  "dele", Dele,
  "user", User,
  "pass", Pass,
  "stat", Stat,
  "list", List,
  "uidl", Uidl,
  "quit", Quit,
  "rset", Rset,
  "last", Last,
  "noop", Noop,
  NULL, NULL
};

char *crypt();

int
checkpasswd(passwd, test)
  char *passwd, *test;
{
  static char pwbuf[14];
  char *pw;

  strncpy(pwbuf, test, 14);
  pw = crypt(pwbuf, passwd);
  return (!strcmp(pw, passwd));
}


abort_server()
{
  log_usies("ABORT");
  close(msock);
  close(sock);
  exit(1);
}

static FILE *
fsocket(domain, type, protocol)
  int domain;
  int type;
  int protocol;
{
  int s;
  FILE *fp;

  if ((s = socket(domain, type, protocol)) < 0)
  {
    return (0);
  }
  else
  {
    if ((fp = fdopen(s, "r+")) == 0)
    {
      close(s);
    }
    return (fp);
  }
}

void
outs(str)
  char *str;
{
  fprintf(cfp, "%s\r\n", str);
  fflush(cfp);
}

void
outfile(filename, linenum)
  char *filename;
  int linenum;
{
  FILE *fp;
  char linebuf[256];

  if (linenum && (fp = fopen(filename, "r")) != NULL)
  {
    while (fgets(linebuf, 256, fp) != NULL && linenum > 0)
    {
      linebuf[strlen(linebuf) - 1] = '\0';
      if (strcmp(linebuf, ".") == 0)
	outs("..");
      else
	outs(linebuf);
      linenum--;
    }
    fclose(fp);
  }
  outs(".");
}


/* timeout - handle timeouts */
static void 
timeout(sig)
  int sig;
{
  longjmp(timebuf, sig);
}


void 
rfc931(rmt_sin, our_sin, dest)
  struct sockaddr_in *rmt_sin;
  struct sockaddr_in *our_sin;
  char *dest;
{
  unsigned rmt_port;
  unsigned our_port;
  struct sockaddr_in rmt_query_sin;
  struct sockaddr_in our_query_sin;
  char user[256];		/* XXX */
  char buffer[512];		/* XXX */
  char *cp;
  FILE *fp;
  char *result = "unknown";
  struct hostent *hp;

  /*
   * Use one unbuffered stdio stream for writing to and for reading from the
   * RFC931 etc. server. This is done because of a bug in the SunOS 4.1.x
   * stdio library. The bug may live in other stdio implementations, too.
   * When we use a single, buffered, bidirectional stdio stream ("r+" or "w+"
   * mode) we read our own output. Such behaviour would make sense with
   * resources that support random-access operations, but not with   sockets.
   */
  if ((fp = fsocket(AF_INET, SOCK_STREAM, 0)) != 0)
  {
    setbuf(fp, (char *) 0);

    /*
     * Set up a timer so we won't get stuck while waiting for the server.
     */

    if (setjmp(timebuf) == 0)
    {
      signal(SIGALRM, timeout);
      alarm(RFC931_TIMEOUT);

      /*
       * Bind the local and remote ends of the query socket to the same IP
       * addresses as the connection under investigation. We go through all
       * this trouble because the local or remote system might have more than
       * one network address. The RFC931 etc.  client sends only port
       * numbers; the server takes the IP    addresses from the query socket.
       */

      our_query_sin = *our_sin;
      our_query_sin.sin_port = htons(ANY_PORT);
      rmt_query_sin = *rmt_sin;
      rmt_query_sin.sin_port = htons(RFC931_PORT);

      if (bind(fileno(fp), (struct sockaddr *) & our_query_sin,
	  sizeof(our_query_sin)) >= 0 &&
	connect(fileno(fp), (struct sockaddr *) & rmt_query_sin,
	  sizeof(rmt_query_sin)) >= 0)
      {

	/*
	 * Send query to server. Neglect the risk that a 13-byte write would
	 * have to be fragmented by the local system and cause trouble with
	 * buggy System V stdio libraries.
	 */

	fprintf(fp, "%u,%u\r\n",
	  ntohs(rmt_sin->sin_port),
	  ntohs(our_sin->sin_port));
	fflush(fp);

	/*
	 * Read response from server. Use fgets()/sscanf() so we can work
	 * around System V stdio libraries that incorrectly assume EOF when a
	 * read from a socket returns less than requested.
	 */

	if (fgets(buffer, sizeof(buffer), fp) != 0
	  && ferror(fp) == 0 && feof(fp) == 0
	  && sscanf(buffer, "%u , %u : USERID :%*[^:]:%255s",
	    &rmt_port, &our_port, user) == 3
	  && ntohs(rmt_sin->sin_port) == rmt_port
	  && ntohs(our_sin->sin_port) == our_port)
	{

	  /*
	   * Strip trailing carriage return. It is part of the protocol, not
	   * part of the data.
	   */

	  if (cp = strchr(user, '\r'))
	    *cp = 0;
	  result = user;
	}
      }
      alarm(0);
    }
    fclose(fp);
  }
  STRN_CPY(dest, result, 60);

  if (strcmp(dest, "unknown") == 0)
    strcpy(dest, "");
  else
    strcat(dest, "@");

  hp = gethostbyaddr((char *) &rmt_sin->sin_addr, sizeof(struct in_addr),
    rmt_sin->sin_family);
  if (hp)
    strcat(dest, hp->h_name);
  else
    strcat(dest, inet_ntoa(rmt_sin->sin_addr));

}

int
Isspace(ch)
  char ch;
{
  return (ch == ' ' || ch == '\t' || ch == 10 || ch == 13);
}

char *
nextwordlower(str)
  char **str;
{
  char *p;

  while (Isspace(**str))
    (*str)++;
  p = (*str);

  while (**str && !Isspace(**str))
  {
    **str = tolower(**str);
    (*str)++;
  }

  if (**str)
  {
    **str = '\0';
    (*str)++;
  }
  return p;
}

char *
nextword(str)
  char **str;
{
  char *p;

  while (Isspace(**str))
    (*str)++;
  p = (*str);

  while (**str && !Isspace(**str))
    (*str)++;

  if (**str)
  {
    **str = '\0';
    (*str)++;
  }
  return p;
}

void
Init()
{
  State = S_CONNECT;
  LowUserid[0] = '\0';
  markdel = 0;
  idletime = 0;
}

void
Login_init()
{
  int fd, i;
  char *ptr;
  struct stat st;

  totalnum = totalbyte = 0;
  sprintf(genbuf, "home/%c/%s/mail/.DIR", *LowUserid, LowUserid);
  if (stat(genbuf, &st) == -1 || st.st_size == 0)
  {
    return;
  }
  totalnum = st.st_size / sizeof(struct fileheader);
  fcache = (struct fileheader *) malloc(st.st_size);
  postlen = (int *) malloc(sizeof(int) * totalnum);
  fd = open(genbuf, O_RDONLY);
  read(fd, fcache, st.st_size);
  close(fd);

  for (i = 0; i < totalnum; i++)
  {
    if (index(fcache[i].owner, '@') == NULL)
    {
      if (ptr = strchr(fcache[i].owner, ' '))
	*ptr = '\0';
      strcat(fcache[i].owner, ".bbs@cis.nctu.edu.tw");
    }
    sprintf(genbuf, "home/%c/%s/mail/%s", *LowUserid, LowUserid, fcache[i].filename);
    if (stat(genbuf, &st) == -1)
      st.st_size = 0;
    postlen[i] = st.st_size + strlen(fcache[i].owner) + 10 + strlen(fcache[i].title)
      + 10 + 40;
    totalbyte += postlen[i];
    fcache[i].savemode = ' ';
  }
}

void
pop3_timeout()
{
  idletime++;
  if (idletime > 5)
  {
    log_usies("ABORT");
    fclose(cfp);
    close(sock);
    exit(1);
  }
  alarm(POP3_TIMEOUT);
}


main(argc, argv)
  int argc;
  char **argv;
{

  struct sockaddr_in fsin, our;
  int on, alen, len, i, n, portnum;
  char *str, flag;

  if (argc != 2)
  {
    printf(" Usage: %s [port]\n", argv[0]);
    exit;
  }

  portnum = atoi(argv[1]);

  if (fork())
    exit(0);
  for (n = 0; n < 10; n++)
    close(n);
  open("/dev/null", O_RDONLY);
  dup2(0, 1);
  dup2(0, 2);
  if ((n = open("/dev/tty", O_RDWR)) > 0)
  {
    ioctl(n, TIOCNOTTY, 0);
    close(n);
  }

  signal(SIGHUP, abort_server);
  (void) signal(SIGCHLD, reaper);

  msock = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
  bzero((char *) &fsin, sizeof(fsin));
  fsin.sin_family = AF_INET;
  fsin.sin_addr.s_addr = htonl(INADDR_ANY);
  fsin.sin_port = htons(portnum);

  if (bind(msock, (struct sockaddr *) & fsin, sizeof(fsin)) < 0)
  {
    exit(1);
  }

  listen(msock, QLEN);

  while (1)
  {
    alen = sizeof(fsin);
    sock = accept(msock, (struct sockaddr *) & fsin, &alen);
    if (sock < 0)
    {
      if (errno != EINTR)
	continue;
    }

    switch (fork())
    {
    case 0:			/* child process */
      close(msock);

      setgid(101);
      setuid(101);

      strcpy(fromhost, inet_ntoa(fsin.sin_addr));
      len = sizeof our;
      getsockname(sock, &our, &len);

      Init();

      rfc931(&fsin, &our, remote_userid);

      cfp = fdopen(sock, "r+");
      setbuf(cfp, (char *) 0);

      outs("+OK NCTU-CIS BBS POP3 server ready (Comments to: helios@cis.nctu.edu.tw)");

      chdir(BBSHOME);

      log_usies("CONNECT");
      alarm(0);
      signal(SIGALRM, pop3_timeout);
      alarm(POP3_TIMEOUT);

      while (fgets(inbuf, sizeof(inbuf), cfp) != 0)
      {

	idletime = 0;

	msg = inbuf;

	inbuf[strlen(inbuf) - 1] = '\0';
	if (inbuf[strlen(inbuf) - 1] == '\r')
	  inbuf[strlen(inbuf) - 1] = '\0';
	cmd = nextwordlower(&msg);

	if (*cmd == 0)
	  continue;

	i = 0;
	while ((str = cmdlist[i].name) != NULL)
	{
	  if (strcmp(cmd, str) == 0)
	    break;
	  i++;
	}

	if (str == NULL)
	  outs("-ERR non-existant command");
	else
	  (*cmdlist[i].fptr) ();

      }

      if (State == S_LOGIN)
      {
	free(fcache);
	free(postlen);
      }
      log_usies("ABORT");
      fclose(cfp);
      close(sock);
      exit(0);
      break;
    default:			/* parent process */
      close(sock);
      break;
    }
  }
}

int
reaper()
{
  int state, pid;
  while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0);
}

int
Noop()
{
  outs("+OK");
  return;
}

int
User()
{
  char *ptr;

  if (State == S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }

  cmd = nextwordlower(&msg);
  if (*cmd == 0)
  {
    outs("-ERR arguments mismatch");
    return;
  }
  if (strstr(cmd, ".bbs") == NULL)
  {
    sprintf(genbuf, "-ERR %s doesn't get his mail here", cmd);
    outs(genbuf);
    return;
  }

  ptr = strchr(cmd, '.');
  *ptr = '\0';
  sprintf(genbuf, "home/%c/%s/.PASSWDS", *cmd, cmd);
  if ((fd = open(genbuf, O_RDWR, 0)) == -1)
  {
    sprintf(genbuf, "-ERR %s.bbs doesn't get his mail here", cmd);
    outs(genbuf);
    return;
  }
  read(fd, &currentuser, sizeof(struct userec));
  strcpy(LowUserid, cmd);
  close(fd);
  sprintf(genbuf, "+OK %s.bbs gets mail here", cmd);
  outs(genbuf);
  return;
}

void
log_usies(buf)
  char *buf;
{
  FILE *fp;
  long ti;

  if ((fp = fopen("log/pop3d.log", "a")) != NULL)
  {
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(fp, "%02d/%02d/%02d %02d:%02d:%02d [%s](%s) %s\n",
      p->tm_year, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min,
      p->tm_sec, currentuser.userid ? currentuser.userid : "",
      remote_userid ? remote_userid : "", buf);
    fflush(fp);
    fclose(fp);
  }
}

int
Retr()
{
  int num;

  if (State != S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }

  cmd = nextword(&msg);

  if (*cmd == 0)
  {
    outs("-ERR arguments mismatch");
    return;
  }

  num = atoi(cmd);
  if (num <= 0 || totalnum < num)
  {
    outs("-ERR message number out of range");
    return;
  }
  else if (fcache[num - 1].savemode == 'X')
  {
    outs("-ERR message has been deleted");
    return;
  }
  num--;
  outs("+OK here comes the message");
  sprintf(genbuf, "From: %s", fcache[num].owner);
  outs(genbuf);
  sprintf(genbuf, "To: %s.bbs@cis.nctu.edu.tw", currentuser.userid);
  outs(genbuf);
  sprintf(genbuf, "Subject: %s", fcache[num].title);
  outs(genbuf);
  outs("");
  sprintf(genbuf, "home/%c/%s/mail/%s", *LowUserid, LowUserid, fcache[num].filename);
  outfile(genbuf, 99999);

}


int
Stat()
{
  if (State != S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }
  sprintf(genbuf, "+OK %d %d", totalnum, totalbyte);
  outs(genbuf);
}


int
Rset()
{
  int i;

  if (State != S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }

  for (i = 0; i < totalnum; i++)
  {
    fcache[i].savemode = ' ';
  }
  markdel = 0;
  sprintf(genbuf, "+OK mail reset %d messages %d octets", totalnum, totalbyte);
  outs(genbuf);
}

int
List()
{
  int i;

  if (State != S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }

  cmd = nextword(&msg);

  if (*cmd == 0)
  {
    outs("+OK");
    for (i = 0; i < totalnum; i++)
    {
      if (fcache[i].savemode == ' ')
      {
	sprintf(genbuf, "%d %d", i + 1, postlen[i]);
	outs(genbuf);
      }
    }
    outs(".");
  }
  else
  {
    i = atoi(cmd);
    if (i <= 0 || totalnum < i)
    {
      outs("-ERR message number out of range");
      return;
    }
    else if (fcache[i - 1].savemode == 'X')
    {
      outs("-ERR message has been deleted");
      return;
    }
    sprintf(genbuf, "+OK %d %d", i, postlen[i - 1]);
    outs(genbuf);
  }

}

int
Uidl()
{
  int i;

  if (State != S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }

  cmd = nextword(&msg);

  if (*cmd == 0)
  {
    outs("+OK");
    for (i = 0; i < totalnum; i++)
    {
      if (fcache[i].savemode == ' ')
      {
	sprintf(genbuf, "%d %s", i + 1, fcache[i].filename);
	outs(genbuf);
      }
    }
    outs(".");
  }
  else
  {
    i = atoi(cmd);
    if (i <= 0 || totalnum < i)
    {
      outs("-ERR message number out of range");
      return;
    }
    else if (fcache[i - 1].savemode == 'X')
    {
      outs("-ERR message has been deleted");
      return;
    }
    sprintf(genbuf, "+OK %d %d", i, fcache[i - 1].filename);
    outs(genbuf);
  }

}


int
Pass()
{
  if (State == S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }

  cmd = nextword(&msg);

  if (*cmd == 0)
  {
    outs("-ERR arguments mismatch");
    return;
  }

  if (LowUserid[0] == '\0')
  {
    outs("-ERR need a USER");
    return;
  }

  if (!checkpasswd(currentuser.passwd, cmd))
  {
    LowUserid[0] = '\0';
    outs("-ERR invalid password");
    log_usies("ERROR PASSWD");
    return;
  }

  if (State == S_CONNECT)
  {
    log_usies("ENTER");
    State = S_LOGIN;
  }

  Login_init();
  outs("+OK mailbox ready");
}

int
Last()
{
  if (State != S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }

  sprintf(genbuf, "+OK %d", totalnum);
  outs(genbuf);
}

int
Dele()
{
  int num;

  if (State != S_LOGIN)
  {
    outs("-ERR invalid command");
    return;
  }

  cmd = nextword(&msg);

  if (*cmd == 0)
  {
    outs("-ERR arguments mismatch");
    return;
  }

  num = atoi(cmd);
  if (num <= 0 || totalnum < num)
  {
    outs("-ERR message number out of range");
    return;
  }
  else if (fcache[num - 1].savemode == 'X')
  {
    outs("-ERR message has been deleted");
    return;
  }
  num--;

  fcache[num].savemode = 'X';
  markdel++;
  outs("+OK message deleted");
}

int
do_delete()
{
  int i, fdr, fdw, count;
  char fpath[80], fnew[80];

  sprintf(fpath, "home/%c/%s/mail/.DIR", *LowUserid, LowUserid);
  sprintf(fnew, "home/%c/%s/mail/.DIR.pop3", *LowUserid, LowUserid);
  if ((fdr = open(fpath, O_RDONLY)) == -1)
    return;
  if ((fdw = open(fnew, O_RDWR | O_CREAT, 0644)) == -1)
    return;
  i = count = 0;
  while (read(fdr, &currentmail, sizeof(currentmail)))
  {
    if (i >= totalnum || fcache[i].savemode == ' ')
    {
      write(fdw, &currentmail, sizeof(currentmail));
      count++;
    }
    else
    {
      sprintf(genbuf, "home/%c/%s/mail/%s", *LowUserid, LowUserid,
	currentmail.filename);
      unlink(genbuf);
    }
    i++;
  }
  close(fdr);
  close(fdw);
  unlink(fpath);
  if (count)
    rename(fnew, fpath);
  else
    unlink(fnew);
}

int
Quit()
{
  if (State == S_LOGIN)
  {
    free(fcache);
    free(postlen);
    if (markdel)
      do_delete();
  }
  log_usies("EXIT");
  outs("+OK NCTU-CIS BBS POP3 server connection terminated");
  fclose(cfp);
  close(sock);
  exit(0);
}

/*-------------------------------------------------------*/
/* more.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : simple & beautiful ANSI/Chinese browser	 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


/* ----------------------------------------------------- */
/* buffered file read					 */
/* ----------------------------------------------------- */


/* it seems 2048 is better */

#if 0
#define MORE_BUFSIZE	2048
#define	MORE_WINSIZE	2048	/* window size : 2048 or 4096 */
#endif


#define MORE_BUFSIZE	4096
#define	MORE_WINSIZE	4096	/* window size : 2048 or 4096 */
#define	NET_WINSIZE	3072


#define	STR_ANSICODE	"[0123456789;,"


static uschar more_pool[MORE_BUFSIZE];
static int more_base, more_size, more_head;
extern BCACHE *bshm;            /* lkchu.981201: for 「不可轉寄」功能 */


char *
mgets(fd)
  int fd;
{
  char *pool, *base, *head, *tail;
  int ch;

  if (fd < 0)
  {
    more_base = more_size = 0;
    return NULL;
  }

  pool = more_pool;
  base = pool + more_base;
  tail = pool + more_size;
  head = base;

  for (;;)
  {
    if (head >= tail)
    {
      if (ch = head - base)
	memcpy(pool, base, ch);

      head = pool + ch;
      ch = read(fd, head, MORE_BUFSIZE - ch);

      if (ch <= 0)
	return NULL;

      base = pool;
      tail = head + ch;
      more_size = tail - pool;
    }

    ch = *head;

    if (ch == '\n')
    {
      *head++ = '\0';
      more_base = head - pool;
      return base;
    }

    if (ch == '\r')
      *head = '\0';

    head++;
  }
}


/* use mgets(-1) to reset */


void *
mread(fd, len)
  int fd, len;
{
  char *pool;
  int base, size;

  base = more_base;
  size = more_size;
  pool = more_pool;

  if (size < len)
  {
    if (size)
    {
      memcpy(pool, pool + base, size);
    }

    base = read(fd, pool + size, MORE_BUFSIZE - size);

    if (base <= 0)
      return NULL;

    size += base;
    base = 0;
  }

  more_base = base + len;
  more_size = size - len;

  return pool + base;
}


#if 0
int
net_puts(sock, str)
  int sock;
  char *str;
{
  char *pool, *head;
  int size, cc;

  pool = more_pool;
  size = more_size;

  if ((size > NET_WINSIZE) || (str == NULL))
  {
    head = pool;
    for (;;)
    {
      cc = send(sock, head, size, 0);
      if (cc <= 0)
	return -1;
      size -= cc;
      if (size <= 0)
      {
	if (str == NULL)
	  return 0;
	break;
      }
      head += cc;
    }
  }

  head = pool + size;
  while (cc = *str++)
  {
    *head++ = cc;
  }
  more_size = head - pool;
  return 0;
}
#endif


static inline void
more_goto(fd, off)
  int fd;
  off_t off;
{
  int base = more_base;

  if (off < base || off >= base + more_size)
  {
    more_base = base = off & (-MORE_WINSIZE);
    lseek(fd, base, SEEK_SET);
    more_size = read(fd, more_pool, MORE_BUFSIZE);
  }
  more_head = off - base;
}


static int
more_line(fd, buf)
  int fd;
  uschar *buf;
{
  int ch;
  uschar *data, *tail;
  int len, ansilen, bytes, in_ansi;
  int size, head;

  len = ansilen = bytes = in_ansi = 0;
  tail = buf + ANSILINELEN - 1;
  size = more_size;
  head = more_head;
  data = &more_pool[head];
  do
  {
    if (head >= size)
    {
      more_base += size;
      data = more_pool;
      more_size = size = read(fd, data, MORE_BUFSIZE);

      if (size == 0)		/* end of file */
	break;

      head = 0;
    }

    ch = *data++;
    head++;
    bytes++;
    if (ch == '\n')
    {
      break;
    }

    if (ch == '\t')
    {
      do
      {
	*buf++ = ' ';
      } while ((++len & 7) && len < 80);
    }
    else if (ch == KEY_ESC)
    {
      if (showansi)
	*buf++ = ch;
      in_ansi = 1;
    }
    else if (in_ansi)
    {
      if (showansi)
	*buf++ = ch;
      if (!strchr(STR_ANSICODE, ch))
	in_ansi = 0;
    }

#ifdef SHOW_USER_IN_TEXT
    else if (isprint2(ch) || ch == 1 || ch == 2)
#else
    else if (isprint2(ch))
#endif

    {
      len++;
      *buf++ = ch;
    }
  } while (len < 80 && buf < tail);
  *buf = '\0';
  more_head = head;
  return bytes;
}


/* ----------------------------------------------------- */


/* Thor.990204: 傳回值 -1 為無法show出
                        0 為全數show完
                       >0 為未全show，中斷所按的key */


int
more(fpath, footer)
  char *fpath;
  char *footer;
{
  char *head[] = {"作者", "標題", "時間", "路徑", "來源"};
  char *ptr, *word=NULL, buf[256], hunt[32], lunt[32];
  struct stat st;
  int fd, fsize, line, viewed, pos, bytes;
  int pagebreak[MAXPAGES], ch1, ch2, pageno, lino=0, have_origin;

  int cmd = 0; /* Thor.990204: 記 中斷時所按的鍵 */

  fd = open(fpath, O_RDONLY);
  if (fd < 0)
    return -1;

  if (fstat(fd, &st) || (fsize = st.st_size) <= 0)
  {
    close(fd);
    return -1;
  }

  hunt[0] = pagebreak[0] = more_base = more_head = more_size =
    pageno = viewed = line = pos = have_origin = 0;
  clear();

  while ((bytes = more_line(fd, buf)) || (line >= b_lines + 1))
  {
    if (bytes)			/* 一般資料處理 */
    {
      if (!viewed)		/* begin of file */
      {
	if (showansi)		/* header processing */
	{
	  if (!memcmp(buf, str_author1, LEN_AUTHOR1)) /* 「作者:」表站內文章 */
	  {
	    line = 3;
	    word = buf + LEN_AUTHOR1;
	  }
	  else if (!memcmp(buf, str_author2, LEN_AUTHOR2)) /* 「發信人:」表轉信文章 */
	  {
	    line = 6;
	    word = buf + LEN_AUTHOR2;
	  }

	  while (pos < line)
	  {
	    if (!pos && ((ptr = strstr(word, str_post1)) ||
		(ptr = strstr(word, str_post2))))
	    {
	      ptr[-1] = ptr[4] = '\0';
	      prints("\033[47;34m %s \033[44;37m%-53.53s\033[47;34m %s \033[44;37m%-13s\033[m\n", head[0], word, ptr, ptr + 5);
	    }
	    else if (pos < 5)	/* lkchu.990430: 多印「來源」欄位*/
	    {
	      prints("\033[47;34m %s \033[44;37m%-72.72s\033[m\n", head[pos], word);
              if (pos == 4)	/* lkchu.990430: 有 Origin 欄位, pos 才會數到 4 */
                have_origin = 1;
	    }

	    viewed += bytes;
	    bytes = more_line(fd, buf);

	    /* 第一行太長了 */

	    if (!pos && viewed >= 79 && memcmp(buf, head[1], 2))
	    {
	      viewed += bytes;
	      bytes = more_line(fd, buf);
	    }

	    pos++;

	    if (!*buf)
	      break;
	  }

	  if (pos)
	  {
            /* lkchu.990430: 站內文章 header 不變 */
            if (line == 3)
              prints("\033[36m%s\033[m\n", msg_seperator);

            if (have_origin)	/* lkchu.990430: 考慮有 Origin 情況 */
              line = pos = 5;
            else
              line = pos = 4;
	  }
	}
	lino = pos;
	word = NULL;
      }

      /* ※處理引用者 & 引言 */

      ch1 = buf[0];
      ch2 = buf[1];
      if (((ch2 == ' ') && (ch1 == '>' || ch1 == ':')) ||
	(ch1 == '?' && ch2 == '?') ||
	(ch1 == '=' && ch2 == '=' && buf[2] == '>'))
      {
	outs("\033[32m");
	outx(buf);
	outs(str_ransi);
      }
      else
      {
	outx(buf);
      }
      outc('\n');

      if (line < b_lines)	/* 一般資料讀取 */
      {
	line++;
      }
      else if (line == 99)	/* 字串搜尋 */
      {
	if (str_str(buf, lunt))
	  line = b_lines;
      }

      if (pos == b_lines)	/* 捲動螢幕 */
	scroll();
      else
	pos++;

      if ((++lino >= b_lines) && (pageno < MAXPAGES - 1))
      {
	pagebreak[++pageno] = viewed;
	lino = 1;
      }

      viewed += bytes;		/* 累計讀過資料 */

      if ((viewed >= fsize) && (line <= b_lines))
	break;
    }
    else
    {
      line = b_lines;		/* end of END */
    }

    if (line == b_lines)
    {
      prints("\033[0;34;46m 瀏覽 P.%d(%d%%) \033[31;47m (h)\033[30m求助 \033[31m[PgUp][PgDn][0][$]\033[30m移動 \033[31m(/n)\033[30m搜尋 \033[31m(C)\033[30m暫存 \033[31m←(q)\033[30m結束 \033[m", pageno, (viewed * 100) / fsize);

      for (;;)
      {
        int key;
        /* Thor.990208: 為了將按鍵傳回 */
	switch (key = vkey())
	{
	case ' ':
	case '\n':
	case KEY_PGDN:
	case KEY_RIGHT:
	case Ctrl('F'):
	  line = 1;
	  break;
	case KEY_UP:
	case KEY_PGUP:
	case Ctrl('B'):
	  if (pageno < 2)
	    continue;

	  pageno -= 2;
	  line = 0;
	  break;

	case '/':
	  if (vget(b_lines, 0, "搜尋：", hunt, sizeof(hunt), GCARRY))
	    str_lower(lunt, hunt);

	case 'n':
	  if (hunt[0])
	  {
	    line = 99;
	    break;
	  }

	case KEY_DOWN:
	  line = b_lines - 1;
	  break;

	case KEY_END:
	case '$':
	  line = b_lines + 1;
	  break;

	case KEY_HOME:
	case '0':
	  pageno = line = 0;
	  break;

	case 'h':
	case '?':
	  film_out(FILM_MORE, -1);
	  pageno--;
	  line = 0;
	  break;

        case 'C': /* Thor.980405: more時可存入暫存檔 */
          {
            int bno;
            
            if ((bno = brd_bno(currboard)) >= 0)
                         /* lkchu.981223: 用 str_cmp 或 brd_bno 判斷都可 */
            {
              BRD *brd;

   	      /* lkchu.981201: 考慮不可轉寄的看板透過暫存檔寄出 */

              brd = bshm->bcache + bno;

              if (HAS_PERM(PERM_SYSOP) || (!(brd->battr & BRD_NOFORWARD)))
              {
                FILE *fp;
                if (fp = tbf_open())
                { 
                  f_suck(fp, fpath); 
                  fclose(fp);
                }
              }
            }
          }
	  pageno--;
	  line = 0;
          break;

	case KEY_LEFT:
	case 'q':
          /* Thor.980823: 將最後一行的訊息清除 */
	  close(fd);
	  return -2;

        default:
          /* continue; */
          /* Thor.990204: 讓key可回傳至more外 */
          cmd = key;
          move(b_lines, 0);
          clrtoeol();
          goto more_exit;

	}

	break;
      }

      if (line)
      {
	move(b_lines, 0);
	clrtoeol();
      }
      else
      {
	pos = 0;
	more_goto(fd, viewed = pagebreak[pageno]);
	clear();
      }
    }
  }

more_exit:

  close(fd);
  if (footer == NULL)
  {
    /* lkchu.981201: 先清一次以免重疊顯示 */
    move(b_lines, 0);
    clrtoeol();

    if ((pos = brd_bno(currboard)) >= 0)
                             /* lkchu.981223: 借 pos 一用 :p */
    {
      BRD *brd;

      /* lkchu.981201: 不可轉寄之看板不可使用 'C' */

      brd = bshm->bcache + pos;
      
      if (HAS_PERM(PERM_SYSOP) && (!(brd->battr & BRD_NOFORWARD)))
      {           

        /* Thor.990204: 特別注意若回傳key的此種情況 */
        while (vmsg(NULL)=='C')
        {
          FILE *fp;

          if (fp = tbf_open()) 
          { 
            f_suck(fp, fpath); 
            fclose(fp);
          }
          move(b_lines, 0);
          clrtoeol();
        }
        clear();
        return cmd;
      }
    }
    
    vmsg(NULL);
    clear();
  }
  else
  {
    if (footer != (char *) -1)
      outz(footer);
    else
      outs(str_ransi);
  }
  /* Thor.990204: 讓key可回傳至more外 */
  return cmd;
}

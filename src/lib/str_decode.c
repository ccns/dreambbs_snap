/*-------------------------------------------------------*/
/* lib/str_decode.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C for QP/BASE64 decoding		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"				 */
/* ----------------------------------------------------- */


static int
qp_code(x)
  register int x;
{
  if (x >= '0' && x <= '9')
    return x - '0';
  if (x >= 'a' && x <= 'f')
    return x - 'a' + 10;
  if (x >= 'A' && x <= 'F')
    return x - 'A' + 10;
  return -1;
}


/* ------------------------------------------------------------------ */
/* BASE64 :							      */
/* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
/* ------------------------------------------------------------------ */


static int
base64_code(x)
  register int x;
{
  if (x >= 'A' && x <= 'Z')
    return x - 'A';
  if (x >= 'a' && x <= 'z')
    return x - 'a' + 26;
  if (x >= '0' && x <= '9')
    return x - '0' + 52;
  if (x == '+')
    return 62;
  if (x == '/')
    return 63;
  return -1;
}


/* ----------------------------------------------------- */
/* judge & decode QP / BASE64				 */
/* ----------------------------------------------------- */

static inline
int isreturn(c)
unsigned char c;
{
  return c == '\r' || c == '\n';
}

static inline
int isspace(c)
  unsigned char c;
{
   return c == ' ' || c == '\t' || isreturn(c);
}

/* static inline */
int mmdecode(src, encode, dst)
  unsigned char *src; /* Thor.980901: src和dst可相同, 但src 一定有?或\0結束 */
  unsigned char encode; /* Thor.980901: 注意, decode出的結果不會自己加上 \0 */
  unsigned char *dst;
{
  unsigned char *t = dst;
  int pattern = 0, bits = 0;
  encode |= 0x20;        /* Thor: to lower */
  switch(encode)
  {
    case 'q':            /* Thor: quoted-printable */
      while(*src && *src != '?') /* Thor: delimiter */
      {                  /* Thor.980901: 0 算是 delimiter */
        if(*src == '=')
        {
          int x = *++src, y = x ? *++src : 0;
          if(isreturn(x)) continue;
          if( (x = qp_code(x)) < 0 || ( y = qp_code(y)) < 0) return -1;
          *t++ = (x << 4) + y , src++;
        }
        else if(*src == '_') 
          *t++ = ' ', src++;
#if 0
        else if(!*src)    /* Thor: no delimiter is not successful */
          return -1;
#endif
        else             /* Thor: *src != '=' '_' */
          *t++ = *src++;
      }
      return t - dst;
    case 'b':            /* Thor: base 64 */
      while(*src && *src != '?') /* Thor: delimiter */ /* Thor.980901: 0也算 */
      {                  /* Thor: pattern & bits are cleared outside */
        int x;
#if 0
        if (!*src) return -1; /* Thor: no delimiter is not successful */
#endif
        x = base64_code(*src++);
        if(x < 0) continue; /* Thor: ignore everything not in the base64,=,.. */
        pattern = (pattern << 6) | x;
        bits += 6;       /* Thor: 1 code gains 6 bits */
        if(bits >= 8)    /* Thor: enough to form a byte */
        {
          bits -= 8;
          *t++ = (pattern >> bits) & 0xff;
        }
      }
      return t - dst;
  }
  return -1;
}

int mmdecode2(src, encode, dst)
  unsigned char *src; /* Thor.980901: src和dst可相同, 但src 一定有?或\0結束 */
  unsigned char encode; /* Thor.980901: 注意, decode出的結果不會自己加上 \0 */
  unsigned char *dst;
{
  unsigned char *t = dst;
  int pattern = 0, bits = 0;
  encode |= 0x20;        /* Thor: to lower */
  switch(encode)
  {
    case 'q':            /* Thor: quoted-printable */
      while(*src) /* Thor: delimiter */
      {                  /* Thor.980901: 0 算是 delimiter */
        if(*src == '=')
        {
          int x = *++src, y = x ? *++src : 0;
          if(isreturn(x)) continue;
          if( (x = qp_code(x)) < 0 || ( y = qp_code(y)) < 0) return -1;
          *t++ = (x << 4) + y , src++;
        }
        else             /* Thor: *src != '=' '_' */
          *t++ = *src++;
      }
      return t - dst;
    case 'b':            /* Thor: base 64 */
      while(*src && *src != '?') /* Thor: delimiter */ /* Thor.980901: 0也算 */
      {                  /* Thor: pattern & bits are cleared outside */
        int x;
        x = base64_code(*src++);
        if(x < 0) continue; /* Thor: ignore everything not in the base64,=,.. */
        pattern = (pattern << 6) | x;
        bits += 6;       /* Thor: 1 code gains 6 bits */
        if(bits >= 8)    /* Thor: enough to form a byte */
        {
          bits -= 8;
          *t++ = (pattern >> bits) & 0xff;
        }
      }
      return t - dst;
  }
  return -1;
}

void
str_decode(str)
  unsigned char *str;
{
  int adj;
  unsigned char *src, *dst;
  unsigned char *buf;
  int size;
  
  size = strlen(str);

  buf = (char *)malloc(strlen(str)+1);

  src = str;
  dst = buf;
  adj = 0;

  while (*src && (dst - buf) < size)
  {
    if (*src != '=')
    {                        /* Thor: not coded */
      unsigned char *tmp = src;
      while(adj && *tmp && isspace(*tmp)) tmp++;
      if(adj && *tmp=='=')
      {                     /* Thor: jump over space */
        adj = 0;
        src = tmp;
      }
      else
        *dst++ = *src++;
      /* continue;*/        /* Thor: take out */
    }
    else                    /* Thor: *src == '=' */
    {
      unsigned char *tmp = src + 1;
      if(*tmp == '?')       /* Thor: =? coded */
      {
        /* "=?%s?Q?" for QP, "=?%s?B?" for BASE64 */
        tmp ++;
        while(*tmp && *tmp != '?') tmp++;
        if(*tmp && tmp[1] && tmp[2]=='?') /* Thor: *tmp == '?' */
        {
          int i = mmdecode(tmp + 3, tmp[1], dst);
          if (i >= 0)
          {
            tmp += 3;       /* Thor: decode's src */
#if 0
            while(*tmp++ != '?'); /* Thor: no ? end, mmdecode -1 */
#endif
            while(*tmp && *tmp++ != '?'); /* Thor: no ? end, mmdecode -1 */
                            /* Thor.980901: 0 也算 decode 結束 */
            if(*tmp == '=') tmp++;
            src = tmp;      /* Thor: decode over */
            dst += i;
            adj = 1;        /* Thor: adjcent */
          }
        }
      }
 
      while(src != tmp)     /* Thor: not coded */
        *dst++ = *src++;
    }
  }
  *dst = 0;
  strcpy(str, buf);
  free(buf);
}

#if 0
void 
main()
{
  char buf2[1024]="=?Big5?B?pl7C0CA6IFtNYXBsZUJCU11UbyB5dW5sdW5nKDE4SzRGTE0pIFtWQUxJ?=\n\t=?Big5?B?RF0=?=";
  char buf[1024];
  str_decode(buf2);
  puts(buf2);
  while(gets(buf))
  {
    str_decode(buf);
    puts(buf);
  }
}
#endif

#if 0
void
str_decode(str)
  unsigned char *str;
{
  int code, c1, c2, c3, c4;
  unsigned char *src, *dst;
  unsigned char buf[256];

#define	IS_QP		0x01
#define	IS_BASE64	0x02
#define	IS_TRAN		0x04

  src = str;
  dst = buf;
  code = 0;

  while (c1 = *src++)
  {
    if (c1 == '?' && *src == '=')
    {
      if (code)
      {
	code &= IS_TRAN;
	if (*++src == ' ')
	  src++;
      }
      continue;
    }

    if (c1 == '\n')		/* chuan: multi line encoding */
      goto line;

    if (code & (IS_QP | IS_BASE64))
    {
      if (c1 == '_')
      {
	*dst++ = ' ';
	continue;
      }

      if ((c1 == '=') && (code & IS_QP))
      {
	if (!(c1 = *src))
	  break;

	if (!(c2 = *++src))
	  break;

	src++;
	*dst++ = (qp_code(c1) << 4) | qp_code(c2);
	code |= IS_TRAN;
	continue;
      }

      if (code & IS_BASE64)
      {
	c2 = *src++;

	if (c1 == '=' || c2 == '=')
	{
	  code ^= IS_BASE64;
	  continue;
	}

	if (!(c3 = *src++))
	  break;

	if (!(c4 = *src++))
	  break;

	c2 = base64_code(c2);
	*dst++ = (base64_code(c1) << 2) | ((c2 & 0x30) >> 4);

	if (c3 == '=')
	{
	  code ^= IS_BASE64;
	}
	else
	{
	  c3 = base64_code(c3);
	  *dst++ = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);

	  if (c4 == '=')
	    code ^= IS_BASE64;
	  else
	    *dst++ = ((c3 & 0x03) << 6) | base64_code(c4);
	}

	code |= IS_TRAN;
	continue;
      }
    }

    /* "=?%s?Q?" for QP, "=?%s?B?" for BASE64 */

    if ((c1 == '=') && (*src == '?'))
    {
      c2 = 0;

      for (;;)
      {
	c1 = *++src;
	if (!c1)
	  goto home;

	if (c1 == '?')
	{
	  if (++c2 >= 2)
	    break;

	  continue;
	}

	if (c2 == 1)
	{
	  if (c1 == 'Q')
	    code = IS_QP;
	  else if (c1 == 'B')
	    code = IS_BASE64;
	}
      }

      src++;
      continue;
    }

    *dst++ = c1;
  }

home:

  if (code & IS_TRAN)
  {
line:
    *dst = '\0';
    strcpy(str, buf);
  }
}
#endif

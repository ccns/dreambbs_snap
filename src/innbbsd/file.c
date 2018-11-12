#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <varargs.h>
#define MAXARGS     100

/*
 * isfile is called by isfile(filenamecomp1, filecomp2,  filecomp3, ...,
 * (char *)0); extern "C" int isfile(const char *, const char *[]) ;
 */


#ifndef	MapleBBS
char FILEBUF[4096];


static char DOLLAR_[8192];
char *
getstream(fp)
  FILE *fp;
{
  return fgets(DOLLAR_, sizeof(DOLLAR_) - 1, fp);
}
#endif

/*
 * The same as sprintf, but return the new string
 * fileglue("%s/%s",home,".newsrc");
 */

char *
fileglue(va_alist)
va_dcl
{
  va_list ap;
  register char *fmt;
  static char gluebuffer[256];

  va_start(ap);
  fmt = va_arg(ap, char *);
  vsprintf(gluebuffer, fmt, ap);
  va_end(ap);
  return gluebuffer;
}

long
filesize(filename)
  char *filename;
{
  struct stat st;

  if (stat(filename, &st))
    return 0;
  return st.st_size;
}

int 
iszerofile(filename)
  char *filename;
{
  struct stat st;

  if (stat(filename, &st))
    return 0;
  if (st.st_size == 0)
    return 1;
  return 0;
}

int 
isfile(filename)
  char *filename;
{
  struct stat st;

  if (stat(filename, &st))
    return 0;
  if (!S_ISREG(st.st_mode))
    return 0;
  return 1;
}


int 
isdir(filename)
  char *filename;
{
  struct stat st;

  if (stat(filename, &st))
    return 0;
  if (!S_ISDIR(st.st_mode))
    return 0;
  return 1;
}


#ifndef	MapleBBS
int 
isfilev(va_alist)
va_dcl
{
  va_list ap;
  struct stat st;
  char *p;
  va_start(ap);

  FILEBUF[0] = '\0';
  while ((p = va_arg(ap, char *)) != (char *) 0)
  {
    strcat(FILEBUF, p);
  }
  printf("file %s\n", FILEBUF);

  va_end(ap);
  return isfile(FILEBUF);
}


int 
isdirv(va_alist)
va_dcl
{
  va_list ap;
  struct stat st;
  char *p;
  va_start(ap);

  FILEBUF[0] = '\0';
  while ((p = va_arg(ap, char *)) != (char *) 0)
  {
    strcat(FILEBUF, p);
  }

  va_end(ap);
  return isdir(FILEBUF);
}

unsigned long 
mtime(filename)
  char *filename;
{
  struct stat st;
  if (stat(filename, &st))
    return 0;
  return st.st_mtime;
}

unsigned long 
mtimev(va_alist)
va_dcl
{
  va_list ap;
  struct stat st;
  char *p;
  va_start(ap);

  FILEBUF[0] = '\0';
  while ((p = va_arg(ap, char *)) != (char *) 0)
  {
    strcat(FILEBUF, p);
  }

  va_end(ap);
  return mtime(FILEBUF);
}

unsigned long 
atime(filename)
  char *filename;
{
  struct stat st;
  if (stat(filename, &st))
    return 0;
  return st.st_atime;
}

unsigned long 
atimev(va_alist)
va_dcl
{
  va_list ap;
  struct stat st;
  char *p;
  va_start(ap);

  FILEBUF[0] = '\0';
  while ((p = va_arg(ap, char *)) != (char *) 0)
  {
    strcat(FILEBUF, p);
  }

  va_end(ap);
  return atime(FILEBUF);
}
#endif


/* #undef TEST */

#ifdef TEST
main(argc, argv)
  int argc;
  char **argv;
{
  int i;
  if (argc > 3)
  {
    if (isfilev(argv[1], argv[2], (char *) 0))
      printf("%s %s %s is file\n", argv[1], argv[2], argv[3]);
    if (isdirv(argv[1], argv[2], (char *) 0))
      printf("%s %s %s is dir\n", argv[1], argv[2], argv[3]);
    printf("mtime %d\n", mtimev(argv[1], argv[2], (char *) 0));
    printf("atime %d\n", atimev(argv[1], argv[2], (char *) 0));
  }
  printf("fileglue %s\n", fileglue("%s/%s", "home", ".test"));
}
#endif

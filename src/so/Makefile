# ------------------------------------------------------ #
#  Makefile	( NTHU CS MapleBBS Ver 2.36 )		 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #

## ------------------------------------------------------
## SunOS 4.1.x
## ------------------------------------------------------
##OS_DEF	= -DSunOS -DTWO_LONGJMP_ARGS
#CC	= gcc
##CPROTO	= cproto -E"gcc -pipe -E" # -s -v
## Thor.980903: lkchu patch -I../include
#CPROTO	= cproto -E"gcc -pipe -E" -I../include # -s -v
##CFLAGS	= -O2 -s -pipe -fomit-frame-pointer -fforce-mem -funroll-loops -Wunused
#CFLAGS	= -O2 -pipe -fomit-frame-pointer -Wunused -I../include
##CFLAGS	= -O2 -s -pipe -fomit-frame-pointer -fstrength-reduce
##LIBS	= # gmalloc.o #-ltermcap 
#LDFLAGS = -s -L../lib -ldao


# ------------------------------------------------------
# Solaris 2.x
# ------------------------------------------------------
# OS_DEF   = -DSOLARIS -DSYSV -DREMAP_LSEEK
# CC       = gcc 
# CFLAGS    = -I/usr/ucbinclude -traditional 
# LIBS     = -ltermcap -lsocket -lnsl
#OS_DEF	= -DSunOS -DTWO_LONGJMP_ARGS
##CC	= gcc
#CPROTO	= cproto -E"gcc -pipe -E" # -s -v
# Thor.980903: lkchu patch -I../include
##CPROTO	= cproto -E"gcc -pipe -E" -I../include # -s -v
#CFLAGS	= -O2 -s -pipe -fomit-frame-pointer -fforce-mem -funroll-loops -Wunused
##CFLAGS	= -O2 -pipe -fomit-frame-pointer -Wunused -I../include -DSOLARIS
#CFLAGS	= -O2 -s -pipe -fomit-frame-pointer -fstrength-reduce
#LIBS	= # gmalloc.o #-ltermcap 
##LDFLAGS = -lresolv -lsocket -ldl -s -L../lib -ldao -lelf
#LDFLAGS = -lresolv -lsocket -s -L../lib -ldao



# ------------------------------------------------------
# Linux : please remove -lrpcsvc in 'bbsrf' section
# ------------------------------------------------------
#OS_DEF   = -DLINUX -DTWO_LONGJMP_ARGS 
#CC       = gcc 
#CFLAGS    = -DLINUX -DREDHAT -O2 -pipe -fomit-frame-pointer -Wunused -I../include
#LDFLAGS	= -s -L../lib -ldao -lcrypt -lresolv -ldl -rdynamic


# ------------------------------------------------------
# NetBSD, 386BSD, FreeBSD 
# ------------------------------------------------------
#OS_DEF   = -DNETBSD -DTWO_LONGJMP_ARGS 
#CC       = gcc 
#CFLAGS    = -DBSD44 -DTWO_LONGJMP_ARGS -O2 -pipe -fomit-frame-pointer -Wunused -I../include
#LDFLAGS	= -s -L../lib -ldao -lcrypt -export-dynamic
#LIBS	= -lncurses -DMINICURSES -DLOG -DSYSVIPC

# ------------------------------------------------------
# HP-UX
# ------------------------------------------------------
#OS_DEF	  = -DHP_UX -DSYSV 
#CC       = gcc 
#CFLAGS    = -traditional -O2 
#LIBS     = -ltermcap 


# ------------------------------------------------------
# AIX
# ------------------------------------------------------
#OS_DEF	 = -DAIX -DSYSV -DTWO_LONGJMP_ARGS  
#CC       = cc 
#CFLAGS    = -O 
#LIBS     = -ltermcap -lbsd
CFLAGS  = -O2 -o

STRIP   = /usr/bin/strip

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

HDR = 	bbs.h config.h global.h modes.h perm.h struct.h

SO =	chat.so vote.so bwboard.so guessnum.so bbcall.so railway.so puzzle.so\
	emailpage.so chess.so admin.so chatmenu.so contact.so mailgem.so\
	memorandum.so aloha.so newboard.so violate.so song.so same_mail.so\
	classtable.so lovepaper.so showvote.so sec_hand.so list.so km.so\
	pnote.so passwd.so pop3mail.so imap4mail.so adminutil.so ascii.so\
	ueequery.so fortune.so ueequery2.so mine.so bbsnet.so bj.so pydict.so\
	dreye.so news_viewer.so icq.so star.so dictd.so classtable2.so \
	observe.so pip.so grade.so graduate.so qkmj.so brdstat.so

EXE =	so

ETC =	Makefile $(HDR)

.SUFFIXES: .o .c .ln .x .so

.c.o:   ;   $(CC) $(MAKEFLAG) $(CFLAGS) -c $*.c
.c.x:   ;   $(CPROTO) -o $*.x $*.c
.c.ln:  ;   lint -abhi $*.c
.o.so:	;   ld -s -G $*.o -o $*.so -L../lib -ldao


all: 
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc" 
	@echo " make linux   : for RedHat 6.0"
	@echo " make redhat7 : for RedHat 7.0"
	@echo " make solaris : for Sun-OS 5.x gcc" 
	@echo " make sol-x86 : for Solaris 7 x86"
	@echo " make freebsd : for BSD 4.4 systems"
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above lists"

sun:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(EXE)

linux:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DLINUX -DREDHAT -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -lcrypt -lresolv -ldl -rdynamic" $(EXE)

redhat7:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DLINUX -DREDHAT -DREDHAT7 -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -lcrypt -lresolv -ldl -rdynamic" $(EXE)

solaris:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DSOLARIS -DSYSV -DREMAP_LSEEK -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -ltermcap -lsocket -lnsl -lresolv -lelf -ldl" $(EXE)

sol-x86:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -E\" -I../include" CFLAGS="-DSOLARIS -DSYSV -DREMAP_LSEEK -O2 -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -ltermcap -lsocket -lnsl -lresolv -lelf -ldl" $(EXE)

freebsd:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" MAKEFLAG=$(MAKEFLAG) CFLAGS="-DBSD44 -DTWO_LONGJMP_ARGS -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -lcrypt -export-dynamic" $(EXE)

bsd:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DBSD44 -DTWO_LONGJMP_ARGS -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(EXE)

so: $(SO)

install: $(EXE)
	csh -c "rm -f ../../bin/*.so"
	csh -c "cp ../../src/so/*.so ../../bin/"

clean: /tmp
	rm -fr $(SO) proto.x *~ *.o DEADJOE

clear:
	rm -rf *~ DEADJOE

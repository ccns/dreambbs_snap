# ------------------------------------------------------- #
#  innbbsd/Makefile	( NTHU CS MapleBBS Ver 2.36 )     #
# ------------------------------------------------------- #
#  target : Makefile for 轉信程式 innbbsd by skhuang      #
#  create : 95/03/29                                      #
#  update : 95/12/15                                      #
# ------------------------------------------------------- #

####################################################
# this is a bbs <--> news gateway
#####################################################
VERSION=0.50beta-4
LASTVERSION=0.50beta-3
ADMINUSER= root.bbs@bbs.yzu.edu.tw
BBSHOME = /home/bbs
YACC = bison -y
BBS_SRC = ../include
BBSADDR = bbs.yzu.edu.tw
#######################
# if you want to make a "bbsnnrp" only, 
# change TARGET=$(BBSNNRP)
TARGET  = $(INNBBSD) #$(BBSLINK) $(BBSNNRP)
####################################################
CC=gcc
INSTALL=cp
#
#OPT= -g -pg 
OPT= -O2 -s -pipe -fomit-frame-pointer

# You must choose at least one: PhoenixBBS, PalmBBS, PivotBBS, SecretBBS 
# or PowerBBS
#
# only Phoenix and Power BBS are supported and others haven't been tested
########################################################################
#BBS_DEP = PhoenixBBS
#BBS_REC = $(BBS_SRC)/record.o r_port.o
#BBS_POST = innd/bbspost
################################################
# For PowerBBS, 
#BBS_DEP = PowerBBS 
#BBS_REC = port.o
#BBS_POST =
################################################
# For PalmBBS
#BBS_DEP = PalmBBS
#BBS_REC = r_port.o $(BBS_SRC)/libbbs.a
#BBS_POST = 
################################################
# For Secret BBS, 
#BBS_DEP = SecretBBS 
#BBS_REC = $(BBS_SRC)/record.o r_port.o
#BBS_POST = innd/bbspost
################################################
# For Pivot BBS, 
#BBS_DEP = PivotBBS
#BBS_REC = $(BBS_SRC)/record.o r_port.o
#BBS_POST = innd/bbspost
################################################
# For Maple BBS, 
BBS_DEP = MapleBBS 
# BBS_REC = $(BBS_SRC)/record.o r_port.o
################################################

#########################
#
###############
DEBUGOBJ = /usr/lib/debug/mallocmap.o
CFLAGS= -c -I. -I$(BBS_SRC) -D$(BBS_DEP) $(EXTRAFLAGS) -D_PATH_BBSHOME=\"$(BBSHOME)\" -DVERSION=\"$(VERSION)\" -DADMINUSER=\"$(ADMINUSER)\" -DBBSADDR=\"$(BBSADDR)\" #-DDBZDEBUG
#
####################################################

OBJS = inndchannel.o innbbsd.o connectsock.o rfc931.o \
       daemon.o file.o pmain.o his.o dbz.o \
       closeonexec.o dbztool.o inntobbs.o echobbslib.o receive_article.o \
       parsdate.o # $(BBS_REC)
SRCS = inndchannel.c innbbsd.c connectsock.c rfc931.c \
       daemon.c file.c pmain.c parsdate.y his.c dbz.c \
       closeonexec.c dbztool.c inntobbs.c bbslib.c receive_article.c \
       port.c

MOBJS    = makedbz.o bbslib.o file.o dbz.o closeonexec.o 
HOBJS    = mkhistory.o bbslib.o file.o his.o dbz.o port.o closeonexec.o
DBZOBJS  = dbzserver.o dbzinnbbsd.o pmain.o connectsock.o bbslib.o his.o \
	   daemon.o file.o dbz.o dbztool.o rfc931.o port.o closeonexec.o
NNRPOBJS = bbsnnrp.o pmain.o bbslib.o connectsock.o file.o 
LINKOBJS = bbslink.o pmain.o inntobbs.o echobbslib.o connectsock.o file.o port.o
CTLOBJS  = ctlinnbbsd.o pmain.o bbslib.o connectsock.o file.o 
INNBBSD  = innbbsd ctlinnbbsd bbsnnrp $(BBSLINK) \
	   # makedbz mkhistory dbzserver $(BBS_POST) dbz_query_sample
BBSNNRP  = bbsnnrp
BBSLINK  = bbslink
#BBSLINK  = 
EXES     = $(TARGET)

.c.o:
	$(CC) $(MAKEFLAG) $(OPT) $(CFLAGS) $*.c


all: 
	@echo "Your Configurations are listed as follows" 
	@echo "  CC = $(CC)"
	@echo "  BBSHOME = $(BBSHOME)"
	@echo "  BBS_SRC = $(BBS_SRC)"
	@echo "  BBS_DEP = $(BBS_DEP)"
	@echo " BBS_POST = $(BBS_POST)"
	@echo "ADMINUSER = $(ADMINUSER)"
	@echo "  BBSADDR = $(BBSADDR)"
	@echo "Change them in Makefile if not correct."
	@echo ""
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc" 
	@echo " make linux   : for Linux 1.x, RedHat 5.1"
	@echo " make redhat7 : for RedHat 7.0"
	@echo " make solaris : for Sun-OS 5.x gcc" 
	@echo " make sol-x86 : for Solaris 2.x x86"
	@echo " make aix     : for IBM AIX" 
	@echo " make hpux    : for HP UX"
	@echo " make irix    : for IRIS"
	@echo " make alpha   : for DEC Alpha running OSF/1" 
	@echo " make freebsd : for BSD 4.4 systems" 
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above listts" 
	@echo " make sysv    : for SYSV systems, cc or gcc, if not in the above lists" 

link:
	rm -f r_port.c dbzserver.c dbzinnbbsd.c  echobbslib.c
	ln -s port.c r_port.c
	ln -s inndchannel.c dbzserver.c
	ln -s innbbsd.c dbzinnbbsd.c
	ln -s bbslib.c echobbslib.c

maple:
	@$(MAKE) EXTRAFLAGS="-DMMAP" YACC="bison -y" LDFLAGS="-lxnet -lsocket -lresolv -L../lib -ldao" target

sun: 
	@$(MAKE) EXTRAFLAGS="-DMMAP -DGETRUSAGE -DMALLOCMAP" LDFLAGS="$(DEBUGOBJ)" target


aix: 
	@$(MAKE) CC=cc EXTRAFLAGS="-DMMAP -DGETRUSAGE -DAIX -DSYSV" target

# Thor.981221: Odysseus patch: for porting on linux 
linux: 
	@$(MAKE) EXTRAFLAGS="-DLINUX -DREDHAT -DGETRUSAGE" target LDFLAGS="-L../lib -ldao"

redhat7: 
	@$(MAKE) EXTRAFLAGS="-DLINUX -DREDHAT -DREDHAT7 -DGETRUSAGE" target LDFLAGS="-L../lib -ldao"

hpux: 
	@$(MAKE) OPT= EXTRAFLAGS="-DMMAP -DHPUX -DSYSV" target

irix: 
	@$(MAKE) EXTRAFLAGS="-DMMAP -DIRIX -DSYSV" target

solaris:
	@$(MAKE) EXTRAFLAGS="-DMMAP -DSOLARIS -DSYSV" LDFLAGS="-lsocket -lnsl" YACC="bison -y" target

sol-x86:
	@$(MAKE) EXTRAFLAGS="-DMMAP -DSOLARIS -DSYSV" LDFLAGS="-lsocket -lnsl -L../lib -ldao" OPT="-O2 -s -fomit-frame-pointer" target

freebsd:
	@$(MAKE) CC=cc MAKEFLAG=$(MAKEFLAG) EXTRAFLAGS="-DMapleBBS -DBSD44 -DMMAP -DGETRUSAGE" LDFLAGS="-L../lib -ldao" LIBCRYPT=-lcrypt  target

bsd:
	@$(MAKE) CC=cc EXTRAFLAGS="-DMMAP -DGETRUSAGE" target

alpha:
	@$(MAKE) CC=cc EXTRAFLAGS="-DMMAP -DDEC_ALPHA -DGETRUSAGE" target

sysv: 
	@$(MAKE) OPT= EXTRAFLAGS="-DMMAP -DSYSV" target

target: $(EXES)

r_port.o: r_port.c
	$(CC) $(OPT) $(CFLAGS) -DWITH_RECORD_O r_port.c 

echobbslib.o: echobbslib.c
	$(CC) $(MAKEFLAG) $(OPT) $(CFLAGS) -DWITH_ECHOMAIL echobbslib.c

tar: 
	test -d innbbsd-$(VERSION) || mkdir innbbsd-$(VERSION)
	cp *.c *.h Makefile innbbsd-$(VERSION)
	tar zcf innbbsd-$(VERSION).tar.gz innbbsd-$(VERSION)
	cp innbbsd-$(VERSION).tar.gz /net/ftphome/CSIE/innbbsd

patch:
	rm -rf /tmp/innbbsd-$(LASTVERSION)
	zcat /net/ftphome/ftp-data/bbs/misc/innbbsd/innbbsd-$(LASTVERSION).tar.gz | (cd /tmp; tar xf -)	
	(cd /tmp/innbbsd-$(VERSION); make clean; rm -f r_port.c dbzserver.c dbzinnbbsd.c  echobbslib.c )
	(cd /tmp/innbbsd-$(LASTVERSION); make clean; rm -f r_port.c dbzserver.c dbzinnbbsd.c  echobbslib.c )
	(cd /tmp; diff -rcs innbbsd-$(LASTVERSION) innbbsd-$(VERSION) > /tmp/innbbsd-patch.$(LASTVERSION)-$(VERSION); echo "diff -rcs")
	(cd /tmp/innbbsd-$(VERSION); ln -s port.c r_port.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s inndchannel.c dbzserver.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s innbbsd.c dbzinnbbsd.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s bbslib.c echobbslib.c)


distribution:
	rm -rf  /tmp/innbbsd-$(VERSION)
	test -d /tmp/innbbsd-$(VERSION) || mkdir /tmp/innbbsd-$(VERSION)
	test -d /tmp/innbbsd-$(VERSION)/innd || mkdir /tmp/innbbsd-$(VERSION)/innd
	rm -f r_port.c
	rm -f dbzserver.c
	rm -f dbzinnbbsd.c
	rm -f echobbslib.c
	cp -r doc hisconvert FEATURES CHANGES README* TODO *.c *.h *.y Makefile boards /tmp/innbbsd-$(VERSION)
	(cd /tmp/innbbsd-$(VERSION); ln -s port.c r_port.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s inndchannel.c dbzserver.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s innbbsd.c dbzinnbbsd.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s bbslib.c echobbslib.c)
	(cd innd ; cp -r Makefile README innbbs.conf in.bntpd mailpost *.pl bntplink bntplink.palmbbs *.active* *.c *.y *.bbs src /tmp/innbbsd-$(VERSION)/innd)
	(cd /tmp/innbbsd-$(VERSION); make clean)
	(cd /tmp; tar zcf innbbsd-$(VERSION).tar.gz innbbsd-$(VERSION) ) 
	ln -s port.c r_port.c
	ln -s inndchannel.c dbzserver.c
	ln -s innbbsd.c dbzinnbbsd.c
	ln -s bbslib.c echobbslib.c

remotetest: distribution
	/usr/local/krb4/rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz skhuang:/tmp
	/usr/local/krb4/rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz linux:/tmp
	mv /u/staff/skhuang/.tcshrc /u/staff/skhuang/skhuang.tcshrc
	rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz ccsun36:/tmp
	rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz ccibm1:/tmp
	mv /u/staff/skhuang/skhuang.tcshrc /u/staff/skhuang/.tcshrc
	rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz cciris3:/tmp

ftp: distribution patch
	rsh ccsun42 mv /home8/ftp/pub/bbs/misc/innbbsd/innbbsd-$(VERSION).tar.gz /home8/ftp/pub/bbs/misc/innbbsd/.innbbsd-$(VERSION).tar.gz
	rcp /tmp/innbbsd-$(VERSION).tar.gz ccsun42:/home8/ftp/pub/bbs/misc/innbbsd
	rcp README* ccsun42:/home8/ftp/pub/bbs/misc/innbbsd
	rcp /tmp/innbbsd-patch.$(LASTVERSION)-$(VERSION) ccsun42:/home8/ftp/pub/bbs/misc/innbbsd

innbbspost.o: innbbspost.c
	$(CC) $(CFLAGS) -I$(BBS_SRC) -c innbbspost.c 

makedbz: $(MOBJS)
	$(CC) $(OPT) -o makedbz $(MOBJS) $(LDFLAGS)

dbzserver.o: dbzserver.c
	$(CC) $(CFLAGS) -DDBZSERVER -I$(BBS_SRC) -c dbzserver.c   

dbzinnbbsd.o: dbzinnbbsd.c
	$(CC) $(CFLAGS) -DDBZSERVER -I$(BBS_SRC) -c dbzinnbbsd.c   

dbzserver: $(DBZOBJS)
	$(CC) $(OPT) -o dbzserver $(DBZOBJS) $(LDFLAGS)

bbsnnrp: $(NNRPOBJS)
	$(CC) $(MAKEFLAG) -o bbsnnrp $(NNRPOBJS) $(OPT) $(LDFLAGS)

bbslink: $(LINKOBJS)
	$(CC) $(MAKEFLAG) -o bbslink $(LINKOBJS) $(OPT) $(LDFLAGS)

ctlinnbbsd: $(CTLOBJS)
	$(CC) $(MAKEFLAG) $(OPT) -o ctlinnbbsd $(CTLOBJS) $(LDFLAGS)

mkhistory: $(HOBJS)
	$(CC) $(OPT) -o mkhistory $(HOBJS) $(LDFLAGS)

dbz_query_sample: dbz_query_sample.o
	$(CC) $(OPT) -o dbz_query_sample dbz_query_sample.o $(LDFLAGS)


innbbsd: $(OBJS)
#	mv tnrpd.exe tnrpd.exe.old
	$(CC) $(MAKEFLAG) -o innbbsd $(OBJS) $(OPT) $(LDFLAGS)

install:
#	@echo "Read README and do the following manually"
#	@echo "copy innbbsd bbsnnrp mkhistory to ../../innd"	
	test -d ../../innd || mkdir ../../innd
	-mv -f ../../innd/innbbsd ../../innd/innbbsd.OLD
	$(INSTALL) innbbsd ../../innd/innbbsd
	-mv -f ../../innd/bbsnnrp ../../innd/bbsnnrp.OLD
	$(INSTALL) bbsnnrp ../../innd/bbsnnrp
	-mv -f ../../innd/ctlinnbbsd ../../innd/ctlinnbbsd.OLD
	$(INSTALL) ctlinnbbsd ../../innd/ctlinnbbsd
	-mv -f ../../innd/bbslink ../../innd/bbslink.OLD 
	$(INSTALL) bbslink ../../innd/bbslink

clean:
	rm -f *.o $(EXES) core innd/src/*.o *~ DEADJOE
	rm -f r_port.c dbzserver.c dbzinnbbsd.c  echobbslib.c

depend:
	 makedepend -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.


#-----------------------------------------------------------------------------
#       WWW to LDAP / X.500 gateway makefile
#-----------------------------------------------------------------------------
#
# $Id: Makefile,v 1.2 2001/04/26 22:16:53 dekarl Exp $
#

# Where to install web500gw
INSTDIR = /usr/local/etc

# Where to install the manual web500gw.8
MANDIR	= /usr/local/man/man8

# Where to install web500gw's config/filter/template/help/message files
# It's recommended to use an own directory for it.
WEB500GWDIR	= $(INSTDIR)/web500

# What language dependent files should be installed? 
# "" stands for english (default) versions
SUFFIX = "" de es fr


# DEFINES - Enable/disable some features
# -DMODIFY  - enables 'MODIFY" and 'ADD' of entries
# -DSUPPORT_OLD_URLS - if you're upgrading from older version you may wish this
# -DWEB500GW_DEBUG  - include web500gw debugging code
#
# Please take a look at config.h to change some default values described there.
# All the values could be overriden within the web500gw.conf file

DEFINES	= -DMODIFY -DWEB500GW_DEBUG -DSUPPORT_OLD_URLS

########################################################
#    What LDAP package you have installed ... and where?
########################################################
#
## If you use OpenLDAP 1.0 or U-Mich LDAP 3.3 package (recommended)
# If the LDAP include files and libraries installed in standard places 
# (/usr/lib, /usr/include) use this:
LDAPINCL = 
LDAPLIBS = -lldap -llber
# set LDAPVERSION (32 for ldap-3.2, 33 for 3.3 etc)
LDAPVERSION = 33
VERSION = 3.3

# To use LDAP include files and libraries from the LDAP source directory 
# use this:
## Tailor this: directory of openldap-1.0 or ldap-3.3 distribution
#LDAPDIR = ../openldap-1.0.2
#LDAPINCL  = -I$(LDAPDIR)/include
#LDAPLIBS = -L$(LDAPDIR)/libraries -lldap -llber
#LDAPVERSION = `sed -e 's/\.//' $(LDAPDIR)/build/version`
#VERSION = `cat $(LDAPDIR)/build/version`

# T.61 <-> ISO-8859-1 translation: This is built in from LDAP 3.2.
# If you didn't define this translation there comment out the following line:
STR_TRANSLATION = -DSTR_TRANSLATION
# If you defined LDAP_DEBUG there and want to enable LDAP debugging
# uncomment the following line:
#LDAP_DEBUG = -DLDAP_DEBUG

#################
## If you have LDAP from an ISODE Consortium Release
#VERSION = IC
#LDAPVERSION = IC
#LIBDIR = /usr/local/ic/usr/lib
#INCDIR = /usr/local/ic/usr/include
# LDAPLIBS = -L$(LIBDIR) -R$(LIBDIR) -lldap -llber -lisode
# LDAPINCL = -I$(INCDIR) -I$(INCDIR)/isode/ldap

############# End of LDAP package selection

# platform specific:
# Linux
PLATFORMCFLAGS= -Dlinux -DSYSV 
# Solaris 2
# PLATFORMCFLAGS= -Dsunos5
# PLATFORMLDFLAGS= -lsocket -lnsl
# SunOS 4
# PLATFORMCFLAGS= -Dsunos4 -DUSE_SYSCONF
# HP-UX 10
#PLATFORMCFLAGS= -Dhpux
# HP-UX 9
# PLATFORMCFLAGS= -Dhpux9
# AIX
# PLATFORMCFLAGS= -Daix
# NetBSD:
#PLATFORMCFLAGS=
#PLATFORMLDFLAGS= -lcompat
# OSF/1
#PLATFORMCFLAGS= -Dosf -DSYSV
#PLATFORMLDFLAGS=
# Unixware 7
#PLATFORMCFLAGS= -DUSE_SYSCONF -Dunixware7 -DUSE_SETSID
#PLATFORMLDFLAGS= -lsocket -lnsl


ALLDEFINES= $(PLATFORMCFLAGS) -I. $(LDAPINCL) $(DEFINES) \
            -DWEB500GWDIR=\"$(WEB500GWDIR)\" -DLDAPVERSION=$(LDAPVERSION) \
            $(LDAP_DEBUG) $(STR_TRANSLATION)

# when using gcc:
CC	= gcc 
#WARN = -Wall -Wno-implicit
#CFLAGS	= -O4 $(WARN) $(ALLDEFINES) -fpcc-struct-return -fwritable-strings
#CFLAGS	= -g $(WARN) $(ALLDEFINES) -fpcc-struct-return -fwritable-strings
CFLAGS = -g $(WARN) $(ALLDEFINES)

# or cc:
#CC	= cc
#CFLAGS	= -O $(ALLDEFINES)
# for debugging:
#CFLAGS	= -g $(ALLDEFINES)

################ end of configuration area ###############
# ALDFLAGS are always passed to ld
ALDFLAGS        = $(PLATFORMLDFLAGS)

INSTALL = util/install.sh
MKDIR   = mkdir
# LIBEFENCE = -lefence

LIBS	= $(LDAPLIBS) $(KRBLIBFLAG) $(KRBLIBS) $(LIBEFENCE)

SRCS	=	web500gw.c read.c search.c bind.c modify.c add.c delete.c modrdn.c\
            navigation.c util.c web_util.c dir_util.c ldap2html.c init.c \
            config.c detach.c setproctitle.c
OBJS	= 	web500gw.o read.o search.o bind.o modify.o add.o delete.o modrdn.o \
            navigation.o util.o web_util.o dir_util.o ldap2html.o messages.o \
            init.o config.o detach.o setproctitle.o gwversion.o

all:	web500gw
	@echo "Compilation ready, now need a 'make install' (maybe as root)"

messages.h:  makemessages.sh $(SRCS)
	./makemessages.sh $(SRCS)

messages.o:	messages.c

$(OBJS): messages.h web500gw.h

#web500gw.o:  messages.h web500gw.h

web500gw:	Makefile $(OBJS)
	$(CC) $(ALDFLAGS) -o $@ $(OBJS)  $(LIBS)

#$(CC) $(ALDFLAGS) -o $@ $(OBJS) -R/usr/local/ic/usr/lib -L/usr/local/ic/usr/lib -lldap -llber -lisode


gwversion.c: web500gw.o detach.o setproctitle.o web500gw.h Versiongw.c
	rm -f $@
	(u=$${USER-root} v=$(VERSION) d=`pwd` h=`hostname` t=`date`; \
	sed -e "s|%WHEN%|$${t}|" \
	-e "s|%WHOANDWHERE%|$${u}@$${h}:$${d}|" \
	-e "s|%VERSION%|$${v}|" \
	< Versiongw.c > $@)

install:	$(INSTDIR)/web500gw install-etc install-man
	@echo ""
	@echo "You should edit $(WEB500GWDIR)/web500gw.conf to your needs"
	@echo "To start web500gw: $(INSTDIR)/web500gw [-d debuglevel]"

$(INSTDIR)/web500gw:	web500gw
	@echo ""
	@if [ ! -d $(INSTDIR) ]; then $(MKDIR) -p $(INSTDIR); fi
	@echo "Installing web500gw to $(INSTDIR)"
	$(INSTALL) $(INSTALLFLAGS) -m 755 web500gw $(INSTDIR)
	@echo "... done!"

install-etc:;
	@echo ""
	@if [ ! -d $(WEB500GWDIR) ]; then $(MKDIR) -p $(WEB500GWDIR); fi
	@echo "Installing web500gw config/template/filter files to $(WEB500GWDIR)"
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/web500gw.conf $(WEB500GWDIR)
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/ldapfilter.conf $(WEB500GWDIR)
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/ldapfilter.conf.internal $(WEB500GWDIR)
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/ldaptemplates.conf $(WEB500GWDIR)
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/ldaptemplates.conf.internal $(WEB500GWDIR)
	@echo ""
	@echo "Installing web500gw help/message/friendly files to $(WEB500GWDIR)"
	@for i in $(SUFFIX); do \
	if [ "x$$i" = "x" ]; then echo " english"; SUFF="";\
	else echo " $$i"; SUFF=".$$i"; fi; \
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/web500gw.help$$SUFF $(WEB500GWDIR); \
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/web500gw.messages$$SUFF $(WEB500GWDIR); \
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/web500gw.attr$$SUFF $(WEB500GWDIR); \
	$(INSTALL) $(INSTALLFLAGS) -m 644 etc/ldapfriendly$$SUFF $(WEB500GWDIR); \
	done;
	@echo "... done!"

install-man:;
	@echo ""
	@echo "Installing web500gw.8 to $(MANDIR)"
	$(INSTALL) $(INSTALLFLAGS) -m 644 doc/web500gw.8 $(MANDIR)
	@echo "... done!"

lint:;
	lint -I. $(SRCS)

5lint:;
	/usr/5bin/lint -I. $(SRCS)

clean:;
	rm -f *.o *.bak */*.bak core a.out gwversion.c messages.[ch] web500gw tmp-version.c msg-tmp*

love:;
	@echo "Not clones."

depend:;
	$(LDAPDIR)/build/mkdep $(CFLAGS) $(SRCS)

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

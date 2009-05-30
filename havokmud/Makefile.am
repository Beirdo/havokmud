ACLOCAL_AMFLAGS = -I scripts
AUTOMAKE_OPTIONS = no-dist
SUBDIRS = libs util corefunc threads plugins

bin_PROGRAMS = havoknew
havoknew_SOURCES = dummy.c

if !HAVE_LIBESMTP
havoknew_LDADD = -L lib -lesmtp
else
havoknew_LDADD = -lesmtp
endif

havoknew_LDADD += -L lib -lhavokcore -lhavokthread -lhavokutils 
havoknew_DEPENDENCIES  = lib/libhavokutils.a lib/libhavokcore.a
havoknew_DEPENDENCIES += lib/libhavokthread.a
havoknew_LDFLAGS = ${PTHREAD_CFLAGS} -g -rdynamic ${LDFLAGS}
havoknew_LINK = ${CCLD} -o $@

include Makefile.local

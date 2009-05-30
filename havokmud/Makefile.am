ACLOCAL_AMFLAGS = -I scripts
AUTOMAKE_OPTIONS = no-dist
SUBDIRS = libs util corefunc threads plugins

bin_PROGRAMS = havoknew
havoknew_SOURCES = dummy.c
havoknew_LDADD = -L lib threads/*.o corefunc/*.o -lhavokutils 
havoknew_LDFLAGS = ${PTHREAD_CFLAGS} -g -rdynamic ${LDFLAGS}
havoknew_DEPENDENCIES = lib/libhavokutils.a

include Makefile.local

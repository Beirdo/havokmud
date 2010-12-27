ACLOCAL_AMFLAGS = -I scripts
AUTOMAKE_OPTIONS = no-dist
SUBDIRS = libs protobuf util corefunc threads plugins

bin_PROGRAMS = havoknew
havoknew_SOURCES =

havoknew_LDADD  = util/*.o corefunc/*.o threads/*.o protobuf/src/*.o
havoknew_LDADD += -L lib -lprotobuf -lprotobuf-c -lcJSON -lmongoose

if !HAVE_LIBESMTP
havoknew_LDADD += -L lib -lesmtp
else
havoknew_LDADD += -lesmtp
endif

havoknew_LDFLAGS = ${PTHREAD_CFLAGS} -g ${LDFLAGS} ${PTHREAD_LIBS}

if !USING_CYGWIN
havoknew_LDFLAGS += -rdynamic
endif

havoknew_LINK = ${CCLD} ${havoknew_LDFLAGS} -o $@

include Makefile.local

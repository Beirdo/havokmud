INSTALL = install
LIBDIR = ../../lib
INCDIR = ../../include
CFLAGS = -g -O2 -Wall
RANLIB = ranlib

.PHONY:	all clean

all:	libcJSON.a

install:	all
	${INSTALL} -m 644 libcJSON.a ${LIBDIR}
	${INSTALL} -m 644 cJSON.h ${INCDIR}

uninstall:
	-${RM} ${LIBDIR}/libcJSON.a ${INCDIR}/cJSON.h

libcJSON.a:	cJSON.o
	${AR} rcv $@ $+
	${RANLIB} $@

cJSON.o:	cJSON.c
	${CC} -c -o $@ ${CFLAGS} $+ 

cJSON.c:	cJSON.h

clean:
	-${RM} libcJSON.a cJSON.o



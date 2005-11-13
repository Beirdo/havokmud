CC=gcc
CFLAGS=-O6 -g -I.. -I ../include

OBJS = linked_list.o logging.o queue.o buffer.o

all:		libhavokutils.a

clean:
	-rm libhavokutils.a *.o

libhavokutils.a:	${OBJS}
	ar rcv $@ $+
	ranlib $@

linked_list.o:	linked_list.c linked_list.h ../include/environment.h
logging.o:	logging.c logging.h ../include/environment.h
queue.o:	queue.c queue.h ../include/environment.h
buffer.o:	buffer.c buffer.h ../include/environment.h


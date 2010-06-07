PROTOCC = ../bin/protoc-c
INSTALL = install

PROTO_FILES  = havokrequest.proto settings.proto accounts.proto pcs.proto
PROTO_SRCS   = ${PROTO_FILES:%.proto=src/%.pb-c.c}
PROTO_HDRS   = ${PROTO_SRCS:%.c=%.h}
PROTO_OBJS   = ${PROTO_SRCS:%.c=%.o}

CC = gcc
CFLAGS = -O3 -g -I ../include

all:	${PROTO_SRCS} ${PROTO_OBJS}

src/%.o:	src/%.c
	${CC} -c ${CFLAGS} -o $@ $+

src/%.pb-c.c::	protos/%.proto
	${PROTOCC} --proto_path=protos --c_out=src $<

src/%.pb-c.h::	protos/%.proto
	${PROTOCC} --proto_path=protos --c_out=src $<

src/%.c::	src/%.h

src/havokrequest.pb-c.c::	src/settings.pb-c.h src/accounts.pb-c.h
src/havokrequest.pb-c.c::	src/pcs.pb-c.h

.PHONY: force

clean:	force
	-${RM} ${PROTO_SRCS} ${PROTO_HDRS} ${PROTO_OBJS}

install:	all
	${INSTALL} ${PROTO_HDRS} ../include/

uninstall:	force
	-for i in ${PROTO_HDRS:src/%=../include/%} ; do ${RM} $$i ; done

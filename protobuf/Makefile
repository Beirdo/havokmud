PROTOCC = ../bin/protoc-c
INSTALL = install

PROTO_FILES  = havokrequest.proto settings.proto accounts.proto pcs.proto
PROTO_FILES += havokresponse.proto reqtype.proto
PROTO_SRCS   = ${PROTO_FILES:%.proto=src/%.pb-c.c}
PROTO_HDRS   = ${PROTO_SRCS:%.c=%.h}
PROTO_OBJS   = ${PROTO_SRCS:%.c=%.o}

INSTL_HDRS   = ${PROTO_HDRS:src/%=../include/%}

CC = gcc
CFLAGS = -O3 -g -I ../include

all:	${PROTO_OBJS} ${INSTL_HDRS}

src/%.o:	src/%.c
	${CC} -c ${CFLAGS} -o $@ $+

src/%.pb-c.c::	protos/%.proto
	${PROTOCC} --proto_path=protos --c_out=src $<

src/%.pb-c.h::	protos/%.proto
	${PROTOCC} --proto_path=protos --c_out=src $<

src/%.c::	src/%.h

src/havokrequest.pb-c.c::	src/settings.pb-c.h src/accounts.pb-c.h
src/havokrequest.pb-c.c::	src/pcs.pb-c.h src/reqtype.pb-c.h

src/havokresponse.pb-c.c::	src/settings.pb-c.h src/accounts.pb-c.h
src/havokresponse.pb-c.c::	src/pcs.pb-c.h src/reqtype.pb-c.h

../include/%.h:	src/%.h
	${INSTALL} $+ $@

.PHONY: force

clean:	force
	-${RM} ${PROTO_SRCS} ${PROTO_HDRS} ${PROTO_OBJS}

install:	all
	${INSTALL} ${PROTO_HDRS} ../include/

uninstall:	force
	-${RM} ${INSTL_HDRS}

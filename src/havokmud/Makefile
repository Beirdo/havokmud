SUBDIRS = corefunc objects plugin thread util
CPPFLAGS = -g -std=c++11 -I. -I ../../include

LIBS  = -L thread -L objects -L corefunc -L util
LIBS += -lhavokthread -lhavokobjects -lhavokcorefunc -lhavokutil
LIBS += -lhavokthread -lhavokobjects -lhavokcorefunc -lhavokutil
LIBS += -lboost_thread -lboost_system -lboost_regex -lcrypto
LIBS += -L ../../lib -lesmtp -lmysqlcppconn

.PHONY: force

all:    ${SUBDIRS} testmain.o
	g++ ${CPPFLAGS} -o test testmain.o ${LIBS}

${SUBDIRS}:     force
	${MAKE} -C $@ all

clean:	force
	- [ "${SUBDIRS}" != "" ] && \
 	for i in ${SUBDIRS} ; do \
		${MAKE} -C $$i clean ; \
	done
	${RM} test testmain.o

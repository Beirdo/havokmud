SUBDIRS = corefunc objects plugin thread util
CPPFLAGS = -g -std=c++11 -I.

LIBS  = -L thread -L objects -L corefunc -lhavokthread -lhavokobjects
LIBS += -lhavokcorefunc
LIBS += -lboost_thread -lboost_system -lboost_regex

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

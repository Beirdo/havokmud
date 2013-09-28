SUBDIRS = core objects plugin thread util
CPPFLAGS = -g -std=c++11 -I. -L thread -L objects

.PHONY: force

all:    ${SUBDIRS} testmain.o
	g++ ${CPPFLAGS} -o test testmain.o -lhavokthread -lhavokobjects -lboost_thread -lboost_system -lboost_regex

${SUBDIRS}:     force
	${MAKE} -C $@ all

clean:	force
	- [ "${SUBDIRS}" != "" ] && \
 	for i in ${SUBDIRS} ; do \
		${MAKE} -C $$i clean ; \
	done

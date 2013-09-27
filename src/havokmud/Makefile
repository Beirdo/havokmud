SUBDIRS = core objects plugin thread util

.PHONY: force

all:    ${SUBDIRS}

${SUBDIRS}:     force
	${MAKE} -C $@ all

clean:	force
	- [ "${SUBDIRS}" != "" ] && \
 	for i in ${SUBDIRS} ; do \
		${MAKE} -C $$i clean ; \
	done

AUTOMAKE_OPTIONS = no-dist
EMPTY_AUTOMAKE_TARGETS  = install-info installdirs check installcheck dvi pdf
EMPTY_AUTOMAKE_TARGETS += ps info html tags ctags distdir all clean
EMPTY_AUTOMAKE_TARGETS += all-recursive clean-recursive
RECURSE_AUTOMAKE_TARGETS  = install install-exec install-data
SUBDIRS = 

if !HAVE_LIBESMTP
SUBDIRS += libesmtp-1.0.4
endif

include Makefile.local

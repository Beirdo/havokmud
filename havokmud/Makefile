##
## The compiler your gonna use to compile this with..
##
CC=gcc
##
LD=$(CC)

##  these are architecture dependent flags
CFLAGS = -g -O6 -Wall -Werror -Wno-trigraphs -ansi -pedantic

##  Libraries
LIBS = -lcrypt
# Solaris needs the following
#LIBS = -lcrypt -lsocket -lnsl


SRCS =	act.comm.c act.info.c act.move.c act.obj1.c act.obj2.c act.off.c \
	act.other.c act.social.c act.wizard.c ansi_parser.c board.c bsd.c \
	comm.c constants.c create.c create.help.c create.mob.c create.obj.c \
	db.c fight.c handler.c hash.c heap.c interpreter.c limits.c magic2.c \
	magic3.c magic.c magicutils.c mail.c mindskills1.c mind_use1.c \
	mobact.c modify.c mod.text.c multiclass.c opinion.c paginate.c \
	parser.c reception.c security.c shop.c signals.c skills.c \
	skills_mage.c skills_necromancer.c \
	sound.c \
	spec_assign.c spec_guildmaster.c spec_procs2.c spec_procs3.c \
	spec_procs4.c spec_procs.c spell_parser.c spells1.c spells2.c trap.c \
	utility.c weather.c

OBJS = ${SRCS:.c=.o}
DEPS = ${SRCS:.c=.d}

all : dmserver

dmserver : ${OBJS}
	$(LD) -g -o $@ $+ ${LIBS}

depend : ${DEPS}

${DEPS} : Makefile

%.d: %.c
	@echo "Making $@ (dependencies for $<)"
	@${CC} -MM ${GAMEFLAGS} $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g'  > $@

.PHONY: clean cleanall

clean:
	rm -f $(OBJS)

cleanall: clean
	rm -f ${DEPS}

-include ${DEPS}


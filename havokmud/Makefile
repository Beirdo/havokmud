##  these are compile time constructs, that affect the running of the ga
## See farther down for explaination of these and more! msw
##
GAMEFLAGS = -DPREP_SPELLS -DPREVENT_PKILL -DLOG_ALL -DLAG_MOBILE -DSITELOCK \
	    -DUSE_EGOS -DLIMITED_ITEMS

##
## The compiler your gonna use to compile this with..
##
CC=gcc
##
LD=$(CC)

##  these are architecture dependent flags
COMPILEFLAGS = -g -O6 -Wall -Werror -Wno-trigraphs -ansi -pedantic
##
CFLAGS = $(COMPILEFLAGS) $(GAMEFLAGS)

## **********************************************************
##      COMPILE TIME DEFINES (GAMEFLAGS)
##
##  For lower exp loss at lower levels -DNEWEXP
##
##  For level loss for really low exp -DLEVEL_LOSS
##
##  To make some optimizations, you need to add things to the line above.
##  to use hash tables for rooms  (More cpu, less storage), add
##  -DHASH    
##
##  To get rid of tracking (lower cpu)
##  -DNOTRACK
##
##  To limit items above a specific rent cost
##  -DLIMITED_ITEMS
##
##  To force new players to be authorized
##  -DPLAYER_AUTH
##
##  To add some new debugging stuff (mostly limited to malloc_debug)
##  -DDEBUG
##  To add some commands for locking out specific hosts
##  -DSITELOCK
##
## To lock out oak.grove.iup.edu from boot time.
## -DLOCKGROVE
##
##
##  To modify some commands to make item duplication near impossible:
##  -DNODUPLICATES
##
##  To have each 58+ god restricted to one site, or set of sites
##  -DIMPL_SECURITY
##
##  To use BSD memory allocation/deallocation routines instead of the
##  native memory routines:
##  -DKLUDGE_MEM
##
##  If your system doesn't have built in functions of strdup() and
##  strstr(), then:
##  -DKLUDGE_STRING
##
##  If you want to use a faster mana/hit/move regen system.
##  -DNEWGAIN
##
##  If you want to save every room and mob items use this.
##  -DSAVEWORLD
##
##  If you want to use the Quest to Gain system use this. Not all classes
##  -DQUEST_GAIN
##
##  If you want to use a lower gold ration for your game use this
## -DLOW_GOLD
##
## - DUSE_EGOS (check ego on give and gets )
##
##
## -DLOG_MOB
##              LOG_MOB logs all mob command sent to command_interpreter
##
##      -DLOG_DEBUG
##              This will add a bit more info to the logs, I use it to try
##              and track down impossible bugs.
##
## -DCLEAN_AT_BOOT
##              Forces the mud to remove outdated or inactive players and
##              demote inactive gods a boot up. Slows boot process down a
##              bit.
##
## -DSTRANGE_WACK
##      When used in combination with CLEAN_AT_BOOT it will clear out corrupted
##      user files. Do this once or twice a month if needed, otherwise do not
##      use it.
##
##
## -DZONE_COMM_ONLY
##              Enabling this will force all communications to be limited to
##              the zone that person is in. Making gossip/auction/tell NOT
##              GLOBAL. Tell is limited to people in the zone you are currently
##              standing in. This does NOT include SHOUT.
##
## -DLAG_MOBILES                
##              This define shold make mobiles lag with commands as
##              pc's do. I.e. bashed pause, etc...
##
## -DPREVENT_PKILL
##              This define will add in a few checks in fight.c to keep
##              pc's from killing each other, willing or Un-willing.
##
## -DCHECK_RENT_INACTIVE
##              Check user inactivity against RENT_INACTIVE, if greater, wack
##              the rent file. RENT_INACTIVE located in db.c
##
## -DPERSONAL_PERM_LOCKOUTS
##              My personal annoying people I lock out perminatly. You can add
##              whomever is annoying in there.
##
## -DNEW_RENT
##              Charge 100 coins per day of rent, regardless of item types.
##

SRCS =	act.comm.c act.info.c act.move.c act.obj1.c act.obj2.c act.off.c \
	act.other.c act.social.c act.wizard.c ansi_parser.c board.c bsd.c \
	comm.c constants.c create.c create.help.c create.mob.c create.obj.c \
	db.c fight.c handler.c hash.c heap.c interpreter.c limits.c magic2.c \
	magic3.c magic.c magicutils.c mail.c mindskills1.c mind_use1.c \
	mobact.c modify.c mod.text.c multiclass.c opinion.c paginate.c \
	parser.c reception.c security.c shop.c signals.c skills.c sound.c \
	spec_assign.c spec_guildmaster.c spec_procs2.c spec_procs3.c \
	spec_procs4.c spec_procs.c spell_parser.c spells1.c spells2.c trap.c \
	utility.c weather.c

OBJS = ${SRCS:.c=.o}
DEPS = ${SRCS:.c=.d}

all : dmserver

dmserver : ${OBJS}
	$(LD) -g -o $@ $+ -lcrypt

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


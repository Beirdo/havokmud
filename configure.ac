# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.59)

AC_INIT(havokmud, [2.0.0], [gjhurlbu@users.sourceforge.net])
AC_CONFIG_SRCDIR([include/version.h])
AC_CONFIG_HEADER([include/config.h])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_MACRO_DIR([scripts])
AC_REVISION($Revision: 1.1.2.6 $)


AC_ARG_VAR(AWK, [location of the awk command])
AC_PROG_AWK
AC_ARG_VAR(CUT, [location of the cut command])
AC_PATH_PROG(CUT, cut)

dnl check package version - formatted like: "1.11.23 July 12, 2004"
AC_MSG_CHECKING(version of the package)

VERSION=`cat include/version.h | grep "define SRC_VERSION" | $CUT -d '"' -f 2`
VERSIONMAJOR=`echo $VERSION | $AWK 'BEGIN {FS="."} {printf("%d", $1)}'`
VERSIONMINOR=`echo $VERSION | $AWK 'BEGIN {FS="."} {printf("%d", $2)}'`
VERSIONPATCH=`echo $VERSION | $AWK 'BEGIN {FS="."} {printf("%d", $3)}'`
VERSIONDATE=`echo $VERSION | $CUT -d " " -f 2-`
AC_MSG_RESULT(${VERSION})

AC_SUBST(VERSION)
AC_SUBST(VERSIONMAJOR)
AC_SUBST(VERSIONMINOR)
AC_SUBST(VERSIONPATCH)
AC_SUBST(VERSIONDATE)

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE(havokmud, "2.0.0")

# Check for Cygwin
AC_MSG_CHECKING([for cygwin])
_using_cygwin=no
test "x$target_os" = "xcygwin" && _using_cygwin=yes
AC_MSG_RESULT($_using_cygwin)
AM_CONDITIONAL([USING_CYGWIN], [test "x$_using_cygwin" = "xyes"])

AC_GNU_SOURCE

# Checks for complier (we want gcc!)
AC_PROG_CC
dnl AC_PROG_CC_C99

# Check for -fgnu89-inline in gcc
AC_MSG_CHECKING([whether gcc supports -fgnu89-inline])
_gcc_cflags_save="$CFLAGS"
CFLAGS="${CFLAGS} -fgnu89-inline"
AC_COMPILE_IFELSE(AC_LANG_PROGRAM([]), _gcc_gnu89_inline=yes, 
                  [_gcc_gnu89_inline=no ; CFLAGS=$_gcc_cflags_save])
AC_MSG_RESULT($_gcc_gnu89_inline)
AM_CONDITIONAL([HAVE_GCC_GNU89_INLINE_OPTION],
               [test "x$_gcc_gnu89_inline" = "xyes"])
#CFLAGS=$_gcc_cflags_save


#
# ranlib
#
AC_PROG_RANLIB

# Check for pthreads
ACX_PTHREAD

CFLAGS="$PTHREAD_CFLAGS $CFLAGS"
LDFLAGS="$PTHREAD_LDFLAGS $LDFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"
CC="$PTHREAD_CC"

# Extra libraries on some systems
case "${host}" in
  *solaris* )
    AC_CHECK_LIB(socket, socket)
    AC_CHECK_LIB(nsl, bind)
    ;;
esac


AC_ARG_WITH(mysql-includes, AS_HELP_STRING([--with-mysql-includes=path], 
    [Set the MySQL Include path [[/usr/include/mysql]] ]))
if test "x$with_mysql_includes" = "x"; then
	ac_cv_option_mysql_includes=/usr/include/mysql
else
	ac_cv_option_mysql_includes=$with_mysql_includes
fi

AC_CACHE_CHECK([the path to MySQL includes], ac_cv_option_mysql_includes)
MYSQL_INCLUDES=-I$ac_cv_option_mysql_includes
INCLUDES=$MYSQL_INCLUDES
AC_SUBST(INCLUDES)

CFLAGS_SAVE=$CFLAGS
CFLAGS="$CFLAGS $MYSQL_INCLUDES"


AC_ARG_WITH(mysql-libs, AS_HELP_STRING([--with-mysql-libs=path], 
    [Set the MySQL Library path [[/usr/lib/mysql]] ]))
if test "x$with_mysql_libs" = "x"; then
	ac_cv_option_mysql_libs=/usr/lib/mysql
else
	ac_cv_option_mysql_libs=$with_mysql_libs
fi

AC_CACHE_CHECK([the path to MySQL libraries], ac_cv_option_mysql_libs)
MYSQL_LIBS=-L$ac_cv_option_mysql_libs
LDFLAGS="$LDFLAGS $MYSQL_LIBS"

AC_CHECK_LIB([z], [compress])
AC_CHECK_LIB([ssl], [MD5_Update])
AC_CHECK_LIB([mysqlclient], [mysql_query],,
             [AC_MSG_ERROR(libmysqlclient not installed)])

# the mysql include dir comes in via the INCLUDES
CFLAGS=$CFLAGS_SAVE

# Check if the library is installed in the system.  If not, use ours.
AC_CHECK_LIB([esmtp], [smtp_version],HAVE_LIBESMTP=yes,HAVE_LIBESMTP=no)
AM_CONDITIONAL([HAVE_LIBESMTP], [test x${HAVE_LIBESMTP} = xyes])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/file.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

AC_CHECK_TYPES(socklen_t)

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STRNLEN
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS([bzero gethostbyaddr gethostbyname gethostname getpagesize gettimeofday inet_ntoa isascii memset select socket strcasecmp strchr strdup strncasecmp strndup strnlen strstr strsep])

dnl HAVOK_SWITCH(arg-name, arg_name, ARG_NAME, help_LHS, help_RHS, cache_string,
dnl              header_comment, default, not_default)
AC_DEFUN([HAVOK_SWITCH],
[AC_ARG_ENABLE($1, AS_HELP_STRING([$4], [$5]))
if test "$enable_$2" = "$9"; then
	ac_cv_option_$2=$9
else
	ac_cv_option_$2=$8
fi

AC_CACHE_CHECK([$6], ac_cv_option_$2)
if test "$ac_cv_option_$2" = "yes" ; then
	AC_DEFINE($3, 1, [$7])
fi] )

#dnl NEWEXP enable option
#HAVOK_SWITCH([newexp], [newexp], [NEWEXP], [--disable-newexp],
#             [Disable the lower experience loss at lower levels],
#             [for lower experience loss at lower levels],
#             [For lower experience loss at lower levels], [yes], [no])
#
#dnl LEVEL_LOSS enable option
#HAVOK_SWITCH([level-loss], [level_loss], [LEVEL_LOSS], [--disable-level-loss],
#             [Disable level loss for really low experience],
#             [for level loss for really low experience],
#             [For level loss for really low experience], [yes], [no])
#
#dnl HASH enable option
#HAVOK_SWITCH([hash], [hash], [HASH], [--enable-hash],
#	     [Enable hash tables for room storage (more CPU, less storage)],
#             [for use of hash tables for room storage],
#             [Use Hash Tables for rooms (more CPU, less storage)], [no], [yes])
#
#
#dnl USE_TRACK enable option
#HAVOK_SWITCH([track], [track], [USE_TRACK], [--disable-track],
#	     [Disable tracking (lower CPU)],
#             [for the use of tracking],
#             [Enable tracking (more CPU)], [yes], [no])
#
#HAVOK_SWITCH([limited-items], [limited_items], [LIMITED_ITEMS], 
#             [--disable-limited-items], [Disable limited items],
#             [for the use of limited items],
#             [To limit items above a specific rent cost], [yes], [no])
#
#HAVOK_SWITCH([player-auth], [player_auth], [PLAYER_AUTH],
#             [--enable-player-auth], 
#             [Enable forcing new players to be authorized],
#             [for player authorization],
#             [To force new players to be authorized], [no], [yes])
#
HAVOK_SWITCH([debug], [debug], [DEBUG], [--enable-debug],
             [Enable some debugging stuff],
             [for extra debugging],
             [To add some extra debugging stuff], [no], [yes])

HAVOK_SWITCH([sitelock], [sitelock], [SITELOCK], [--disable-sitelock],
             [Disable locking out specific hosts],
             [for site-locking ability],
             [To add site-locking commands], [yes], [no])

#HAVOK_SWITCH([duplicates], [duplicates], [DUPLICATES], [--enable-duplicates],
#             [Enable duplicate items],
#             [for duplicate item support],
#             [To allow item duplication], [no], [yes])
#
#HAVOK_SWITCH([impl-security], [impl_security], [IMPL_SECURITY],
#             [--enable-impl-security], [Enable implementor security measures],
#             [for implementor security measures],
#             [To have each 58+ god restricted to a set of sites], [no], [yes])
#
#HAVOK_SWITCH([newgain], [newgain], [NEWGAIN], [--disable-newgain],
#             [Disable the new mana/hit/move regen system],
#             [for new mana/hit/move handling],
#             [To use the new mana/hit/move handling], [yes], [no])
#
#HAVOK_SWITCH([saveworld], [saveworld], [SAVEWORLD], [--enable-saveworld],
#             [Enable saving the entire world], [for saving the entire world],
#             [If you want to be able to save the entire world], [no], [yes])
#
#HAVOK_SWITCH([quest-gain], [quest_gain], [QUEST_GAIN], [--enable-quest-gain],
#             [Enable Quest-to-Gain system], [for Quest-to-Gain system],
#             [If you want to use the Quest-to-Gain system], [no], [yes])
#
#HAVOK_SWITCH([low-gold], [low_gold], [LOW_GOLD], [--disable-low-gold],
#             [Disable lower gold rations], [for lower gold rations],
#             [If you want to use a lower gold ration], [yes], [no])
#
#HAVOK_SWITCH([use-egos], [use_egos], [USE_EGOS], [--disable-use-egos],
#             [Disable the use of object egos], [for object egos],
#             [Check ego on give and gets], [yes], [no])
#
#HAVOK_SWITCH([log-mob], [log_mob], [LOG_MOB], [--enable-log-mob],
#             [Enable logging all mob commands], [for mobile logging],
#             [Log all mob commands sent to command_interpreter], [no], [yes])
#
#HAVOK_SWITCH([log-debug], [log_debug], [LOG_DEBUG], [--enable-log-debug],
#             [Enable extra debugging logging], [for debugging logging],
#             [Add even more info to the logs], [no], [yes])
#
#HAVOK_SWITCH([clean-at-boot], [clean_at_boot], [CLEAN_AT_BOOT],
#             [--enable-clean-at-boot], 
#             [Enable removing inactive players, demoting inactive gods at boot time],
#             [for cleaning of inactive at boot],
#             [Force the mud to remove inactive mortals, demote inactive immortals at boot time],
#             [no], [yes])
#
#HAVOK_SWITCH([strange-wack], [strange_wack], [STRANGE_WACK], 
#             [--enable-strange-wack], [Enable removal of corrupted user files],
#             [for whacking of corrupted user files],
#             [Clears out corrupted user files.  Use periodically if at all],
#             [no], [yes])
#
#HAVOK_SWITCH([zone-comm-only], [zone_comm_only], [ZONE_COMM_ONLY],
#             [--disable-zone-comm-only], [Disable zone-only communications],
#             [for zone only communications],
#             [Forces gossip/tell to only work within the local zone], [yes], 
#             [no])
#
#HAVOK_SWITCH([lag-mobiles], [lag_mobiles], [LAG_MOBILES], 
#             [--disable-lag-mobiles], [Disable mobiles lagging like players],
#             [for mobile lagging], [Makes mobs lag like players], [yes], [no])
#
#HAVOK_SWITCH([prevent-pkill], [prevent_pkill], [PREVENT_PKILL],
#             [--disable-prevent-pkill], [Disable player killing prevention],
#             [for player killing prevention], 
#             [Prevents player killing], [yes], [no])
#
#HAVOK_SWITCH([check-rent-inactive], [check_rent_inactive], 
#             [CHECK_RENT_INACTIVE], [--enable-check-rent-inactive],
#             [Enable killing inactive users' rent files],
#             [for inactive player rent whacking],
#             [If users are inactive, remove the rent file], [no], [yes])
#
#HAVOK_SWITCH([new-rent], [new_rent], [NEW_RENT], [--enable-new-rent],
#             [Enable the new rent system], [for new rent system],
#             [Charge 100 coins per day of rent, regardless of item types],
#             [no], [yes])
#
#HAVOK_SWITCH([prep-spells], [prep_spells], [PREP_SPELLS], 
#             [--disable-prep-spells], [Disable prep spells (?)],
#             [for prep spells], [Needs a description], [yes], [no])
#
#HAVOK_SWITCH([log-all], [log_all], [LOG_ALL], [--disable-log-all],
#             [Disable logging all actions], [for logging all actions],
#             [Log all actions], [yes], [no])
#
#HAVOK_SWITCH([monk-item-limit], [monk_item_limit], [MONK_ITEM_LIMIT],
#             [--enable-monk-item-limit], [Enable limiting monks to 20 items],
#             [for monk item limiting], [Limit monks to 20 items], [no], [yes])
#
#HAVOK_SWITCH([byte-count], [byte_count], [BYTE_COUNT], [--enable-byte-count],
#             [Enable counting the size of object/mob/room as saving],
#             [for byte counting of object/mob/room],
#             [Count the size of object/mob/room on saving], [no], [yes])
#
#HAVOK_SWITCH([old-zone-stuff], [old_zone_stuff], [OLD_ZONE_STUFF],
#             [--disable-old-zone-stuff], [Disable old zone stuff],
#             [for old zone stuff], [for temp testing of reset_zon], [yes], [no])
#
#HAVOK_SWITCH([n-save-world], [n_save_world], [N_SAVE_WORLD],
#             [--enable-n-save-world], [Enable new save world (?)],
#             [for new save world], [Enable new save world (?)], [no], [yes])
#

AC_CONFIG_FILES([Makefile libs/Makefile threads/Makefile util/Makefile corefunc/Makefile])
AC_OUTPUT

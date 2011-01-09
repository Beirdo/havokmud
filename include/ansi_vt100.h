#ifndef _ansi_vt100_h
#define _ansi_vt100_h
/**
 * @file 
 * @brief ANSI color codes and VT100 codes
 */

/*
 * First the ANSI codes
 */

/**
 * Clear from cursor to end of line 
 */
#define CLEAR_CURSOR_TO_END    "\033[0K"

/**
 * Clear from begin of line to cursor 
 */
#define CLEAR_BEGIN_TO_CURSOR  "\033[1K"

/** 
 * Clear line containing cursor 
 */
#define CLEAR_LINE             "\033[2K"

/** 
 * Clear screen from cursor 
 */ 
#define CLEAR_FROM_CURSOR      "\033[0J"

/** 
 * Clear screen to cursor 
 */
#define CLEAR_TO_CURSOR        "\033[1J"

/** 
 * Clear entire screen 
 */
#define CLEAR_SCREEN           "\033[2J"


#define MOD_NORMAL    "\033[0m"
#define MOD_BOLD      "\033[1m"
#define MOD_FAINT     "\033[2m"
#define MOD_UNDERLINE "\033[4m"
#define MOD_BLINK     "\033[5m"
#define MOD_REVERSE   "\033[7m"

/*
 * 00-09 
 */
#define FG_BLACK      "\033[30m"
#define FG_RED        "\033[31m"
#define FG_GREEN      "\033[32m"
#define FG_BROWN      "\033[33m"
#define FG_BLUE       "\033[34m"
#define FG_MAGENTA    "\033[35m"
#define FG_CYAN       "\033[36m"
#define FG_LT_GRAY    "\033[37m"
#define FG_DK_GRAY    "\033[1;30m"
#define FG_LT_RED     "\033[1;31m"
#define FG_LT_GREEN   "\033[1;32m"
#define FG_YELLOW     "\033[1;33m"
#define FG_LT_BLUE    "\033[1;34m"
#define FG_LT_MAGENTA "\033[1;35m"
#define FG_LT_CYAN    "\033[1;36m"
#define FG_WHITE      "\033[1;37m"

#define BK_BLACK      "\033[0;40m"
#define BK_RED        "\033[0;41m"
#define BK_GREEN      "\033[0;42m"
#define BK_BROWN      "\033[0;43m"
#define BK_BLUE       "\033[0;44m"
#define BK_MAGENTA    "\033[0;45m"
#define BK_CYAN       "\033[0;46m"
#define BK_LT_GRAY    "\033[0;47m"



/*
 * Now the VT100 codes
 */

#define UNKNOWN       0
#define VT100         1

#define VT_INITSEQ    "\033[1;24r"
#define VT_CURSPOS    "\033[%d;%dH"
#define VT_CURSRIG    "\033[%dC"
#define VT_CURSLEF    "\033[%dD"
#define VT_HOMECLR    "\033[2J\033[0;0H"
#define VT_CTEOTCR    "\033[K"
#define VT_CLENSEQ    "\033[r\033[2J"
#define VT_INDUPSC    "\033M"
#define VT_INDDOSC    "\033D"
#define VT_SETSCRL    "\033[%d;24r"
#define VT_INVERTT    "\033[0;1;7m"
#define VT_BOLDTEX    "\033[0;1m"
#define VT_NORMALT    "\033[0m"
#define VT_MARGSET    "\033[%d;%dr"
#define VT_CURSAVE    "\0337"
#define VT_CURREST    "\0338"

/*
 * Infobar stuffage
 */
#define INFO_HP            1
#define INFO_MANA          2
#define INFO_MOVE          4
#define INFO_EXP           8
#define INFO_GOLD          16


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

#ifndef _vt100c_h
#define _vt100c_h

/**
 * @file
 * @brief VT100 code definitions
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

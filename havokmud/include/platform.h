#ifndef _platform_h
#define _platform_h

#if defined( __CYGWIN__ )
#undef __STRICT_ANSI__
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

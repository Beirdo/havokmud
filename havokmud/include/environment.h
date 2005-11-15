/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2005 Gavin Hurlbut
 *
 *  havokmud is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*HEADER---------------------------------------------------
* $Id$
*
* Copyright 2005 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*
*----------------------------------------------------------
*/

#ifndef environment_h_
#define environment_h_

#ifdef __GNUC__
/* We are compiling with GCC */

#define _UNUSED_ __attribute__ ((unused))
#define _PACKED_ __attribute__ ((packed))
#ifndef __cplusplus
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#ifndef _REENTRANT
#define _REENTRANT
#endif
#define _THREAD_SAFE

#else

/* Other compilers */
#define _UNUSED_
#define _PACKED_
#endif

/* CVS generated ID string (optional for h files) */
static char environment_h_ident[] _UNUSED_ = 
    "$Id$";

#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64

#define _LARGEFILE_SOURCE

#define O_LARGEFILE 0


/* NOTE: to use 'long long' in GCC, you need -Wno-long-long if using -pedantic
 * -Wall -Werror (which you should be)
 */
#ifndef __cplusplus
typedef int                    bool;
#endif
typedef char                   int8;
typedef short int              int16;
typedef int                    int32;
typedef long long int          int64;
typedef unsigned char          uint8;
typedef unsigned short int     uint16;
typedef unsigned int           uint32;
typedef unsigned long long int uint64;


/* Values for bool type */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#define BOOL(x)  ((x) ? TRUE : FALSE)

#define NELEMENTS(x)                (sizeof((x)) / sizeof((x)[0]))
#define OFFSETOF(elem,StructType)   ((char *)&(((StructType *)NULL)->elem) - \
                                       (char *)((StructType *)NULL))
#define ELEMSIZE(elem,StructType)   (sizeof(((StructType *)NULL)->elem))
#define FORCE_UINT32(x)             (*(uint32 *)&(x))

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

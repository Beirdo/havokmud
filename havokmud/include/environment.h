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

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

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

/**
 * @brief Returns the number of elements in an array
 * @param x the structure
 * @return the number of elements in the structure
 */
#define NELEMENTS(x)                (sizeof((x)) / sizeof((x)[0]))

/**
 * @brief Returns the byte offset of a given element within a struct
 * @param elem the structure element
 * @param StructType the type definition of the structure (e.g. struct blah)
 * @return the byte offset of elem within StructType
 */
#define OFFSETOF(elem,StructType)   ((char *)&(((StructType *)NULL)->elem) - \
                                       (char *)((StructType *)NULL))

/**
 * @brief Returns the size (in bytes) of a given element within a struct
 * @param elem the structure element
 * @param StructType the type definition of the structure (e.g. struct blah)
 * @return the size (in bytes) of elem within StructType
 */
#define ELEMSIZE(elem,StructType)   (sizeof(((StructType *)NULL)->elem))

/**
 * @brief Does a forced conversion to uint32 (can be an lval as well as rval)
 * @param x the data item to treat as a uint32
 * @return an entity at the same address as x, but as a uint32
 */
#define FORCE_UINT32(x)             (*(uint32 *)&(x))

/**
 * @brief Gives a pointer to the element within a structure by offset
 * @param offset the byte offset within the buffer to point at
 * @param buffer a structure (usually) that we need an element in
 * @return void pointer to the element at offset within the buffer
 *
 * This is useful to take the output of OFFSETOF() which is a byte offset of an
 * element in a structure, and get a pointer to that element later by offset.
 * This is the converse of OFFSETOF()
 */
#define PTR_AT_OFFSET(offset,buffer)    ((void *)((char *)(buffer) + \
                                                  (int)(offset)))

#define MIN(x,y)                    ( (x) < (y) ? (x) : (y) )
#define MAX(x,y)                    ( (x) > (y) ? (x) : (y) )

#define IS_SET(flag, bit)           ( ((flag) & (bit)) != 0 )

#define IS_SET_FLAG(buffer, offset, bit)    \
                IS_SET( (*(long *)PTR_AT_OFFSET((offset),(buffer))), bit )

#define IS_OBJ_STAT(obj,elem,stat) (IS_SET((obj)->elem,stat))

/**
 * @todo create a "macros.h" with these macros
 */

#define WEAR_FLAG(o,f)  IS_SET(o->wear_flags, f)
#define EXTRA_FLAG(o,f) IS_SET(o->extra_flags, f)
#define CLASS_FLAG(o,f) IS_SET(o->anti_class, f)
#define ANTI_FLAG(o,f)  IS_SET(o->anti_flags, f)
#define ONLY_CLASS_FLAG(o,f)  (CLASS_FLAG(o, f) && \
                               CLASS_FLAG(o, ITEM_ONLY_CLASS))
#define ANTI_CLASS_FLAG(o,f)  (CLASS_FLAG(o, f) && \
                               !CLASS_FLAG(o, ITEM_ONLY_CLASS))

/**
 * @brief Returns the maximum level this player has on any class
 * @param ch the character structure
 * @return the maximum level of that player
 * @todo implement the setting of max_level on gain and on char load
 */
#define GetMaxLevel(ch) ( ch == NULL ? 0 : ch->player.max_level )


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

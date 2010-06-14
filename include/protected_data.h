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
*--------------------------------------------------------*/

/**
 * @file
 * @brief Protected Data Structure (access controlled by a mutex)
 */

#ifndef protected_data_h_
#define protected_data_h_

#include "environment.h"

/* CVS generated ID string (optional for h files) */
static char protected_data_h_ident[] _UNUSED_ = 
    "$Id$";


typedef struct
{
    pthread_mutex_t mutex;
    void           *data;
} ProtectedData_t;


#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes */
ProtectedData_t *ProtectedDataCreate( void );
void ProtectedDataDestroy( ProtectedData_t *blob );
void ProtectedDataLock( ProtectedData_t *blob );
void ProtectedDataUnlock( ProtectedData_t *blob );

#ifdef __cplusplus
}
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

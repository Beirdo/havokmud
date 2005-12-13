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
*--------------------------------------------------------*/

/* INCLUDE FILES */
#include "environment.h"
#include <pthread.h>
#include "protected_data.h"
#include "logging.h"
#include <stdlib.h>

/* INTERNAL CONSTANT DEFINITIONS */

/* INTERNAL TYPE DEFINITIONS */

/* INTERNAL MACRO DEFINITIONS */

/* INTERNAL FUNCTION PROTOTYPES */

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

void ProtectedDataLock( ProtectedData_t *blob )
{
    if( blob == NULL )
    {
        return;
    }

    pthread_mutex_lock( &blob->mutex );
}


void ProtectedDataUnlock( ProtectedData_t *blob )
{
    if( blob == NULL )
    {
        return;
    }

    pthread_mutex_unlock( &blob->mutex );
}


ProtectedData_t *ProtectedDataCreate( void )
{
    ProtectedData_t *blob;

    blob = (ProtectedData_t *)malloc(sizeof(ProtectedData_t));
    if( blob == NULL )
    {
        LogPrintNoArg( LOG_UNKNOWN, "Couldn't create protected data blob" );
        return( NULL );
    }

    pthread_mutex_init( &blob->mutex, NULL );
    blob->data = NULL;

    return( blob );
}

void ProtectedDataDestroy( ProtectedData_t *blob )
{
    /* Assumes the list is locked by the caller */
    pthread_mutex_unlock( &blob->mutex );
    pthread_mutex_destroy( &blob->mutex );

    free( blob );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

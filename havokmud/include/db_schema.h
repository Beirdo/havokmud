/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2008 Gavin Hurlbut
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
* Copyright 2008 Gavin Hurlbut
* All rights reserved
*/

#ifndef db_schema_h_
#define db_schema_h_

#include "environment.h"
#include "structs.h"

static char db_schema_h_ident[] _UNUSED_ = 
    "$Id$";

#define CURRENT_SCHEMA  1


static QueryTable_t defCoreSchema[] = {
  { "CREATE TABLE `settings` (\n"
    "  `name` varchar(40) NOT NULL,\n"
    "  `value` varchar(40) NOT NULL default '',\n"
    "  PRIMARY KEY  (`name`)\n"
    ") TYPE=MyISAM\n", NULL, NULL, FALSE }
};
static int defCoreSchemaCount = NELEMENTS(defCoreSchema);

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

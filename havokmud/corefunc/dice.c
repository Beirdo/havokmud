/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2010 Gavin Hurlbut
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
* Copyright 2010 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*--------------------------------------------------------*/

/* INCLUDE FILES */
#include "environment.h"
#include <stdlib.h>
#include "oldstructs.h"

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

/**
 * @file
 * @brief Dice and random numbers
 */


/**
 * @brief Creates a random number in interval [from;to]
 * @param from minimum result
 * @param to maximum result
 * @return a random number between from and to (inclusive)
 *
 * Creates random numbers in arbitrary ranges
 */
int number(int from, int to)
{
    int     range;

    range = to - from + 1;
    if (range > 0) {
        return ((random() % range) + from);
    } else {
        return (from);
    }
}

/**
 * @brief Simulates dice roll
 * @param number number of dice to roll
 * @param size number of sides on each die
 * @return total of the dice rolled
 *
 * Rolls a user-defined number of dice with user-defined numbers of sides (all
 * the same size)
 */
int dice(int number, int size)
{
    int             r;
    int             sum = 0;

    /*
     * instead of crashing the mud we set it to 1
     */
    if (size <= 0) {
        size = 1;
    }
    for (r = 1; r <= number; r++) {
        sum += ((random() % size) + 1);
    }
    return (sum);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

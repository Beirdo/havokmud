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
 * Copyright 2010 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Handles converting json <-> binary trees of attributes
 */

#include "config.h"
#include "environment.h"
#include <stdio.h>
#include <string.h>
#include "logging.h"
#include "structs.h"
#include "interthread.h"
#include "protos.h"
#include "balanced_btree.h"
#include "cJSON.h"
#include "memory.h"

char *strip_whitespace(char *string)
{
    int     len;
    char   *ch;

    ch = string;
    while( *ch == ' ' || *ch == '\t' || *ch == '\r' || *ch == '\n' ) {
        ch++;
    }

    if( *ch == '\0' ) {
        return( NULL );
    }

    string = ch;
    len = strlen(string);

    ch = &string[len-1];
    while( *ch == ' ' || *ch == '\t' || *ch == '\r' || *ch == '\n' ) {
        *(ch--) = '\0';
    }

    return( string );
}


cJSON *cJSON_Clone(cJSON *obj)
{
    cJSON *clone;

    if( !obj ) {
        return( NULL );
    }

    /* We don't care about siblings, or children */
    clone = CREATE(cJSON);

    clone->type = obj->type;
    if( obj->valuestring ) {
        clone->valuestring = memstrlink(obj->valuestring);
    }
    clone->valueint    = obj->valueint;
    clone->valuedouble = obj->valuedouble;
    clone->string = memstrlink(obj->string);

    return( clone ); 
}


void AddJSONToTrees( JSONSource_t *js, BalancedBTree_t *attribs, 
                     BalancedBTree_t *sources )
{
    static char            *empty = "{ }";
    char                   *json;
    cJSON                  *jsonTree;
    cJSON                  *jsonItem;
    BalancedBTreeItem_t    *item;
    BalancedBTree_t        *subTree;
    BalancedBTree_t        *srcSubTree;

    if( !js || !js->source || !js->json || !attribs || !sources ) {
        return;
    }

    json = strip_whitespace(js->json);
    if( !json ) {
        return;
    }

    if( strlen(json) == 0 ) {
        json = empty;
    }

    jsonTree = cJSON_Parse(json);
    BalancedBTreeLock( attribs );
    BalancedBTreeLock( sources );

    /* Find or make the subtree for the sources tree.  Once. */
    item = BalancedBTreeFind( sources, js->source, LOCKED, FALSE );
    if( item ) {
        srcSubTree = (BalancedBTree_t *)item->item;
        BalancedBTreeLock( srcSubTree );
    } else {
        srcSubTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
        BalancedBTreeLock( srcSubTree );

        item = CREATE(BalancedBTreeItem_t);
        item->key  = memstrlink(js->source);
        item->item = srcSubTree;
        BalancedBTreeAdd( sources, item, LOCKED, TRUE );
    }

    for( jsonItem = jsonTree->child; jsonItem; jsonItem = jsonItem->next ) {
        /* For each data item, stuff it into two separate two-layer trees */

        /* First the attribs tree.  attrib->source */
        item = BalancedBTreeFind( attribs, jsonItem->string, LOCKED, FALSE );
        if( item ) {
            subTree = (BalancedBTree_t *)item->item;
        } else {
            subTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
            item = CREATE(BalancedBTreeItem_t);
            item->key  = memstrlink(jsonItem->string);
            item->item = subTree;
            BalancedBTreeAdd( attribs, item, LOCKED, FALSE );
        }

        item = CREATE(BalancedBTreeItem_t);
        item->key  = memstrlink(js->source);
        item->item = cJSON_Clone(jsonItem);
        BalancedBTreeAdd( subTree, item, UNLOCKED, TRUE );

        /* Second the sources tree.  source->attrib */
        item = CREATE(BalancedBTreeItem_t);
        item->key  = memstrlink(jsonItem->string);
        item->item = cJSON_Clone(jsonItem);
        BalancedBTreeAdd( srcSubTree, item, UNLOCKED, FALSE );
    }

    /* Rebalance the two unbalanced trees */
    BalancedBTreeAdd( attribs, NULL, LOCKED, TRUE );
    BalancedBTreeAdd( srcSubTree, NULL, LOCKED, TRUE );

    BalancedBTreeUnlock( srcSubTree );
    BalancedBTreeUnlock( sources );
    BalancedBTreeUnlock( attribs );
    cJSON_Delete(jsonTree);
}

void AppendAttribToJSON( cJSON *jsonTree, BalancedBTreeItem_t *item )
{
    if( !item ) {
        return;
    }

    AppendAttribToJSON( jsonTree, item->left );

    cJSON_AddItemToObject( jsonTree, (char *)item->key, 
                           cJSON_Clone( (cJSON *)item->item ) );

    AppendAttribToJSON( jsonTree, item->right );
}

int AppendSourceToJSON( cJSON *jsonTree, BalancedBTreeItem_t *item )
{
    int             count = 0;
    cJSON          *jsonItem;

    if( !item ) {
        return( 0 );
    }

    count += AppendSourceToJSON( jsonTree, item->left );
    count += 1;

    jsonItem = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonTree, (char *)item->key, jsonItem);
    AppendAttribToJSON( jsonItem, ((BalancedBTree_t *)item->item)->root );

    count += AppendSourceToJSON( jsonTree, item->right );

    return( count );
}

JSONSource_t *ExtractJSONFromTree( BalancedBTree_t *sources )
{
    JSONSource_t           *js;
    cJSON                  *jsonTree;
    cJSON                  *jsonItem;
    int                     srcCount;
    int                     i;

    if( !sources ) {
        return;
    }

    jsonTree = cJSON_CreateObject();

    BalancedBTreeLock( sources );
    srcCount = AppendSourceToJSON( jsonTree, sources->root );
    BalancedBTreeUnlock( sources );

    js = CREATEN(JSONSource_t, srcCount+1);

    for( i = 0, jsonItem = jsonTree->child; 
         i < srcCount && jsonItem; 
         i++, jsonItem = jsonItem->next ) {
        js[i].source = memstrlink(jsonItem->string);
        js[i].json   = cJSON_Print(jsonItem);
    }

    cJSON_Delete(jsonTree);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

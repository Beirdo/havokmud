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

char *strip_whitespace(char *string);
cJSON *cJSON_Clone(cJSON *obj);
void AppendAttribToJSON( cJSON *jsonTree, BalancedBTreeItem_t *item );
int AppendSourceToJSON( cJSON *jsonTree, BalancedBTreeItem_t *item );
void SourceDeleteAttrib( char *attrib, BalancedBTree_t *tree );
void SourceTreeDeleteAttrib( char *attrib, BalancedBTreeItem_t *item );
bool SourceTreeCleanup( BalancedBTreeItem_t *item );
bool AttribTreeItemsDestroy( BalancedBTreeItem_t *item );

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


void AddJSONToTrees( JSONSource_t *js, PlayerPC_t *pc )
{
    static char            *empty = "{ }";
    char                   *json;
    cJSON                  *jsonTree;
    cJSON                  *jsonItem;
    BalancedBTreeItem_t    *item;
    BalancedBTree_t        *subTree;
    BalancedBTree_t        *srcSubTree;

    if( !js || !js->source || !js->json || !pc || !pc->attribs || 
        !pc->sources ) {
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
    BalancedBTreeLock( pc->attribs );
    BalancedBTreeLock( pc->sources );

    /* Find or make the subtree for the sources tree.  Once. */
    item = BalancedBTreeFind( pc->sources, js->source, LOCKED, FALSE );
    if( item ) {
        srcSubTree = (BalancedBTree_t *)item->item;
        BalancedBTreeLock( srcSubTree );
    } else {
        srcSubTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
        BalancedBTreeLock( srcSubTree );

        item = CREATE(BalancedBTreeItem_t);
        item->key  = memstrlink(js->source);
        item->item = srcSubTree;
        BalancedBTreeAdd( pc->sources, item, LOCKED, TRUE );
    }

    for( jsonItem = jsonTree->child; jsonItem; jsonItem = jsonItem->next ) {
        /* For each data item, stuff it into two separate two-layer trees */

        /* First the attribs tree.  attrib->source */
        item = BalancedBTreeFind( pc->attribs, jsonItem->string, LOCKED, 
                                  FALSE );
        if( item ) {
            subTree = (BalancedBTree_t *)item->item;
        } else {
            subTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
            item = CREATE(BalancedBTreeItem_t);
            item->key  = memstrlink(jsonItem->string);
            item->item = subTree;
            BalancedBTreeAdd( pc->attribs, item, LOCKED, FALSE );
        }

        item = BalancedBTreeFind( subTree, js->source, UNLOCKED, FALSE );
        if( item ) {
            cJSON_Delete(item->item);
            item->item = cJSON_Clone(jsonItem);
        } else {
            item = CREATE(BalancedBTreeItem_t);
            item->key  = memstrlink(js->source);
            item->item = cJSON_Clone(jsonItem);
            BalancedBTreeAdd( subTree, item, UNLOCKED, TRUE );
        }

        /* Second the sources tree.  source->attrib */
        item = BalancedBTreeFind( srcSubTree, jsonItem->string, LOCKED, FALSE );
        if( item ) {
            cJSON_Delete(item->item);
            item->item = cJSON_Clone(jsonItem);
        } else {
            item = CREATE(BalancedBTreeItem_t);
            item->key  = memstrlink(jsonItem->string);
            item->item = cJSON_Clone(jsonItem);
            BalancedBTreeAdd( srcSubTree, item, LOCKED, FALSE );
        }
    }

    /* Rebalance the two unbalanced trees */
    BalancedBTreeAdd( pc->attribs, NULL, LOCKED, TRUE );
    BalancedBTreeAdd( srcSubTree, NULL, LOCKED, TRUE );

    BalancedBTreeUnlock( srcSubTree );
    BalancedBTreeUnlock( pc->sources );
    BalancedBTreeUnlock( pc->attribs );
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

JSONSource_t *ExtractJSONFromTree( PlayerPC_t *pc )
{
    JSONSource_t           *js;
    cJSON                  *jsonTree;
    cJSON                  *jsonItem;
    int                     srcCount;
    int                     i;

    if( !pc || !pc->sources ) {
        return( NULL );
    }

    jsonTree = cJSON_CreateObject();

    BalancedBTreeLock( pc->sources );
    srcCount = AppendSourceToJSON( jsonTree, pc->sources->root );
    BalancedBTreeUnlock( pc->sources );

    js = CREATEN(JSONSource_t, srcCount+1);

    for( i = 0, jsonItem = jsonTree->child; 
         i < srcCount && jsonItem; 
         i++, jsonItem = jsonItem->next ) {
        js[i].source = memstrlink(jsonItem->string);
        js[i].json   = cJSON_Print(jsonItem);
    }

    cJSON_Delete(jsonTree);
}

char *CombineJSON( JSONSource_t *js )
{
    cJSON          *jsonTree;
    cJSON          *jsonItem;
    char           *json;

    jsonTree = cJSON_CreateObject();
    while( js->source ) {
        jsonItem = cJSON_Parse( js->json );
        cJSON_AddItemToObject( jsonTree, js->source, jsonItem );
    }

    json = cJSON_Print( jsonTree );
    cJSON_Delete( jsonTree );
}

JSONSource_t *SplitJSON( char *json )
{
    cJSON          *jsonTree;
    cJSON          *jsonItem;
    JSONSource_t   *js;
    int             count;
    int             i;
    
    jsonTree = cJSON_Parse( json );
    for( count = 0, jsonItem = jsonTree->child; jsonItem; 
         jsonItem = jsonItem->next, count++ );

    js = CREATEN(JSONSource_t, count+1);
    for( i = 0, jsonItem = jsonTree->child; i < count && jsonItem;
         jsonItem = jsonItem->next, i++ ) {
        js[i].source = memstrlink(jsonItem->string);
        js[i].json   = cJSON_Print(jsonItem);
    }
         
    cJSON_Delete( jsonTree );
    return( js );
}

void DestroyJSONSource( JSONSource_t *js )
{
    JSONSource_t       *jsItem;

    for( jsItem = js; jsItem->source; jsItem++ ) {
        memfree( jsItem->source );
        memfree( jsItem->json );
    }
    memfree( js );
}

void AddAttribute( char *json, char *source, PlayerPC_t *pc )
{
    JSONSource_t       *js;

    if( !json || !source || !pc ) {
        return;
    }

    js = CREATE(JSONSource_t);
    if( !js ) {
        return;
    }

    js->source = memstrlink(source);
    js->json = memstrlink(json);

    AddJSONToTrees( js, pc );

    DestroyJSONSource( js );
}

void DeleteAtribute( char *attrib, char *source, PlayerPC_t *pc )
{
    BalancedBTreeItem_t    *item;
    BalancedBTree_t        *subTree;
    BalancedBTree_t        *srcSubTree;
    bool                    cleaned;

    if( !attrib || !pc ) {
        return;
    }

    BalancedBTreeLock( pc->attribs );
    BalancedBTreeLock( pc->sources );

    /* First the attribs tree.  attrib->source */
    item = BalancedBTreeFind( pc->attribs, attrib, LOCKED, FALSE );
    if( item ) {
        subTree = (BalancedBTree_t *)item->item;
        BalancedBTreeLock( subTree );
        if( !source ) {
            /* Want to remove the entire attribute */
            while( AttribTreeItemsDestroy( subTree->root ) );
            BalancedBTreeDestroy( subTree );
            BalancedBTreeRemove( pc->attribs, item, LOCKED, TRUE );
            memfree( item->key );
            memfree( item );
        } else {
            item = BalancedBTreeFind( subTree, source, LOCKED, FALSE );
            if( item ) {
                BalancedBTreeRemove( subTree, item, LOCKED, TRUE );
                cJSON_Delete( item->item );
                memfree( item->key );
                memfree( item );
            }
            BalancedBTreeUnlock( subTree );
        }
    }

    /* Second the sources tree.  source->attrib */
    if( !source ) {
        /* Gotta check each source tree */
        SourceTreeDeleteAttrib( attrib, pc->sources->root );
        cleaned = FALSE;
        while( SourceTreeCleanup( pc->sources->root ) ) {
            cleaned = TRUE;
        }

        if( cleaned ) {
            /* Rebalance after cleaning */
            BalancedBTreeAdd( pc->sources, NULL, LOCKED, TRUE );
        }
    } else {
        item = BalancedBTreeFind( pc->sources, source, LOCKED, FALSE );
        if( item ) {
            subTree = (BalancedBTree_t *)item->item;
            BalancedBTreeLock( subTree );
            SourceDeleteAttrib( attrib, subTree );
            if( !subTree->root ) {
                /* subTree now empty! */
                BalancedBTreeDestroy( subTree );
                BalancedBTreeRemove( pc->sources, item, LOCKED, TRUE );
                memfree( item->key );
                memfree( item );
            } else {
                BalancedBTreeUnlock( subTree );
            }
        }
    }

    BalancedBTreeUnlock( pc->sources );
    BalancedBTreeUnlock( pc->attribs );
}

void SourceDeleteAttrib( char *attrib, BalancedBTree_t *tree )
{
    BalancedBTreeItem_t    *item;

    BalancedBTreeLock( tree );
    item = BalancedBTreeFind( tree, attrib, LOCKED, FALSE );
    if( item ) {
        BalancedBTreeRemove( tree, item, LOCKED, TRUE );
        cJSON_Delete(item->item);
        memfree(item->key);
        memfree(item);
    }
    BalancedBTreeUnlock( tree );
}

void SourceTreeDeleteAttrib( char *attrib, BalancedBTreeItem_t *item )
{
    SourceTreeDeleteAttrib( attrib, item->left );

    SourceDeleteAttrib( attrib, (BalancedBTree_t *)item->item );

    SourceTreeDeleteAttrib( attrib, item->right );
}

bool SourceTreeCleanup( BalancedBTreeItem_t *item )
{
    BalancedBTree_t        *subTree;

    if( !item ) {
        return( FALSE );
    }

    if( SourceTreeCleanup( item->left ) ) {
        return( TRUE );
    }

    if( SourceTreeCleanup( item->right ) ) {
        return( TRUE );
    }

    subTree = (BalancedBTree_t *)item->item;
    BalancedBTreeLock( subTree );
    if( !subTree->root ) {
        BalancedBTreeDestroy( subTree );
        BalancedBTreeRemove( item->btree, item, LOCKED, FALSE );
        memfree( item->key );
        memfree( item );
        return( TRUE );
    }
    BalancedBTreeUnlock( subTree );
    return( FALSE );
}

bool AttribTreeItemsDestroy( BalancedBTreeItem_t *item )
{
    if( !item ) {
        return( FALSE );
    }

    if( AttribTreeItemsDestroy( item->left ) ) {
        return( TRUE );
    }

    if( AttribTreeItemsDestroy( item->right ) ) {
        return( TRUE );
    }

    BalancedBTreeRemove( item->btree, item, LOCKED, FALSE );
    cJSON_Delete( item->item );
    memfree( item->key );
    memfree( item );

    return( TRUE );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

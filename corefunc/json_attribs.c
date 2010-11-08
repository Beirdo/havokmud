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
#include <math.h>
#include <float.h>
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
cJSON *GetAttribute( PlayerPC_t *pc, char *attrib, char *source );
bool DestroyAttrib2Tree( BalancedBTreeItem_t *item );
bool DestroyAttribTree( BalancedBTreeItem_t *item );

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

    if( !js || !js->source || !js->json || !pc ) {
        return;
    }

    if( !pc->attribs ) {
        pc->attribs = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
    }

    if( !pc->sources ) {
        pc->sources = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
    }

    json = strip_whitespace(js->json);
    if( !json ) {
        return;
    }

    if( strlen(json) == 0 ) {
        json = empty;
    }

    jsonTree = cJSON_Parse(json);
#ifdef DEBUG_JSON
    LogPrint(LOG_DEBUG, "JSON parsed - %s", json);
#endif
    BalancedBTreeLock( pc->attribs );
    BalancedBTreeLock( pc->sources );

    /* Find or make the subtree for the sources tree.  Once. */
    item = BalancedBTreeFind( pc->sources, &js->source, LOCKED, FALSE );
    if( item ) {
        srcSubTree = (BalancedBTree_t *)item->item;
        BalancedBTreeLock( srcSubTree );
    } else {
        srcSubTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
        BalancedBTreeLock( srcSubTree );

        item = CREATE(BalancedBTreeItem_t);
        item->key  = CREATE(char *);
        *(char **)item->key = memstrlink(js->source);
        item->item = srcSubTree;
        BalancedBTreeAdd( pc->sources, item, LOCKED, TRUE );
    }

    for( jsonItem = jsonTree->child; jsonItem; jsonItem = jsonItem->next ) {
        /* For each data item, stuff it into two separate two-layer trees */

        /* First the attribs tree.  attrib->source */
        item = BalancedBTreeFind( pc->attribs, &jsonItem->string, LOCKED, 
                                  FALSE );
        if( item ) {
            subTree = (BalancedBTree_t *)item->item;
        } else {
            subTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
            item = CREATE(BalancedBTreeItem_t);
            item->key  = CREATE(char *);
            *(char **)item->key = memstrlink(jsonItem->string);
            item->item = subTree;
            BalancedBTreeAdd( pc->attribs, item, LOCKED, FALSE );
        }

        item = BalancedBTreeFind( subTree, &js->source, UNLOCKED, FALSE );
        if( item ) {
            cJSON_Delete(item->item);
            item->item = cJSON_Clone(jsonItem);
        } else {
            item = CREATE(BalancedBTreeItem_t);
            item->key  = CREATE(char *);
            *(char **)item->key = memstrlink(js->source);
            item->item = cJSON_Clone(jsonItem);
            BalancedBTreeAdd( subTree, item, UNLOCKED, TRUE );
        }

        /* Second the sources tree.  source->attrib */
        item = BalancedBTreeFind( srcSubTree, &jsonItem->string, LOCKED, 
                                  FALSE );
        if( item ) {
            cJSON_Delete(item->item);
            item->item = cJSON_Clone(jsonItem);
        } else {
            item = CREATE(BalancedBTreeItem_t);
            item->key  = CREATE(char *);
            *(char **)item->key = memstrlink(jsonItem->string);
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
#ifdef DEBUG_JSON
    LogPrintNoArg(LOG_DEBUG, "JSON inserted into trees");
#endif
}

void AppendAttribToJSON( cJSON *jsonTree, BalancedBTreeItem_t *item )
{
    if( !item ) {
        return;
    }

    AppendAttribToJSON( jsonTree, item->left );

    cJSON_AddItemToObject( jsonTree, *(char **)item->key, 
                           cJSON_Clone( (cJSON *)item->item ) );
#ifdef DEBUG_JSON
    LogPrint(LOG_DEBUG, "Appended: %s", *(char **)item->key);
#endif

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
    cJSON_AddItemToObject(jsonTree, *(char **)item->key, jsonItem);
#ifdef DEBUG_JSON
    LogPrint(LOG_DEBUG, "Appended Tree: %s", *(char **)item->key);
#endif
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
#ifdef DEBUG_JSON
    LogPrint(LOG_DEBUG, "Appended %d items", srcCount);
#endif

    js = CREATEN(JSONSource_t, srcCount+1);

    for( i = 0, jsonItem = jsonTree->child; 
         i < srcCount && jsonItem; 
         i++, jsonItem = jsonItem->next ) {
        js[i].source = memstrlink(jsonItem->string);
        js[i].json   = cJSON_Print(jsonItem);
#ifdef DEBUG_JSON
        LogPrint(LOG_DEBUG, "Source: %s, JSON: %s", js[i].source, js[i].json );
#endif
    }

    cJSON_Delete(jsonTree);

    return( js );
}

char *CombineJSON( JSONSource_t *js )
{
    cJSON          *jsonTree;
    cJSON          *jsonItem;
    char           *json;

    jsonTree = cJSON_CreateObject();
    for( ; js->source; js++ ) {
        jsonItem = cJSON_Parse( js->json );
        cJSON_AddItemToObject( jsonTree, js->source, jsonItem );
    }

    json = cJSON_Print( jsonTree );
#ifdef DEBUG_JSON
    LogPrint(LOG_DEBUG, "Merged JSON: %s", json);
#endif
    cJSON_Delete( jsonTree );

    return( json );
}

JSONSource_t *SplitJSON( char *json )
{
    cJSON          *jsonTree;
    cJSON          *jsonItem;
    JSONSource_t   *js;
    int             count;
    int             i;
    
#ifdef DEBUG_JSON
    LogPrint(LOG_DEBUG, "JSON to split: %s", json );
#endif
    jsonTree = cJSON_Parse( json );
    for( count = 0, jsonItem = jsonTree->child; jsonItem; 
         jsonItem = jsonItem->next, count++ );

#ifdef DEBUG_JSON
    LogPrint(LOG_DEBUG, "Count: %d", count );
#endif
    js = CREATEN(JSONSource_t, count+1);
    for( i = 0, jsonItem = jsonTree->child; i < count && jsonItem;
         jsonItem = jsonItem->next, i++ ) {
        js[i].source = memstrlink(jsonItem->string);
        js[i].json   = cJSON_Print(jsonItem);
#ifdef DEBUG_JSON
        LogPrint(LOG_DEBUG, "Source: %s, JSON: %s", js[i].source, js[i].json );
#endif
    }
         
    cJSON_Delete( jsonTree );
    return( js );
}

void DestroyJSONSource( JSONSource_t *js )
{
    JSONSource_t       *jsItem;

    for( jsItem = js; jsItem->source; jsItem++ ) {
        if( jsItem->source ) {
            memfree( jsItem->source );
        }
        if( jsItem->json ) {
            memfree( jsItem->json );
        }
    }
    memfree( js );
}

void AddAttribute( char *json, char *source, PlayerPC_t *pc )
{
    JSONSource_t       *js;

    if( !json || !source || !pc ) {
        return;
    }

    js = CREATEN(JSONSource_t,2);
    if( !js ) {
        return;
    }

    js->source = memstrlink(source);
    js->json = memstrlink(json);

    AddJSONToTrees( js, pc );

    DestroyJSONSource( js );
}

void DeleteAttribute( char *attrib, char *source, PlayerPC_t *pc )
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
    item = BalancedBTreeFind( pc->attribs, &attrib, LOCKED, FALSE );
    if( item ) {
        subTree = (BalancedBTree_t *)item->item;
        BalancedBTreeLock( subTree );
        if( !source ) {
            /* Want to remove the entire attribute */
            while( AttribTreeItemsDestroy( subTree->root ) );
            BalancedBTreeDestroy( subTree );
            BalancedBTreeRemove( pc->attribs, item, LOCKED, TRUE );
            memfree( *(char **)item->key );
            memfree( item->key );
            memfree( item );
        } else {
            item = BalancedBTreeFind( subTree, &source, LOCKED, FALSE );
            if( item ) {
                BalancedBTreeRemove( subTree, item, LOCKED, TRUE );
                cJSON_Delete( item->item );
                memfree( *(char **)item->key );
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
        item = BalancedBTreeFind( pc->sources, &source, LOCKED, FALSE );
        if( item ) {
            subTree = (BalancedBTree_t *)item->item;
            BalancedBTreeLock( subTree );
            SourceDeleteAttrib( attrib, subTree );
            if( !subTree->root ) {
                /* subTree now empty! */
                BalancedBTreeDestroy( subTree );
                BalancedBTreeRemove( pc->sources, item, LOCKED, TRUE );
                memfree( *(char **)item->key );
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
    item = BalancedBTreeFind( tree, &attrib, LOCKED, FALSE );
    if( item ) {
        BalancedBTreeRemove( tree, item, LOCKED, TRUE );
        cJSON_Delete(item->item);
        memfree(*(char **)item->key);
        memfree( item->key );
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
        memfree( *(char **)item->key );
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
    memfree( *(char **)item->key );
    memfree( item->key );
    memfree( item );

    return( TRUE );
}

typedef enum {
    AGG_SUM,
    AGG_SUB_FROM_CORE,
    AGG_AVERAGE,
    AGG_RECIP_SUM,
    AGG_BIN_OR,
    AGG_BIN_AND,
    AGG_LOG_OR,
    AGG_LOG_AND,
    AGG_CONCAT
} AggregateOp_t;

typedef struct {
    char           *text;
    AggregateOp_t   op;
    int             type;
} AggregateMap_t;

static AggregateMap_t aggOpMap[] = {
    { "+",    AGG_SUM,           cJSON_Number },
    { "-",    AGG_SUB_FROM_CORE, cJSON_Number },
    { "avg",  AGG_AVERAGE,       cJSON_Number },
    { "+1/x", AGG_RECIP_SUM,     cJSON_Number },
    { "|",    AGG_BIN_OR,        cJSON_Number },
    { "&",    AGG_BIN_AND,       cJSON_Number },
    { "||",   AGG_LOG_OR,        cJSON_False  },
    { "&&",   AGG_LOG_AND,       cJSON_False  },
    { ".",    AGG_CONCAT,        cJSON_String }
};
static int aggOpMapCount = NELEMENTS(aggOpMap);

typedef struct {
    int                     iVal;
    double                  fVal;
    bool                    bVal;
    char                   *sVal;
    int                     count;
    int                     type;
} AggregateVal_t;
    

void AggregateAttrib( BalancedBTreeItem_t *item, AggregateOp_t aggOp, 
                      AggregateVal_t *aggVal )
{
    cJSON          *jsonItem;
    char           *tmp;
    int             len1;
    int             len2;

    if( !item ) {
        return;
    }

    AggregateAttrib( item->left, aggOp, aggVal );

    if( strcmp(*(char **)item->key, "core-pc") && 
        strcmp(*(char **)item->key, "aggregate") ) {
        jsonItem = (cJSON *)item->item;

        switch( aggOp ) {
        case AGG_SUM:
        case AGG_AVERAGE:
            if( jsonItem->type == cJSON_Number ) {
                aggVal->fVal += jsonItem->valuedouble;
                aggVal->count++;
            }
            break;
        case AGG_SUB_FROM_CORE:
            if( jsonItem->type == cJSON_Number ) {
                aggVal->fVal -= jsonItem->valuedouble;
                aggVal->count++;
            }
            break;
        case AGG_RECIP_SUM:
            if( jsonItem->type == cJSON_Number && 
                jsonItem->valuedouble != 0.0 ) {
                aggVal->fVal += (1.0 / jsonItem->valuedouble);
                aggVal->count++;
            }
            break;
        case AGG_BIN_OR:
            if( jsonItem->type == cJSON_Number ) {
                aggVal->iVal |= jsonItem->valueint;
                aggVal->count++;
            }
            break;
        case AGG_BIN_AND:
            if( jsonItem->type == cJSON_Number ) {
                aggVal->iVal &= jsonItem->valueint;
                aggVal->count++;
            }
            break;
        case AGG_LOG_OR:
            if( jsonItem->type == cJSON_True || 
                jsonItem->type == cJSON_False ) {
                aggVal->bVal |= jsonItem->type;
                aggVal->count++;
            }
            break;
        case AGG_LOG_AND:
            if( jsonItem->type == cJSON_True || 
                jsonItem->type == cJSON_False ) {
                aggVal->bVal &= jsonItem->type;
                aggVal->count++;
            }
            break;
        case AGG_CONCAT:
            len1 = (aggVal->sVal ? strlen(aggVal->sVal) : 0);
            len2 = (jsonItem->valuestring ? strlen(jsonItem->valuestring) : 0);

            if( len1+len2 ) {
                tmp = CREATEN(char, len1+len2+1);
                *tmp = '\0';
                
                if( aggVal->sVal ) {
                    strcat(tmp, aggVal->sVal);
                    memfree( aggVal->sVal );
                }
                if( jsonItem->valuestring ) {
                    strcat(tmp, jsonItem->valuestring);
                }
                aggVal->sVal = tmp;
                aggVal->count++;
            }
            break;
        default:
            break;
        }
    }

    AggregateAttrib( item->right, aggOp, aggVal );
}

cJSON *GetAttribute( PlayerPC_t *pc, char *attrib, char *source )
{
    BalancedBTree_t        *subTree;
    BalancedBTreeItem_t    *item;
    cJSON                  *jsonItem;
    char                   *aggOpText;
    static char            *aggOpDefault = "+";
    int                     i;
    AggregateOp_t           aggOp;
    AggregateVal_t          aggVal;

    if( !pc || !attrib || !pc->attribs ) {
        return( NULL );
    }

    BalancedBTreeLock( pc->attribs );
    item = BalancedBTreeFind( pc->attribs, &attrib, LOCKED, FALSE );
    if( !item ) {
        BalancedBTreeUnlock( pc->attribs );
        return( NULL );
    }

    subTree = (BalancedBTree_t *)item->item;
    BalancedBTreeLock( subTree );

    if( !source ) {
        /* 
         * This is an aggregated attribute.  Aggregation method is in source
         * "aggregate", defaulting to + if not found
         */
        jsonItem = GetAttribute( pc, attrib, "aggregate" );
        if( jsonItem ) {
            aggOpText = jsonItem->valuestring;
        } else {
            aggOpText = aggOpDefault;
        }

        aggOp = AGG_SUM;
        memset( &aggVal, 0x00, sizeof(AggregateVal_t) );
        for( i = 0; i < aggOpMapCount; i++ ) {
            if( !strcmp( aggOpMap[i].text, aggOpText ) ) {
                aggOp = aggOpMap[i].op;
                aggVal.type = aggOpMap[i].type;
                break;
            }
        }

        item = BalancedBTreeFind( subTree, &"core-pc", LOCKED, FALSE );
        if( item ) {
            jsonItem = (cJSON *)item->item;
            aggVal.type = jsonItem->type;
            aggVal.iVal = jsonItem->valueint;
            aggVal.fVal = jsonItem->valuedouble;
            aggVal.bVal = ((aggVal.type == cJSON_True || 
                            aggVal.type == cJSON_False) ? aggVal.type : 0);
            aggVal.sVal = memstrlink(jsonItem->valuestring);
            aggVal.count = 1;
        }
        AggregateAttrib( subTree->root, aggOp, &aggVal );

        switch( aggOp ) {
        case AGG_RECIP_SUM:
            if( aggVal.fVal != 0.0 ) {
                aggVal.fVal = 1.0 / aggVal.fVal;
                aggVal.iVal = (int)aggVal.fVal;
            }
            break;
        case AGG_AVERAGE:
            if( aggVal.count ) {
                aggVal.fVal = aggVal.fVal / (double)aggVal.count;
                aggVal.iVal = (int)aggVal.fVal;
            }
            break;
        case AGG_SUM:
        case AGG_SUB_FROM_CORE:
            aggVal.iVal = (int)aggVal.fVal;
            break;
        case AGG_BIN_OR:
        case AGG_BIN_AND:
            aggVal.fVal = (double)aggVal.iVal;
            break;
        default:
            break;
        }

        jsonItem = CREATE(cJSON);
        if( aggVal.type == cJSON_False || aggVal.type == cJSON_True ) {
            jsonItem->type = (aggVal.bVal ? cJSON_True : cJSON_False);
        } else {
            jsonItem->type = aggVal.type;
        }

        jsonItem->valuedouble = aggVal.fVal;
        jsonItem->valueint    = aggVal.iVal;
        jsonItem->valuestring = aggVal.sVal;
        jsonItem->string      = memstrlink(attrib);
    } else {
        item = BalancedBTreeFind( subTree, &source, LOCKED, FALSE );
        if( !item ) {
            jsonItem = NULL;
        }
        jsonItem = cJSON_Clone( item->item );
    }

    BalancedBTreeUnlock( subTree );
    BalancedBTreeUnlock( pc->attribs );

    return( jsonItem );
}

char *GetAttributeString( PlayerPC_t *pc, char *attrib, char *source )
{
    cJSON      *jsonItem;
    char       *string;

    jsonItem = GetAttribute(pc, attrib, source);
    if( !jsonItem ) {
        return( NULL );
    }
    string = memstrlink(jsonItem->valuestring);
    cJSON_Delete(jsonItem);

    return( string );
}

int GetAttributeInt( PlayerPC_t *pc, char *attrib, char *source )
{
    cJSON      *jsonItem;
    int         value;

    jsonItem = GetAttribute(pc, attrib, source);
    if( !jsonItem ) {
        return( 0 );
    }
    value = jsonItem->valueint;
    cJSON_Delete(jsonItem);

    return( value );
}

double GetAttributeDouble( PlayerPC_t *pc, char *attrib, char *source )
{
    cJSON      *jsonItem;
    double      value;

    jsonItem = GetAttribute(pc, attrib, source);
    if( !jsonItem ) {
        return( 0.0 );
    }
    value = jsonItem->valuedouble;
    cJSON_Delete(jsonItem);

    return( value );
}

bool GetAttributeBool( PlayerPC_t *pc, char *attrib, char *source )
{
    cJSON      *jsonItem;
    bool        value;

    jsonItem = GetAttribute(pc, attrib, source);
    if( !jsonItem ) {
        return( FALSE );
    }
    value = (jsonItem->type == cJSON_True ? TRUE : FALSE );
    cJSON_Delete(jsonItem);

    return( value );
}

void SetAttributeString( PlayerPC_t *pc, char *attrib, char *source, 
                         char *val )
{
    char           *string;
    int             len;

    if( !attrib | !source ) {
        return;
    }

    if( !val ) {
        len = 2;
    } else {
        len = strlen(val);
    }
    string = CREATEN(char, strlen(attrib)+len+11);
    if( !val ) {
        sprintf( string, "{ \"%s\": null }", attrib );
    } else {
        sprintf( string, "{ \"%s\": \"%s\" }", attrib, val );
    }

    AddAttribute( string, source, pc );
    memfree( string );
}

void SetAttributeInt( PlayerPC_t *pc, char *attrib, char *source, int val )
{
    char           *string;

    if( !attrib | !source ) {
        return;
    }

    string = CREATEN(char, strlen(attrib)+21+9);
    sprintf( string, "{ \"%s\": %d }", attrib, val );

    AddAttribute( string, source, pc );
    memfree( string );
}

void SetAttributeDouble( PlayerPC_t *pc, char *attrib, char *source, 
                         double val )
{
    char           *string;
    char           *sval;
    int             len;

    if( !attrib | !source ) {
        return;
    }

    sval   = CREATEN(char, 64);

    if( fabs(floor(val)-val) <= DBL_EPSILON ) {
        sprintf( sval, "%.0f", val );
    } else if( fabs(val) < 1.0e-6 || fabs(val) > 1.0e9 ) {
        sprintf( sval, "%e", val );
    } else {
        sprintf( sval, "%f", val );
    }

    string = CREATEN(char, strlen(attrib)+strlen(sval)+9);
    sprintf( string, "{ \"%s\": %s }", attrib, sval );

    AddAttribute( string, source, pc );
    memfree( string );
    memfree( sval );
}

void SetAttributeBool( PlayerPC_t *pc, char *attrib, char *source, bool val )
{
    char           *string;

    if( !attrib | !source ) {
        return;
    }

    string = CREATEN(char, strlen(attrib)+5+9);
    sprintf( string, "{ \"%s\": %s }", attrib, (val ? "true" : "false") );

    AddAttribute( string, source, pc );
    memfree( string );
}

bool DestroyAttrib2Tree( BalancedBTreeItem_t *item )
{
    if( !item ) {
        return( FALSE );
    }

    if( DestroyAttrib2Tree( item->left ) ) {
        return( TRUE );
    }

    if( DestroyAttrib2Tree( item->right ) ) {
        return( TRUE );
    }

    BalancedBTreeRemove( item->btree, item, LOCKED, FALSE );
    cJSON_Delete( item->item );
    memfree( *(char **)item->key );
    memfree( item->key );
    memfree( item );
    return( TRUE );
}

bool DestroyAttribTree( BalancedBTreeItem_t *item )
{
    BalancedBTree_t    *subTree;

    if( !item ) {
        return( FALSE );
    }

    if( DestroyAttribTree( item->left ) ) {
        return( TRUE );
    }

    if( DestroyAttribTree( item->right ) ) {
        return( TRUE );
    }

    subTree = (BalancedBTree_t *)item->item;
    BalancedBTreeLock( subTree );
    while( DestroyAttrib2Tree( subTree->root ) );
    BalancedBTreeDestroy( subTree );

    BalancedBTreeRemove( item->btree, item, LOCKED, FALSE );
    memfree( *(char **)item->key );
    memfree( item->key );
    memfree( item );
    return( TRUE );
}

void DestroyAttributes( PlayerPC_t *pc )
{
    if( !pc ) {
        return;
    }

    if( pc->attribs ) {
        BalancedBTreeLock( pc->attribs );
        while( DestroyAttribTree( pc->attribs->root ) );
        BalancedBTreeDestroy( pc->attribs );
        pc->attribs = NULL;
    }

    if( pc->sources ) {
        BalancedBTreeLock( pc->sources );
        while( DestroyAttribTree( pc->sources->root ) );
        BalancedBTreeDestroy( pc->sources );
        pc->sources = NULL;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

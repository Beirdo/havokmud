
#include <stdio.h>

#include "protos.h"

#define HASH_KEY(ht,key)    ( (((unsigned int)(key)) * 17) % (ht)->table_size )

void init_hash_table(struct hash_header *ht, int rec_size, int table_size)
{
    ht->rec_size = rec_size;
    ht->table_size = table_size;
    ht->buckets = (void *) calloc(sizeof(struct hash_link **), table_size);
    ht->keylist = (void *) malloc(sizeof(ht->keylist) * (ht->klistsize = 128));
    ht->klistlen = 0;
}

void init_world(struct room_data *room_db[])
{
    bzero(room_db, sizeof(struct room_data *) * WORLD_SIZE);
}

void destroy_hash_table(struct hash_header *ht, void (*gman) ())
{
    int             i;
    struct hash_link *scan,
                   *temp;

    for (i = 0; i < ht->table_size; i++) {
        for (scan = ht->buckets[i]; scan;) {
            temp = scan->next;
            (*gman) (scan->data);
            if (scan) {
                free(scan);
            }
            scan = temp;
        }
    }
    if (ht->buckets) {
        free(ht->buckets);
    }
    if (ht->keylist) {
        free(ht->keylist);
    }
}

void _hash_enter(struct hash_header *ht, long key, void *data)
{
    struct hash_link *temp;
    int             i;

    /* 
     * precondition: there is no entry for <key> yet 
     */

    temp = (struct hash_link *) malloc(sizeof(struct hash_link));
    temp->key = key;
    temp->next = ht->buckets[HASH_KEY(ht, key)];
    temp->data = data;
    ht->buckets[HASH_KEY(ht, key)] = temp;
    if (ht->klistlen >= ht->klistsize) {
        ht->keylist = (void *) realloc(ht->keylist, sizeof(*ht->keylist) *
                                                    (ht->klistsize *= 2));
    }

    for (i = ht->klistlen; i >= 0; i--) {
        if (ht->keylist[i - 1] < key) {
            ht->keylist[i] = key;
            break;
        }
        ht->keylist[i] = ht->keylist[i - 1];
    }
    ht->klistlen++;
}

struct room_data *room_find(struct room_data *room_db[], long key)
{
    return ((key < WORLD_SIZE && key > -1) ? room_db[key] : 0);
}

void           *hash_find(struct hash_header *ht, long key)
{
    struct hash_link *scan;

    scan = ht->buckets[HASH_KEY(ht, key)];

    while (scan && scan->key != key) {
        scan = scan->next;
    }
    return scan ? scan->data : NULL;
}

int room_enter(struct room_data *rb[], long key, struct room_data *rm)
{
    struct room_data *temp;

    temp = room_find(rb, key);
    if (temp) {
        return (0);
    }
    rb[key] = rm;
    return (1);

}

int hash_enter(struct hash_header *ht, long key, void *data)
{
    void           *temp;
    temp = hash_find(ht, key);
    if (temp) {
        return 0;
    }
    _hash_enter(ht, key, data);
    return 1;
}

struct room_data *room_find_or_create(struct room_data *rb[], long key)
{
    struct room_data *rv;

    rv = room_find(rb, key);
    if (rv) {
        return rv;
    }
    rv = (struct room_data *) malloc(sizeof(struct room_data));

    rb[key] = rv;

    return rv;
}

void           *hash_find_or_create(struct hash_header *ht, long key)
{
    void           *rval;

    rval = hash_find(ht, key);
    if (rval) {
        return rval;
    }
    rval = (void *) malloc(ht->rec_size);
    _hash_enter(ht, key, rval);
    return rval;
}

int room_remove(struct room_data *rb[], long key)
{

    struct room_data *tmp;

    tmp = room_find(rb, key);

    if (tmp) {
        rb[key] = 0;
        free(tmp);
    }
    return (0);
}

void           *hash_remove(struct hash_header *ht, long key)
{
    struct hash_link **scan;
    int             i;

    struct hash_link *temp,
                   *aux;

    scan = ht->buckets + HASH_KEY(ht, key);

    while (*scan && (*scan)->key != key) {
        scan = &(*scan)->next;
    }
    if (*scan) {
        temp = (*scan)->data;
        aux = *scan;
        *scan = aux->next;
        if (aux) {
            free(aux);
        }
        for (i = 0; i < ht->klistlen; i++) {
            if (ht->keylist[i] == key) {
                break;
            }
        }
        if (i < ht->klistlen) {
            bcopy(ht->keylist + i + 1, ht->keylist + i,
                  (ht->klistlen - i) * sizeof(*ht->keylist));
            ht->klistlen--;
        }

        return temp;
    }

    return NULL;
}

void room_iterate(struct room_data *rb[], void (*func) (), void *cdata)
{
    register int    i;
    struct room_data *temp;

    for (i = 0; i < WORLD_SIZE; i++) {
        temp = room_find(rb, i);
        if (temp) {
            (*func) (i, temp, cdata);
        }
    }
}

void hash_iterate(struct hash_header *ht, void (*func) (), void *cdata)
{
    int             i;
    void           *temp;
    register long   key;

    for (i = 0; i < ht->klistlen; i++) {
        key = ht->keylist[i];
        temp = hash_find(ht, key);
        (*func) (key, temp, cdata);
        if (ht->keylist[i] != key) {
            /* 
             * They must have deleted this room 
             * Hit this slot again. 
             */
            i--;
        }
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

#ifndef _hash_h
#define _hash_h

struct hash_link {
    int             key;
    struct hash_link *next;
    void           *data;
};

struct hash_header {
    int             rec_size;
    int             table_size;
    int            *keylist,
                    klistsize,
                    klistlen;   /* this is really lame, AMAZINGLY lame */
    struct hash_link **buckets;
};

#define WORLD_SIZE 60000        /* raised due to limit being hit - Mythos
                                 * 9-25-01 */
/*
 * Lowered a bit just because... (from 80k to 60k) 
 */
#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

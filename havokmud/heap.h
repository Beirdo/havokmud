#ifndef _heap_h
#define _heap_h

struct StrHeapList {
    char           *string;     /* the matching string */
    int             total;      /* total # of occurences */
};

struct StrHeap {
    int             uniq;       /* number of uniq items in list */
    struct StrHeapList *str;    /* the list of strings and totals */
};

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

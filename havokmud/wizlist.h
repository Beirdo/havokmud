#ifndef _wizlist_h
#define _wizlist_h

struct wiznest {
    char           *name;
    char           *title;
    time_t          last_logon;
    int             active;
};

struct wiznode {
    struct wiznest  stuff[150];
};

struct wizlistgen {
    int             number[MAX_CLASS];
    struct wiznode  lookup[10];
};

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

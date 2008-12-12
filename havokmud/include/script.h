#ifndef _script_h
#define _script_h

struct foo_data {
    char           *line;
};

struct scripts {
    char           *filename;   /* Script name to execute */
    long            vnum;       /* Virtual number of the mob associated
                                 * with the script */
    struct foo_data *script;    /* actual script */
}              *script_data;

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

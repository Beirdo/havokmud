#ifndef _parser_h
#define _parser_h

#define MAGIC    96             /* Magic number for the hash table */

typedef struct command_node NODE;

struct command_node {
    char           *name;
    void            (*func) (struct char_data * ch, char *arg, int cmd);
    int             number;
    byte            min_pos;
    byte            min_level;
    byte            log;
    struct command_node *next;
    struct command_node *previous;
};

struct radix_list {
    struct command_node *next;
    unsigned short int number;
    byte            max_len;
};

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

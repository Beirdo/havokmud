#ifndef _dimd_h
#define _dimd_h

#define DIMD_PASSWORD "sdfj3r878jkasdjhk2ee987w"
#define DIMD_PORT 7002

#define DIMD_OPENING    1
#define DIMD_OPEN       2
#define DIMD_CLOSED     3
#define DIMD_CLOSING    4

#define DIMDLOG(msg) Log(msg)

#define UNDEFINED -1
#define GET_KEYNAME(ch) GET_NAME(ch)
#define PER(ch) GET_KEYNAME(ch)
#define HUMAN(ch) IsHumanoid(ch)

#define GET_DIMD(ch) ((ch)->pc->dimd_credits)

#define CLR_THINK       0
#define CLR_MUSE        0
#define CLR_GOSSIP      0
#define CLR_TELL        0

#define COMM_NOGOSSIP   1
#define COMM_NOMUSE     2
#define COMM_NOTHINK    4
#define COMM_AFK        8

/*
 * external functions 
 */
void            construct_who_list(char *buf, int cmd,
                                   struct char_data *ch, int godlevel,
                                   char *argument);
int             init_socket(int port);

/*
 * external vars 
 */
extern struct char_data *character_list;

/*
 * internal vars 
 */
extern int      dimd_s;
extern bool     dimd_on;
extern bool     dimd_gossip;
extern bool     dimd_muse;
extern int      dimd_state;
extern int      dimd_port;
extern int      dimd_local_mud;

struct mud_data {
    char           *formalname;
    char           *nicknames;
    char           *address;
    int             port;
    int             min_level;
    int             immort_level;
    int             immort_level_mult;

    unsigned long   flags;

    int             desc;

    struct txt_q    input;
    struct txt_q    output;

    char            buf[MAX_STRING_LENGTH];

    char            pcname[50];

    char            host[50];
};

/*
 * Only options that have a plus(+) in their comments should be used in
 * the mud table.  All other options are basically internal. 
 */
#define DD_CONNECTED    0x00000001
#define DD_VERIFIED     0x00000002
#define DD_AUTOTRY      0x00000004      /* +Try to connect at bootup? */
#define DD_REFUSE       0x00000008      /* +Refuse connections to this
                                         * mud? */
#define DD_NOGOSSIP     0x00000010      /* Mud refusing our gossips? */
#define DD_NOMUSE       0x00000020      /* Mud refusing our muses? */
#define DD_NOTHINK      0x00000040      /* Mud refusing our thinks? */

/*
 * functions 
 */

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

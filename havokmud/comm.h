#ifndef _comm_h
#define _comm_h

#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3

#ifndef BLOCK_WRITE
#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output)
#else
#define SEND_TO_Q(messg, desc)  write_to_output((messg), desc)
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

#include "platform.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"
#include "db.h"


void display_usage(char *progname)
{
    fprintf(stderr, "Usage:\n"
                    "%s [-l] [-d libdir] [-D sqlDB] [-U sqlUser] "
                    "[-P sqlPass] [-H sqlHost] [-h]\n\r", progname );
    fprintf(stderr, "\t=d\tDefine the library dir (default %s)\n"
                    "\t-D\tDefine the MySQL database (default %s)\n",
                    DFLT_DIR, DEF_MYSQL_DB );
    fprintf(stderr, "\t-U\tDefine the MySQL user (default %s)\n"
                    "\t-P\tDefine the MySQL password (default %s)\n"
                    "\t-H\tDefine the MySQL host (default %s)\n",
                    DEF_MYSQL_USER, DEF_MYSQL_PASSWD, DEF_MYSQL_HOST );
    fprintf(stderr, "\t-h\tShow this help page\n" );
}

int main(int argc, char **argv)
{
    char           *dir;
    int             opt;
    extern char    *optarg;

    dir = NULL;

    while( (opt = getopt(argc, argv, "d:D:U:P:H:h")) != -1 ) {
        switch (opt) {
        case 'd':
            if( dir ) {
                free( dir );
            }
            dir = strdup(optarg);
            break;

        case 'D':
            /* Database */
            if( mysql_db ) {
                free( mysql_db );
            }
            mysql_db = strdup(optarg);
            break;

        case 'U':
            /* Database user */
            if( mysql_user ) {
                free( mysql_user );
            }
            mysql_user = strdup(optarg);
            break;

        case 'P':
            /* Database password */
            if( mysql_passwd ) {
                free( mysql_passwd );
            }
            mysql_passwd = strdup(optarg);
            break;

        case 'H':
            /* Database hose */
            if( mysql_host ) {
                free( mysql_host );
            }
            mysql_host = strdup(optarg);
            break;

        case ':':
        case '?':
        case 'h':
        default:
            display_usage(argv[0]);
            exit(1);
            break;
        }
    }

    printf("Using %s as data directory.", dir);

    if (chdir(dir) < 0) {
        perror("chdir");
        exit(1);
    }

    free( dir );

    db_setup();
    boot_db();
    do_orebuild(NULL, NULL, 0);

    return (0);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

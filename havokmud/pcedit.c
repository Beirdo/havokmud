#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "protos.h"

/*
 * once file is converted BE SURE to update the structs.h file 
 */

#define NEW_MAX_SKILLS          400
#define NEW_MAX_AFFECT          40
#define NEW_ABS_MAX_CLASS       20
#define NEW_MAX_TOUNGE          3
#define NEW_MAX_NEW_LANGUAGES   10
#define NEW_MAX_SAVES           8
#define NEW_MAX_CONDITIONS      5

/*
 * new structs here 
 */
#if 0
struct char_file_u_new {
    int             class;
    byte            sex;
    byte            level[ABS_MAX_CLASS];
    unsigned short int q_points;        /* Quest points of player */
    time_t          birth;      /* Time of birth of character */
    int             played;     /* Number of secs played in total */
    int             race;
    unsigned int    weight;
    unsigned int    height;
    char            title[80];
    char            extra_str[255];
    sh_int          hometown;
    char            description[240];
    bool            talks[MAX_TOUNGE];
    long            extra_flags;
    sh_int          load_room;  /* Which room to place char in */
    struct char_ability_data abilities;
    struct char_point_data points;
    struct char_skill_data skills[MAX_SKILLS];
    struct affected_type affected[MAX_AFFECT];
    /*
     * specials 
     */
    byte            spells_to_learn;
    int             alignment;
    long            affected_by;
    long            affected_by2;
    time_t          last_logon; /* Time (in secs) of last logon */
    long            act;        /* ACT Flags */
    /*
     * char data 
     */
    char            name[20];
    char            alias[30];  /* for later use.... */
    char            pwd[11];
    sh_int          apply_saving_throw[MAX_SAVES];
    int             conditions[MAX_CONDITIONS];
    int             startroom;  /* which room the player should start in */
    long            user_flags; /* no-delete,use ansi,etc... */
    int             speaking;   /* language currently speakin in */

};
#else

struct char_file_u_new {
    int             class;
    byte            sex;
    byte            level[ABS_MAX_CLASS];
    unsigned short int q_points;        /* Quest points of player */
    time_t          birth;      /* Time of birth of character */
    int             played;     /* Number of secs played in total */
    int             race;
    unsigned int    weight;
    unsigned int    height;
    char            title[80];
    char            extra_str[255];
    sh_int          hometown;
    char            description[240];
    bool            talks[MAX_TOUNGE];
    long            extra_flags;
    sh_int          load_room;  /* Which room to place char in */
    struct char_ability_data abilities;
    struct char_point_data points;
    struct char_skill_data skills[400];
    struct affected_type affected[MAX_AFFECT];
    /*
     * specials 
     */
    byte            spells_to_learn;
    int             alignment;
    long            affected_by;
    long            affected_by2;
    time_t          last_logon; /* Time (in secs) of last logon */
    long            act;        /* ACT Flags */

    /*
     * char data 
     */
    char            name[20];
    char            alias[30];  /* for later use.... */
    char            pwd[11];
    sh_int          apply_saving_throw[MAX_SAVES];
    int             conditions[MAX_CONDITIONS];
    int             startroom;  /* which room the player should start in */
    long            user_flags; /* no-delete,use ansi,etc... */
    int             speaks;     /* language currently speakin in */

    int             a_deaths;
    int             a_kills;
    int             m_deaths;
    long            m_kills;

    int             remortclass;

};
#endif

struct my_char_data {
    struct char_file_u grunt;   /* contained in structs.h */
    struct char_file_u_new grunt2;      /* contained here */
    short           AXE;
}             **dummy;

void            muck(int orig_ammt, char name[80]);
void            specific_axe(int orig_ammt, char name[80]);
void            inactive_god_axe(int orig_ammt, time_t CURRENT_TIME);
void            change_struct(int orig_ammt);
void            inactive_axe(int orig_ammt, time_t CURRENT_TIME);
void            zero_bank();
char           *time_print(long et);
char           *lower(char *s);
int             load_playerfile(char *argv[]);
int             convert_playerfile(char *argv[]);
void            test(int orig_ammt);
int             spit_out_remains(char *argv[], int ammt);

int             orig_ammt = 0,
                after_ammt = 0;
int             our_pos = 0;

int get_int(int min, int max, int zero_ok)
{
    char            buff[20];
    int             i,
                    isok = FALSE;
    do {
        fflush(stdin);
        printf(" ->");
        fgets(buff, sizeof(buff), stdin);
        buff[strlen(buff) - 1] = '\0';
        i = atoi(buff);

        if (i > max || i < min) {
            printf("Hey bozo, you can't do that.\n");
        } else if (i == 0 && !zero_ok) {
            printf("Sorry peanut-head, that was unacceptable.\n");
        } else {
            isok = TRUE;
        }
    } while (!isok);

    return i;
}

void get_string(char *s, int len)
{
    do {
        fflush(stdin);
        printf(" =>");
        fgets(s, len, stdin);
        s[strlen(s) - 1] = '\0';
        if (*s == '\0') {
            printf("There was nothing there!\n");
        } else if (*s == ' ') {
            printf("Please do not start with a blank.\n");
        }
    } while (*s == '\0' || *s == ' ');
}

int main(int argc, char *argv[])
{
    char            tempbuf[255];
    time_t          CURRENT_TIME;
    int             choice;
    int             i;

    if (argc != 3) {
        printf("please use this program in the following format:");
        printf("\n<%s> <input file> <output file>\n", argv[0]);
        exit(0);
    }

    if (!strcmp(argv[1], argv[2])) {
        printf("\nPlease use a seperate files for input and output.\n");
        exit(0);
    }
    orig_ammt = load_playerfile(argv);
    CURRENT_TIME = time(0);

    for (i = 0; i < orig_ammt; i++) {
        dummy[i]->AXE = FALSE;
    }
    printf("\nStructures initialized.");
    printf("\nCurrent time: %s\n", time_print(CURRENT_TIME));

    do {
        printf("\n    What to do?");
        printf("\n0 -\t Exit");
        printf("\n1 -\t Routine deletion of low level inactive characters.");
        printf("\n2 -\t Routine demotion of inactive gods.");
        printf("\n3 -\t Delete a specific player.");
        printf("\n4 -\t Muck with a player in depth.");
        printf("\n5 -\t Find those who have a certain item number.");
        printf("\n6 -\t Zero all bank accounts.");
        printf("\n7 -\t Convert Playerfile to new structure.");
        printf("\n\nChanges are updated upon entering 0");
        printf("\nWell?");
        choice = get_int(0, 7, TRUE);
        switch (choice) {
        case 1:
            inactive_axe(orig_ammt, CURRENT_TIME);
            break;
        case 2:
            inactive_god_axe(orig_ammt, CURRENT_TIME);
            break;
        case 3:
            printf("\nWhom do you want to delete?\n");
            get_string(tempbuf, 255);
            specific_axe(orig_ammt, tempbuf);
            break;
        case 4:
            printf("\nWhom do you want to mess around with?\n");
            get_string(tempbuf, 255);
            printf("\nMucking with:  %s\n", tempbuf);
            muck(orig_ammt, tempbuf);
            break;
        case 5:
            printf("\n -- dis-abeled --\n");

            test(orig_ammt);
            /*
             * printf("\nWhat item number?\n"); j=get_int(1,999999,FALSE);
             * access_rent_files(orig_ammt,j); 
             */
            break;
        case 6:
            zero_bank();
            break;
        case 7:
            convert_playerfile(argv);
            break;
        default:
            after_ammt = spit_out_remains(argv, orig_ammt);
            choice = 0;
            printf("Wrote %d players.\n", after_ammt);
            printf("Playerfile adjusted by %d players.\n",
                   after_ammt - orig_ammt);
            printf("\nbye!\n");
            break;
        }
    } while (choice);
    return(0);
}

void test(int orig_ammt)
{
    int             i;
    char            best[256],
                    best2[256];
    long            bestmkill = 0;
    int             bestrich = 0;

    for (i = 0; i < orig_ammt; i++) {
        if (dummy[i]->grunt.m_kills > bestmkill) {
            bestmkill = dummy[i]->grunt.m_kills;
            sprintf(best, "%s", dummy[i]->grunt.name);
        }

        if (dummy[i]->grunt.points.gold > bestrich) {
            bestrich = dummy[i]->grunt.points.gold;
            sprintf(best2, "%s", dummy[i]->grunt.name);
        }
    }

    printf("%s is the mad killer of the mud with %ld kills \n\r", best,
           bestmkill);
    printf("%s is the richest of the mud with %d kills \n\r", best2,
           bestrich);
}

void change_struct(int orig_ammt)
{
}

void menu2()
{
    printf("\n\n\n\t You can anything below, and more :)");
    printf("\n1\t- edit levels");
    printf("\n2\t- change the room that someone starts in.");
    printf("\n3\t- change race (ghost is 29 (no can cast)).");
    printf("\n> ");
}

void muck(int orig_ammt, char name[80])
{
    void            menu2();
    int             i;
    int             l,
                    m,
                    count,
                    f;
    char            buffer[255];

    printf("\ndoing Upper on:%s\n", name);
    name[0] = toupper(name[0]);
    printf("\nLooking for :%s\n", name);
    printf("Do you wish to search from the beginning? (Y/N) ");
    get_string(buffer, 255);

    if ((buffer[0] == 'y') || (buffer[0] == 'Y')) {
        f = 0;
    } else {
        f = our_pos;
    }

    printf("\nf:%d:our_pos:%d\n", f, our_pos);

    for (i = f; i < orig_ammt; i++) {
        printf("\nChecking :%d:%s\n", i, dummy[i]->grunt.name);
        if (!(strcmp(name, dummy[i]->grunt.name))) {
            our_pos = i;

            printf("Levels: M:%d C:%d W:%d T:%d D:%d B:%d S:%d P:%d R:%d P:%d "
                    "N:%d" ,
                 dummy[i]->grunt.level[0], dummy[i]->grunt.level[1],
                 dummy[i]->grunt.level[2], dummy[i]->grunt.level[3],
                 dummy[i]->grunt.level[4], dummy[i]->grunt.level[5],
                 dummy[i]->grunt.level[6], dummy[i]->grunt.level[7],
                 dummy[i]->grunt.level[8], dummy[i]->grunt.level[9],
                 dummy[i]->grunt.level[10]);

            printf("\nCurrent STR is %d.", dummy[i]->grunt.abilities.str);
            printf("\nCurrent STR_add is %d.",
                   dummy[i]->grunt.abilities.str_add);
            printf("\nCurrent INT is %d.",
                   dummy[i]->grunt.abilities.intel);
            printf("\nCurrent WIS is %d.", dummy[i]->grunt.abilities.wis);
            printf("\nCurrent DEX is %d.", dummy[i]->grunt.abilities.dex);
            printf("\nCurrent CON is %d.", dummy[i]->grunt.abilities.con);
            printf("\nCurrent CHA is %d.", dummy[i]->grunt.abilities.chr);

            f = 1;
            while (f) {
                menu2();
                l = get_int(0, 4, TRUE);
                switch (l) {
                case 1:
                    printf("%s:\n", dummy[i]->grunt.name);
                    printf("Levels: M:%d C:%d W:%d T:%d D:%d B:%d S:%d P:%d "
                            "R:%d P:%d N:%d ",
                         dummy[i]->grunt.level[0], dummy[i]->grunt.level[1],
                         dummy[i]->grunt.level[2], dummy[i]->grunt.level[3],
                         dummy[i]->grunt.level[4], dummy[i]->grunt.level[5],
                         dummy[i]->grunt.level[6], dummy[i]->grunt.level[7],
                         dummy[i]->grunt.level[8], dummy[i]->grunt.level[9],
                         dummy[i]->grunt.level[10]);

                    for (count = 0; count <= 10; count++) {
                        printf("Current Level %d: %d\n", count,
                               dummy[i]->grunt.level[count]);
                        printf("New Level [%d]: ",
                               dummy[i]->grunt.level[count]);
                        f = get_int(0, 60, TRUE);
                        if (f) {
                            dummy[i]->grunt.level[count] = f;
                        }
                    }

                    printf("Levels: M:%d C:%d W:%d T:%d D:%d B:%d S:%d P:%d "
                           "R:%d P:%d N:%d ",
                         dummy[i]->grunt.level[0], dummy[i]->grunt.level[1],
                         dummy[i]->grunt.level[2], dummy[i]->grunt.level[3],
                         dummy[i]->grunt.level[4], dummy[i]->grunt.level[5],
                         dummy[i]->grunt.level[6], dummy[i]->grunt.level[7],
                         dummy[i]->grunt.level[8], dummy[i]->grunt.level[9],
                         dummy[i]->grunt.level[10]);
                    break;
                case 2:
                    printf("\n%s currently starts in room %d.",
                           dummy[i]->grunt.name, dummy[i]->grunt.load_room);
                    printf("\nNew room?\n");
                    dummy[i]->grunt.load_room = get_int(0, 40000, TRUE);
                    printf("\nStart room set to %d.",
                           dummy[i]->grunt.load_room);
                    break;
                case 3:
                    printf("\nCurrent race is %d. (go look it up in race.h)",
                           dummy[i]->grunt.race);
                    printf("\nNew race is?\n");
                    dummy[i]->grunt.race = get_int(0, 84, TRUE);
                    printf("\nRace set to %d.", dummy[i]->grunt.race);
                    break;
                case 4:
                    printf("\n 0) exit 1) str 2) int 3) wis 4) dex 5) con 6) "
                           "cha 7) str_add\n");
                    printf("\nWhich stat?? -> ");
                    m = get_int(0, 7, TRUE);

                    switch (m) {
                    case 1:
                        printf("\nCurrent STR is %d.",
                               dummy[i]->grunt.abilities.str);
                        printf("\nNew STR? -> \n");
                        dummy[i]->grunt.abilities.str = get_int(1, 25, FALSE);
                        printf("\nSTR set to %d.", 
                               dummy[i]->grunt.abilities.str);
                        break;
                    case 2:
                        printf("\nCurrent INT is %d.",
                               dummy[i]->grunt.abilities.intel);
                        printf("\nNew INT? -> \n");
                        dummy[i]->grunt.abilities.intel = get_int(1, 25, FALSE);
                        printf("\nINT set to %d.",
                               dummy[i]->grunt.abilities.intel);
                        break;
                    case 3:
                        printf("\nCurrent WIS is %d.",
                               dummy[i]->grunt.abilities.wis);
                        printf("\nNew WIS? -> \n");
                        dummy[i]->grunt.abilities.wis = get_int(1, 25, FALSE);
                        printf("\nWIS set to %d.",
                               dummy[i]->grunt.abilities.wis);
                        break;
                    case 4:
                        printf("\nCurrent DEX is %d.",
                               dummy[i]->grunt.abilities.dex);
                        printf("\nNew DEX? -> \n");
                        dummy[i]->grunt.abilities.dex = get_int(1, 25, FALSE);
                        printf("\nDEX set to %d.",
                               dummy[i]->grunt.abilities.dex);
                        break;
                    case 5:
                        printf("\nCurrent CON is %d.",
                               dummy[i]->grunt.abilities.con);
                        printf("\nNew CON? -> ");
                        dummy[i]->grunt.abilities.con = get_int(1, 25, FALSE);
                        printf("\nCON set to %d.",
                               dummy[i]->grunt.abilities.con);
                        break;
                    case 6:
                        printf("\nCurrent CHA is %d.",
                               dummy[i]->grunt.abilities.chr);
                        printf("New CHA? -> ");
                        dummy[i]->grunt.abilities.chr = get_int(1, 25, FALSE);
                        printf("\nCHA set to %d.",
                               dummy[i]->grunt.abilities.chr);
                        break;
                    case 7:
                        printf("\nCurrent STR_add is %d.",
                               dummy[i]->grunt.abilities.str_add);
                        printf("\nNew STR_add? -> \n");
                        dummy[i]->grunt.abilities.str_add = 
                            get_int(0, 99, TRUE);
                        printf("\nSTR_add set to %d.",
                               dummy[i]->grunt.abilities.str_add);
                        break;
                    default:
                        printf("\nFinished with Stats ... \n");
                        break;
                    }
                    break;
                default:
                    printf("finished mucking ... \n");
                    break;
                }
            }
        }
    }
}

void specific_axe(int orig_ammt, char *name)
{
    int             i;
    int             j = FALSE;

    name[0] = toupper(name[0]);
    for (i = 0; i < orig_ammt && !j; i++) {
        if (!(strcmp(name, dummy[i]->grunt.name))) {
            dummy[i]->AXE = TRUE;
            printf("%s's head found, putting it on the block.\n",
                   dummy[i]->grunt.name);
            j = TRUE;
        }
    }

    if (!j) {
        printf("%s's head could not be found.", name);
    }
}

void inactive_god_axe(int orig_ammt, time_t CURRENT_TIME)
{
    int             i,
                    j,
                    max;
    int             amt = 0;

    i = j = max = 0;

    for (i = 0; i < orig_ammt; i++) {
        if (CURRENT_TIME - dummy[i]->grunt.last_logon > (long) (86400 * 30)) {
            for (j = 0, max = 0; j < 6; j++) {
                if (dummy[i]->grunt.level[j] > max) {
                    max = dummy[i]->grunt.level[j];
                }
            }

            if (max > 51 && max < 59) {
                printf("%s prepared for demotion.", dummy[i]->grunt.name);
                printf("  Was %d, now is", max);
                max--;
                for (j = 0; j < 6; j++) {
                    dummy[i]->grunt.level[j] = max;
                }
                printf(" %d.\n", max);
                amt++;
            } else if (max == 51) {
                printf("%s will be deleted.\n", dummy[i]->grunt.name);
                dummy[i]->AXE = TRUE;
            }
        }
    }
}

void inactive_axe(int orig_ammt, time_t CURRENT_TIME)
{
    int             i,
                    j,
                    max;
    char            buf[254];
    int             amt = 0;
    i = j = max = 0;

    for (i = 0; i < orig_ammt; i++) {

#if 0
        if ((CURRENT_TIME - dummy[i]->grunt.last_logon) >
            (long) (86400 * 120))
#else
        /*
         * new 
         */
        if ((CURRENT_TIME - dummy[i]->grunt.last_logon) >
            (long) (16400 * 120))
#endif
        {
            for (j = 0, max = 0; j < MAX_CLASS; j++) {
                if (dummy[i]->grunt.level[j] > max) {
                    max = dummy[i]->grunt.level[j];
                }
            }

            if (max < 51) {
                dummy[i]->AXE = TRUE;
                amt++;
                sprintf(buf, "rent/%s", lower(dummy[i]->grunt.name));
                unlink(buf);
                sprintf(buf, "rent/%s.aux", dummy[i]->grunt.name);
                unlink(buf);
            }
        }
    }
    printf("\n%d players prepared for axing.", amt);
}

int spit_out_remains(char *argv[], int ammt)
{
    FILE           *fl;
    int             i,
                    j;

    if (!(fl = fopen(argv[2], "w"))) {
        printf("\nCan not open %s, bye!\n", argv[2]);
        exit(0);
    }

    i = j = 0;
    for (; i < ammt; i++) {
        if (!(dummy[i]->AXE)) {
            fwrite(&(dummy[i]->grunt), sizeof(struct char_file_u), 1, fl);
            j++;
        }
    }
    return j;
}

int load_playerfile(char *argv[])
{
    int             ammt = 0;
    int             test;
    FILE           *fl;

    if (!(fl = fopen(argv[1], "r"))) {
        printf("\nCan not open %s, bye!\n", argv[1]);
        exit(0);
    }
#if 0
    if (!(fl2 = fopen(argv[2], "w"))) {
        printf("\nCan not open %s, bye!\n", argv[2]);
        exit(0);
    }
#endif

    dummy = (struct my_char_data **) malloc(5500 * sizeof(dummy));
    /*
     * access_rent_files(); 
     */
    if (dummy == NULL) {
        printf("ack\n");
    }

    for (; !feof(fl);) {
        dummy[ammt] =
            (struct my_char_data *) malloc(sizeof(struct my_char_data));
        test = fread(&(dummy[ammt]->grunt), sizeof(struct char_file_u), 1, fl);

        if (!feof(fl) && strcmp(dummy[ammt]->grunt.name, "111111") &&
            strcmp(dummy[ammt]->grunt.name, "")) {
            printf("\nloading: [%s] <%d> <test:%d>",
                   dummy[ammt]->grunt.name, ammt, test);
            ammt++;
        }
    }
    printf("\n%d players read.\n", ammt);
    return ammt;
}

int convert_playerfile(char *argv[])
{
    int             tmpi,
                    ammt = 0;
    FILE           *fl2;

    if (!(fl2 = fopen(argv[2], "w"))) {
        printf("\nCan not open %s, bye!\n", argv[2]);
        exit(0);
    }

    if (dummy == NULL) {
        printf("ack\n");
    }

    for (ammt = 0; ammt < orig_ammt; ammt++) {
        /* 
         * NEW Class 
         */
        dummy[ammt]->grunt2.class = dummy[ammt]->grunt.class;
        dummy[ammt]->grunt2.sex = dummy[ammt]->grunt.sex;

        for (tmpi = 0; tmpi <= ABS_MAX_CLASS; tmpi++) {
            dummy[ammt]->grunt2.level[tmpi] = dummy[ammt]->grunt.level[tmpi];
        }
        for (tmpi = ABS_MAX_CLASS; tmpi <= NEW_ABS_MAX_CLASS; tmpi++) {
            dummy[ammt]->grunt2.level[tmpi] = 0;
        }
        dummy[ammt]->grunt2.birth = dummy[ammt]->grunt.birth;
        dummy[ammt]->grunt2.played = dummy[ammt]->grunt.played;
        dummy[ammt]->grunt2.race = dummy[ammt]->grunt.race;
        dummy[ammt]->grunt2.weight = dummy[ammt]->grunt.weight;
        dummy[ammt]->grunt2.height = dummy[ammt]->grunt.height;

        for (tmpi = 0; tmpi <= strlen(dummy[ammt]->grunt.title); tmpi++) {
            dummy[ammt]->grunt2.title[tmpi] = dummy[ammt]->grunt.title[tmpi];
        }
        for (tmpi = 0; tmpi <= strlen(dummy[ammt]->grunt.extra_str); tmpi++) {
            dummy[ammt]->grunt2.extra_str[tmpi] = 
                dummy[ammt]->grunt.extra_str[tmpi];
        }

        dummy[ammt]->grunt2.hometown = dummy[ammt]->grunt.hometown;

        for (tmpi = 0; tmpi <= strlen(dummy[ammt]->grunt.description); tmpi++) {
            dummy[ammt]->grunt2.description[tmpi] =
                dummy[ammt]->grunt.description[tmpi];
        }

        for (tmpi = 0; tmpi <= MAX_TOUNGE; tmpi++) {
            dummy[ammt]->grunt2.talks[tmpi] = dummy[ammt]->grunt.talks[tmpi];
        }

        for (tmpi = MAX_TOUNGE; tmpi <= NEW_MAX_TOUNGE; tmpi++) {
            dummy[ammt]->grunt2.talks[tmpi] = 0;
        }

        /*
         * removed 3/7/94, msw 
         */
#if 0
        for (tmpi = 0; tmpi <= MAX_NEW_LANGUAGES; tmpi++) {
            dummy[ammt]->grunt2.new_languages[tmpi] =
                dummy[ammt]->grunt.new_languages[tmpi];
        }
        for (tmpi = MAX_NEW_LANGUAGES; tmpi <= NEW_MAX_NEW_LANGUAGES;
             tmpi++)
            dummy[ammt]->grunt2.new_languages[tmpi] = 0;
#endif

        dummy[ammt]->grunt2.extra_flags = dummy[ammt]->grunt.extra_flags;
        dummy[ammt]->grunt2.load_room = dummy[ammt]->grunt.load_room;
        dummy[ammt]->grunt2.abilities = dummy[ammt]->grunt.abilities;
        dummy[ammt]->grunt2.points = dummy[ammt]->grunt.points;

        for (tmpi = 0; tmpi <= MAX_SKILLS; tmpi++) {
            dummy[ammt]->grunt2.skills[tmpi] = dummy[ammt]->grunt.skills[tmpi];
        }

        for (tmpi = MAX_SKILLS; tmpi <= NEW_MAX_SKILLS; tmpi++) {
            dummy[ammt]->grunt2.skills[tmpi].learned = 0;
            dummy[ammt]->grunt2.skills[tmpi].flags = 0;
            dummy[ammt]->grunt2.skills[tmpi].special = 0;
            dummy[ammt]->grunt2.skills[tmpi].nummem = 0;
        }

        for (tmpi = 0; tmpi <= MAX_AFFECT; tmpi++) {
            dummy[ammt]->grunt2.affected[tmpi].type =
                dummy[ammt]->grunt.affected[tmpi].type;
            dummy[ammt]->grunt2.affected[tmpi].duration =
                dummy[ammt]->grunt.affected[tmpi].duration;
            dummy[ammt]->grunt2.affected[tmpi].modifier =
                dummy[ammt]->grunt.affected[tmpi].modifier;
            dummy[ammt]->grunt2.affected[tmpi].location =
                dummy[ammt]->grunt.affected[tmpi].location;
            dummy[ammt]->grunt2.affected[tmpi].bitvector =
                dummy[ammt]->grunt.affected[tmpi].bitvector;
            dummy[ammt]->grunt2.affected[tmpi].next =
                dummy[ammt]->grunt.affected[tmpi].next;
        }

        for (tmpi = MAX_AFFECT; tmpi <= NEW_MAX_AFFECT; tmpi++) {
            dummy[ammt]->grunt2.affected[tmpi].type = 0;
            dummy[ammt]->grunt2.affected[tmpi].duration = 0;
            dummy[ammt]->grunt2.affected[tmpi].modifier = 0;
            dummy[ammt]->grunt2.affected[tmpi].location = 0;
            dummy[ammt]->grunt2.affected[tmpi].bitvector = 0;
            dummy[ammt]->grunt2.affected[tmpi].next = NULL;
        }

        dummy[ammt]->grunt2.spells_to_learn =
            dummy[ammt]->grunt.spells_to_learn;
        dummy[ammt]->grunt2.alignment = dummy[ammt]->grunt.alignment;
        dummy[ammt]->grunt2.affected_by = dummy[ammt]->grunt.affected_by;
        dummy[ammt]->grunt2.affected_by2 = dummy[ammt]->grunt.affected_by2;
        dummy[ammt]->grunt2.last_logon = dummy[ammt]->grunt.last_logon;
        dummy[ammt]->grunt2.act = dummy[ammt]->grunt.act;

        for (tmpi = 0; tmpi <= strlen(dummy[ammt]->grunt.name); tmpi++) {
            dummy[ammt]->grunt2.name[tmpi] = dummy[ammt]->grunt.name[tmpi];
        }

        for (tmpi = 0; tmpi <= strlen(dummy[ammt]->grunt.alias); tmpi++) {
            dummy[ammt]->grunt2.alias[tmpi] = dummy[ammt]->grunt.alias[tmpi];
        }

        for (tmpi = 0; tmpi <= 11; tmpi++) {
            dummy[ammt]->grunt2.pwd[tmpi] = dummy[ammt]->grunt.pwd[tmpi];
        }

        for (tmpi = 0; tmpi <= MAX_SAVES; tmpi++) {
            dummy[ammt]->grunt2.apply_saving_throw[tmpi] =
                dummy[ammt]->grunt2.apply_saving_throw[tmpi];
        }

        for (tmpi = MAX_SAVES; tmpi <= NEW_MAX_SAVES; tmpi++) {
            dummy[ammt]->grunt2.apply_saving_throw[tmpi] = 0;
        }

        for (tmpi = 0; tmpi <= MAX_CONDITIONS; tmpi++) {
            dummy[ammt]->grunt2.conditions[tmpi] =
                dummy[ammt]->grunt2.conditions[tmpi];
        }

        for (tmpi = MAX_CONDITIONS; tmpi <= NEW_MAX_CONDITIONS; tmpi++) {
            dummy[ammt]->grunt2.conditions[tmpi] = 0;
        }

        dummy[ammt]->grunt2.startroom = dummy[ammt]->grunt.startroom;

        /*
         * added 3/7/94 msw 
         */
        dummy[ammt]->grunt2.user_flags = 0;
        dummy[ammt]->grunt2.speaks = 0;

        dummy[ammt]->grunt2.a_deaths = 0;
        dummy[ammt]->grunt2.a_kills = 0;
        dummy[ammt]->grunt2.m_deaths = 0;
        dummy[ammt]->grunt2.m_kills = 0;

        fwrite(&(dummy[ammt]->grunt2), sizeof(struct char_file_u_new), 1, fl2);
    }

    printf("\n%d players converted.\n", ammt);
    exit(0);
    return ammt;
}

char           *time_print(long et)
{
    char           *buff;

    buff = (char *) asctime(localtime(&et));
    *(buff + strlen(buff) - 1) = '\0';
    return (buff);
}
#if 0
int SearchForNameFromPos(char *arg, int pos) 
{   
    register int i;
    
    if(pos > orig_amnt) {
        return(-1);
    }
    for(i = pos; i < orig_amnt; i++) {
        if(!strcasecmp(dummy[i]->grunt.name, arg)) {
            return(i);
        }
    }
    return(-1); 
} 
#endif

void access_rent_files(int number, int ITEM, char buf[40])
{
#if 0
    int ReadObjs(FILE *fl, struct obj_file_u *st);
    char buff2[80]; 
    FILE *fl; 
    int i,j,tried,succeed; 
    struct obj_file_umuck;
      
    for (i=0;i<number;i++) { 
        j=tried=succeed=0; 
        while (buf[j] != '\0' && j<30) { 
            buf[j]=tolower(buf[j]); 
            j++; 
        } 
        buf[j]='\0'; 
        if (buf[0]=='e') {
            printf("\nAtteming to open %s's rent file.",buf); 
        }
        if (strlen(buff) > sprintf) { 
            (,"rent/%s",lower(buff)); 
            if ((fl=fopen(buf,"r")) != NULL) { 
                if (ReadObjs(fl,&muck)) { 
                    succeed++; 
                    printf("\nReading the %d objects in %s's rent file.", 
                           muck.number,buff);
                    for (j=0;j<muck.number && j< 250;j++) {
                        if (muck.objects[j].item_number == (sh_int) ITEM) { 
                            printf("\n%s has it!",buff); 
                        } 
                    } 
                } 
            } 
        } 
    } 
    printf("\nAttempted to read %d files, read %d in actuality.",i,succeed); 
#endif     
}

int ReadObjs(FILE * fl, struct obj_file_u *st)
{
    int             i;

    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
        printf("\nEmpty file.");
    }

    fread(st->owner, sizeof(st->owner), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        printf("\nFailed to read file owner's name, [%s].", st->owner);
        return (FALSE);
    }

    fread(&st->gold_left, sizeof(st->gold_left), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        printf("\nFailed to read gold_left.");
        return (FALSE);
    }

    fread(&st->total_cost, sizeof(st->total_cost), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        printf("\nFailed to read total_cost.");
        return (FALSE);
    }

    fread(&st->last_update, sizeof(st->last_update), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        printf("\nFailed to read last update time.");
        return (FALSE);
    }

    fread(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        printf("\nFailed to read minimum stay.");
        return (FALSE);
    }

    fread(&st->number, sizeof(st->number), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        printf("\nFailed to read the number of items stored.");
        return (FALSE);
    }

    for (i = 0; i < st->number; i++) {
        fread(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
    }
    return (TRUE);
}

void zero_bank(void)
{
    int             i;

    for (i = 0; i < orig_ammt; i++) {
        dummy[i]->grunt.points.bankgold = 0;
    }

    printf("All bank accounts set to 0.\n");
}

char           *lower(char *s)
{
    static char     c[1000];
    static char    *p;
    int             i = 0;

    strcpy(c, s);

    while (c[i]) {
        if (c[i] < 'a' && c[i] >= 'A' && c[i] <= 'Z') {
            c[i] = (char) (int) c[i] + 32;
        }
        i++;
    }
    p = c;
    return (p);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

#include "config.h"
#include "environment.h"
#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"

static MYSQL *sql;

static char sqlbuf[MAX_STRING_LENGTH];

/* Externally accessible */
char *mysql_db    = NULL;
char *mysql_user   = NULL;
char *mysql_passwd = NULL;
char *mysql_host   = NULL;

struct class_def *classes;
int             classCount;

struct skill_def *skills;
int             skillCount;

struct dir_data *direction;
int             directionCount;

struct clan    *clan_list;
int             clanCount;

struct sector_data *sectors;
int             sectorCount;

struct message_list *fightMessages;
int             fightMessageCount;

struct banned_user *bannedUsers = NULL;
int             bannedUserCount;

struct social_messg *socialMessages = NULL;
int                 socialMessageCount = 0;

struct kick_messg  *kickMessages;
int                 kickMessageCount;

struct pose_type   *poseMessages;
int                 poseMessageCount;

struct race_type   *races;
int                 raceCount;

struct lang_def    *languages;
int                 languageCount;


/* Internal protos */
char *db_quote(char *string);
void db_load_classes(void);
void db_load_skills(void);
void db_load_structures(void);
void db_load_messages(void);
void db_load_bannedUsers(void);
void db_load_socials(void);
void db_load_kick_messages(void);
void db_load_poses(void);
void db_load_races(void);
void db_load_languages(void);


void db_setup(void)
{
    if( !mysql_db ) {
        mysql_db = strdup(DEF_MYSQL_DB);
    }

    if( !mysql_user ) {
        mysql_user = strdup(DEF_MYSQL_USER);
    }

    if( !mysql_passwd ) {
        mysql_passwd = strdup(DEF_MYSQL_PASSWD);
    }

    if( !mysql_host ) {
        mysql_host = strdup(DEF_MYSQL_HOST);
    }

    if( !(sql = mysql_init(NULL)) ) {
        Log("Unable to initialize a MySQL structure!!");
        exit(1);
    }

    Log("Using database %s at %s", mysql_db, mysql_host);

    if( !mysql_real_connect(sql, mysql_host, mysql_user, mysql_passwd, 
                            mysql_db, 0, NULL, 0) ) {
        Log("Unable to connect to the database");
        mysql_error(sql);
        exit(1);
    }
}

char *db_quote(char *string)
{
    int             len,
                    i,
                    j,
                    count;
    char           *retString;

    len = strlen(string);

    for(i = 0, count = 0; i < len; i++) {
        if( string[i] == '\'' || string[i] == '\"' ) {
            count++;
        }
    }

    if( !count ) {
        return( strdup(string) );
    }

    retString = (char *)malloc(len + count + 1);
    for(i = 0, j = 0; i < len; i++, j++) {
        if( string[i] == '\'' || string[i] == '\"' ) {
            retString[j++] = '\\';
        }
        retString[j] = string[i];
    }
    retString[j] = '\0';

    return( retString );
}

void db_report_entry(int reportId, struct char_data *ch, char *report)
{
    char           *string;

    if( !report ) {
        return;
    }

    string = db_quote(report);
    if( string ) {
        sprintf( sqlbuf, "INSERT INTO `userReports` (`reportId`, `reportTime`, "
                         "`character`, `roomNum`, `report`) "
                         "VALUES ( %d, NULL, '%s', %ld, '%s' )", reportId,
                         GET_NAME(ch), ch->in_room, string );
        mysql_query(sql, sqlbuf);
        free( string );
    }
}

struct user_report *db_get_report(int reportId, struct user_report *report)
{
    MYSQL_RES          *res;
    MYSQL_ROW           row;

    if( !report ) {
        /* Need to to the query */
        sprintf( sqlbuf, "SELECT `reportTime`, `character`, `roomNum`, "
                         "`report` FROM `userReports` WHERE `reportId` = %d "
                         "ORDER BY `reportTime` ASC", reportId );
        mysql_query(sql, sqlbuf);

        report = (struct user_report *)malloc(sizeof(struct user_report));
        if( !report ) {
            Log( "Can't allocate a user report buffer!" );
            return( NULL );
        }

        memset( report, 0, sizeof(struct user_report) );

        res = mysql_store_result(sql);
        report->res = (void *)res;

        if( !res || !(report->resCount = mysql_num_rows(res))) {
            /* No reports */
            report->resCount  = 0;
            report->timestamp = NULL;
            report->character = NULL;
            report->report    = strdup("No reports at this time");
            return(report);
        }
    }

    if( report->timestamp ) {
        free( report->timestamp );
    }

    if( report->character ) {
        free( report->character );
    }

    if( report->report ) {
        free( report->report );
    }

    res = (MYSQL_RES *)report->res;

    if( report->resCount == 0 ) {
        if( res ) {
            mysql_free_result(res);
        }
        free( report );
        return( NULL );
    }

    report->resCount--;

    row = mysql_fetch_row(res);
    report->timestamp = strdup(row[0]);
    report->character = strdup(row[1]);
    report->roomNum   = atoi(row[2]);
    report->report    = strdup(row[3]);

    return( report );
}

void db_clean_report(int reportId)
{

    sprintf( sqlbuf, "DELETE FROM `userReports` WHERE `reportId` = %d", 
                     reportId );
    mysql_query(sql, sqlbuf);
}

void db_initial_load(void)
{
    db_load_classes();
    db_load_skills();
    db_load_structures();
    db_load_messages();
    db_load_bannedUsers();
    db_load_socials();
    db_load_kick_messages();
    db_load_poses();
    db_load_races();
    db_load_languages();
}

void db_load_classes(void)
{
    int                 i,
                        j,
                        level;
    int                 classId;
    int                 skillCount;
    int                 err = FALSE;
    MYSQL_RES           *resClass, *resSkill;
    MYSQL_ROW           row;

    Log("Loading classes[] from SQL");

    strcpy(sqlbuf, "SELECT classId, className, classAbbrev "
                   "FROM classes ORDER BY classId ASC");
    mysql_query(sql, sqlbuf);

    resClass = mysql_store_result(sql);
    if( !resClass || !(classCount = mysql_num_rows(resClass))) {
        /* No classes!? */
        Log( "No classes defined in the database!" );
        exit(1);
    }

    classes = (struct class_def *)malloc(classCount * sizeof(struct class_def));
    if( !classes ) {
        Log( "Out of memory allocating classes[]" );
        exit(1);
    }

    for( i = 0; i < classCount; i++ ) {
        row = mysql_fetch_row(resClass);
        classId = atoi(row[0]);
        classes[i].name = strdup(row[1]);
        classes[i].abbrev = strdup(row[2]);

        sprintf( sqlbuf, "SELECT skills.skillName, skills.skillID, "
                         "classSkills.minLevel, classSkills.maxTeach "
                         "FROM skills, classSkills "
                         "WHERE skills.skillId = classSkills.skillId AND "
                         "classSkills.classId = %d AND "
                         "classSkills.mainSkill = 0 "
                         "ORDER BY skills.skillName ASC", classId );
        mysql_query(sql, sqlbuf);

        resSkill = mysql_store_result(sql);
        if( !resSkill || !(skillCount = mysql_num_rows(resSkill)) ) {
            /* No result for this query */
            classes[i].skillCount = 0;
            classes[i].skills = NULL;
        } else {
            classes[i].skillCount = skillCount;
            classes[i].skills = (struct skillset *)malloc(skillCount * 
                                                     sizeof(struct skillset));
            if( !classes[i].skills ) {
                classes[i].skillCount = 0;
                Log( "Dumping skills due to lack of memory" );
                err = TRUE;
            } else {
                for( j = 0; j < skillCount; j++ ) {
                    row = mysql_fetch_row(resSkill);
                    classes[i].skills[j].name = strdup(row[0]);
                    classes[i].skills[j].skillnum = atoi(row[1]);
                    classes[i].skills[j].level = atoi(row[2]);
                    classes[i].skills[j].maxlearn = atoi(row[3]);
                }
            }
            mysql_free_result(resSkill);
        }


        sprintf( sqlbuf, "SELECT skills.skillName, skills.skillID, "
                         "classSkills.minLevel, classSkills.maxTeach "
                         "FROM skills, classSkills "
                         "WHERE skills.skillId = classSkills.skillId AND "
                         "classSkills.classId = %d AND "
                         "classSkills.mainSkill = 1 "
                         "ORDER BY skills.skillName ASC", classId );
        mysql_query(sql, sqlbuf);

        resSkill = mysql_store_result(sql);
        if( !resSkill || !(skillCount = mysql_num_rows(resSkill)) ) {
            /* No result for this query */
            classes[i].mainskillCount = 0;
            classes[i].mainskills = NULL;
        } else {
            classes[i].mainskillCount = skillCount;
            classes[i].mainskills = (struct skillset *)malloc(skillCount * 
                                                     sizeof(struct skillset));
            if( !classes[i].mainskills ) {
                classes[i].mainskillCount = 0;
                Log( "Dumping mainskills due to lack of memory" );
                err = TRUE;
            } else {
                for( j = 0; j < skillCount; j++ ) {
                    row = mysql_fetch_row(resSkill);
                    classes[i].mainskills[j].name = strdup(row[0]);
                    classes[i].mainskills[j].skillnum = atoi(row[1]);
                    classes[i].mainskills[j].level = atoi(row[2]);
                    classes[i].mainskills[j].maxlearn = atoi(row[3]);
                }
            }
        }
        mysql_free_result(resSkill);

        sprintf( sqlbuf, "SELECT level, thaco, maleTitle, femaleTitle, minExp "
                         "FROM classLevels WHERE classId = %d ORDER BY level",
                         classId );
        mysql_query(sql, sqlbuf);

        resSkill = mysql_store_result(sql);
        if( !resSkill || !(skillCount = mysql_num_rows(resSkill)) ) {
            /* No result for this query */
            classes[i].levelCount = 0;
            classes[i].levels = NULL;
        } else {
            classes[i].levelCount = skillCount;
            classes[i].levels = (struct class_level_t *)malloc(skillCount * 
                                                 sizeof(struct class_level_t));
            if( !classes[i].levels ) {
                classes[i].levelCount = 0;
                Log( "Dumping levels due to lack of memory" );
                err = TRUE;
            } else {
                for( j = 0; j < skillCount; j++ ) {
                    row = mysql_fetch_row(resSkill);
                    level = atoi(row[0]);
                    classes[i].levels[level].thaco = atoi(row[1]);
                    classes[i].levels[level].title_m = strdup(row[2]);
                    classes[i].levels[level].title_f = strdup(row[3]);
                    classes[i].levels[level].exp = atol(row[4]);
                }
            }
            mysql_free_result(resSkill);
        }
    }
    mysql_free_result(resClass);
    Log("Finished loading classes[] from SQL");

    if( err ) {
        exit(1);
    }
}

void db_load_skills(void)
{
    int             i,
                    j;

    int             skillId;
    int             err = FALSE;
    MYSQL_RES      *resSkill, *resMsg;
    MYSQL_ROW       row;

    Log("Loading skills[] from SQL");

    strcpy(sqlbuf, "SELECT skillId, skillName, skillType "
                   "FROM skills ORDER BY skillId ASC");
    mysql_query(sql, sqlbuf);

    resSkill = mysql_store_result(sql);
    if( !resSkill || !(skillCount = mysql_num_rows(resSkill)) ) {
        /* No skills!? */
        Log( "No skills defined in the database!" );
        exit(1);
    }

    skills = (struct skill_def *)malloc((skillCount + 1) * 
                                        sizeof(struct skill_def));
    if( !skills ) {
        Log( "Out of memory allocating skills[]" );
        exit(1);
    }

    /* Leave skill 0 emtpy */
    memset( skills, 0, (skillCount + 1) * sizeof(struct skill_def) );

    for( i = 1; i <= skillCount; i++ ) {
        row = mysql_fetch_row(resSkill);
        skillId = atoi(row[0]);
        skills[i].skillId   = skillId;
        skills[i].name      = strdup(row[1]);
        skills[i].skillType = atoi(row[2]);

        for(j = 0; j < SKILL_MSG_COUNT; j++) {
            sprintf( sqlbuf, "SELECT text FROM skillMessages "
                             "WHERE `skillId` = %d AND `msgId` = 1 AND "
                             "`index` = %d", skillId, j+1 );
            mysql_query(sql, sqlbuf);

            resMsg = mysql_store_result(sql);
            if( resMsg && mysql_num_rows(resMsg) ) {
                row = mysql_fetch_row(resMsg);
                skills[i].message[j] = strdup(row[0]);
            }
            mysql_free_result(resMsg);
        }
    }
    mysql_free_result(resSkill);

    if( err ) {
        exit(1);
    }

    Log("Finished loading skills[] from SQL");
}

void db_load_structures(void)
{
    int             i;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading misc structures from SQL");

    /* direction[] */
    strcpy(sqlbuf, "SELECT forward, reverse, trapBits, exit, listExit, "
                   "direction, description "
                   "FROM directions ORDER BY forward ASC");
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(directionCount = mysql_num_rows(res)) ) {
        Log( "No directions defined in the database!" );
        exit(1);
    }

    direction = (struct dir_data *)malloc(directionCount * 
                                          sizeof(struct dir_data));
    if( !direction ) {
        Log( "Out of memory allocating direction[]" );
        exit(1);
    }

    for( i = 0; i < directionCount; i++ ) {
        row = mysql_fetch_row(res);
        direction[i].forward  = atoi(row[0]);
        direction[i].rev      = atoi(row[1]);
        direction[i].trap     = atoi(row[2]);
        direction[i].exit     = strdup(row[3]);
        direction[i].listexit = strdup(row[4]);
        direction[i].dir      = strdup(row[5]);
        direction[i].desc     = strdup(row[6]);
    }
    mysql_free_result(res);


    /* clan_list[] */
    strcpy(sqlbuf, "SELECT clanId, clanName, shortName, description, homeRoom "
                   "FROM playerClans ORDER BY clanId ASC");
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(clanCount = mysql_num_rows(res)) ) {
        Log( "No clans defined in the database!" );
        exit(1);
    }

    clan_list = (struct clan *)malloc(clanCount * sizeof(struct clan));
    if( !direction ) {
        Log( "Out of memory allocating clan_list[]" );
        exit(1);
    }

    for( i = 0; i < clanCount; i++ ) {
        row = mysql_fetch_row(res);
        clan_list[i].number    = atoi(row[0]);
        clan_list[i].name      = strdup(row[1]);
        clan_list[i].shortname = strdup(row[2]);
        clan_list[i].desc      = strdup(row[3]);
        clan_list[i].home      = atoi(row[4]);
    }
    mysql_free_result(res);

    /* sectors[] */
    strcpy(sqlbuf, "SELECT sectorType, mapChar, moveLoss "
                   "FROM sectorType ORDER BY sectorId ASC");
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(sectorCount = mysql_num_rows(res)) ) {
        Log( "No sector types defined in the database!" );
        exit(1);
    }

    sectors = (struct sector_data *)malloc(sectorCount * 
                                           sizeof(struct sector_data));
    if( !sectors ) {
        Log( "Out of memory allocating sectors[]" );
        exit(1);
    }

    for( i = 0; i < sectorCount; i++ ) {
        row = mysql_fetch_row(res);
        sectors[i].type     = strdup(row[0]);
        sectors[i].mapChar  = strdup(row[1]);
        sectors[i].moveLoss = atoi(row[2]);
    }
    mysql_free_result(res);

    Log("Finished loading misc structures from SQL");
}


void db_load_messages(void)
{
    int             i,
                    j,
                    skill;

    MYSQL_RES      *res, *resMsg;
    MYSQL_ROW       row;

    Log("Loading fight messages from SQL");

    strcpy( sqlbuf, "SELECT DISTINCT skillId FROM skillMessages "
                    "WHERE msgId = 2 ORDER BY skillId" );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(fightMessageCount = mysql_num_rows(res)) ) {
        Log( "No fight messages defined in the database!" );
        exit(1);
    }

    fightMessages = (struct message_list *)malloc(fightMessageCount * 
                                                  sizeof(struct message_list));
    if( !direction ) {
        Log( "Out of memory allocating fightMessages[]" );
        exit(1);
    }

    memset( fightMessages, 0, fightMessageCount * sizeof(struct message_list) );

    for( i = 0; i < fightMessageCount; i++ ) {
        row = mysql_fetch_row(res);
        skill = atoi(row[0]);
        fightMessages[i].a_type = skill;

        for( j = 0; j < FIGHT_MSG_COUNT; j++ ) {
            sprintf( sqlbuf, "SELECT text FROM skillMessages "
                             "WHERE `skillId` = %d AND `msgId` = 2 AND "
                             "`index` = %d", skill, j+1 );
            mysql_query(sql, sqlbuf);

            resMsg = mysql_store_result(sql);
            if( resMsg && mysql_num_rows(resMsg) ) {
                row = mysql_fetch_row(resMsg);
                fightMessages[i].msg[j] = strdup(row[0]);
                mysql_free_result(resMsg);
            }
        }
    }
    mysql_free_result(res);

    Log("Finished loading fight messages from SQL");
}

void db_load_bannedUsers(void)
{
    /*
     * 0 - full match
     * 1 - from start of string
     * 2 - from end of string
     * 3 - somewhere in string
     */
    int             i;
    char           *tmp;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading banned usernames from SQL");

    strcpy( sqlbuf, "SELECT name FROM bannedName" );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(bannedUserCount = mysql_num_rows(res)) ) {
        Log( "No banned usernames defined in the database!" );
        exit(1);
    }

    bannedUsers = (struct banned_user *)malloc(bannedUserCount * 
                                               sizeof(struct banned_user));
    if( !bannedUsers ) {
        Log( "Out of memory allocating bannedUsers[]" );
        exit(1);
    }

    for (i = 0; i < bannedUserCount; i++) {
        row = mysql_fetch_row(res);
        tmp = row[0];
        if (*tmp == '*') {
            if (tmp[strlen(tmp) - 1] == '*') {
                bannedUsers[i].how = 3;
                tmp[strlen(tmp) - 1] = '\0';
            } else {
                bannedUsers[i].how = 2;
            }
            tmp++;
        } else if (tmp[strlen(tmp) - 1] == '*') {
                bannedUsers[i].how = 1;
                tmp[strlen(tmp) - 1] = '\0';
        } else {
            bannedUsers[i].how = 0;
        }
        bannedUsers[i].name = strdup(tmp);
        bannedUsers[i].len  = strlen(bannedUsers[i].name);
    }
    mysql_free_result(res);

    Log("Finished loading banned usernames from SQL");
}

void db_load_socials(void)
{
    int             i,
                    j;
    int             tmp;

    MYSQL_RES      *res, *resMsg;
    MYSQL_ROW       row;


    Log("Loading social messages from SQL");

    strcpy( sqlbuf, "SELECT socialId, hide, minPosition FROM socials "
                    "ORDER BY socialId" );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(socialMessageCount = mysql_num_rows(res)) ) {
        Log( "No social messages defined in the database!" );
        exit(1);
    }

    socialMessages = (struct social_messg *)malloc(socialMessageCount * 
                                                   sizeof(struct social_messg));
    if( !socialMessages ) {
        Log( "Out of memory allocating socialMessages[]" );
        exit(1);
    }

    memset( socialMessages, 0, 
            socialMessageCount * sizeof(struct social_messg) );

    for (i = 0; i < socialMessageCount; i++) {
        row = mysql_fetch_row(res);
        tmp = atoi(row[0]);
        socialMessages[i].act_nr = tmp;
        socialMessages[i].hide = atoi(row[1]);
        socialMessages[i].min_victim_position = atoi(row[2]);

        for( j = 0; j < SOCIAL_MSG_COUNT; j++ ) {
            sprintf( sqlbuf, "SELECT text FROM skillMessages "
                             "WHERE `skillId` = %d AND `msgId` = 3 AND "
                             "`index` = %d", tmp, j+1 );
            mysql_query(sql, sqlbuf);

            resMsg = mysql_store_result(sql);
            if( resMsg && mysql_num_rows(resMsg) ) {
                row = mysql_fetch_row(resMsg);
                socialMessages[i].msg[j] = strdup(row[0]);
            }
            mysql_free_result(resMsg);
        }
    }
    mysql_free_result(res);

    Log("Finishing loading social messages from SQL");
}

void db_load_kick_messages(void)
{
    int             i,
                    j;
    int             tmp;

    MYSQL_RES      *res, *resMsg;
    MYSQL_ROW       row;

    Log("Loading kick messages from SQL");

    strcpy( sqlbuf, "SELECT DISTINCT skillId FROM skillMessages "
                    "WHERE msgId = 4 ORDER BY skillId" );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(kickMessageCount = mysql_num_rows(res)) ) {
        Log( "No kick messages defined in the database!" );
        exit(1);
    }

    kickMessages = (struct kick_messg *)malloc(kickMessageCount * 
                                               sizeof(struct kick_messg));
    if( !kickMessages ) {
        Log( "Out of memory allocating kickMessages[]" );
        exit(1);
    }

    memset( kickMessages, 0, 
            kickMessageCount * sizeof(struct kick_messg) );

    for (i = 0; i < kickMessageCount; i++) {
        row = mysql_fetch_row(res);
        tmp = atoi(row[0]);

        for( j = 0; j < KICK_MSG_COUNT; j++ ) {
            sprintf( sqlbuf, "SELECT text FROM skillMessages "
                             "WHERE `skillId` = %d AND `msgId` = 4 AND "
                             "`index` = %d", tmp, j+1 );
            mysql_query(sql, sqlbuf);

            resMsg = mysql_store_result(sql);
            if( resMsg && mysql_num_rows(resMsg) ) {
                row = mysql_fetch_row(resMsg);
                kickMessages[i].msg[j] = strdup(row[0]);
                mysql_free_result(resMsg);
            }
        }
    }
    mysql_free_result(res);

    Log("Finishing loading kick messages from SQL");
}

void db_load_poses(void)
{
    int             i,
                    j;
    int             tmp;

    MYSQL_RES      *res, *resMsg;
    MYSQL_ROW       row;

    Log("Loading pose messages from SQL");

    strcpy( sqlbuf, "SELECT DISTINCT skillId FROM skillMessages "
                    "WHERE msgId = 5 OR msgId = 6 ORDER BY skillId" );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(poseMessageCount = mysql_num_rows(res)) ) {
        Log( "No pose messages defined in the database!" );
        exit(1);
    }

    poseMessages = (struct pose_type *)malloc(poseMessageCount * 
                                              sizeof(struct pose_type));
    if( !poseMessages ) {
        Log( "Out of memory allocating poseMessages[]" );
        exit(1);
    }

    memset( poseMessages, 0, poseMessageCount * sizeof(struct pose_type) );

    for (i = 0; i < poseMessageCount; i++) {
        row = mysql_fetch_row(res);
        tmp = atoi(row[0]);
        poseMessages[i].level = tmp;

        poseMessages[i].poser_msg = (char **)malloc(classCount * sizeof(char*));
        poseMessages[i].room_msg  = (char **)malloc(classCount * sizeof(char*));
        if( !poseMessages[i].poser_msg || !poseMessages[i].room_msg ) {
            Log( "Out of memory allocating poses" );
            exit(1);
        }

        for( j = 0; j < classCount; j++ ) {
            sprintf( sqlbuf, "SELECT text FROM skillMessages "
                             "WHERE `skillId` = %d AND `msgId` = 5 AND "
                             "`index` = %d", tmp, j+1 );
            mysql_query(sql, sqlbuf);

            resMsg = mysql_store_result(sql);
            if( resMsg && mysql_num_rows(resMsg) ) {
                row = mysql_fetch_row(resMsg);
                poseMessages[i].poser_msg[j] = strdup(row[0]);
            }
            mysql_free_result(resMsg);

            sprintf( sqlbuf, "SELECT text FROM skillMessages "
                             "WHERE `skillId` = %d AND `msgId` = 6 AND "
                             "`index` = %d", tmp, j+1 );
            mysql_query(sql, sqlbuf);

            resMsg = mysql_store_result(sql);
            if( resMsg && mysql_num_rows(resMsg) ) {
                row = mysql_fetch_row(resMsg);
                poseMessages[i].room_msg[j] = strdup(row[0]);
            }
            mysql_free_result(resMsg);
        }
    }
    mysql_free_result(res);

    Log("Finishing loading pose messages from SQL");
}

/*
 * assign special procedures to mobiles 
 */
void assign_mobiles( void )
{
    int             i,
                    rnum,
                    vnum,
                    count;
    int_func        func;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading mobile procs from SQL");

    sprintf( sqlbuf, "SELECT `vnum`, `procedure` FROM procAssignments "
                     "WHERE `procType` = %d ORDER BY `vnum`", PROC_MOBILE );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(count = mysql_num_rows(res)) ) {
        Log( "No mobile procedures defined in the database!" );
        return;
    }

    for (i = 0; i < count; i++) {
        row = mysql_fetch_row(res);

        if( !(func = procGetFuncByName( row[1], PROC_MOBILE )) ) {
            Log( "assign_mobiles: proc for mobile %s (%s) not registered.",
                 row[0], row[1] );
            continue;
        }
        
        vnum = atoi(row[0]);
        rnum = real_mobile(vnum);
        if (rnum < 0) {
            Log("assign_mobiles: Mobile %d not found in database.", vnum);
            continue;
        }

        mob_index[rnum].func = func;
    }
    mysql_free_result(res);

    Log("Finished mobile procs from SQL");
}

/*
 * assign special procedures to objects 
 */
void assign_objects( void )
{
    int             i,
                    rnum,
                    vnum,
                    count;
    int_func        func;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading object procs from SQL");

    sprintf( sqlbuf, "SELECT `vnum`, `procedure` FROM procAssignments "
                     "WHERE `procType` = %d ORDER BY `vnum`", PROC_OBJECT );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(count = mysql_num_rows(res)) ) {
        Log( "No object procedures defined in the database!" );
        return;
    }

    for (i = 0; i < count; i++) {
        row = mysql_fetch_row(res);

        if( !(func = procGetFuncByName( row[1], PROC_OBJECT )) ) {
            Log( "assign_objects: proc for object %s (%s) not registered.", 
                 row[0], row[1] );
            continue;
        }
        
        vnum = atoi(row[0]);
        rnum = real_object(vnum);
        if (rnum < 0) {
            Log("assign_objects: Object %d not found in database.", vnum);
            continue;
        }

        obj_index[rnum].func = func;
    }
    mysql_free_result(res);

    Log("Finished object procs from SQL");
}

/*
 * assign special procedures to rooms 
 */
void assign_rooms( void )
{
    int             i,
                    vnum,
                    count;
    struct room_data *rp;
    int_func        func;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading room procs from SQL");

    sprintf( sqlbuf, "SELECT `vnum`, `procedure` FROM procAssignments "
                     "WHERE `procType` = %d ORDER BY `vnum`", PROC_ROOM );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(count = mysql_num_rows(res)) ) {
        Log( "No room procedures defined in the database!" );
        return;
    }

    for (i = 0; i < count; i++) {
        row = mysql_fetch_row(res);

        if( !(func = procGetFuncByName( row[1], PROC_ROOM )) ) {
            Log( "assign_rooms: proc for room %s (%s) not registered.", row[0],
                 row[1] );
            continue;
        }
        
        vnum = atoi(row[0]);
        rp = real_roomp(vnum);
        if (!rp) {
            Log( "assign_objects: Room %d not found in database.", vnum);
            continue;
        }

        rp->funct = func;
    }
    mysql_free_result(res);

    Log("Finished room procs from SQL");
}


void db_load_races(void) 
{
    int             i,
                    j;

    MYSQL_RES      *res, *resMax;
    MYSQL_ROW       row;

    Log("Loading races[] from SQL");

    strcpy( sqlbuf, "SELECT raceId, raceName, description, raceSize, ageStart, "
                    "ageYoung, ageMature, ageMiddle, ageOld, ageAncient, "
                    "ageVenerable, nativeLanguage "
                    "FROM races ORDER BY raceId" );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(raceCount = mysql_num_rows(res)) ) {
        Log( "No races defined in the database!" );
        return;
    }

    races = (struct race_type *)malloc(raceCount * sizeof(struct race_type));
    if( !races ) {
        Log( "Out of memory allocating races[]" );
        exit(1);
    }

    memset( races, 0, raceCount * sizeof(struct race_type) );

    for (i = 0; i < raceCount; i++) {
        row = mysql_fetch_row(res);

        races[i].race = atoi(row[0]) - 1;
        races[i].racename = strdup(row[1]);
        if( row[2] ) {
            races[i].desc = strdup(row[2]);
        } else {
            races[i].desc = NULL;
        }
        races[i].size = atoi(row[4]);
        races[i].start = atoi(row[5]);
        races[i].young = atoi(row[6]);
        races[i].mature = atoi(row[7]);
        races[i].middle = atoi(row[8]);
        races[i].old = atoi(row[9]);
        races[i].venerable = atoi(row[10]);
        races[i].nativeLanguage = atoi(row[11]) - 1;
        if( races[i].nativeLanguage < 0 ) {
            races[i].nativeLanguage = 0;
        }

        races[i].racialMax = (int *)malloc(classCount * sizeof(int));
        if( !races[i].racialMax ) {
            Log( "Out of memory allocating races[i].racialMax" );
            exit(1);
        }

        memset( races[i].racialMax, 0, classCount * sizeof(int) );

        for( j = 0; j < classCount; j++ ) {
            sprintf( sqlbuf, "SELECT maxLevel FROM racialMax "
                             "WHERE raceId = %d AND classId = %d",
                             races[i].race + 1, j + 1 );
            mysql_query(sql, sqlbuf);

            resMax = mysql_store_result(sql);
            if( resMax && mysql_num_rows(resMax) ) {
                row = mysql_fetch_row(resMax);
                races[i].racialMax[j] = atoi(row[0]);
            }
            mysql_free_result(resMax);
        }
    }
    mysql_free_result(res);

    Log("Finished loading races[] from SQL");
}


void db_load_languages(void) 
{
    int             i;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading languages[] from SQL");

    strcpy( sqlbuf, "SELECT `langId`, `skillId`, `name` FROM languages "
                    "ORDER BY `langId`" );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(languageCount = mysql_num_rows(res)) ) {
        Log( "No languages defined in the database!" );
        return;
    }

    languages = (struct lang_def *)malloc(languageCount * 
                                          sizeof(struct lang_def));
    if( !languages ) {
        Log( "Out of memory allocating languages[]" );
        exit(1);
    }


    for (i = 0; i < languageCount; i++) {
        row = mysql_fetch_row(res);

        languages[i].langSpeaks = atoi(row[0]);
        languages[i].langSkill  = atoi(row[1]);
        languages[i].name       = strdup(row[2]);
    }
    mysql_free_result(res);

    Log("Finished loading languages[] from SQL");
}

void db_load_textfiles(void)
{
    MYSQL_RES      *res;
    MYSQL_ROW       row;


    Log("Loading Essential Text Files.");

    if( news ) {
        free( news );
    }
    strcpy( sqlbuf, "SELECT fileContents FROM textFiles WHERE fileId = 5" );
    mysql_query(sql, sqlbuf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        news = strdup(row[0]);
    }
    mysql_free_result(res);

    if( credits ) {
        free( credits );
    }
    strcpy( sqlbuf, "SELECT fileContents FROM textFiles WHERE fileId = 1" );
    mysql_query(sql, sqlbuf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        credits = strdup(row[0]);
    }
    mysql_free_result(res);

    if( motd ) {
        free( motd );
    }
    strcpy( sqlbuf, "SELECT fileContents FROM textFiles WHERE fileId = 4" );
    mysql_query(sql, sqlbuf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        motd = strdup(row[0]);
    }
    mysql_free_result(res);

    if( wmotd ) {
        free( wmotd );
    }
    strcpy( sqlbuf, "SELECT fileContents FROM textFiles WHERE fileId = 6" );
    mysql_query(sql, sqlbuf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        wmotd = strdup(row[0]);
    }
    mysql_free_result(res);

    if( info ) {
        free( info );
    }
    strcpy( sqlbuf, "SELECT fileContents FROM textFiles WHERE fileId = 2" );
    mysql_query(sql, sqlbuf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        info = strdup(row[0]);
    }
    mysql_free_result(res);

    if( login ) {
        free( login );
    }
    strcpy( sqlbuf, "SELECT fileContents FROM textFiles WHERE fileId = 3" );
    mysql_query(sql, sqlbuf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        login = strdup(row[0]);
    }
    mysql_free_result(res);
}

int db_save_textfile(struct char_data *ch)
{
    char           *outbuf;
    struct edit_txt_msg *tfd;
    int             fileId;
    char            author[100];

    if (!ch) {
        return (FALSE);
    }
    tfd = ch->specials.txtedit;

    if (!tfd) {
        return (FALSE);
    }

    switch (tfd->file) {
    case 1:
        /* News */
        fileId = 5;
        break;
    case 2:
        /* MOTD */
        fileId = 4;
        break;
    case 3:
        /* WMOTD */
        fileId = 6;
        break;
    default:
        send_to_char("Cannot save this file type.\n\r", ch);
        return (FALSE);
        break;
    }

    strcpy(sqlbuf, "\n");
    if (tfd->date) {
        strcat(sqlbuf, tfd->date);
        while(sqlbuf[strlen(sqlbuf) - 1] == '~') {
            sqlbuf[strlen(sqlbuf) - 1] = '\0';
        }
        strcat(sqlbuf, "\n");
    }

    if (tfd->body) {
        strcat(sqlbuf, "\n");
        remove_cr(&sqlbuf[strlen(sqlbuf)], tfd->body);
        while (sqlbuf[strlen(sqlbuf) - 1] == '~') {
            sqlbuf[strlen(sqlbuf) - 1] = '\0';
        }
    }

    if (tfd->author) {
        strcat(sqlbuf, "\nLast edited by ");
        strcat(sqlbuf, tfd->author);
        strcat(sqlbuf, ".");
        sprintf( author, "'%s'", tfd->author );
    } else {
        strcpy( author, "NULL" );
    }

    strcat(sqlbuf, "\n");

    outbuf = db_quote(sqlbuf);

    sprintf( sqlbuf, "DELETE FROM textFiles WHERE `fileId` = %d", fileId );
    mysql_query(sql, sqlbuf);

    sprintf( sqlbuf, "INSERT INTO textFiles (`fileId`, `lastModified`, "
                     "`lastModBy`, `fileContents`) VALUES (%d, NULL, %s, '%s')",
                     fileId, author, outbuf );
    free( outbuf );

    mysql_query(sql, sqlbuf);

    return (TRUE);
}




/*
 *  Recursively delete a post and all it's replies.
 */
void db_delete_board_message(struct board_def *board, short message_id)
{
    int             i,
                    count,
                    msgId;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    /*
     * First check to see if we have a reply to this message -- if so
     * axe it
     */
    sprintf(sqlbuf, "SELECT `messageNum` FROM `boardMessages` "
                    "WHERE `boardId` = %d AND `replyToMessageNum` = %d",
                    board->boardId, message_id );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( res && (count = mysql_num_rows(res)) ) {
        for( i = 0; i < count; i++ ) {
            row = mysql_fetch_row(res);
            msgId = atoi(row[0]);
            db_delete_board_message(board, msgId);
        }
    }
    mysql_free_result(res);

    sprintf(sqlbuf, "DELETE FROM `boardMessages` where `messageNum` = %d AND "
                    "`boardId` = %d", message_id, board->boardId );
    mysql_query(sql, sqlbuf);
    sprintf(sqlbuf, "UPDATE `boardMessages` "
                    "SET `messageNum` = `messageNum` - 1 "
                    "WHERE `messageNum` > %d AND `boardId` = %d", message_id, 
                    board->boardId );
    mysql_query(sql, sqlbuf);
    sprintf(sqlbuf, "UPDATE `boards` SET `maxPostNum` = `maxPostNum` - 1 "
                    "WHERE boardId = %d", board->boardId );
    mysql_query(sql, sqlbuf);
}

struct board_def *db_lookup_board(int vnum)
{
    struct board_def *board;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT `boardId`, `maxPostNum`, `minLevel` "
                    "FROM `boards` WHERE `vnum` = %d", vnum);
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !mysql_num_rows(res) ) {
        mysql_free_result(res);
        return( NULL );
    }

    row = mysql_fetch_row(res);
    
    board = (struct board_def *)malloc(sizeof(struct board_def));
    if( !board ) {
        Log( "Out of memory allocating a board structure!" );
        mysql_free_result(res);
        return( NULL );
    }

    board->boardId = atoi(row[0]);
    board->messageCount = atoi(row[1]);
    board->minLevel = atoi(row[2]);
    mysql_free_result(res);

    return( board );
}

struct bulletin_board_message *db_get_board_message(int boardId, int msgNum)
{
    struct bulletin_board_message *msg;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT `poster`, `topic`, `post`, "
                    "UNIX_TIMESTAMP(`postTime`), `replyToMessageNum` "
                    "FROM `boardMessages` "
                    "WHERE `boardId` = %d AND messageNum = %d", boardId,
                    msgNum );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !mysql_num_rows(res) ) {
        mysql_free_result(res);
        return( NULL );
    }

    row = mysql_fetch_row(res);
    
    msg = (struct bulletin_board_message *)
              malloc(sizeof(struct bulletin_board_message));
    if( !msg ) {
        Log( "Out of memory allocating a message structure!" );
        mysql_free_result(res);
        return( NULL );
    }

    msg->author = strdup(row[0]);
    msg->title = strdup(row[1]);
    msg->text = strdup(row[2]);
    msg->date = atoi(row[3]);
    msg->message_id = msgNum;
    msg->reply_to - atoi(row[4]);
    mysql_free_result(res);

    return( msg );
}

void db_free_board_message(struct bulletin_board_message *msg)
{
    if( !msg ) {
        return;
    }

    if( msg->author ) {
        free( msg->author );
    }

    if( msg->title ) {
        free( msg->title );
    }

    if( msg->text ) {
        free( msg->text );
    }

    free( msg );
}

int db_get_board_replies(struct board_def *board, int msgId, 
                         struct bulletin_board_message **msg)
{
    int             count,
                    i;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT `poster`, `topic`, `post`, `messageNum`, "
                    "UNIX_TIMESTAMP(`postTime`), `replyToMessageNum` "
                    "FROM `boardMessages` "
                    "WHERE `boardId` = %d AND replyToMessageNum = %d "
                    "ORDER BY `messageNum`", board->boardId, msgId );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(count = mysql_num_rows(res)) ) {
        mysql_free_result(res);
        *msg = NULL;
        return( 0 );
    }

    *msg = (struct bulletin_board_message *)
              malloc(sizeof(struct bulletin_board_message) * count);
    if( !*msg ) {
        Log( "Out of memory allocating a message structure!" );
        mysql_free_result(res);
        return( 0 );
    }

    for(i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        (*msg)[i].author = strdup(row[0]);
        (*msg)[i].title = strdup(row[1]);
        (*msg)[i].text = strdup(row[2]);
        (*msg)[i].message_id = atoi(row[3]);
        (*msg)[i].date = atoi(row[4]);
        (*msg)[i].reply_to = atoi(row[5]);
    }
    mysql_free_result(res);

    return( count );
}

void db_free_board_replies(struct bulletin_board_message *msg, int count)
{
    int             i;

    if( !msg ) {
        return;
    }

    for( i = 0; i < count; i++ ) {
        if( msg[i].author ) {
            free( msg[i].author );
        }

        if( msg[i].title ) {
            free( msg[i].title );
        }

        if( msg[i].text ) {
            free( msg[i].text );
        }
    }

    free( msg );
}

void db_post_message(struct board_def *board, 
                     struct bulletin_board_message *msg)
{
    char           *topic,
                   *post;

    topic = db_quote(msg->title);
    post  = db_quote(msg->text);

    sprintf(sqlbuf, "INSERT INTO `boardMessages` (`boardId`, `messageNum`, "
                    "`replyToMessageNum`, `topic`, `poster`, `postTime`, "
                    "`post`) VALUES (%d, %d, %d, '%s', '%s', NULL, '%s')",
                    board->boardId, board->messageCount + 1, msg->reply_to,
                    topic, msg->author, post );
    mysql_query(sql, sqlbuf);
    
    free(topic);
    free(post);

    sprintf(sqlbuf, "UPDATE `boards` SET `maxPostNum` = `maxPostNum` + 1 "
                    "WHERE `boardId` = %d", board->boardId );
    mysql_query(sql, sqlbuf);

    db_free_board_message(msg);
}

void db_store_mail(char *to, char *from, char *message_pointer)
{
    char           *post;

    post = db_quote(message_pointer);

    sprintf(sqlbuf, "INSERT INTO `mailMessages` (`mailFrom`, `mailTo`, "
                    "`timestamp`, `message`) VALUES ('%s', '%s', NULL, '%s')",
                    from, to, post );
    mysql_query(sql, sqlbuf);

    free(post);
}

int             db_has_mail(char *recipient)
{
    int             count;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT HIGH_PRIORITY COUNT(*) as count FROM "
                    "`mailMessages` WHERE LOWER(`mailTo`) = LOWER('%s')", 
                    recipient );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !mysql_num_rows(res) ) {
        mysql_free_result(res);
        return( 0 );
    }

    row = mysql_fetch_row(res);
    count = atoi(row[0]);
    mysql_free_result(res);

    return( count );
}

int db_get_mail_ids(char *recipient, int *messageNum, int count)
{
    int             i;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT `messageNum` FROM `mailMessages` "
                    "WHERE LOWER(`mailTo`) = LOWER('%s') ORDER BY `timestamp` "
                    "LIMIT %d", recipient, count );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(count = mysql_num_rows(res)) ) {
        mysql_free_result(res);
        return(0);
    }

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        messageNum[i] = atoi(row[0]);
    }
    mysql_free_result(res);

    return( count );
}

char *db_get_mail_message(int messageId)
{
    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT `mailFrom`, `mailTo`, "
                    "DATE_FORMAT(`timestamp`, '%%a %%b %%c %%Y  %%H:%%i:%%S'), "
                    "`message` FROM `mailMessages` WHERE `messageNum` = %d",
                    messageId );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !mysql_num_rows(res) ) {
        mysql_free_result(res);
        return(NULL);
    }

    row = mysql_fetch_row(res);

    sprintf(sqlbuf, " * * * * Havok Mail System * * * *\n\r"
                    "Date: %s\n\r"
                    "  To: %s\n\r"
                    "From: %s\n\r\n\r%s\n\r", row[2], row[1], row[0], row[3]);
    mysql_free_result(res);

    return( strdup(sqlbuf) );
}

void db_delete_mail_message(int messageId)
{
    sprintf(sqlbuf, "DELETE FROM `mailMessages` WHERE `messageNum` = %d",
                    messageId );
    mysql_query(sql, sqlbuf);
}

int CheckKillFile(long virtual)
{
    MYSQL_RES      *res;
    int             count;

    sprintf(sqlbuf, "SELECT `vnum` FROM `mobKillfile` WHERE `vnum` = %ld", 
                    virtual );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);

    count = (res ? mysql_num_rows(res) : 0 );

    mysql_free_result(res);
    return (count);
}


char *db_lookup_help( int type, char *keywords )
{
    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT `keywords`, `helpText` FROM `helpTopics` "
                    "WHERE `helpType` = %d AND "
                    "UPPER(`keywords`) = UPPER('%s')", type, keywords );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !mysql_num_rows(res) ) {
        mysql_free_result(res);
        return(NULL);
    }

    row = mysql_fetch_row(res);

    sprintf(sqlbuf, "\"%s\"\n\r\n\r"
                 "%s\n\r", row[0], row[1] );
    mysql_free_result(res);

    return( strdup(sqlbuf) );
}

char *db_lookup_help_similar( int type, char *keywords )
{
    char            line[256];
    int             count,
                    i;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT UPPER(`keywords`), "
                    "MATCH(`keywords`, `helpText`) AGAINST('%s') AS score "
                    "FROM  `helpTopics` WHERE `helpType` = %d AND "
                    "MATCH(keywords, helpText) AGAINST('%s') "
                    "ORDER BY score DESC LIMIT 10", keywords, type, keywords );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(count = mysql_num_rows(res)) ) {
        mysql_free_result(res);
        return(NULL);
    }

    sprintf(sqlbuf, "No exact matches found for \"%s\".  Top %d relevant "
                    "topics are:\n\r  %-68s Score\n\r", keywords, count,
                    "Keywords" );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        snprintf(line, 255, "    %-66s %6.3f\n\r", row[0],
                            strtod(row[1], NULL));
        strcat(sqlbuf, line);
    }

    mysql_free_result(res);

    strcat(sqlbuf, "Please retry help using one of these keywords.\n\r");

    return( strdup(sqlbuf) );
}

void db_save_object(struct obj_data *obj, int owner, int ownerItem )
{
    char           *name,
                   *shortDesc,
                   *desc,
                   *actDesc,
                   *keyword,
                   *quoted;
    int             i,
                    j;
    struct extra_descr_data *descr;
    int             vnum;

    shortDesc = db_quote(obj->short_description);
    desc      = db_quote(obj->description);
    actDesc   = db_quote(obj->action_description);
    vnum      = obj_index[obj->item_number].virtual;

    i = strlen(actDesc) - 1;
    while( i > 0 &&
           (actDesc[i] == '\n' || actDesc[i] == '\r') &&
           (actDesc[i-1] == '\n' || actDesc[i-1] == '\r') ) {
        actDesc[i--] = '\0';
    }

    if( i == 0 && (actDesc[0] == '\n' || actDesc[0] == '\r') ) {
        actDesc[0] = '\0';
    }

    sprintf(sqlbuf, "REPLACE INTO `objects` (`vnum`, `ownerId`, `ownedItemId`, "
                    "`shortDescription`, `description`, "
                    "`actionDescription`, `modBy`, `itemType`, `value0`, "
                    "`value1`, `value2`, `value3`, `weight`, `cost`, "
                    "`costPerDay`, `level`, `max`, `modified`, "
                    "`speed`, `weaponType`, `tweak`) VALUES ( %d, %d, %d, "
                    "'%s', '%s', '%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, "
                    "%d, %d, FROM_UNIXTIME(%ld), %d, %d, %d)", 
                    vnum, owner, ownerItem,
                    shortDesc, desc, actDesc, obj->modBy,
                    obj->obj_flags.type_flag, obj->obj_flags.value[0],
                    obj->obj_flags.value[1], obj->obj_flags.value[2],
                    obj->obj_flags.value[3], obj->obj_flags.weight,
                    obj->obj_flags.cost, obj->obj_flags.cost_per_day, 
                    obj->level, obj->max, (long)obj->modified, 
                    (IS_WEAPON(obj)? obj->speed : 0),
                    (IS_WEAPON(obj) ? obj->weapontype : 0), obj->tweak );
    mysql_query(sql, sqlbuf);
    free(shortDesc);
    free(desc);
    free(actDesc);

    name = strdup(obj->name);
    i = 0;
    while( (keyword = strsep(&name, " \t\n\r")) ) {
        quoted = db_quote(keyword);
        sprintf(sqlbuf, "REPLACE INTO `objectKeywords` (`vnum`, `ownerId`, "
                        "`ownedItemId`, `seqNum`, `keyword`) VALUES ( %d, %d, "
                        "%d, %d, '%s')", vnum, owner, ownerItem, i, quoted);
        mysql_query(sql, sqlbuf);
        free(quoted);
        i++;
    }
    free(name);

    sprintf(sqlbuf, "DELETE FROM `objectKeywords` WHERE `vnum` = %d AND "
                    "`ownerId` = %d AND `ownedItemId` = %d AND `seqNum` >= %d",
                    vnum, owner, ownerItem, i);
    mysql_query(sql, sqlbuf);

    strcpy(sqlbuf, "REPLACE INTO `objectFlags` (`vnum`, `ownerId`, "
                   "`ownedItemId`, `takeable`, `wearFinger`, `wearNeck`, "
                   "`wearBody`, `wearHead`, `wearLegs`, `wearFeet`, "
                   "`wearHands`, `wearArms`, `wearShield`, `wearAbout`, "
                   "`wearWaist`, `wearWrist`, `wearBack`, `wearEar`, "
                   "`wearEye`, `wearLightSource`, `wearHold`, `wearWield`, "
                   "`wearThrow`, `glow`, `hum`, `metal`, `mineral`, `organic`, "
                   "`invisible`, `magic`, `cursed`, `brittle`, `resistant`, "
                   "`immune`, `rare`, `uberRare`, `quest`, `antiSun`, "
                   "`antiGood`, `antiEvil`, ");
    strcat(sqlbuf, "`antiNeutral`, `antiMale`, `antiFemale`, `onlyMage`, "
                   "`onlyCleric`, `onlyWarrior`, `onlyThief`, `onlyDruid`, "
                   "`onlyMonk`, `onlyBarbarian`, `onlySorcerer`, "
                   "`onlyPaladin`, `onlyRanger`, `onlyPsionicist`, "
                   "`onlyNecromancer`, `antiMage`, `antiCleric`, "
                   "`antiWarrior`, `antiThief`, `antiDruid`, `antiMonk`, "
                   "`antiBarbarian`, `antiSorcerer`, `antiPaladin`, "
                   "`antiRanger`, `antiPsionicist`, `antiNecromancer`) "
                   "VALUES (" );
    sprintf( &sqlbuf[strlen(sqlbuf)], "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
                                      "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
                                      "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
                                      "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
                                      "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
                                      "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
                                      "%d, %d, %d, %d, %d, %d, %d)", 
                                      vnum, owner, ownerItem,
                                      WEAR_FLAG(obj, ITEM_TAKE),
                                      WEAR_FLAG(obj, ITEM_WEAR_FINGER),
                                      WEAR_FLAG(obj, ITEM_WEAR_NECK),
                                      WEAR_FLAG(obj, ITEM_WEAR_BODY),
                                      WEAR_FLAG(obj, ITEM_WEAR_HEAD),
                                      WEAR_FLAG(obj, ITEM_WEAR_LEGS),
                                      WEAR_FLAG(obj, ITEM_WEAR_FEET),
                                      WEAR_FLAG(obj, ITEM_WEAR_HANDS),
                                      WEAR_FLAG(obj, ITEM_WEAR_ARMS),
                                      WEAR_FLAG(obj, ITEM_WEAR_SHIELD),
                                      WEAR_FLAG(obj, ITEM_WEAR_ABOUT),
                                      WEAR_FLAG(obj, ITEM_WEAR_WAISTE),
                                      WEAR_FLAG(obj, ITEM_WEAR_WRIST),
                                      WEAR_FLAG(obj, ITEM_WEAR_BACK),
                                      WEAR_FLAG(obj, ITEM_WEAR_EAR),
                                      WEAR_FLAG(obj, ITEM_WEAR_EYE),
                                      WEAR_FLAG(obj, ITEM_LIGHT_SOURCE),
                                      WEAR_FLAG(obj, ITEM_HOLD),
                                      WEAR_FLAG(obj, ITEM_WIELD),
                                      WEAR_FLAG(obj, ITEM_THROW),
                                      EXTRA_FLAG(obj, ITEM_GLOW),
                                      EXTRA_FLAG(obj, ITEM_HUM),
                                      EXTRA_FLAG(obj, ITEM_METAL),
                                      EXTRA_FLAG(obj, ITEM_MINERAL),
                                      EXTRA_FLAG(obj, ITEM_ORGANIC),
                                      EXTRA_FLAG(obj, ITEM_INVISIBLE),
                                      EXTRA_FLAG(obj, ITEM_MAGIC),
                                      EXTRA_FLAG(obj, ITEM_NODROP),
                                      EXTRA_FLAG(obj, ITEM_BRITTLE),
                                      EXTRA_FLAG(obj, ITEM_RESISTANT),
                                      EXTRA_FLAG(obj, ITEM_IMMUNE),
                                      EXTRA_FLAG(obj, ITEM_RARE),
                                      EXTRA_FLAG(obj, ITEM_UNUSED),
                                      EXTRA_FLAG(obj, ITEM_QUEST),
                                      EXTRA_FLAG(obj, ITEM_ANTI_SUN),
                                      EXTRA_FLAG(obj, ITEM_ANTI_GOOD),
                                      EXTRA_FLAG(obj, ITEM_ANTI_EVIL),
                                      EXTRA_FLAG(obj, ITEM_ANTI_NEUTRAL),
                                      EXTRA_FLAG(obj, ITEM_ANTI_MEN),
                                      EXTRA_FLAG(obj, ITEM_ANTI_WOMEN),
                                      ONLY_FLAG(obj, ITEM_ANTI_MAGE),
                                      ONLY_FLAG(obj, ITEM_ANTI_CLERIC),
                                      ONLY_FLAG(obj, ITEM_ANTI_FIGHTER),
                                      ONLY_FLAG(obj, ITEM_ANTI_THIEF),
                                      ONLY_FLAG(obj, ITEM_ANTI_DRUID),
                                      ONLY_FLAG(obj, ITEM_ANTI_MONK),
                                      ONLY_FLAG(obj, ITEM_ANTI_BARBARIAN),
                                      ONLY_FLAG(obj, ITEM_ANTI_MAGE),
                                      ONLY_FLAG(obj, ITEM_ANTI_PALADIN),
                                      ONLY_FLAG(obj, ITEM_ANTI_RANGER),
                                      ONLY_FLAG(obj, ITEM_ANTI_PSI),
                                      ONLY_FLAG(obj, ITEM_ANTI_NECROMANCER),
                                      ANTI_FLAG(obj, ITEM_ANTI_MAGE),
                                      ANTI_FLAG(obj, ITEM_ANTI_CLERIC),
                                      ANTI_FLAG(obj, ITEM_ANTI_FIGHTER),
                                      ANTI_FLAG(obj, ITEM_ANTI_THIEF),
                                      ANTI_FLAG(obj, ITEM_ANTI_DRUID),
                                      ANTI_FLAG(obj, ITEM_ANTI_MONK),
                                      ANTI_FLAG(obj, ITEM_ANTI_BARBARIAN),
                                      ANTI_FLAG(obj, ITEM_ANTI_MAGE),
                                      ANTI_FLAG(obj, ITEM_ANTI_PALADIN),
                                      ANTI_FLAG(obj, ITEM_ANTI_RANGER),
                                      ANTI_FLAG(obj, ITEM_ANTI_PSI),
                                      ANTI_FLAG(obj, ITEM_ANTI_NECROMANCER));
    mysql_query(sql, sqlbuf);

    for (descr = obj->ex_description, i = 0; descr; descr = descr->next, i++) {
        desc = db_quote(descr->description);
        name = db_quote(descr->keyword);
        sprintf(sqlbuf, "REPLACE INTO `objectExtraDesc` (`vnum`, `ownerId`, "
                        "`ownedItemId`, `seqNum`, `keyword`, `description`) "
                        "VALUES ( %d, %d, %d, %d, '%s', '%s')",
                        vnum, owner, ownerItem, i, name, desc );
        mysql_query(sql, sqlbuf);
        free(name);
        free(desc);
    }

    sprintf(sqlbuf, "DELETE FROM `objectExtraDesc` WHERE `vnum` = %d AND "
                    "`ownerId` = %d AND `ownedItemId` = %d AND `seqNum` > %d",
                    vnum, owner, ownerItem, i );
    mysql_query(sql, sqlbuf);

    for (i = 0, j = 0; i < MAX_OBJ_AFFECT; i++) {
        if (obj->affected[i].location != APPLY_NONE) {
            sprintf(sqlbuf, "REPLACE INTO `objectAffects` (`vnum`, `ownerId`, "
                            "`ownedItemId`, `seqNum`, `location`, `modifier`) "
                            "VALUES ( %d, %d, %d, %d, %d, %ld )",
                            vnum, owner, ownerItem, j, 
                            obj->affected[i].location,
                            obj->affected[i].modifier);
            mysql_query(sql, sqlbuf);
            j++;
        }
    }

    sprintf(sqlbuf, "DELETE FROM `objectAffects` WHERE `vnum` = %d AND "
                    "`ownerId` = %d AND `ownedItemId` = %d AND `seqNum` > %d",
                    vnum, owner, ownerItem, j);
    mysql_query(sql, sqlbuf);
}

struct obj_flag_bits {
    int var;
    unsigned int set;
    unsigned int clear;
};

struct obj_flag_bits obj_flags[] = {
    { 0, ITEM_TAKE, 0 },
    { 0, ITEM_WEAR_FINGER, 0 },
    { 0, ITEM_WEAR_NECK, 0 },
    { 0, ITEM_WEAR_BODY, 0 },
    { 0, ITEM_WEAR_HEAD, 0 },
    { 0, ITEM_WEAR_LEGS, 0 },
    { 0, ITEM_WEAR_FEET, 0 },
    { 0, ITEM_WEAR_HANDS, 0 },
    { 0, ITEM_WEAR_ARMS, 0 },
    { 0, ITEM_WEAR_SHIELD, 0 },
    { 0, ITEM_WEAR_ABOUT, 0 },
    { 0, ITEM_WEAR_WAISTE, 0 },
    { 0, ITEM_WEAR_WRIST, 0 },
    { 0, ITEM_WEAR_BACK, 0 },
    { 0, ITEM_WEAR_EAR, 0 },
    { 0, ITEM_WEAR_EYE, 0 },
    { 0, ITEM_LIGHT_SOURCE, 0 },
    { 0, ITEM_HOLD, 0 },
    { 0, ITEM_WIELD, 0 },
    { 0, ITEM_THROW, 0 },
    { 1, ITEM_GLOW, 0 },
    { 1, ITEM_HUM, 0 },
    { 1, ITEM_METAL, 0 },
    { 1, ITEM_MINERAL, 0 },
    { 1, ITEM_ORGANIC, 0 },
    { 1, ITEM_INVISIBLE, 0 },
    { 1, ITEM_MAGIC, 0 },
    { 1, ITEM_NODROP, 0 },
    { 1, ITEM_BRITTLE, 0 },
    { 1, ITEM_RESISTANT, 0 },
    { 1, ITEM_IMMUNE, 0 },
    { 1, ITEM_RARE, 0 },
    { 1, ITEM_UNUSED, 0 },
    { 1, ITEM_QUEST, 0 },
    { 1, ITEM_ANTI_SUN, 0 },
    { 1, ITEM_ANTI_GOOD, 0 },
    { 1, ITEM_ANTI_EVIL, 0 },
    { 1, ITEM_ANTI_NEUTRAL, 0 },
    { 1, ITEM_ANTI_MEN, 0 },
    { 1, ITEM_ANTI_WOMEN, 0 },
    { 1, ITEM_ANTI_MAGE | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_CLERIC | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_FIGHTER | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_THIEF | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_DRUID | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_MONK | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_BARBARIAN | ITEM_ONLY_CLASS, 0 },
    { 1, 0, 0 },
    { 1, ITEM_ANTI_PALADIN | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_RANGER | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_PSI | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_NECROMANCER | ITEM_ONLY_CLASS, 0 },
    { 1, ITEM_ANTI_MAGE, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_CLERIC, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_FIGHTER, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_THIEF, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_DRUID, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_MONK, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_BARBARIAN, ITEM_ONLY_CLASS },
    { 1, 0, 0 },
    { 1, ITEM_ANTI_PALADIN, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_RANGER, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_PSI, ITEM_ONLY_CLASS },
    { 1, ITEM_ANTI_NECROMANCER, ITEM_ONLY_CLASS }
};

struct obj_data *db_read_object(struct obj_data *obj, int vnum, int owner,
                                int ownerItem )
{
    unsigned int   *var;
    struct extra_descr_data *descr;
    struct extra_descr_data *prev;
    int             count;
    int             i;
    int             len;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(sqlbuf, "SELECT `shortDescription`, `description`, "
                    "`actionDescription`, `modBy`, `itemType`, `value0`, "
                    "`value1`, `value2`, `value3`, `weight`, `cost`, "
                    "`costPerDay`, `level`, `max`, UNIX_TIMESTAMP(`modified`), "
                    "`speed`, `weaponType`, `tweak` FROM `objects` "
                    "WHERE `vnum` = %d AND `ownerId` = %d AND "
                    "`ownedItemId` = %d", vnum, owner, ownerItem);
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !mysql_num_rows(res) ) {
        mysql_free_result(res);
        return(NULL);
    }

    row = mysql_fetch_row(res);

    obj->short_description = strdup(row[0]);
    obj->description = strdup(row[1]);
    obj->action_description = strdup(row[2]);
    obj->modBy = strdup(row[3]);
    obj->obj_flags.type_flag = atoi(row[4]);
    obj->obj_flags.value[0] = atoi(row[5]);
    obj->obj_flags.value[1] = atoi(row[6]);
    obj->obj_flags.value[2] = atoi(row[7]);
    obj->obj_flags.value[3] = atoi(row[8]);
    obj->obj_flags.weight = atoi(row[9]);
    obj->obj_flags.cost = atoi(row[10]);
    obj->obj_flags.cost_per_day = atoi(row[11]);
    obj->level = atoi(row[12]);
    obj->max = atoi(row[13]);
    obj->modified = atol(row[14]); 
    obj->speed = atoi(row[15]);
    obj->weapontype = atoi(row[16]);
    obj->tweak = atoi(row[17]);

    mysql_free_result(res);

    sprintf(sqlbuf, "SELECT `keyword` FROM `objectKeywords` WHERE `vnum` = %d "
                    "AND `ownerId` = %d AND `ownedItemId` = %d ORDER BY "
                    "`seqNum`", vnum, owner, ownerItem );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(count = mysql_num_rows(res)) ) {
        mysql_free_result(res);
        free_obj(obj);
        return(NULL);
    }

    obj->name = NULL;
    len = 0;

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        obj->name = (char *)realloc(obj->name, len + strlen(row[0]) + 2);
        if( !len ) {
            strcpy( obj->name, row[0] );
        } else {
            strcat( obj->name, " " );
            strcat( obj->name, row[0] );
        }
        len = strlen(obj->name);
    }
    mysql_free_result(res);


    strcpy(sqlbuf, "SELECT `takeable`, `wearFinger`, `wearNeck`, "
                   "`wearBody`, `wearHead`, `wearLegs`, `wearFeet`, "
                   "`wearHands`, `wearArms`, `wearShield`, `wearAbout`, "
                   "`wearWaist`, `wearWrist`, `wearBack`, `wearEar`, "
                   "`wearEye`, `wearLightSource`, `wearHold`, `wearWield`, "
                   "`wearThrow`, `glow`, `hum`, `metal`, `mineral`, `organic`, "
                   "`invisible`, `magic`, `cursed`, `brittle`, `resistant`, "
                   "`immune`, `rare`, `uberRare`, `quest`, `antiSun`, "
                   "`antiGood`, `antiEvil`, ");
    strcat(sqlbuf, "`antiNeutral`, `antiMale`, `antiFemale`, `onlyMage`, "
                   "`onlyCleric`, `onlyWarrior`, `onlyThief`, `onlyDruid`, "
                   "`onlyMonk`, `onlyBarbarian`, `onlySorcerer`, "
                   "`onlyPaladin`, `onlyRanger`, `onlyPsionicist`, "
                   "`onlyNecromancer`, `antiMage`, `antiCleric`, "
                   "`antiWarrior`, `antiThief`, `antiDruid`, `antiMonk`, "
                   "`antiBarbarian`, `antiSorcerer`, `antiPaladin`, "
                   "`antiRanger`, `antiPsionicist`, `antiNecromancer` "
                   "FROM `objectFlags` ");
                
    sprintf( &sqlbuf[strlen(sqlbuf)], "WHERE `vnum` = %d AND `ownerId` = %d "
                                      "AND `ownedItemId` = %d", 
                                      vnum, owner, ownerItem );
    mysql_query(sql, sqlbuf);
    
    res = mysql_store_result(sql);
    if( !res || !mysql_num_rows(res) ) {
        mysql_free_result(res);
        free_obj(obj);
        return(NULL);
    }

    obj->obj_flags.wear_flags = 0;
    obj->obj_flags.extra_flags = 0;

    row = mysql_fetch_row(res);

    for( i = 0; i < 64; i++ ) {
        if( *row[i] == '1' ) {
            switch( obj_flags[i].var ) {
            case 0:
                var = (unsigned int *)&(obj->obj_flags.wear_flags);
                break;
            default:
            case 1:
                var = (unsigned int *)&(obj->obj_flags.extra_flags);
                break;
            }

            SET_BIT(*var, obj_flags[i].set);
            REMOVE_BIT(*var, obj_flags[i].clear);
        }
    }

    mysql_free_result(res);

    obj->ex_description = NULL;

    sprintf(sqlbuf, "SELECT `keyword`, `description` FROM `objectExtraDesc` "
                    "WHERE `vnum` = %d AND `ownerId` = %d AND "
                    "`ownerItemId` = %d ORDER BY `seqNum`", 
                    vnum, owner, ownerItem );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( res && (count = mysql_num_rows(res)) ) {
        descr = NULL;
        prev = NULL;

        for( i = 0; i < count; i++ ) {
            row = mysql_fetch_row(res);

            descr = (struct extra_descr_data *)
                                malloc(sizeof(struct extra_descr_data));
            if( !prev ) {
                obj->ex_description = descr;
            } else {
                prev->next = descr;
            }

            descr->next = NULL;
            descr->keyword = strdup(row[0]);
            descr->description = strdup(row[1]);

            prev = descr;
        }
    }
    mysql_free_result(res);

    sprintf(sqlbuf, "SELECT `location`, `modifier` FROM `objectAffects` "
                    "WHERE `vnum` = %d AND `ownerId` = %d AND "
                    "`ownedItemId` = %d ORDER BY `seqNum`",
                    vnum, owner, ownerItem);
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( res && (count = mysql_num_rows(res)) ) {
        for( i = 0; i < count && i < MAX_OBJ_AFFECT; i++ ) {
            row = mysql_fetch_row(res);

            obj->affected[i].location = atoi(row[0]);
            obj->affected[i].modifier = atoi(row[1]);
        }
    }
    mysql_free_result(res);

    return(obj);
}

struct keyword_list;
struct keyword_list {
    struct keyword_list *next;
    char *keyword;
};

int db_find_object_named(char *string, int owner, int ownerItem)
{
    char            tempbuf[256];
    int             count;
    int             i;
    char           *keyword;
    char           *quoted;
    struct keyword_list *keywordList;
    struct keyword_list *prev;
    struct keyword_list *curr;
    int             vnum;

    MYSQL_RES      *res;
    MYSQL_ROW       row;


    keywordList = NULL;
    prev = NULL;
    count = 0;

    while( ( keyword = strsep( &string, "- \t\n\r" ) ) ) {
        quoted = db_quote(keyword);

        curr = (struct keyword_list *)malloc(sizeof(struct keyword_list));
        curr->next = NULL;
        curr->keyword = quoted;

        if( !prev ) {
            keywordList = curr;
        } else {
            prev->next = curr;
        }
        prev = curr;
        count++;
    }

    strcpy(sqlbuf, "SELECT a1.vnum from " );
    for( i = 0; i < count; i++ ) {
        if( i ) {
            strcat(sqlbuf, ", ");
        }
        sprintf(tempbuf, "`objectKeywords` as a%d", i+1);
        strcat(sqlbuf, tempbuf);
    }
    strcat(sqlbuf, " WHERE ");

    if( count > 1 ) {
        for( i = 1; i < count; i++ ) {
            sprintf(tempbuf, "a%d.vnum = a1.vnum AND ", i+1);
            strcat(sqlbuf, tempbuf);
        }
    }

    for( i = 0, curr = keywordList; 
         i < count && curr; 
         i++, curr = curr->next ) {

        sprintf(tempbuf, "a%d.keyword REGEXP '^%s' AND ", i+1, curr->keyword );
        strcat(sqlbuf, tempbuf);
    }
    strcat(sqlbuf, "1 LIMIT 1");
    mysql_query(sql, sqlbuf);

    vnum = -1;

    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);

        vnum = atoi(row[0]);
    }
    mysql_free_result(res);

    for( curr = keywordList; curr; curr = prev ) {
        prev = curr->next;
        free( curr->keyword );
        free( curr );
    }

    return( vnum );
}


struct index_data *db_generate_object_index(int *top, int *sort_top,
                                            int *alloc_top)
{
    struct index_data *index = NULL;
    int             i,
                    vnum,
                    j,
                    len;
    int             count,
                    keyCount;

    MYSQL_RES      *res,
                   *resKeywords;
    MYSQL_ROW       row;

    strcpy(sqlbuf, "SELECT `vnum` FROM `objects` WHERE `ownerId` = -1 AND "
                   "ownedItemId = -1 ORDER BY `vnum`" );
    mysql_query(sql, sqlbuf);

    res = mysql_store_result(sql);
    if( !res || !(count = mysql_num_rows(res)) ) {
        mysql_free_result(res);
        return( NULL );
    }

    index = (struct index_data *)malloc(count * sizeof(struct index_data));
    if( !index ) {
        mysql_free_result(res);
        return( NULL );
    }

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        vnum = atoi(row[0]);
        index[i].virtual = vnum;
        index[i].pos = -1;
        index[i].number = 0;
        index[i].data = NULL;
        index[i].func = NULL;

        sprintf( sqlbuf, "SELECT `keyword` FROM `objectKeywords` "
                         "WHERE `vnum` = %d AND `ownerId` = -1 AND "
                         "`ownedItemId` = -1 ORDER BY `seqNum`", vnum );
        mysql_query(sql, sqlbuf);

        index[i].name = NULL;
        len = 0;

        resKeywords = mysql_store_result(sql);
        if( !resKeywords || !(keyCount = mysql_num_rows(resKeywords)) ) {
            mysql_free_result(resKeywords);
            continue;
        }

        for( j = 0; j < keyCount; j++ ) {
            row = mysql_fetch_row(resKeywords);

            index[i].name = (char *)realloc(index[i].name, 
                                            len + strlen(row[0]) + 2);
            if( !len ) {
                strcpy( index[i].name, row[0] );
            } else {
                strcat( index[i].name, " " );
                strcat( index[i].name, row[0] );
            }
            len = strlen(index[i].name);
        }

        mysql_free_result(resKeywords);
    }
    mysql_free_result(res);

    *sort_top = count - 1;
    *alloc_top = count;
    *top = count;

    return (index);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

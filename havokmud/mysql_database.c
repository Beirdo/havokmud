#include "config.h"
#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"

static MYSQL *sql;

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
    char            buf[MAX_STRING_LENGTH];
    char           *string;

    if( !report ) {
        return;
    }

    string = db_quote(report);
    if( string ) {
        sprintf( buf, "INSERT INTO `userReports` (`reportId`, `reportTime`, "
                      "`character`, `roomNum`, `report`) "
                      "VALUES ( %d, NULL, '%s', %ld, '%s' )", reportId,
                      GET_NAME(ch), ch->in_room, string );
        mysql_query(sql, buf);
        free( string );
    }
}

struct user_report *db_get_report(int reportId, struct user_report *report)
{
    char                buf[MAX_STRING_LENGTH];
    MYSQL_RES          *res;
    MYSQL_ROW           row;

    if( !report ) {
        /* Need to to the query */
        sprintf( buf, "SELECT `reportTime`, `character`, `roomNum`, `report` "
                      "FROM `userReports` WHERE `reportId` = %d "
                      "ORDER BY `reportTime` ASC", reportId );
        mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    sprintf( buf, "DELETE FROM `userReports` WHERE `reportId` = %d", reportId );
    mysql_query(sql, buf);
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
    char                buf[MAX_STRING_LENGTH];
    int                 i,
                        j,
                        level;
    int                 classId;
    int                 skillCount;
    int                 err = FALSE;
    MYSQL_RES           *resClass, *resSkill;
    MYSQL_ROW           row;

    Log("Loading classes[] from SQL");

    strcpy(buf, "SELECT classId, className, classAbbrev "
                "FROM classes ORDER BY classId ASC");
    mysql_query(sql, buf);

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

        sprintf( buf, "SELECT skills.skillName, skills.skillID, "
                      "classSkills.minLevel, classSkills.maxTeach "
                      "FROM skills, classSkills "
                      "WHERE skills.skillId = classSkills.skillId AND "
                      "classSkills.classId = %d AND classSkills.mainSkill = 0 "
                      "ORDER BY skills.skillName ASC", classId );
        mysql_query(sql, buf);

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


        sprintf( buf, "SELECT skills.skillName, skills.skillID, "
                      "classSkills.minLevel, classSkills.maxTeach "
                      "FROM skills, classSkills "
                      "WHERE skills.skillId = classSkills.skillId AND "
                      "classSkills.classId = %d AND classSkills.mainSkill = 1 "
                      "ORDER BY skills.skillName ASC", classId );
        mysql_query(sql, buf);

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

        sprintf( buf, "SELECT level, thaco, maleTitle, femaleTitle, minExp "
                      "FROM classLevels WHERE classId = %d ORDER BY level",
                      classId );
        mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    int             skillId;
    int             err = FALSE;
    MYSQL_RES      *resSkill, *resMsg;
    MYSQL_ROW       row;

    Log("Loading skills[] from SQL");

    strcpy(buf, "SELECT skillId, skillName, skillType "
                "FROM skills ORDER BY skillId ASC");
    mysql_query(sql, buf);

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
            sprintf( buf, "SELECT text FROM skillMessages "
                          "WHERE `skillId` = %d AND `msgId` = 1 AND "
                          "`index` = %d", skillId, j+1 );
            mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading misc structures from SQL");

    /* direction[] */
    strcpy(buf, "SELECT forward, reverse, trapBits, exit, listExit, "
                "direction, description "
                "FROM directions ORDER BY forward ASC");
    mysql_query(sql, buf);

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
    strcpy(buf, "SELECT clanId, clanName, shortName, description, homeRoom "
                "FROM playerClans ORDER BY clanId ASC");
    mysql_query(sql, buf);

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
    strcpy(buf, "SELECT sectorType, mapChar, moveLoss "
                "FROM sectorType ORDER BY sectorId ASC");
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    MYSQL_RES      *res, *resMsg;
    MYSQL_ROW       row;

    Log("Loading fight messages from SQL");

    strcpy( buf, "SELECT DISTINCT skillId FROM skillMessages "
                 "WHERE msgId = 2 ORDER BY skillId" );
    mysql_query(sql, buf);

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
            sprintf( buf, "SELECT text FROM skillMessages "
                          "WHERE `skillId` = %d AND `msgId` = 2 AND "
                          "`index` = %d", skill, j+1 );
            mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    char           *tmp;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading banned usernames from SQL");

    strcpy( buf, "SELECT name FROM bannedName" );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    int             tmp;

    MYSQL_RES      *res, *resMsg;
    MYSQL_ROW       row;


    Log("Loading social messages from SQL");

    strcpy( buf, "SELECT socialId, hide, minPosition FROM socials "
                 "ORDER BY socialId" );
    mysql_query(sql, buf);

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
            sprintf( buf, "SELECT text FROM skillMessages "
                          "WHERE `skillId` = %d AND `msgId` = 3 AND "
                          "`index` = %d", tmp, j+1 );
            mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    int             tmp;

    MYSQL_RES      *res, *resMsg;
    MYSQL_ROW       row;

    Log("Loading kick messages from SQL");

    strcpy( buf, "SELECT DISTINCT skillId FROM skillMessages "
                 "WHERE msgId = 4 ORDER BY skillId" );
    mysql_query(sql, buf);

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
            sprintf( buf, "SELECT text FROM skillMessages "
                          "WHERE `skillId` = %d AND `msgId` = 4 AND "
                          "`index` = %d", tmp, j+1 );
            mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    int             tmp;

    MYSQL_RES      *res, *resMsg;
    MYSQL_ROW       row;

    Log("Loading pose messages from SQL");

    strcpy( buf, "SELECT DISTINCT skillId FROM skillMessages "
                 "WHERE msgId = 5 OR msgId = 6 ORDER BY skillId" );
    mysql_query(sql, buf);

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
            sprintf( buf, "SELECT text FROM skillMessages "
                          "WHERE `skillId` = %d AND `msgId` = 5 AND "
                          "`index` = %d", tmp, j+1 );
            mysql_query(sql, buf);

            resMsg = mysql_store_result(sql);
            if( resMsg && mysql_num_rows(resMsg) ) {
                row = mysql_fetch_row(resMsg);
                poseMessages[i].poser_msg[j] = strdup(row[0]);
            }
            mysql_free_result(resMsg);

            sprintf( buf, "SELECT text FROM skillMessages "
                          "WHERE `skillId` = %d AND `msgId` = 6 AND "
                          "`index` = %d", tmp, j+1 );
            mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    int_func        func;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading mobile procs from SQL");

    sprintf( buf, "SELECT `vnum`, `procedure` FROM procAssignments "
                  "WHERE `procType` = %d ORDER BY `vnum`", PROC_MOBILE );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    int_func        func;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading object procs from SQL");

    sprintf( buf, "SELECT `vnum`, `procedure` FROM procAssignments "
                  "WHERE `procType` = %d ORDER BY `vnum`", PROC_OBJECT );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    int_func        func;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading room procs from SQL");

    sprintf( buf, "SELECT `vnum`, `procedure` FROM procAssignments "
                  "WHERE `procType` = %d ORDER BY `vnum`", PROC_ROOM );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    MYSQL_RES      *res, *resMax;
    MYSQL_ROW       row;

    Log("Loading races[] from SQL");

    strcpy( buf, "SELECT raceId, raceName, description, raceSize, ageStart, "
                 "ageYoung, ageMature, ageMiddle, ageOld, ageAncient, "
                 "ageVenerable, nativeLanguage "
                 "FROM races ORDER BY raceId" );
    mysql_query(sql, buf);

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
            sprintf( buf, "SELECT maxLevel FROM racialMax "
                          "WHERE raceId = %d AND classId = %d",
                          races[i].race + 1, j + 1 );
            mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    Log("Loading languages[] from SQL");

    strcpy( buf, "SELECT `langId`, `skillId`, `name` FROM languages "
                 "ORDER BY `langId`" );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    MYSQL_RES      *res;
    MYSQL_ROW       row;


    Log("Loading Essential Text Files.");

    if( news ) {
        free( news );
    }
    strcpy( buf, "SELECT fileContents FROM textFiles WHERE fileId = 5" );
    mysql_query(sql, buf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        news = strdup(row[0]);
    }
    mysql_free_result(res);

    if( credits ) {
        free( credits );
    }
    strcpy( buf, "SELECT fileContents FROM textFiles WHERE fileId = 1" );
    mysql_query(sql, buf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        credits = strdup(row[0]);
    }
    mysql_free_result(res);

    if( motd ) {
        free( motd );
    }
    strcpy( buf, "SELECT fileContents FROM textFiles WHERE fileId = 4" );
    mysql_query(sql, buf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        motd = strdup(row[0]);
    }
    mysql_free_result(res);

    if( wmotd ) {
        free( wmotd );
    }
    strcpy( buf, "SELECT fileContents FROM textFiles WHERE fileId = 6" );
    mysql_query(sql, buf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        wmotd = strdup(row[0]);
    }
    mysql_free_result(res);

    if( help ) {
        free( help );
    }
    help = file_to_string(HELP_PAGE_FILE);

    if( info ) {
        free( info );
    }
    strcpy( buf, "SELECT fileContents FROM textFiles WHERE fileId = 2" );
    mysql_query(sql, buf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        info = strdup(row[0]);
    }
    mysql_free_result(res);

    if( login ) {
        free( login );
    }
    strcpy( buf, "SELECT fileContents FROM textFiles WHERE fileId = 3" );
    mysql_query(sql, buf);
    res = mysql_store_result(sql);
    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        login = strdup(row[0]);
    }
    mysql_free_result(res);
}

int db_save_textfile(struct char_data *ch)
{
    char            buf[MAX_STRING_LENGTH * 2];
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

    strcpy(buf, "\n");
    if (tfd->date) {
        strcat(buf, tfd->date);
        while(buf[strlen(buf) - 1] == '~') {
            buf[strlen(buf) - 1] = '\0';
        }
        strcat(buf, "\n");
    }

    if (tfd->body) {
        strcat(buf, "\n");
        remove_cr(&buf[strlen(buf)], tfd->body);
        while (buf[strlen(buf) - 1] == '~') {
            buf[strlen(buf) - 1] = '\0';
        }
    }

    if (tfd->author) {
        strcat(buf, "\nLast edited by ");
        strcat(buf, tfd->author);
        strcat(buf, ".");
        sprintf( author, "'%s'", tfd->author );
    } else {
        strcpy( author, "NULL" );
    }

    strcat(buf, "\n");

    outbuf = db_quote(buf);

    sprintf( buf, "DELETE FROM textFiles WHERE `fileId` = %d", fileId );
    mysql_query(sql, buf);

    sprintf( buf, "INSERT INTO textFiles (`fileId`, `lastModified`, "
                  "`lastModBy`, `fileContents`) VALUES (%d, NULL, %s, '%s')",
                  fileId, author, outbuf );
    free( outbuf );

    mysql_query(sql, buf);

    return (TRUE);
}




/*
 *  Recursively delete a post and all it's replies.
 */
void db_delete_board_message(struct board_def *board, short message_id)
{
    char            buf[MAX_STRING_LENGTH];
    int             i,
                    count,
                    msgId;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    /*
     * First check to see if we have a reply to this message -- if so
     * axe it
     */
    sprintf(buf, "SELECT `messageNum` FROM `boardMessages` "
                 "WHERE `boardId` = %d AND `replyToMessageNum` = %d",
                 board->boardId, message_id );
    mysql_query(sql, buf);

    res = mysql_store_result(sql);
    if( res && (count = mysql_num_rows(res)) ) {
        for( i = 0; i < count; i++ ) {
            row = mysql_fetch_row(res);
            msgId = atoi(row[0]);
            db_delete_board_message(board, msgId);
        }
    }
    mysql_free_result(res);

    sprintf(buf, "DELETE FROM `boardMessages` where `messageNum` = %d AND "
                 "`boardId` = %d", message_id, board->boardId );
    mysql_query(sql, buf);
    sprintf(buf, "UPDATE `boardMessages` SET `messageNum` = `messageNum` - 1 "
                 "WHERE `messageNum` > %d AND `boardId` = %d", message_id, 
                 board->boardId );
    mysql_query(sql, buf);
    sprintf(buf, "UPDATE `boards` SET `maxPostNum` = `maxPostNum` - 1 "
                 "WHERE boardId = %d", board->boardId );
    mysql_query(sql, buf);
}

struct board_def *db_lookup_board(int vnum)
{
    struct board_def *board;
    char            buf[MAX_STRING_LENGTH];

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(buf, "SELECT `boardId`, `maxPostNum`, `minLevel` "
                 "FROM `boards` WHERE `vnum` = %d", vnum);
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(buf, "SELECT `poster`, `topic`, `post`, "
                 "UNIX_TIMESTAMP(`postTime`), `replyToMessageNum` "
                 "FROM `boardMessages` "
                 "WHERE `boardId` = %d AND messageNum = %d", boardId, msgNum );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    int             count,
                    i;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(buf, "SELECT `poster`, `topic`, `post`, `messageNum`, "
                 "UNIX_TIMESTAMP(`postTime`), `replyToMessageNum` "
                 "FROM `boardMessages` "
                 "WHERE `boardId` = %d AND replyToMessageNum = %d "
                 "ORDER BY `messageNum`", board->boardId, msgId );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    char           *topic,
                   *post;

    topic = db_quote(msg->title);
    post  = db_quote(msg->text);

    sprintf(buf, "INSERT INTO `boardMessages` (`boardId`, `messageNum`, "
                 "`replyToMessageNum`, `topic`, `poster`, `postTime`, "
                 "`post`) VALUES (%d, %d, %d, '%s', '%s', NULL, '%s')",
                 board->boardId, board->messageCount + 1, msg->reply_to,
                 topic, msg->author, post );
    mysql_query(sql, buf);
    
    free(topic);
    free(post);

    sprintf(buf, "UPDATE `boards` SET `maxPostNum` = `maxPostNum` + 1 "
                 "WHERE `boardId` = %d", board->boardId );
    mysql_query(sql, buf);

    db_free_board_message(msg);
}

void db_store_mail(char *to, char *from, char *message_pointer)
{
    char            buf[MAX_STRING_LENGTH];
    char           *post;

    post = db_quote(message_pointer);

    sprintf(buf, "INSERT INTO `mailMessages` (`mailFrom`, `mailTo`, "
                 "`timestamp`, `message`) VALUES ('%s', '%s', NULL, '%s')",
                 from, to, post );
    mysql_query(sql, buf);

    free(post);
}

int             db_has_mail(char *recipient)
{
    char            buf[MAX_STRING_LENGTH];
    int             count;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(buf, "SELECT HIGH_PRIORITY COUNT(*) as count FROM `mailMessages` "
                 "WHERE LOWER(`mailTo`) = LOWER('%s')", recipient );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];
    int             i;

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(buf, "SELECT `messageNum` FROM `mailMessages` "
                 "WHERE LOWER(`mailTo`) = LOWER('%s') ORDER BY `timestamp` "
                 "LIMIT %d", recipient, count );
    mysql_query(sql, buf);

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
    char            buf[MAX_STRING_LENGTH];

    MYSQL_RES      *res;
    MYSQL_ROW       row;

    sprintf(buf, "SELECT `mailFrom`, `mailTo`, "
                 "DATE_FORMAT(`timestamp`, '%%a %%b %%c %%Y  %%H:%%i:%%S'), "
                 "`message` FROM `mailMessages` WHERE `messageNum` = %d",
                 messageId );
    mysql_query(sql, buf);

    res = mysql_store_result(sql);
    if( !res || !mysql_num_rows(res) ) {
        mysql_free_result(res);
        return(NULL);
    }

    row = mysql_fetch_row(res);

    sprintf(buf, " * * * * Havok Mail System * * * *\n\r"
                 "Date: %s\n\r"
                 "  To: %s\n\r"
                 "From: %s\n\r\n\r%s\n\r", row[2], row[1], row[0], row[3]);
    mysql_free_result(res);

    return( strdup(buf) );
}

void db_delete_mail_message(int messageId)
{
    char            buf[MAX_STRING_LENGTH];

    sprintf(buf, "DELETE FROM `mailMessages` WHERE `messageNum` = %d",
                 messageId );
    mysql_query(sql, buf);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

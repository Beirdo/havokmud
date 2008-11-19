FILE           *log_f;

void Log(char *s, ...)
{
    va_list ap;

    va_start(ap, s);

    vsnprintf(logBuf, MAX_STRING_LENGTH, s, ap);
    log_sev(logBuf, 1);
}



/*
 * writes a string to the log
 */
void log_sev(char *str, int sev)
{
    time_t            ct;
    char           *tmstr;
    char            buf[500];
    struct descriptor_data *i;

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    sprintf(buf, "%s :: %s\n", tmstr, str);

    if (spy_flag) {
        fprintf(stderr, "%s", buf);
    }
    /*
     * My Addon to log into file... useful, he?
     */
    if (!log_f) {
        if (!(log_f = fopen("output.log", "w"))) {
            perror("log_sev");
            return;
        }
    }
    fputs(buf, log_f);
    fflush(log_f);

    if (sev > 1) {
        return;
    }
    if (str) {
        sprintf(buf, "/* %s */\n\r", str);
    }
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected && IS_IMMORTAL(i->character) &&
            i->character->specials.sev <= sev &&
            !IS_SET(i->character->specials.act, PLR_NOSHOUT)) {
            SEND_TO_Q(buf, i);
        }
    }
}

void slog(char *str)
{
    log_sev(str, 2);
}


void dlog(char *s)
{
#ifdef DEBUG_LOG
    slog(s);
#endif
}

void qlog(struct char_data *ch, char *text)
{
    db_report_entry(REPORT_QUEST, ch, text );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */


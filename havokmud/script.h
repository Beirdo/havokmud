
struct foo_data {
   char *line;
 };

struct scripts {
   char *filename; /* Script name to execute */
   long virtual; /* Virtual number of the mob associated with the script */
   struct foo_data *script; /* actual script */
 } *script_data;

struct script_com {
   void (*p) (char *arg, struct char_data *ch);
   char *arg;
 } *comp;



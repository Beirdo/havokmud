
struct wiznest {
  char *name;
  char *title;
  time_t last_logon;
  int active;
};

struct wiznode {
  struct wiznest stuff[150];
};

struct wizlistgen {
   int number[MAX_CLASS];
   struct wiznode lookup[10];
};


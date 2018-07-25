struct remote {
  char *name;
  char *user;
  int  port;
  char *host;
  char *path;
  bool is_home;
  char *scp;
  char *ssh;
};

struct ent {
  char name[1024];
  int tid;
  int bid;
  struct ent *next;
};


/* commands */

void commit(char *msg);
void checkout(int16_t from);
void rebuild_index();
void diff(int from, int to);
void init();
void add(int argc, char **list);
void push(char *name);
void list(int start, int qty);
void usage(char *command);


/* internal methods indexing */

int next_id();
void push_index(int16_t id, char *fname);
void generate_cindex(int16_t id);
void index_fnc(void (*fnc)(char *name));
void clear_fnc(char *name);
void clear_index();
void read_fnc(char *name);
void read_index();
void generate_cindex(int16_t id);
int16_t find_prior(int16_t id, char *fname);
struct ent *range_list(int from, int to);
struct ent *diff_list(int from, int to);


/* internal methods - commit flow */

void make_next(int id);
struct ent *flist(int id);
void commit(char *msg);
void make_next(int id);
char *sanatize(char *fname);


/* internal methods - changeset operations */

struct ent *diff_list(int from, int to);
int climb(int id, char *fname);
void print_flist(FILE *out, struct ent *file);
struct ent *flist(int id);
void show_flist(int id);

/* internal methods - list flow */

void show(int id);


/* internal methods - remote flow */

void parse_remote(struct remote *r, char *name);
void remote_free(struct remote *r);
char *ssh_cmd(struct remote *r, char *shell_cmd);
void parse_remote(struct remote *r, char *name);
void scp_commit(struct remote *r, int id);
char *ssh_cmd(struct remote *r, char *shell_cmd);
void init_remote(char *name);
int latest_remote(struct remote *r);



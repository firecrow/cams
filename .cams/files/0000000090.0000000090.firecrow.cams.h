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

int usage(int argc, char **argv);
void init();
void add(int argc, char **list);
void commit(char *msg);
void list(int start, int qty);
void diff(int from, int to);
void checkout(int16_t from);
void push(char *name);
void rebuild_index();
void read_index();
void read_cindex();


/* internal methods indexing */

int next_id();
int16_t find_prior(int16_t id, char *fname);
void push_index(int16_t id, char *fname);
int parse_cindex_line(FILE *file, int16_t *id, char *fname);
void generate_cindex(int16_t id);
void clear_fnc(char *name);
void clear_index();
void index_fnc(void (*fnc)(char *name));
void read_fnc(char *name);


/* internal methods - commit flow */

void make_next(int id);
char *sanatize(char *fname);


/* internal methods - changeset operations */

struct ent *flist(int id);
struct ent *diff_list(int from, int to);
struct ent *range_list(int from, int to);
int climb(int id, char *fname);
void print_flist(FILE *out, struct ent *file);
void show_flist(int id);
void show(int id);


/* internal methods - remote flow */

void parse_remote(struct remote *r, char *name);
void remote_free(struct remote *r);
int latest_remote(struct remote *r);
void init_remote(char *name);
char *ssh_cmd(struct remote *r, char *shell_cmd);
void scp_commit(struct remote *r, int id);

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
  char *name;
  char *sname;
  int vid;
  int tid;
  int bid;
};

struct opt_cmd {
  char *name;
  int (*fnc)(int argc, char **argv);
};


/* commands */

int usage(int argc, char **argv);
int init(int argc, char **argv);
int add(int argc, char **argv);
int unadd(int argc, char **argv);
int rm(int argc, char **argv);
int commit(int argc, char **argv);
int list(int argc, char **argv);
int diff(int argc, char **argv);
int status(int argc, char **argv);
void checkout(int16_t from);
int push(int argc, char **argv);
void rebuild_index();
void read_index();
void read_cindex();

/* internal methods misc */

struct ent *ent_init(char *fname, char *sfname);

/* internal methods indexing */

int next_id(char *path);
int next_commit_id();
int16_t find_prior(int16_t id, struct ent *file);
void push_index(int16_t id, struct ent *file);
int parse_cindex_line(FILE *file, int16_t *id, char *fname);
void generate_cindex(int16_t id);
void clear_fnc(char *name);
void clear_index();
void index_fnc(void (*fnc)(char *name));
void read_fnc(char *name);


/* internal methods - commit flow */

void make_next(int id);
char *san_fname(char *name, bool unsan);


/* internal methods - changeset operations */

struct ct_tree *flist(int id);
struct ct_tree *rmlist(int id);
struct ct_tree *diff_list(int from, int to);
struct ct_tree *range_list(int from, int to);
int climb(int id, char *fname);
void show_flist(int id);
void show(int id);


/* internal methods - remote flow */

void parse_remote(struct remote *r, char *name);
void remote_free(struct remote *r);
int latest_remote(struct remote *r);
void init_remote(char *name);
char *ssh_cmd(struct remote *r, char *shell_cmd);
void scp_commit(struct remote *r, int id);
int run_cmd(int argc, char **argv, struct opt_cmd cmds[]);

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
  char *path;
  char *spath;
  char *hex;
  char *prior;
};

struct commit {
  char *hex;
  char *prior;
  char *message;
  long time;
  struct ct_tree files;
  struct ct_tree tags;
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

struct ent *ent_init(char *path, char *sname);
void ent_free(struct ent *cur);
struct ent *ent_from_line(char *line);
struct ent *ent_to_line(struct ent *file);
char *san_fname(char *path, bool san);
char *gen_path(struct ent *file, bool current);
char *time_hex();
struct ct_tree *cindex_to_tree(char *path);

/* internal methods indexing */

int next_id(char *path);
int next_commit_id();
char *find_prior(int16_t id, struct ent *file);
void push_index(int16_t id, struct ent *file);
int parse_cindex_line(FILE *file, int16_t *id, char *fname);
void generate_cindex(int16_t id);
void clear_fnc(char *name);
void clear_index();
void index_fnc(void (*fnc)(char *name));
void read_fnc(char *name);

/* internal methods - commit flow */

void make_next(int id);


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

/* utils */
char *hexstr(char *bytes, int len);
struct ct_tree *diff_list_staged();
struct ct_tree *cfiles(int id);
void scp_batch(struct remote *r, char *path);
struct ct_tree *ent_tree_init();

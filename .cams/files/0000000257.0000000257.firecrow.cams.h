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
  int16_t id;
  int16_t prior;
  char *hex;
  char *message;
  long time;
  struct ct_tree *files;
  struct ct_tree *rmfiles;
  struct ct_tree *cindex;
  /*struct ct_tree tags;*/
};


/* cams.c */
struct opt_cmd {
  char *name;
  int (*fnc)(int argc, char **argv);
};
int next_commit_id();

/* utils.c */
char *dupstr(char *str);
int flen(char *path);
int fexists(char *path);
bool feq(char *path_a, char *path_b);
char *ct_fread(char *path, int size);
char *hexstr(char *bytes, int len);
int localize_fname(char *fname);
int ct_split(char *str, char c, int limit, char ***ret);
void trimnl(char *str);

/* commit.c */
static size_t HEXLEN = sizeof(long)*4;
int hex(int argc, char **argv);
char *hex_from_id(int id);
void generate_cindex(int16_t id);
void make_next(int id);
char *san_fname(char *path, bool san);
char *time_hex(struct timespec *t);
char *gen_fid(char *path);
struct ct_tree *flist(int id);
bool cfiles_filter(struct ct_key_data *kv, void *data);
struct ct_tree *cfiles(int id);
struct ct_tree *rmlist(int id);
struct ct_tree *cindex_to_tree(int id);
struct commit *commit_init(int id);
int show_commit(int argc, char **argv);
int add(int argc, char **argv);
int unadd(int argc, char **argv);
int rm(int argc, char **argv);
int commit(int argc, char **argv);

/* ent.c */
char *ent_tostr(struct ent *cur);
void ct_free_ent_node(void *key, void *data);
struct ct_tree *ent_tree_init();
char *gen_path(struct ent* cur, bool current);
struct ent *ent_init(char *path, char *spath);
void ent_free(struct ent *cur);
struct ent *ent_from_line(char *line);

/* sync.c */
int push(int argc, char **argv);
void parse_remote(struct remote *r, char *name);
void remote_free(struct remote *r);
int latest_remote(struct remote *r);
void init_remote(char *name);
char *ssh_cmd(struct remote *r, char *shell_cmd);
void scp_batch(struct remote *r, char *path);
void gather_files_for_push(struct ct_tree *tree, int id);

/* list.c */
int list(int argc, char **argv);

/* diff.c */
int diff(int argc, char **argv);
int difftool(int argc, char **argv);

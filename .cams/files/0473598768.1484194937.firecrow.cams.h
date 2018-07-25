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
  char *cid;
  char *prior;
};

struct commit {
  char *cid;
  char *parent;
  char *millis;
  char *epoch;
  char *name;
  char *message;
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
void generate_cindex(char *cid);
char *san_fname(char *path, bool san);
char *gen_cid();
struct ct_tree *flist();
bool cfiles_filter(struct ct_key_data *kv, void *data);
struct ct_tree *cfiles(char *cid);
struct ct_tree *rmlist(char *cid);
struct ct_tree *cindex_to_tree(char *cid);
struct commit *commit_init(char *cid);
int show_commit(int argc, char **argv);
int add(int argc, char **argv);
int unadd(int argc, char **argv);
int rm(int argc, char **argv);
int commit(int argc, char **argv);
char *get_current();
int com_before(struct commit *a, struct commit *b);

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
char *latest_remote(struct remote *r);
void init_remote(char *name);
char *ssh_cmd(struct remote *r, char *shell_cmd);
void scp_batch(struct remote *r, char *path);
void gather_files_for_push(struct ct_tree *tree, char *cid);

/* list.c */
int list(int argc, char **argv);

/* diff.c */
int diff(int argc, char **argv);
int difftool(int argc, char **argv);

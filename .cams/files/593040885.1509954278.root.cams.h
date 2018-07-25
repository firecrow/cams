#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <crowtils.h>
#include <time.h>
#include <sys/timespec.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ftw.h>
#include <crowtree/tree.h>
#include <crycomp/crycomp.h>
#include "crowopt/opt.h"

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

enum REMOTE_PARSE_STATES {
  CT_ROPEN=0,
  CT_RHOST=1,
  CT_RPORT=2,
  CT_RPATH=3,
  CT_RDONE=4
};

struct ent {
  char *path;
  char *spath;
  char *cid;
  char *prior;
  struct timespec time;
};

struct commit {
  char *cid;
  char *parent;
  struct timespec time;
  char *name;
  char *message;
};

#define MAX_FILEREAD = 4096;

enum CT_SUBP_FLAGS {
  CT_USE_STDIN = 1,
  CT_USE_STDOUT = 2,
  CT_USE_STDERR = 4,
  CT_SUBP_ASYNC = 8
};

/* intls.c */
struct intls {
	int (*printf)(const char * format, ...);
	int (*fprintf)(FILE *stream, const char *format, ...);
	struct ct_tree *(*flist)();
	struct ct_tree *(*cfiles)(char *cid);
	struct ct_tree *(*rmlist)(char *cid);
	struct ct_tree *(*slist)();
};

void init_intls(struct intls *intls);

/* cams.c */
struct opt_cmd {
  char *name;
  int (*fnc)(int argc, char **argv, struct intls *intls);
};
int next_commit_id();
int checkout(int argc, char **argv, struct intls *intls);

/* utils.c */
struct ct_subp {
  char *cmd;
  char **argv;
  FILE *stdin;
  FILE *stdout;
  FILE *stderr;
  int pid;
  int ret;
  int ins[2];
  int outs[2];
  int errs[2];
  int flags;
};
struct ct_strbuff {
  char *str;
  size_t len;
  size_t size;
};
char *dupstr(char *str);
char *dupnstr(char *str, int len);
int flen(char *path);
int fexists(char *path);
bool feq(char *path_a, char *path_b);
char *ct_fread(char *path);
int localize_fname(char *fname);
int ct_split(char *str, char c, int limit, char ***ret);
void trimnl(char *str);
int ct_subp(struct ct_subp *subp);
int ct_subpwait(struct ct_subp *subp);
struct ct_strbuff *ct_strbuff_init();
struct ct_strbuff *ct_strbuff_clear(struct ct_strbuff *buff);
void ct_strbuff_free(struct ct_strbuff *buff);
int ct_strbuff_push(struct ct_strbuff *buff, char *str, size_t len);
int ct_strbuff_shift(struct ct_strbuff *buff, size_t len, char **dest);

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
int show_commit(int argc, char **argv, struct intls *intls);
int add(int argc, char **argv, struct intls *intls);
int unadd(int argc, char **argv, struct intls *intls);
int rm(int argc, char **argv, struct intls *intls);
int commit(int argc, char **argv, struct intls *intls);
char *get_current();
int com_before(struct commit *a, struct commit *b);
void register_millis(char *cid);
char *lookup_or_die(char *millis);
void parse_cid(char *cid, struct timespec *time, char **name);

/* ent.c */
char *ent_tostr(struct ent *cur);
void ct_free_ent_node(void *key, void *data);
struct ct_tree *ent_tree_init();
char *gen_path(struct ent* cur, bool current);
struct ent *ent_init(char *path, char *spath);
void ent_free(struct ent *cur);
struct ent *ent_from_line(char *line);

/* sync.c */
int push(int argc, char **argv, struct intls *intls);
void remote_by_name(struct remote *r, char *name);
void remote_free(struct remote *r);
char *latest_remote(struct remote *r);
void init_remote(char *name);
char *ssh_cmd(struct remote *r, char *shell_cmd);
void scp_batch(struct remote *r, char *path);
void gather_files_for_push(struct ct_tree *tree, char *cid);
void parse_remote(struct remote *r, char *uri);
void scp_down(struct remote *r, char *from, char *to,
  void (*reporter)(int count, char *content)
);
int subp_emitter(
  struct ct_subp *subp, 
  void *data,
  void (*emit)(int idx, char *line, void *data),
  void (*done)(struct ct_subp *subp, void *data)
);

/* list.c */
int list(int argc, char **argv, struct intls *intls);
int status(int argc, char **argv, struct intls *intls);

/* diff.c */
int diff(int argc, char **argv, struct intls *intls);
int difftool(int argc, char **argv, struct intls *intls);

/* slist.c */
struct ct_tree *slist();

#ifndef TIME_UTC
  #define TIME_UTC 1
#endif


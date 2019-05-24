#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timespec.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "../crowarg/crowarg.c"
#include "../crowopt/opt.h"
#include "../crowtils/crowtils.c"
#include "../crowtree/tree.c"
#include "../crray/crray.c"
#include "../crowx/crowx.c"


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

struct stbuckets {
  struct ct_tree *staged;
  struct ct_tree *removed;
  struct ct_tree *modified;
  struct ct_tree *untracked;
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


#include "comp/utils.h"

/* commit.c */
void generate_cindex(char *cid);
char *san_fname(char *path, bool san);
char *gen_cid();
struct ct_tree *flist();
int cfiles_filter(struct ct_leaf *kv, void *data);
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
struct stbuckets *gen_stbuckets(char *cid);

/* diff.c */
int diff(int argc, char **argv, struct intls *intls);
int difftool(int argc, char **argv, struct intls *intls);

/* slist.c */
struct ct_tree *slist();

#ifndef TIME_UTC
  #define TIME_UTC 1
#endif


#include "comp/commit.c"
#include "comp/ent.c"
#include "comp/list.c"
#include "comp/slist.c"
#include "comp/utils.c"

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

enum CT_SUBP_FLAGS {
  CT_USE_STDIN = 1,
  CT_USE_STDOUT = 2,
  CT_USE_STDERR = 4,
  CT_SUBP_ASYNC = 8
};

#define CT_FCOMPARE_SIZE 1024
char *dupstr(char *str);
char *dupnstr(char *str, int len);
int flen(char *path);
int fexists(char *path);
bool feq(char *path_a, char *path_b);
char *ct_fread(char *path);
int ct_split(char *_str, char c, struct crray *arr);
void trimnl(char *str);
int ct_subp(struct ct_subp *subp);
int ct_subpwait(struct ct_subp *subp);
struct ct_strbuff *ct_strbuff_init();
struct ct_strbuff *ct_strbuff_clear(struct ct_strbuff *buff);
void ct_strbuff_free(struct ct_strbuff *buff);
int ct_strbuff_push(struct ct_strbuff *buff, char *str, size_t len);
int ct_strbuff_shift(struct ct_strbuff *buff, size_t len, char **dest);
int ct_fcompare(char *apath, int alen, char *bpath, int blen);



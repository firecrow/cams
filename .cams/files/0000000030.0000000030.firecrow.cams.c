#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>

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

void make_next(int id);
void parse_remote(struct remote *r, char *name);
void remote_free(struct remote *r);
char *ssh_cmd(struct remote *r, char *shell_cmd);
void show(int id);

char *substr(char *src, size_t s, size_t e){
  size_t n = e-s;
  char *str = malloc(sizeof(char)*(n+1));
  if(!str);
  memcpy(str, src+s, n);
  str[n] = '\0';
  return str;
}

int next_id(){
  int l;
  char b[1024];
  FILE *idf;
  idf = fopen(".cams/next/id", "r");
  if(!idf){
    fprintf(stderr, "unable to open id file\n");
    exit(123);
  }
  l = fread(b, 1, 1023, idf);
  b[l] = '\0';
  return atoi(b);
}

void *commit(char *msg){
  int id;
  char b[1024];
  FILE *msgf;

  msgf = fopen(".cams/next/message", "w+");
  if(!msgf){
    fprintf(stderr, "unable to open message file\n");
    exit(123);
  }
  fprintf(msgf, "%s\n", msg);

  id = next_id();
  snprintf(b, 1024, ".cams/%d", id);
  rename(".cams/next", b);
  make_next(id+1);
}

void checkout(char *revision){
  /* request commit message */
}

int climb(int id, char *fname){
  /* climb back to the last change of a file */
  char path[1024];
  while(--id > 0){
    if(snprintf(path, 1024, ".cams/%d/files/%s", id, fname) > 1023 ){
      fprintf(stderr, "file path in climb too long");
      exit(123);
    }
    if(access(path, F_OK) != -1){
      return id;
    }
  }
  return -1;
}

int contained(int id, char *fname){
  char path[1024];
  if(snprintf(path, 1024, ".cams/%d/files/%s", id, fname) > 1023 ){
    fprintf(stderr, "file path in commit too long");
    exit(123);
  }
}

struct ent {
  char name[1024];
  int tid;
  int bid;
  struct ent *next;
};

struct ent *flist(int id){
  char b[1024];
  DIR *d;
  struct dirent *dp;
  bool f;
  struct ent *head = NULL;
  struct ent *prev = NULL;
  struct ent *cur = NULL;
  if(snprintf(b, 1024, ".cams/%d/files", id) > 1023){
    fprintf(stderr, "file dir list too long");
    exit(123);
  }
  if((d = opendir(b)) == NULL){
    fprintf(stderr, "unable to open dir: %s", d);
    exit(123);
  }
  while((dp = readdir(d)) != NULL){
    if(!strncmp(".", dp->d_name, 1) || !strncmp("..", dp->d_name, 2))
      continue;
    if((cur = malloc(sizeof(struct ent))) == NULL){
      fprintf(stderr, "cannot allocate new ent");
    }
    if(snprintf(cur->name, 1024, dp->d_name) > 1023){
      fprintf(stderr, "unable to copy string to ent string too long");
    }
    cur->next = NULL;
    cur->tid = 0;
    cur->bid = 0;
    if(!prev){
      head = prev = cur;
    }else{
      prev->next = cur;
      prev = cur;
    }
  }
  return head;
}

void show_flist(int id){
  struct ent *next = flist(id); 
  struct ent *prev;
  bool f = true;
  while(next){
    if(!f)
      printf(", ");
    f = false;
    printf("%s", next->name);
    prev = next;
    next = prev->next;
    free(prev);
  }
}

void find_bottom(struct ent *files){
  while(files){
    /* find file in revision or continie
     * repeat until all revisions
     * or until all files have revisions
     */
    files = files->next;
  }
}

struct ent *range_list(int from, int to){
  struct ent *all = NULL;
  struct ent *allp = NULL;
  struct ent *fl = NULL;
  struct ent *found = NULL;
  int open = 0;
  bool below = false;
  while(true){
    if(to < from){
      below = true;
    }
    fl = flist(to);
    while(fl){
      found = NULL;
      allp = all;
      while(allp){
        if(!strcmp(allp->name, fl->name)){
          found = allp;
          if(found->bid == 0)
            open--;
          if(!below || found->bid == 0)
            found->bid = to;
          break;
        }
        allp = allp->next;
      }
      if(!found){
        found = fl;
        found->tid = to;
        fl = fl->next;
        found->next = all;
        all = found;
        open++;
      }else{
        fl = fl->next;
      }
    }
    if(--to < 1 || (!open && to < from)){
      break;
    }
  }
  return all;
}

struct ent *diff_list(int from, int to){
  struct ent *files = range_list(from, to); 
  while(files){
    printf("%s %d..%d\n", files->name, files->bid, files->tid);
    files = files->next;
  }
}

void diff(int from, int to){
  int status;
  char cmd[1024];
  struct ent *f = diff_list(from, to);
  while(f){ 
    if(snprintf(cmd, 1024, "diff .cams/%d/files/%s .cams/%d/files/%s", f->bid, f->name, f->tid, f->name) > 1023){
      fprintf(stderr, "diff cmd too long\n");
      exit(123);
    };
    printf("[%s] %d..%d\n", f->name, f->bid, f->tid);
    fflush(stdout);
    system(cmd);
    wait(&status);
    f = f->next;
  }
  /* generate patches and diffs for output and counts */
}

void make_next(int id){
  if(id >= 1024){
    fprintf(stderr, "id too large, recompile with larger buffer for id in commit() function\n");
    exit(123);
  }
  system("mkdir .cams/next");
  system("mkdir .cams/next/files");
  FILE *idf = fopen(".cams/next/id", "w+");
  if(!idf){
    fprintf(stderr, "unable to open id file");
    exit(123);
  }
  fprintf(idf, "%d", id);
  fclose(idf);
}

void init(){
  system("mkdir .cams");  
  make_next(1);
}

char *sanatize(char *fname){
  char c;
  int l = strlen(fname);
  int i = 0;
  char *s = (char *) malloc(sizeof(char)*(l+1));
  if(!s){
    fprintf(stderr, "error allocating sanatized string");
    exit(123);
  }
  while((c=fname[i]) != '\0'){
    s[i++] = (c == '/') ? '+' : c;
  }
  return s;
}

void add(int argc, char **list){
  int i;
  char *fname;
  char *sfname;
  char cmd[1024];
  for(i = 0; i<argc; i++){
    fname = list[i];
    sfname = sanatize(fname);
    if(fname[0] == '.' && fname[1] == '.'){
      fprintf(stderr, "error cannot copy files outside root directory: %s\n", fname);
      exit(1);
    }
    snprintf(cmd, 1024, "cp -v %s .cams/next/files/%s", fname, sfname);
    system(cmd);
  }
}

void parse_remote(struct remote *r, char *name){
  /*open remotes file*/
  char b[1024];
  char *cur_remote;
  char *value;
  size_t l;
  int field = 0;
  int s_pos = 0;
  int e_pos = 0;
  FILE * rfile = fopen(".cams/remotes", "r");
  if(!rfile){
    /* bad things have happened */
  }
  while(l = fgets(b, 1023, rfile) != 0){
    if(b[0] == '#')
      continue;
    /* validate that the line is the expected name
     * if name is specified
     * if name is not specified, use the first line found
     */
    if(name != NULL && !strncmp(name, b, strlen(name)))
      continue;
    while(field < 5){
      while(
        b[e_pos] != ' ' 
        && b[e_pos] != '\0' 
        && b[e_pos] != '\n'
      ){ e_pos++; };
      value = substr(b, s_pos, e_pos);
      switch(field){
        case 0:
          r->name = value;
          break;
        case 1:
           r->user = value;
           break;
        case 2:
          r->host = value;
          break;
        case 3:
          r->port = atoi(value);
          break;
        case 4:
          r->path = value;
          break;
      }
      s_pos = ++e_pos;
      field++;
    }
    if(field < 5){
      fprintf(stderr, "Oops misconfigured remote: %s", b);
      exit(123);
    }else{
      break;
    }
  }
}

void scp_commit(struct remote *r, int id){
  char cmd[1024];
  if(snprintf(cmd, 1024, "scp -r -P %d .cams/%d %s@%s:%s", r->port, id, r->user, r->host, r->path) > 1023){
    fprintf(stderr, "error formulating scp comand arguments too long");
    exit(123);
  }
  if(system(cmd)){
    fprintf(stderr, "error with scp\n");
    exit(123);
  }
}

char *ssh_cmd(struct remote *r, char *shell_cmd){
  char c[] = "c";
  size_t l = snprintf(c, 1, "ssh -p %d %s@%s %s", r->port, r->user, r->host, shell_cmd);
  char *cmd = malloc(sizeof(char)*(l+1));
  if(!malloc);
  snprintf(cmd, l+1, "ssh -p %d %s@%s %s", r->port, r->user, r->host, shell_cmd);
  return cmd;
}

void init_remote(char *name){
  struct remote r;
  parse_remote(&r, name);
  /* ssh to see if the directory exists */
  /* create it if it doesn't*/
}

int latest_remote(struct remote *r){
  char cmd[1024];
  char b[1024];
  FILE *out;
  int lt;
  if(snprintf(cmd, 1024, "ls %s/ | sort -nr | head -n 1", r->path) > 1023){
    fprintf(stderr, "cmd too long\n");
    exit(123);
  }
  out = popen(ssh_cmd(r, cmd), "r");
  fgets(b, 1024, out);
  lt = atoi(b);
  return lt;
}

void push(char *name){
  struct remote r;
  int exists;
  int nid;
  int i;
  char cmd[1024];
  int rlatest;
  parse_remote(&r, name);
  if(snprintf(cmd, 1024, "ls %s 1>/dev/null", r.path) > 1023){
    fprintf(stderr, "path to long\n");
    exit(123);
  }
  if(system(ssh_cmd(&r, cmd))){
    fprintf(stderr, "directory not found: %s\n", r.path);
  }
  rlatest = latest_remote(&r);
  nid = next_id();
  if(nid < rlatest){
    printf("%s is ahead of local at %d\n", name, rlatest);
  }else if(nid == rlatest){
    printf("%s is up to date at %d\n", name, nid);
  }else{
    i = rlatest;
    while(++i < nid){
      scp_commit(&r, i);
    }
    printf("pushed %d..%d to %s\n", rlatest+1, nid-1, name);
  }
}

void show(int id){
  int l;
  char b[1024];
  FILE *msg;
  if(!id){
   id = next_id()-1;
  }
  if(snprintf(b, 1024, ".cams/%d/message", id) > 1023){
    fprintf(stderr, "error show path too long");
    exit(123);
  }
  if(!(msg = fopen(b, "r"))){
    fprintf(stderr, "error opening file");
    exit(123);
  }
  printf("%d: ", id);
  while((l = fread(b, 1, 1024, msg)) != 0){
    if(b[l-1] == '\n') l--;
    fwrite(b, 1, l, stdout);
  }
  printf(" (");
  show_flist(id);
  printf(")\n");
}

void list(){
  int id = next_id();
  while(--id > 0){
    show(id);
  }
}

void reset(){
  printf("Resetting files\n");
  system("rm -v .cams/next/files/*");
}

void usage(char *command){
  if(!strncmp("./", command, 2))
    command +=2;

  printf("%s\n    help|init|commit|list|checkout|diff|push|show [args...]\n", command);
}

int main(int argc, char **argv){
  if(argc < 2){
    usage(argv[0]);
    return 1;
  }
  /* todo, make a switch statement here */
  if(!strncmp("help", argv[1], 1024)){
    usage(argv[0]);
  }
  if(!strncmp("init", argv[1], 1024)){
    init();
  }
  if(!strncmp("add", argv[1], 1024)){
    add(argc-2, argv+2);
  }
  if(!strncmp("reset", argv[1], 1024)){
    reset();
  }
  if(!strncmp("commit", argv[1], 1024)){
    commit((argc == 3) ? argv[2] : NULL);
  }
  if(!strncmp("list", argv[1],  1024)){
    list();
  }
  if(!strncmp("push", argv[1],  1024)){
    push((argc == 3) ? argv[2] : NULL); 
  }
  if(!strncmp("checkout", argv[1],  1024)){
    checkout(argv[2]);
  }
  if(!strncmp("show", argv[1],  1024)){
    show(0);
  }
  if(!strncmp("diff", argv[1],  1024)){
    if(argc != 4){
      printf("too few arguments to diff expected\n   cams diff from_id to_id\n");
      exit(1);
    } 
    diff(atoi(argv[2]), atoi(argv[3]));
  }
  return 0;
}

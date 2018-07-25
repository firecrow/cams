#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include "utils.h"
#include "cams.h"


/* commands */

int usage(int argc, char **argv){
  char *command = argv[0];
  int l = strlen(command);
  while(*command == '.' || *command == '/'){
    command++;
  }

  printf("%s\n    help|init|commit|list|checkout|diff|push|show|reindex|index [args...]\n", command);
  return 1;
}

int init(int argc, char **argv){
  system("mkdir .cams");  
  system("mkdir .cams/index");  
  make_next(1);
  return 0;
}

int add(int argc, char **argv){
  int i;
  char *fname;
  char *sfname;
  char cmd[1024];
  int len = argc-2;
  char **list = argv+2;
  int id = next_id();
  for(i = 0; i<len; i++){
    fname = list[i];
    sfname = sanatize(fname);
    if(fname[0] == '.' && fname[1] == '.'){
      fprintf(stderr, "error cannot copy files outside root directory: %s\n", fname);
      return 123;
    }
    snprintf(cmd, 1024, "cp -v %s .cams/%d/files/%s", fname, id, sfname);
    system(cmd);
  }
  return 0;
}

void commit(char *msg){
  int id = next_id();
  char *b;
  FILE *msgf;
  b = dk_fmtmem(".cams/%d/message", id);
  msgf = dk_open(b, "w+");
  struct ent *files;
  fprintf(msgf, "%s\n", msg);
  fclose(msgf);
  files = flist(id);
  while(files){
    push_index(id, files->name); 
    files = files->next;
  }
  generate_cindex(id);
  make_next(id+1);
  dk_free(b);
}

void list(int start, int qty){
  int id;
  int count = 0;
  if(start == 0){
    id = next_id()-1;
  }else{
    id = start;
  }
  while(id > 0){
    show(id--);
    if(qty && ++count == qty){
      break;
    };
  }
}

void diff(int from, int to){
  int status;
  char *fname;
  char *cmd;
  struct ent *f = diff_list(from, to);
  while(f){ 
    if(f->bid > 0){
      fname = dk_fmtmem(".cams/%d/files/%s", f->bid, f->name);
    }else{
      fname = dk_fmtmem("/dev/null");
    }
    cmd = dk_fmtmem("diff %s .cams/%d/files/%s", fname, f->tid, f->name);
    printf("[%s] %d..%d\n", f->name, f->bid, f->tid);
    fflush(stdout);
    system(cmd);
    f = f->next;
    dk_free(fname);
    dk_free(cmd);
  }
}

void checkout(int16_t from){
  struct ent *f = diff_list(from, next_id());
  char *cmd;
  while(f){
    printf("%s: %d\n", f->name, f->bid);
    cmd = dk_fmtmem("cp -v .cams/%d/files/%s %s", f->bid, f->name, f->name);
    system(cmd);
    f = f->next;
    dk_free(cmd);
  }
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
    /* todo handle cases where nothing was pushed */
    while(++i < nid){
      scp_commit(&r, i);
    }
    printf("pushed %d..%d to %s\n", rlatest+1, nid-1, r.name);
  }
}

void rebuild_index(){
  clear_index();
  int16_t next = next_id();
  int16_t id = 0;
  struct ent *files;
  while(++id <= next){
    files = flist(id);
    while(files){
      push_index(id, files->name);
      files = files->next;
    }
    generate_cindex(id);
  }
}

void read_index(){
  index_fnc(read_fnc);
}

void read_cindex(){
  int16_t top = next_id();
  int16_t id = 0;
  FILE *cindex;
  char fname_buff[1024];
  uint16_t latest_id;
  while(++id < top){
    show(id);
  	cindex = dk_open(dk_fmtmem(".cams/%d/cindex", id), "r");
    while(true){
      if(parse_cindex_line(cindex, &latest_id, fname_buff)){
        break;
      }
      if(id == latest_id){
        printf("   >");
      }else{
        printf("    ");
      }
      printf("%d: %s\n", latest_id, fname_buff);
    }
    fclose(cindex);
  }
}


/* internal method indexing */

int next_id(){
  int l;
  char b[1024];
  FILE *idf;
  idf = dk_open(".cams/next", "r");
  l = fread(b, 1, 1023, idf);
  b[l] = '\0';
  fclose(idf);
  return atoi(b);
}

int16_t find_prior(int16_t id, char *fname){
  char *b;
  FILE *file;
  int16_t x = 0;
  int16_t p = 0;
  b = dk_fmtmem(".cams/index/%s", fname);
  file = fopen(b, "r");
  /* if not file exists, no index exists */
  if(!file)
    return 0;
  while(fread(&x, 1, sizeof(int16_t), file) != 0){
    if(x >= id){
      return p;
    }
    p = x;
  }
  dk_free(b);
  fclose(file);
  return p;
}

void push_index(int16_t id, char *fname){
  char *b;
  FILE *file;
  b = dk_fmtmem(".cams/index/%s", fname);
  file = dk_open(b, "a");
  fseek(file, 0, SEEK_END);
  fwrite(&id, 1, sizeof(int16_t), file);
  fclose(file);
  dk_free(b);
}

int parse_cindex_line(FILE *file, int16_t *id, char *fname){
  int l;
  if(!fread(id, 1, sizeof(int16_t), file)){
    return 1;
  }
  fgets(fname, 1023, file);
  l = strlen(fname);
  if(fname[l-1] != '\n'){
    fprintf(stderr, "file name too long for reading cindex\n");
    return 1;
  }else{
    fname[l-1] = '\0';
  }
  return 0;
}

/**
 * generate a cindex from the files in a commit
 * and then the previos positions of files from
 * the previous commit
 */
void generate_cindex(int16_t id){
  FILE *cindex;
  FILE *prev_cindex;
  int l;
  char *prev_fname;
  char prev_buff[1024];
  int16_t prev_id;
  char *cont;
  struct ent *files = flist(id);
  struct ent *filesp = files;
  char *fname = dk_fmtmem(".cams/%d/cindex", id);
  cindex = dk_open(fname, "w+");
  /* write files that are in this commit */
  while(filesp){
		fwrite(&id, sizeof(int16_t), 1, cindex);
    fwrite(filesp->name, sizeof(char), strlen(filesp->name), cindex);
    fwrite("\n", sizeof(char), 1, cindex);
    filesp = filesp->next;
  }
  /* write files from the previous commit */
  if(id > 1){
  	prev_fname = dk_fmtmem(".cams/%d/cindex", id-1);
  	prev_cindex = dk_open(prev_fname, "r");
    while(true){
      if(parse_cindex_line(prev_cindex, &prev_id, prev_buff)){
        break;
      }
      filesp = files;
      while(filesp){
        if(!strcmp(prev_buff, filesp->name))
          break;
        filesp = filesp->next;
      }
      if(filesp == NULL){
        fwrite(&prev_id, sizeof(int16_t), 1, cindex);
        fwrite(prev_buff, 1, strlen(prev_buff), cindex);
        fwrite("\n", sizeof(char), 1, cindex);
      }
    }
    fclose(prev_cindex);
    dk_free(prev_fname);
  }
  fclose(cindex);
  dk_free(fname);
}

void clear_fnc(char *name){
  remove(name);
}

void clear_index(){
  index_fnc(clear_fnc);
}

void index_fnc(void (*fnc)(char *name)){
  DIR *d;
  struct dirent *dp;
  char *b;
  if((d = opendir(".cams/index")) == NULL){
    fprintf(stderr, "unable to open dir: .cams/index");
    exit(123);
  }
  while((dp = readdir(d)) != NULL){
    b = dk_fmtmem(".cams/index/%s", dp->d_name);
    if(!strncmp(".", dp->d_name, 1) || 
       !strncmp("..", dp->d_name, 2))
      continue;
    fnc(b);
    dk_free(b);
  }
}

void read_fnc(char *name){
  FILE *file = fopen(name, "r");
  /* skip files that dont exist */
  if(!file)
    return;
  int16_t x;
  bool first = true;
  printf("%s\n", name);
  while(fread(&x, 1, sizeof(int16_t), file) != 0){
    if(!first){
      printf(":");
    }
    first = false;
    printf("%d", x);
  }
  printf("\n");
  fclose(file);
}


/* internal methods - commit flow */

void make_next(int id){
  char *b;
  FILE *idf;

  b = dk_fmtmem("mkdir .cams/%d", id);
  system(b);
  dk_free(b);

  b = dk_fmtmem("mkdir .cams/%d/files", id);
  system(b);
  dk_free(b);

  idf = dk_open(".cams/next", "w+");
  fprintf(idf, "%d", id);
  fclose(idf);
}

char *sanatize(char *fname){
  char c;
  int l = strlen(fname);
  int i = 0;
  char *s = (char *) dk_malloc(sizeof(char)*(l+1));
  if(!s){
    fprintf(stderr, "error allocating sanatized string");
    exit(123);
  }
  while((c=fname[i]) != '\0'){
    s[i++] = (c == '/') ? '+' : c;
  }
  s[i] = '\0';
  return s;
}


/* internal methods - changeset operations */

struct ent *flist(int id){
  char *b;
  DIR *d;
  struct dirent *dp;
  bool f;
  struct ent *head = NULL;
  struct ent *prev = NULL;
  struct ent *cur = NULL;
  b = dk_fmtmem(".cams/%d/files", id);
  if((d = opendir(b)) == NULL){
    fprintf(stderr, "unable to open dir: %s", d);
    exit(123);
  }
  while((dp = readdir(d)) != NULL){
    if(!strncmp(".", dp->d_name, 1) || !strncmp("..", dp->d_name, 2))
      continue;
    if((cur = dk_malloc(sizeof(struct ent))) == NULL){
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

struct ent *diff_list(int from, int to){
  struct ent *files = range_list(from, to);
  struct ent *p = files;
  while(p){
    /* plus one because we are seeking the prior commit */
    p->bid = find_prior(from+1, p->name);
    p = p->next;
  }
  return files;
}

struct ent *range_list(int from, int to){
  struct ent *all = NULL;
  struct ent *allp = NULL;
  struct ent *fl = NULL;
  struct ent *cur = NULL;
  int open = 0;
  bool below = false;
  bool fisnew = true;
  while(to > from){
    fl = flist(to);
    while(fl){
      allp = all;
      while(allp){
        if(!strcmp(allp->name, fl->name)){
          break;
        }
        allp = allp->next;
      }
      /* null means the linked list concluded to the end */
      if(allp == NULL){
        cur = fl;
        fl = fl->next;
        cur->tid = to;
        cur->next = all;
        all = cur;
      }else{
        fl = fl->next;
      }
    }
    to--;
  }
  return all;
}

int climb(int id, char *fname){
  /* climb back to the last change of a file */
  char *path;
  while(--id > 0){
    path = dk_fmtmem(".cams/%d/files/%s", id, fname);
    if(access(path, F_OK) != -1){
      return id;
    }
    dk_free(path);
  }
  return -1;
}

void print_flist(FILE *out, struct ent *file){
  while(file){
    fwrite(file->name, 1, strlen(file->name), out);
    file = file->next;
    if(file){
      fwrite(", ",1 ,2, out);
    }
  }
  fwrite("\n",1 ,1, out);
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


/* internal methods - remote flow */

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
    if(name != NULL && strncmp(name, b, strlen(name)))
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
      return;
    }
  }
  fprintf(stderr, "error: no remote found for: %s\n", name);
  exit(123);
}

void remote_free(struct remote *r){

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

void init_remote(char *name){
  struct remote r;
  parse_remote(&r, name);
  /* ssh to see if the directory exists */
  /* create it if it doesn't*/
}

char *ssh_cmd(struct remote *r, char *shell_cmd){
  char c[] = "c";
  size_t l = snprintf(c, 1, "ssh -p %d %s@%s %s", r->port, r->user, r->host, shell_cmd);
  char *cmd = malloc(sizeof(char)*(l+1));
  if(!malloc);
  snprintf(cmd, l+1, "ssh -p %d %s@%s %s", r->port, r->user, r->host, shell_cmd);
  return cmd;
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

int run_cmd(int argc, char **argv, struct opt_cmd cmds[]){
  int i = 0;
  while(cmds[i].name != NULL){
    if(!strncmp(cmds[i].name, argv[1], strlen(cmds[i].name))){
      return cmds[i].fnc(argc, argv);
    }
    i++;
  }
  return -1;
}


int main(int argc, char **argv){
  int ret;
  struct opt_cmd help_cmd = {"help",  usage};
  struct opt_cmd init_cmd = {"init",  init};
  struct opt_cmd add_cmd = {"add",  add};

  struct opt_cmd cmds[] = {
    help_cmd, 
    init_cmd, 
    add_cmd,
    NULL
  };
  ret = run_cmd(argc, argv, cmds);
  if(ret != -1){
    return ret;
  }

  int16_t to;
  int16_t from;
  int count;
  if(argc < 2){
    usage(argc, argv);
    return 1;
  }else if(!strncmp("commit", argv[1], 1024)){
    commit((argc == 3) ? argv[2] : NULL);
  }else if(!strncmp("list", argv[1],  1024)){
    list(
      (argc >= 3) ? atoi(argv[2]) : 0,
      (argc == 4) ? atoi(argv[3]) : 0
    ); 
  }else if(!strncmp("push", argv[1],  1024)){
    push((argc == 3) ? argv[2] : NULL); 
  }else if(!strncmp("checkout", argv[1],  1024)){
    checkout(atoi(argv[2]));
  }else if(!strncmp("show", argv[1],  1024)){
    show(0);
  }else if(!strncmp("reindex", argv[1],  1024)){
    rebuild_index();
  }else if(!strncmp("index", argv[1],  1024)){
    read_index();
  }else if(!strncmp("cindex", argv[1],  1024)){
    read_cindex();
  }else if(!strncmp("diff", argv[1],  1024)){
    if(argc == 2){
      to = next_id();
      from = to-1;
    }else if(argc == 4){
      from = atoi(argv[2]);
      to = atoi(argv[3]);
    }else{
      printf("wrong number of arguments to diff expected 0 or 2\n   cams diff [from_id to_id]\n");
      exit(1);
    } 
    diff(from, to);
  }else{
    fprintf(stderr, "command not found\n");
    return 1;
  }
  /*
  if(count = dk_count()){
    fprintf(stderr, "error memory count not zero: %d\n", count);
    return 123;
  }
  */
  return 0;
}

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
#include <crowtils.h>
#include "crowtree/tree.h"
#include "cams.h"


static size_t HEXLEN = sizeof(long)*4;

char *gen_path(struct ent* cur, bool current){
  return dk_fmtmem("%s.%s", (current ? cur->hex : cur->prior), cur->spath);
}

int flen(char *path){
  FILE *file = dk_open(path, "r");
  fseek(file, 0, SEEK_END);
  int len = ftell(file);
  fclose(file);
}

int fexists(char *path){
  return (access(path, F_OK) != -1);
}

bool feq(char *path_a, char *path_b){
  char buff_a[1025];
  char buff_b[1025];
  FILE *a = dk_open(path_a, "r");
  FILE *b = dk_open(path_b, "r");
  fseek(a, 0, SEEK_END);
  fseek(a, 0, SEEK_END);
  if(ftell(a) != ftell(b)){
    return false;
  }
  rewind(a);
  rewind(b);
  while(fread(buff_a, 1, 1024, a) != 0){
    fread(buff_b, 1, 1024, b);
    if(strncmp(buff_a, buff_b, 1024)){
      return false;
    }
  }
  return true;
}

char *hex_from_id(int id){
  char buff[1024];
  FILE *idf;
  char *hex_path = dk_fmtmem(".cams/%d/hex", id);
  char *hex;
  if(fexists(hex_path)){
    idf = dk_open(hex_path, "r");
    int l = fread(buff, 1, 1023, idf);
    buff[l] = '\0';
    fclose(idf);
    dk_free(hex_path);
    hex = dk_fmtmem("%s", buff);
  }else{
    char *hex = dk_fmtmem("%016d", 0);
  }
  return hex; 
}


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
  system("mkdir .cams/files");  
  make_next(1);
  return 0;
}

int add(int argc, char **argv){
  int i;
  char *fname;
  char *sfname;
  char cmd[1024];
  struct ent *file;
  int len = argc-2;
  char **list = argv+2;
  int id = next_commit_id();
  for(i = 0; i<len; i++){
    file = ent_init(dk_fmtmem(list[i]), NULL);
    if(snprintf(cmd, 1024, "cp -v %s .cams/%d/files/%s", file->path, id, file->spath) > 1023){
      fprintf(stderr, "file too long for buffer\n");
    }
    system(cmd); 
  }
  return 0;
}

int unadd(int argc, char **argv){
  int16_t id;
  char *fpath;
  struct ent *file;
  id = next_commit_id();
  argc -= 2;
  argv += 2;
  while(--argc >= 0){
    file = ent_init(dk_fmtmem(argv[argc]), NULL);
    char *fpath = dk_fmtmem(".cams/%d/files/%s", id, file->spath);
    printf("%s\n", fpath);
    unlink(fpath);
  }
  return 0;
}


int rm(int argc, char **argv){
  int16_t id;
  char *fpath;
  char *fname;
  struct ent *file;
  id = next_commit_id();
  char *b = dk_fmtmem(".cams/%d/remove", id);
  FILE *rmfile = dk_open(b, "a");
  fseek(rmfile, 0, SEEK_END);
  argc -= 2;
  argv += 2;
  while(--argc >= 0){
    file = ent_init(dk_fmtmem(argv[argc]), NULL);
    fwrite(file->path, 1, strlen(file->path), rmfile);
    fwrite("\n", 1, 1, rmfile);
    unlink(file->path);
  }
  return 0;
}

int commit(int argc, char **argv){
  char *fname;
  char *filedir;
  FILE *propf;
  int id = next_commit_id();

  char *msg = (argc == 3) ? argv[2] : NULL;
  fname = dk_fmtmem(".cams/%d/message", id);
  propf = dk_open(fname, "w+");
  fprintf(propf, "%s\n", msg);
  fclose(propf);
  dk_free(fname);

  struct timespec t;
  timespec_get(&t, TIME_UTC);
  fname = dk_fmtmem(".cams/%d/time", id);
  propf = dk_open(fname, "w+");
  fprintf(propf, "%d", t.tv_sec);
  fclose(propf);
  dk_free(fname);

  char *hex = time_hex(&t);
  fname = dk_fmtmem(".cams/%d/hex", id);
  propf = dk_open(fname, "w+");
  fprintf(propf, "%s", hex);
  fclose(propf);
  dk_free(fname);

  generate_cindex(id);

  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *files = flist(id);
  char *cmd;
  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
    struct ent *file = (struct ent*)kv.data;
    file->hex = hex;
    cmd = dk_fmtmem("mv .cams/%d/files/%s .cams/files/%s", id, file->spath, gen_path(file, true));
    if(system(cmd)){
      fprintf(stderr, "error moving file: %s\n", file->path);
    }
    dk_free(cmd);
  }

  filedir = dk_fmtmem(".cams/%d/files", id);
  remove(filedir);
  dk_free(filedir);

  printf("%s commit created\n", hex);

  make_next(id+1);

  return 0;
}

int list(int argc, char **argv){
  int id;
  int count = 0;
  int start = (argc >= 3) ? atoi(argv[2]) : 0;
  int qty = (argc == 4) ? atoi(argv[3]) : 0;
  if(start == 0){
    id = next_commit_id()-1;
  }else{
    id = start;
  }
  while(id > 0){
    show(id--);
    if(qty && ++count == qty){
      break;
    };
  }
  return 0;
}

int diff(int argc, char **argv){
  int from;
  int to;
  char *cmd;
  struct ct_tree *files;
  struct ct_key_data kv = {NULL, NULL};
  struct ent *file;
  bool staged = false;
  if(argc == 2){
    files = diff_list_staged();
    staged = true;
  }else if(argc == 4){
    from = atoi(argv[2]);
    to = atoi(argv[3]);
    files = diff_list(from, to);
  }else{
    printf("wrong number of arguments to diff expected 0 or 2\n   cams diff [from_id to_id]\n");
    return 1;
  } 

  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){ 
    file = (struct ent *)kv.data; 
    char *from_path;
    char *to_path;

    if(staged){
      to_path = dk_fmtmem(".cams/%d/files/%s", next_commit_id(), file->spath);
    }else if(file->hex){
      to_path = dk_fmtmem(".cams/files/%s", gen_path(file, true));
    }else{
      to_path = dk_fmtmem("/dev/null");
    }
    if(file->prior){
      from_path = dk_fmtmem(".cams/files/%s", gen_path(file, false));
    }else{
      from_path = dk_fmtmem("/dev/null");
    }

    cmd = dk_fmtmem("diff %s %s", from_path, to_path);
    printf("[%s]\n", file->path);
    fflush(stdout);
    system(cmd);
    printf("\n");

    dk_free(to_path);
    dk_free(from_path);
    dk_free(cmd);
  }
  return 0;
}

void checkout(int16_t from){
}

int push(int argc, char **argv){
  struct remote r;
  int exists;
  int nid;
  int i;
  char cmd[1024];
  int rlatest;
  r.name = (argc == 3) ? argv[2] : NULL;
  parse_remote(&r, r.name);
  if(snprintf(cmd, 1024, "ls %s 1>/dev/null", r.path) > 1023){
    fprintf(stderr, "path to long\n");
    exit(123);
  }
  if(system(ssh_cmd(&r, cmd))){
    fprintf(stderr, "directory not found: %s\n", r.path);
  }
  rlatest = latest_remote(&r);
  nid = next_commit_id();
  if(nid < rlatest){
    printf("%s is ahead of local at %d\n", r.name, rlatest);
  }else if(nid == rlatest){
    printf("%s is up to date at %d\n", r.name, nid);
  }else{
    i = rlatest;
    /* todo handle cases where nothing was pushed */
    while(++i < nid){
      scp_commit(&r, i);
    }
    printf("pushed %d..%d to %s\n", rlatest+1, nid-1, r.name);
  }
}

void read_cindex(){
  int16_t top = next_commit_id();
  int16_t id = 0;
  char *path;
  struct ct_tree *files;
  struct ct_key_data kv = {NULL, NULL};
  struct ent *cur;
  char fname_buff[1024];
  uint16_t latest_id;
  while(++id < top){
    kv.key = NULL;
    show(id);
  	path = dk_fmtmem(".cams/%d/cindex", id);
    files = cindex_to_tree(path);
    dk_free(path);
    while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
      if(id == latest_id){
        printf("   >");
      }else{
        printf("    ");
      }
      cur = (struct ent *)kv.data;
      printf("%s: %s\n", cur->hex, cur->path);
    }
  }
}

/* internal method indexing */

int next_id(char *path){
  char b[1024];
  FILE *idf;
  char *next_path = dk_fmtmem("%s/next", path);
  idf = dk_open(next_path, "r");
  int l = fread(b, 1, 1023, idf);
  fclose(idf);
  dk_free(next_path);
  return atoi(b);
}

int next_commit_id(){
  return next_id(".cams");
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
  struct ct_tree *ctree;
  char *hex = hex_from_id(id);
  struct ent *cur;
  if(id > 1){
    char *prior = dk_fmtmem(".cams/%d/cindex", id-1);
    ctree = cindex_to_tree(prior);
    dk_free(prior);
  }else{
    ctree = ct_tree_alpha_init();
  }

  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *files = flist(id);
  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
    cur = (struct ent *)kv.data;
    cur->hex = hex;
    ct_tree_set(ctree, &kv); 
  }

  kv.key = NULL;
  struct ct_tree *rmfiles = rmlist(id);
  while(ct_tree_next(rmfiles, &kv) != CT_NOT_FOUND){
    ct_tree_unset(ctree, &kv);
  }

  char *cindex_path = dk_fmtmem(".cams/%d/cindex", id);
  FILE *cindexf = dk_open(cindex_path, "w+");
  dk_free(cindex_path);

  kv.key = NULL;
  while(ct_tree_next(ctree, &kv) != CT_NOT_FOUND){
    fprintf(cindexf, "%s%s\n", ((struct ent *)kv.data)->hex, kv.key);
  }
  fclose(cindexf);
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

int localize_fname(char *fname){
  int count = 0;
  char *p = fname;
  while(*fname != '\0'){
    if(*fname == '.' && *fname+1 == '.'){
      fname += 2;
    }
    *p++ = *fname++;
  }
  *fname = '\0';
  return count;
}

struct ent *ent_init(char *path, char *spath){
  struct ent *cur = dk_malloc(sizeof(struct ent));
  if(!path){
    path = san_fname(spath, true);
  }
  if(!spath){
    spath = san_fname(path, true);
  }
  cur->path = path;
  cur->spath = spath;
  cur->hex = NULL;
  return cur;
}

struct ent *ent_from_line(char *line){
  char *hex = dk_malloc(HEXLEN+1);
  memcpy(hex, line, HEXLEN); 
  hex[HEXLEN] = '\0';
  /* ahead of . */
  line += HEXLEN+1;
  int pathlen = strlen(line);
  char *path= dk_malloc(sizeof(char)*pathlen+1);
  memcpy(path, line, pathlen); 
  path[pathlen] = '\0';
  struct ent *cur =  ent_init(path, san_fname(path, true));
  cur->hex = hex;
  return cur;
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

char *san_fname(char *path, bool san){
  char *name = dk_malloc(sizeof(char)*(strlen(path)+1));
  char *ptr = name;
  while(*path != '\0'){
    if(san){
      if(*path == '/'){
        *ptr = '+';
      }
      if(*path == '+'){
        *path = 26;
      }else{
        *ptr = *path;
      }
    }else{
      if(*path == '+'){
        *ptr = '/';
      }
      if(*path == 26){
        *ptr = '+';
      }else{
        *ptr = *path;
      }
    }
    ptr++;
    path++;
  }
  *ptr = '\0';
  return name;
}

#ifdef debug

char *time_hex(struct timespec *t){
  DIR *d;
  struct dirent *dp;
  if((d = opendir(".cams")) == NULL){
    fprintf(stderr, "unable to open dir: %s", d);
    exit(123);
  }
  int count = 0;
  while((dp = readdir(d)) != NULL){
    count++;
  }
  count = count-4; /* . .. next files */
  char *hex = dk_fmtmem("%016d", count);
  return hex;
}

#else

char *time_hex(struct timespec *t){
  struct timespec _t = *t;
  char *shex = hexstr((char *)&_t.tv_sec, sizeof(_t.tv_sec));
  char *nhex = hexstr((char *)&_t.tv_nsec, sizeof(_t.tv_nsec));
  return dk_fmtmem("%s%s", shex, nhex);
}

#endif


char *gen_fid(char *path){
  struct timespec t;
  timespec_get(&t, TIME_UTC);
  char *shex = hexstr((char *)&t.tv_sec, sizeof(t.tv_sec));
  char *nhex = hexstr((char *)&t.tv_nsec, sizeof(t.tv_nsec));
  char *fid = dk_fmtmem("%s%s.%s", shex, nhex, path);
  return fid;
}

/* internal methods - changeset operations */

struct ct_tree *flist(int id){
  DIR *d;
  struct dirent *dp;
  char *b = dk_fmtmem(".cams/%d/files", id);
  if((d = opendir(b)) == NULL){
    fprintf(stderr, "unable to open dir: %s", d);
    exit(123);
  }
  char *hex = hex_from_id(id);
  struct ent *cur;
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *tree = ct_tree_alpha_init();
  while((dp = readdir(d)) != NULL){
    if(!strncmp(".", dp->d_name, 1) || !strncmp("..", dp->d_name, 2))
      continue;
    cur = ent_init(NULL, dk_fmtmem(dp->d_name));
    cur->hex = hex;
    kv.key = cur->path;
    kv.data = cur;
    ct_tree_set(tree, &kv);
  }
  return tree;
}

struct ct_tree *cfiles(int id){
  char *path = dk_fmtmem(".cams/%d/cindex", id);
  char *hex = hex_from_id(id);
  struct ct_tree *tree = cindex_to_tree(path);
  struct ct_tree *dest = ct_tree_alpha_init();
  struct ct_key_data kv = {NULL, NULL};
  struct ent *cur;
  while(ct_tree_next(tree, &kv) != CT_NOT_FOUND){
    cur = (struct ent *)kv.data;  
    if(!strncmp(cur->hex, hex, HEXLEN)){
      ct_tree_set(dest, &kv);
    }
  }
  return dest;
}

struct ct_tree *rmlist(int id){
  int len;
  struct ct_key_data kv;
  struct ct_tree *tree = ct_tree_alpha_init();
  char line[1024];
  char *fname = dk_fmtmem(".cams/%d/remove", id);
  if(access(fname, F_OK) != -1){
    FILE *file = dk_open(fname, "r");
    while(fgets(line, 1023, file) != NULL){
      len = strlen(line); 
      if(line[len-1] == '\n'){
        line[len-1] = '\0';
      }else{
        fprintf(stderr, "error name too long\n");
        exit(123);
      }
      kv.key = line;
      ct_tree_set(tree, &kv);
    }
  }
  return tree;
}

struct ct_tree *cindex_to_tree(char *path){
  /*fprintf(stderr, "%s\n", path);*/
  char buff[1024];
  char *ptr;
  char *hex;
  char *fname;
  int len;
  struct ent *cur;
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *tree = ct_tree_alpha_init();
  FILE *file = dk_open(path, "r");

  while(fgets(buff, 1024, file) != NULL){
    hex = dk_malloc(HEXLEN+1);
    memcpy(hex, buff, HEXLEN);
    hex[HEXLEN] = '\0';
    ptr = buff+HEXLEN;
    /*fprintf(stderr, "ptr:'%s'\n", ptr);*/
    len = strlen(ptr);
    ptr += len-1;
    if(*ptr == '\n'){
      *ptr = '\0';
    }else{
      fprintf(stderr, "unatural end of line for cindex line\n");
      exit(123);
    }
    fname = dk_fmtmem("%s", buff+HEXLEN);
    /*
    fprintf(stderr, "fname:%s\n", fname);
    fprintf(stderr, "hex:%s\n", hex);
    */
    kv.key = fname;
    cur = ent_init(fname, NULL);
    cur->hex = hex;
    kv.data = cur;
    ct_tree_set(tree, &kv);
  }

  fclose(file);
  return tree;
}

void *cindex_tree_merge(void *key, void *a, void *b){
  struct ent *cur_a = (struct ent *)a;
  struct ent *cur_b = (struct ent *)b;
  if(!strncmp(cur_a->hex, cur_b->hex, HEXLEN)){
    return NULL;
  }
  struct ent *cur = ent_init(key, NULL);
  cur->hex = cur_a->hex;
  cur->prior = cur_b->hex;
  return cur;
}

struct ct_tree *diff_list(int from, int to){
  char *path_from = dk_fmtmem(".cams/%d/cindex", from);
  char *path_to = dk_fmtmem(".cams/%d/cindex", to);
  struct ct_tree *from_files = cindex_to_tree(path_from);
  struct ct_tree *to_files = cindex_to_tree(path_to);
  dk_free(path_from);
  dk_free(path_to);

  struct ct_tree *files = ct_tree_alpha_init();
  struct ct_key_data kv = {NULL, NULL};
  struct ent *cur;
  struct ct_tree_xresult *xres = ct_tree_intersection(to_files, from_files, cindex_tree_merge);

  if(xres->a->len){
    while(ct_tree_next(xres->a, &kv) != CT_NOT_FOUND){
      ct_tree_set(files,  &kv);
    }
  }
  if(xres->b->len){
    kv.key = NULL;
    while(ct_tree_next(xres->b, &kv) != CT_NOT_FOUND){
      cur = (struct ent *)kv.data;
      cur->prior = cur->hex;
      cur->hex = NULL;
      ct_tree_set(files,  &kv);
    }
  }
  if(xres->both->len){
    kv.key = NULL;
    while(ct_tree_next(xres->both, &kv) != CT_NOT_FOUND){
      ct_tree_set(files,  &kv);
    }
  }
  return files;
}

struct ct_tree *diff_list_staged(){
  int16_t id = next_commit_id();
  struct ct_tree *files = flist(next_commit_id());
  char *path_from = dk_fmtmem(".cams/%d/cindex", id-1);
  struct ct_tree *from_files =  cindex_to_tree(path_from);
  dk_free(path_from);
  struct ct_key_data kv = {NULL, NULL};
  char *hex = hex_from_id(id);
  struct ent *cur;
  struct ent *prior;
  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
    cur = (struct ent *)kv.data;
    cur->hex = hex;
    if(ct_tree_get(from_files, &kv) == CT_FOUND){
      prior = (struct ent *)kv.data;
      cur->prior = prior->hex;
    }
  }
  return files;
}

void show_files(int id){
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *files =  cfiles(id);
  bool first = true;
  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
    if(!first)
      printf(", ");
    first = false;
    printf("%s", kv.key);
  }
  ct_tree_free(files);
}

void show(int id){
  int l;
  char b[1024];
  char *hex;
  FILE *msg;
  if(!id){
   id = next_commit_id()-1;
  }
  if(snprintf(b, 1024, ".cams/%d/message", id) > 1023){
    fprintf(stderr, "error show path too long");
    exit(123);
  }
  if(!(msg = fopen(b, "r"))){
    fprintf(stderr, "error opening file");
    exit(123);
  }
  hex = hex_from_id(id);
  printf("%s: ", hex);
  while((l = fread(b, 1, 1024, msg)) != 0){
    if(b[l-1] == '\n') l--;
    fwrite(b, 1, l, stdout);
  }
  printf(" (");
  show_files(id);
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
          r->path = value;
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

/* utils */

char *hexstr(char *bytes, int len){
  char *ret = dk_malloc(sizeof(char)*len*2+1);
  char *s = ret;
  char *b = bytes;
  while(len-- > 0){
    snprintf(s, 3, "%02x", *b);
    b++;
    s+=2;
  }
  return ret;
}


/* main entry point */

int main(int argc, char **argv){
  int ret;
  struct opt_cmd cmds[] = {
    {"help",  usage},
    {"init",  init},
    {"add",  add},
    {"unadd",  unadd},
    {"rm",  rm},
    {"commit",  commit},
    {"list",  list},
    {"diff",  diff},
    {"push",  push},
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
  }else if(!strncmp("checkout", argv[1],  1024)){
    checkout(atoi(argv[2]));
  }else if(!strncmp("show", argv[1],  1024)){
    show(0);
  }else if(!strncmp("cindex", argv[1],  1024)){
    read_cindex();
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

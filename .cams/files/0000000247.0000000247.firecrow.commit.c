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


int hex(int argc, char **argv){
  struct timespec t;
  timespec_get(&t, TIME_UTC);
  char *hexs = time_hex(&t);
  printf("%s\n", hexs);
  dk_free(hexs);
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
    hex = dk_fmtmem("%s", buff);
    /* hack
    struct timespec t;
    timespec_get(&t, TIME_UTC);
    hex = dk_fmtmem("%09d.%010d.firecrow", t.tv_nsec, t.tv_sec);
    */
  }else{
    hex = dk_fmtmem("%016d", 0);
  }
  dk_free(hex_path);
  return hex; 
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
    ctree = cindex_to_tree(id-1);
  }else{
    ctree = ent_tree_init();
  }

  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *files = flist(id);
  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
    cur = (struct ent *)kv.data;
    cur->hex = dupstr(hex);
    ct_tree_set(ctree, &kv);
  }

  kv.key = NULL;
  struct ct_tree *rmfiles = rmlist(id);
  while(ct_tree_next(rmfiles, &kv) != CT_NOT_FOUND){
    ct_tree_unset(ctree, &kv);
  }

  char *cindex_path = dk_fmtmem(".cams/%d/cindex", id);
  FILE *cindexf = dk_open(cindex_path, "w+");

  kv.key = NULL;
  while(ct_tree_next(ctree, &kv) != CT_NOT_FOUND){
    fprintf(cindexf, "%s%s\n", ((struct ent *)kv.data)->hex, kv.key);
  }
  dk_free(hex);
  dk_free(cindex_path);
  files->free = NULL;
  ct_tree_free(files);
  ct_tree_free(rmfiles);
  ct_tree_free(ctree);
  fclose(cindexf);
}

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
      }else if(*path == '+'){
        *path = 26;
      }else{
        *ptr = *path;
      }
    }else{
      if(*path == '+'){
        *ptr = '/';
      }else if(*path == 26){
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
  char *hex = dk_fmtmem("%s%s", shex, nhex);
  dk_free(shex);
  dk_free(nhex);
  return hex;
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
  int count = dk_count();
  DIR *d;
  struct dirent *dp;
  char *b = dk_fmtmem(".cams/%d/files", id);
  if((d = opendir(b)) == NULL){
    fprintf(stderr, "unable to open dir: %s", d);
    exit(123);
  }
  dk_free(b);
  char *hex = hex_from_id(id);
  struct ent *cur;
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *tree = ent_tree_init();
  while((dp = readdir(d)) != NULL){
    if(!strncmp(".", dp->d_name, 1) || !strncmp("..", dp->d_name, 2))
      continue;
    cur = ent_init(NULL, dk_fmtmem(dp->d_name));
    kv.key = cur->path;
    kv.data = cur;
    ct_tree_set(tree, &kv);
  }
  dk_free(hex);
  return tree;
}

bool cfiles_filter(struct ct_key_data *kv, void *data){
    char *hex = (char *)data;
    struct ent *cur = (struct ent *)kv->data;  
    if(!strncmp(cur->hex, hex, HEXLEN)){
      return true;
    }
    return false;
}

struct ct_tree *cfiles(int id){
  char *hex = hex_from_id(id);
  struct ct_tree *tree = cindex_to_tree(id);
  struct ct_tree *dest = ct_tree_filter(tree, cfiles_filter, hex);
  dk_free(hex);
  tree->free = NULL; 
  ct_tree_free(tree);
  return dest;
}

struct ct_tree *rmlist(int id){
  int len;
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *tree = ct_tree_init(ct_cmp_alpha, ct_tree_free_key_data);
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
      kv.key = dk_fmtmem(line);
      ct_tree_set(tree, &kv);
    }
  }
  dk_free(fname);
  return tree;
}

struct ct_tree *cindex_to_tree(int id){
  char buff[1024];
  char *ptr;
  char *hex;
  char *fname;
  int len;
  struct ent *cur;
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *tree = ent_tree_init();
  char *path = dk_fmtmem(".cams/%d/cindex", id);
  FILE *file = dk_open(path, "r");

  while(fgets(buff, 1024, file) != NULL){
    hex = dk_malloc(HEXLEN+1);
    memcpy(hex, buff, HEXLEN);
    hex[HEXLEN] = '\0';
    ptr = buff+HEXLEN;
    len = strlen(ptr);
    ptr += len-1;
    if(*ptr == '\n'){
      *ptr = '\0';
    }else{
      fprintf(stderr, "unatural end of line for cindex line\n");
      exit(123);
    }
    fname = dk_fmtmem("%s", buff+HEXLEN);
    kv.key = fname;
    cur = ent_init(fname, NULL);
    cur->hex = dupstr(hex);
    kv.data = cur;
    ct_tree_set(tree, &kv);
    dk_free(hex);
  }
  fclose(file);
  return tree;
}

struct commit *commit_init(int id){
  struct commit *com = dk_malloc(sizeof(struct commit));
  char *basedir = dk_fmtmem(".cams/%d", id);
  char *path;

  com->id = id;

  /*
  path = dk_fmtmem("%s/%s", basedir, "prior");
  com->prior = atoi(ct_fread(path, sizeof(int16_t)));
  */

  path = dk_fmtmem("%s/%s", basedir, "hex");
  com->hex = ct_fread(path, HEXLEN);
  /* hack 
  com->hex = hex_from_id(id);
  */

  path = dk_fmtmem("%s/%s", basedir, "message");
  com->message = ct_fread(path, 4096);

  path = dk_fmtmem("%s/%s", basedir, "time");
  com->time =  atol(ct_fread(path, 16));

  com->cindex = cindex_to_tree(id);
  com->rmfiles = rmlist(id);
  com->files = cfiles(id);

  dk_free(basedir);
  return com;
}

int show_commit(int argc, char **argv){
  int id = atoi(argv[2]);
  struct commit *com = commit_init(id);
  fprintf(stderr, "commit:%d, time:%d, hex:%s\n", id, com->time, com->hex);
  fprintf(stderr, "files:\n");
  struct ct_key_data kv = {NULL, NULL};
  while(!ct_tree_next(com->files, &kv)){
    struct ent *cur = (struct ent *)kv.data;
    fprintf(stderr, "  %s:%s\n", kv.key, cur->hex);
  }
  fprintf(stderr, "all:\n");
  kv.key = NULL;
  while(!ct_tree_next(com->cindex, &kv)){
    struct ent *cur = (struct ent *)kv.data;
    fprintf(stderr, "  %s:%s\n", kv.key, cur->hex);
  }
  fprintf(stderr, "removed:\n");
  kv.key = NULL;
  while(!ct_tree_next(com->rmfiles, &kv)){
    fprintf(stderr, "  %s\n", kv.key);
  }
  return 0;
}

int add(int argc, char **argv){
  int i;
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
    ent_free(file);
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
  dk_free(b);
  fseek(rmfile, 0, SEEK_END);
  argc -= 2;
  argv += 2;
  while(--argc >= 0){
    file = ent_init(dk_fmtmem(argv[argc]), NULL);
    fwrite(file->path, 1, strlen(file->path), rmfile);
    fwrite("\n", 1, 1, rmfile);
    unlink(file->path);
    ent_free(file);
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
  fprintf(propf, "%s", msg);
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
  char *file_path;
  while(!ct_tree_next(files, &kv)){
    struct ent *file = (struct ent*)kv.data;
    file->hex = dupstr(hex);
    file_path = gen_path(file, true);
    cmd = dk_fmtmem("mv .cams/%d/files/%s .cams/files/%s", id, file->spath, file_path);
    dk_free(file_path);
    if(system(cmd)){
      fprintf(stderr, "error moving file: %s\n", file->path);
    }
    dk_free(cmd);
  }
  ct_tree_free(files);

  filedir = dk_fmtmem(".cams/%d/files", id);
  remove(filedir);
  dk_free(filedir);

  printf("%s commit created\n", hex);

  make_next(id+1);

  dk_free(hex);
  return 0;
}


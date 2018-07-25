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
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "crowtree/tree.h"
#include "cams.h"


/**
 * generate a cindex from the files in a commit
 * and then the previos positions of files from
 * the previous commit
 */
void generate_cindex(char *cid){
  struct ct_tree *ctree;
  struct ent *cur;
  struct commit *com = commit_init(cid);
  if(com->parent){
    ctree = cindex_to_tree(com->parent);
  }else{
    ctree = ent_tree_init();
  }

  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *files = flist();
  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
    cur = (struct ent *)kv.data;
    cur->cid = dupstr(cid);
    ct_tree_set(ctree, &kv);
  }

  kv.key = NULL;
  struct ct_tree *rmfiles = rmlist(cid);
  while(ct_tree_next(rmfiles, &kv) != CT_NOT_FOUND){
    ct_tree_unset(ctree, &kv);
  }

  char *cindex_path = dk_fmtmem(".cams/%s/cindex", cid);
  FILE *cindexf = dk_open(cindex_path, "w+");

  kv.key = NULL;
  while(ct_tree_next(ctree, &kv) != CT_NOT_FOUND){
    fprintf(cindexf, "%s:%s\n", ((struct ent *)kv.data)->cid, kv.key);
  }
  dk_free(cindex_path);
  files->free = NULL;
  ct_tree_free(files);
  ct_tree_free(rmfiles);
  ct_tree_free(ctree);
  fclose(cindexf);
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

char *gen_cid(char *name){
  DIR *d;
  int epoch;
  struct dirent *dp;
  if((d = opendir(".cams")) == NULL){
    fprintf(stderr, "unable to open dir: %s", d);
    exit(123);
  }
  int count = 1;
  while((dp = readdir(d)) != NULL){
    if(!strncmp("0000", dp->d_name, 4)){
      count++;
    }
  }
  epoch = count;
  if(count == 3){
    count = 2;
  }
  char *cid = dk_fmtmem("%09d.%010d.%s", count, epoch, "testuser");
  return cid;
}

#else

char *gen_cid(char *name){
  struct timespec t;
  timespec_get(&t, TIME_UTC);
  char *cid = dk_fmtmem("%09d.%010d.%s", t.tv_nsec, t.tv_sec, name);
  return cid;
}

#endif

/* internal methods - changeset operations */

struct ct_tree *flist(){
  int count = dk_count();
  DIR *d;
  struct dirent *dp;
  if((d = opendir(".cams/stage/files")) == NULL){
    fprintf(stderr, "unable to open dir: %s", d);
    exit(123);
  }
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
  return tree;
}

bool cfiles_filter(struct ct_key_data *kv, void *data){
    char *cid = (char *)data;
    struct ent *cur = (struct ent *)kv->data;  
    if(!strcmp(cur->cid, cid)){
      return true;
    }
    return false;
}

struct ct_tree *cfiles(char *cid){
  struct ct_tree *tree = cindex_to_tree(cid);
  struct ct_tree *dest = ct_tree_filter(tree, cfiles_filter, cid);
  tree->free = NULL; 
  ct_tree_free(tree);
  return dest;
}

struct ct_tree *rmlist(char *cid){
  int len;
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *tree = ct_tree_init(ct_cmp_alpha, ct_tree_free_key_data);
  char line[1024];
  char *fname = dk_fmtmem(".cams/%s/remove", cid);
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

struct ct_tree *cindex_to_tree(char *cid){
  struct ct_tree *tree = ent_tree_init();
  if(!cid){
    return tree;
  }
  char buff[1024];
  int len;
  struct ent *cur;
  struct ct_key_data kv = {NULL, NULL};
  struct ct_key_data ckv = {NULL, NULL};
  char *path = dk_fmtmem(".cams/%s/cindex", cid);
  FILE *file = dk_open(path, "r");
  while(fgets(buff, 1024, file) != NULL){
    cur = ent_from_line(buff);
    parse_cid(cur->cid, &cur->time, NULL);
    kv.key = cur->path;
    kv.data = cur;
    ct_tree_set(tree, &kv);
  }
  fclose(file);
  return tree;
}

void parse_cid(char *cid, struct timespec *time, char **name){
  char **pieces;
  if(ct_split(cid, '.', 0, &pieces) != 3){
    fprintf(stderr, "invalid id, split by . did not result in 3 pieces: %s\n", cid);
    exit(123);
  }
  if(name){
    *name = pieces[2];
  }
  time->tv_sec = atoi(pieces[1]);
  time->tv_nsec = atoi(pieces[0]);
}

struct commit *commit_init(char *cid){
  struct commit *com = dk_malloc(sizeof(struct commit));
  char *basedir = dk_fmtmem(".cams/%s", cid);
  char *path;

  com->cid = cid;
  parse_cid(cid, &com->time, &com->name);

  path = dk_fmtmem("%s/%s", basedir, "parent");
  com->parent = ct_fread(path, 4096);

  path = dk_fmtmem("%s/%s", basedir, "message");
  com->message = ct_fread(path, 4096);

  dk_free(basedir);
  return com;
}

int show_commit(int argc, char **argv){
  char *cid = argv[2];
  struct commit *com = commit_init(cid);
  struct ct_key_data kv = {NULL, NULL};
  while(!ct_tree_next(cfiles(com->cid), &kv)){
    struct ent *cur = (struct ent *)kv.data;
    fprintf(stderr, "  %s:%s\n", kv.key, cur->cid);
  }
  fprintf(stderr, "all:\n");
  kv.key = NULL;
  while(!ct_tree_next(cindex_to_tree(com->cid), &kv)){
    struct ent *cur = (struct ent *)kv.data;
    fprintf(stderr, "  %s:%s\n", kv.key, cur->cid);
  }
  fprintf(stderr, "removed:\n");
  kv.key = NULL;
  while(!ct_tree_next(rmlist(com->cid), &kv)){
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
  for(i = 0; i<len; i++){
    file = ent_init(dk_fmtmem(list[i]), NULL);
    struct ct_subp subp;
    subp.cmd = "cp";
    char *path = dk_fmtmem(".cams/stage/files/%s", file->spath);
    char *_argv[] = {"cp", "-v", file->path, path, NULL};
    subp.argv = _argv;
    ct_subp(&subp);
    if(subp.ret){
      fprintf(stderr, "error moving file\n");
    }
    ent_free(file);
  }
  return 0;
}

int unadd(int argc, char **argv){
  char *fpath;
  struct ent *file;
  argc -= 2;
  argv += 2;
  while(--argc >= 0){
    file = ent_init(dk_fmtmem(argv[argc]), NULL);
    char *fpath = dk_fmtmem(".cams/stage/files/%s", file->spath);
    printf("%s\n", fpath);
    unlink(fpath);
  }
  return 0;
}


int rm(int argc, char **argv){
  char *fpath;
  char *fname;
  struct ent *file;
  FILE *rmfile = dk_open(".cams/stage/remove", "a");
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
  FILE *propf;

  char *name = ct_fread(".cams/name", 4096);
  char *cid = gen_cid(name);
  dk_free(name);

  char *cid_path = dk_fmtmem(".cams/%s", cid);
  mkdir(cid_path, S_IFDIR | S_IRWXU | S_IRWXG); 

  char *msg = (argc == 3) ? argv[2] : NULL;
  fname = dk_fmtmem("%s/message", cid_path);
  propf = dk_open(fname, "w+");
  fprintf(propf, "%s", msg);
  fclose(propf);
  dk_free(fname);

  if(fexists(".cams/current")){
    char *parent_path = dk_fmtmem(".cams/%s/parent", cid);
    rename(".cams/current", parent_path); 
    dk_free(parent_path);
  }

  if(fexists(".cams/stage/remove")){
    char *newpath = dk_fmtmem("%s/remove", cid_path);
    rename(".cams/stage/remove", newpath);
    dk_free(newpath);
  }

  generate_cindex(cid);

  if(fexists(".cams/stage/cindex")){
    char *newpath = dk_fmtmem("%s/cindex", cid_path);
    rename(".cams/stage/cindex", newpath);
    dk_free(newpath);
  }
   
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *files = flist();
  char *cmd;
  char *dest_path;
  char *src_path;
  struct ct_subp subp;
  subp.cmd = "mv";
  while(!ct_tree_next(files, &kv)){
    struct ent *file = (struct ent*)kv.data;
    file->cid = dupstr(cid);
    dest_path = dk_fmtmem(".cams/files/%s", gen_path(file, true));
    src_path = dk_fmtmem(".cams/stage/files/%s", file->spath);

    char *_argv[] = {"mv", src_path, dest_path, NULL};
    subp.argv = _argv;
    ct_subp(&subp);
    if(subp.ret){
      fprintf(stderr, "error moving file: %s\n", file->path);
    }

    dk_free(dest_path);
    dk_free(src_path);
  }


  propf = dk_open(".cams/current", "w+");
  fprintf(propf, "%s", cid);
  fclose(propf);

  register_millis(cid);
  printf("%s commit created\n", cid);
  
  ct_tree_free(files);
  dk_free(cid_path);

  return 0;
}

char *get_current(){
  return ct_fread(".cams/current", 4096);
}

void register_millis(char *cid){
  char **pieces;
  ct_split(cid, '.', 0, &pieces);
  char *millis = pieces[0];

  char *expected = dk_fmtmem(".cams/millis/%s", millis);
  char *conflicted = dk_fmtmem(".cams/millis/%s.conflicts", millis);
  bool exists = false;

  if(fexists(expected)){
    rename(expected, conflicted);
    exists = true;
  }
  if(exists || fexists(conflicted)){
    FILE *con = dk_open(conflicted ,"a");
    if(exists){
      fwrite("\n", 1, 1, con);
    }
    fwrite(cid, 1, strlen(cid), con);
    fwrite( "\n", 1, 1,  con);
    fclose(con);
  }else{
    FILE *exp = dk_open(expected ,"w");
    fwrite(cid, 1, strlen(cid), exp);
    fclose(exp);
  }
}

char *lookup_or_die(char *millis){
  if(strlen(millis) > 20){ /* millis=9 period=. epoch=10 */
    return millis;
  }
  char *expected = dk_fmtmem(".cams/millis/%s", millis);
  char *conflicted = dk_fmtmem(".cams/millis/%s.conflicts", millis);
  if(fexists(expected)){
    return ct_fread(expected, 4096);
  }else if(fexists(conflicted)){
    char *conflicts = ct_fread(conflicted, 4096);
    printf("conflicts for this millisecond count, use full cid for one of the following:\n\n");
    printf(conflicts);
  }else{
    printf("commit not found my millis: %s\n", millis);
  }
}



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
#include "crowopt/opt.h"
#include "cams.h"

static struct ct_key_data *cindex_tree_merge(void *key, void *a, void *b){

  struct ent *cur_a = (struct ent *)a;
  struct ent *cur_b = (struct ent *)b;
  struct ct_key_data *kv = dk_malloc(sizeof(struct ct_key_data));
  if(!strcmp(cur_a->cid, cur_b->cid)){
    /*
    ent_free(cur_a);
    ent_free(cur_b);
    */
    kv->key = NULL;
    kv->data = NULL;
    return kv;
  }
  struct ent *cur = ent_init(dupstr(key), NULL);
  cur->cid = dupstr(cur_a->cid);
  cur->prior = dupstr(cur_b->cid);
  kv->key = cur->path;
  kv->data = cur;
  
  /*
  ent_free(cur_a);
  ent_free(cur_b);
  */
  return kv;
}

static struct ct_tree *diff_list_staged(){
  struct ct_tree *files = flist();
  struct ct_tree *from_files =  cindex_to_tree(get_current());
  struct ct_key_data kv = {NULL, NULL};
  struct ent *cur;
  struct ent *prior;
  while(!ct_tree_next(files, &kv)){
    cur = (struct ent *)kv.data;
    if(!ct_tree_get(from_files, &kv)){
      prior = (struct ent *)kv.data;
      cur->prior = prior->cid;
    }
  }
  return files;
}

static struct ct_tree *diff_list(char *from, char *to){
  struct ct_tree *from_files = cindex_to_tree(from);
  from_files->free = NULL;
  struct ct_tree *to_files = cindex_to_tree(to);
  to_files->free = NULL;

  struct ct_tree *files = ent_tree_init();
  struct ct_key_data kv = {NULL, NULL};
  struct ent *cur;

  struct ct_tree_xresult *xres = ct_tree_intersection(from_files, to_files, cindex_tree_merge);

  ct_tree_free(from_files);
  ct_tree_free(to_files);

  if(xres->a->len){
    while(!ct_tree_next(xres->a, &kv)){
      struct ent *cur_a = (struct ent *)kv.data;
      ct_tree_set(files,  &kv);
    }
  }
  if(xres->b->len){
    kv.key = NULL;
    while(!ct_tree_next(xres->b, &kv)){
      cur = (struct ent *)kv.data;
      cur->prior = cur->cid;
      cur->cid = NULL;
      ct_tree_set(files,  &kv);
    }
  }
  if(xres->both->len){
    kv.key = NULL;
    while(!ct_tree_next(xres->both, &kv)){
      ct_tree_set(files,  &kv);
    }
  }
  xres->a->free = NULL;
  xres->b->free = NULL;
  xres->both->free = NULL;
  ct_tree_free(xres->a);
  ct_tree_free(xres->b);
  ct_tree_free(xres->both);
  dk_free(xres);

  return files;
}

int diff(int argc, char **argv){
  /* remove difftool and make it one target `diff`
    -m --modified, [deault] by modified time in the local folders, any file in the current cindex
    -a --added, added files
    -e --editor, what use to be difftool
    -s --shallow, list added files only 
  */
  
  char *prog;
  char *from;
  char *to = NULL;
  char *cmd;
  struct ct_tree *files;
  struct ct_key_data kv = {NULL, NULL};
  struct ent *file;
  bool staged = true;
  bool shallow = false;
  bool editor = false;
  bool added = true;

  struct opts *opts = getopts(argc, argv);
  struct opt_value *optval;

  added = (find_opt(opts, 'a', "added")) ? true : false;
  staged = (find_opt(opts, 'm', "modified") || opts->argc == 1) ? true : false;
  shallow = (find_opt(opts, 's', "shallow")) ?  true : false;
  editor = (find_opt(opts, 'e', "editor")) ? true : false;

  if(opts->argc >= 2){
    from = opts->argv[1];
    if(opts->argc == 3){
      to = opts->argv[2];
    }
  }

  if(!to){
    to = get_current();
  }
  from = lookup_or_die(from);
  to = lookup_or_die(to);

  if(staged){
    files = diff_list_staged(from);
  }else{
    files = diff_list(from, to);
  }

  while(!ct_tree_next(files, &kv)){
    file = (struct ent *)kv.data;
    char *from_path;
    char *to_path;
    char *path;

    if(staged){
      to_path = dk_fmtmem("%s", file->path);
    }else if(file->cid){
      path = gen_path(file, true);
      to_path = dk_fmtmem(".cams/files/%s", path);
      dk_free(path);
    }else{
      to_path = dk_fmtmem("/dev/null");
    }
    if(file->prior){
      path = gen_path(file, false);
      from_path = dk_fmtmem(".cams/files/%s", path);
      dk_free(path);
    }else{
      from_path = dk_fmtmem("/dev/null");
    }

    if(shallow){
      printf("%s\n", file->path);
    }else{
      if(editor){
        prog = "vimdiff";
      }else{
        prog = "diff";
      }
      cmd = dk_fmtmem("%s %s %s", prog, from_path, to_path);
      printf("[%s]\n", file->path);
      fflush(stdout);
      system(cmd);
      printf("\n");

      dk_free(to_path);
      dk_free(from_path);
      dk_free(cmd);
    }
  }
  ct_tree_free(files);
  return 0;
}

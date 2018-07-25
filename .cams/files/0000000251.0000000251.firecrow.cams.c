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


/* commands */

int usage(int argc, char **argv){
  char *command = argv[0];
  int l = strlen(command);
  while(*command == '.' || *command == '/'){
    command++;
  }

  printf("%s\n    help|init|add|commit|list|diff|push [args...]\n", command);
  return 1;
}

int init(int argc, char **argv){
  system("mkdir .cams");  
  system("mkdir .cams/files");  
  make_next(1);
  return 0;
}

int _diff(int argc, char **argv, char *prog){
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
    char *path;

    if(staged){
      to_path = dk_fmtmem(".cams/%d/files/%s", next_commit_id(), file->spath);
    }else if(file->hex){
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

    cmd = dk_fmtmem("%s %s %s", prog, from_path, to_path);
    printf("[%s]\n", file->path);
    fflush(stdout);
    system(cmd);
    printf("\n");

    dk_free(to_path);
    dk_free(from_path);
    dk_free(cmd);
  }
  ct_tree_free(files);
  return 0;
}

int diff(int argc, char **argv){
  return _diff(argc, argv, "diff");
}

int difftool(int argc, char **argv){
  return _diff(argc, argv, "vimdiff");
}

void checkout(int16_t from){
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

struct ct_key_data *cindex_tree_merge(void *key, void *a, void *b){
  struct ent *cur_a = (struct ent *)a;
  struct ent *cur_b = (struct ent *)b;
  struct ct_key_data *kv = dk_malloc(sizeof(struct ct_key_data));
  if(!strncmp(cur_a->hex, cur_b->hex, HEXLEN)){
    ent_free(cur_a);
    ent_free(cur_b);
    return NULL;
  }
  struct ent *cur = ent_init(dupstr(key), NULL);
  cur->hex = dupstr(cur_a->hex);
  cur->prior = dupstr(cur_b->hex);
  kv->key = cur->path;
  kv->data = cur;
  
  ent_free(cur_a);
  ent_free(cur_b);
  return kv;
}

struct ct_tree *diff_list(int from, int to){
  struct ct_tree *from_files = cindex_to_tree(from);
  from_files->free = NULL;
  struct ct_tree *to_files = cindex_to_tree(to);
  to_files->free = NULL;

  struct ct_tree *files = ent_tree_init();
  struct ct_key_data kv = {NULL, NULL};
  struct ent *cur;
  struct ct_tree_xresult *xres = ct_tree_intersection(to_files, from_files, cindex_tree_merge);
  ct_tree_free(from_files);
  ct_tree_free(to_files);

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
  xres->a->free = NULL;
  xres->b->free = NULL;
  xres->both->free = NULL;
  ct_tree_free(xres->a);
  ct_tree_free(xres->b);
  ct_tree_free(xres->both);
  dk_free(xres);

  return files;
}

struct ct_tree *diff_list_staged(){
  int16_t id = next_commit_id();
  struct ct_tree *files = flist(next_commit_id());
  struct ct_tree *from_files =  cindex_to_tree(id-1);
  struct ct_key_data kv = {NULL, NULL};
  char *hex = hex_from_id(id);
  struct ent *cur;
  struct ent *prior;
  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
    cur = (struct ent *)kv.data;
    cur->hex = dupstr(hex);
    if(ct_tree_get(from_files, &kv) == CT_FOUND){
      prior = (struct ent *)kv.data;
      cur->prior = prior->hex;
    }
  }
  dk_free(hex);
  return files;
}


/* internal methods - remote flow */




int run_cmd(int argc, char **argv, struct opt_cmd cmds[]){
  int i = 0;
  while(cmds[i].name != NULL){
    if(strlen(cmds[i].name) == strlen(argv[1]) 
        && !strncmp(cmds[i].name, argv[1], strlen(cmds[i].name))){
      return cmds[i].fnc(argc, argv);
    }
    i++;
  }
  return -1;
}

/* utils */


/* main entry point */

int main(int argc, char **argv){
  int ret;
  int count;

  if(argc < 2){
    usage(argc, argv);
    return 1;
  }

  struct opt_cmd cmds[] = {
    {"help",  usage},
    {"init",  init},
    {"hex",  hex},
    {"add",  add},
    {"unadd",  unadd},
    {"rm",  rm},
    {"commit",  commit},
    {"list",  list},
    {"diff",  diff},
    {"difftool",  difftool},
    {"push",  push},
    {"show",  show_commit},
    NULL
  };

  ret = run_cmd(argc, argv, cmds);
  if(ret != -1){
    if(count = dk_count()){
      fprintf(stderr, "\033[34merror memory count not zero for %s: %d\n\033[0m", argv[1], count);
      /*return 123;*/
    }
    return ret;
  }
  fprintf(stderr, "command not found\n");
  return 1;
}

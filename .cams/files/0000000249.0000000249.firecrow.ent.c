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

char *ent_tostr(struct ent *cur){
  return dk_fmtmem("<ent path:%s, spath:%s, hex:%s, prior:%s>", cur->path, cur->spath, cur->hex, cur->prior);
}

void ct_free_ent_node(void *key, void *data){
  struct ent *cur = (struct ent *)data;
  ent_free(cur);
}

struct ct_tree *ent_tree_init(){
  return ct_tree_init(ct_cmp_alpha, ct_free_ent_node);
}

char *gen_path(struct ent* cur, bool current){
  return dk_fmtmem("%s.%s", (current ? cur->hex : cur->prior), cur->spath);
}

struct ent *ent_init(char *path, char *spath){
  struct ent *cur = dk_malloc(sizeof(struct ent));
  if(!path && !spath){
    return NULL;
  }
  if(!path){
    path = san_fname(spath, false);
  }
  if(!spath){
    spath = san_fname(path, true);
  }
  cur->path = path;
  cur->spath = spath;
  cur->hex = NULL;
  cur->prior = NULL;
  return cur;
}

void ent_free(struct ent *cur){
  if(cur->path){
    dk_free(cur->path);
  }
  if(cur->spath){
    dk_free(cur->spath);
  }
  if(cur->hex){
    dk_free(cur->hex);
  }
  if(cur->prior){
    dk_free(cur->prior);
  }
  dk_free(cur);
}

struct ent *ent_from_line(char *line){
  char *hex = dk_malloc(HEXLEN+1);
  memcpy(hex, line, HEXLEN); 
  hex[HEXLEN] = '\0';
  /* ahead of . */
  line += HEXLEN+1;
  int pathlen = strlen(line);
  fprintf(stderr, "pathlen:%d\n", pathlen);
  char *path= dk_malloc(sizeof(char)*pathlen+1);
  memcpy(path, line, pathlen); 
  path[pathlen] = '\0';
  struct ent *cur =  ent_init(path, san_fname(path, true));
  cur->hex = hex;
  return cur;
}

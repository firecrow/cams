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
  return dk_fmtmem("<ent path:%s, spath:%s, cid:%s, prior:%s>", cur->path, cur->spath, cur->cid, cur->prior);
}

void ct_free_ent_node(void *key, void *data){
  struct ent *cur = (struct ent *)data;
  ent_free(cur);
}

struct ct_tree *ent_tree_init(){
  return ct_tree_init(ct_cmp_alpha, ct_free_ent_node);
}

char *gen_path(struct ent* cur, bool current){
  return dk_fmtmem("%s.%s", (current ? cur->cid : cur->prior), cur->spath);
}

struct ent *ent_init(char *path, char *spath){
  if(!path && !spath){
    return NULL;
  }
  struct ent *cur = dk_malloc(sizeof(struct ent));
  if(!path){
    path = san_fname(spath, false);
  }
  if(!spath){
    spath = san_fname(path, true);
  }
  cur->path = path;
  cur->spath = spath;
  cur->cid = NULL;
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
  if(cur->cid){
    dk_free(cur->cid);
  }
  if(cur->prior){
    dk_free(cur->prior);
  }
  dk_free(cur);
}

struct ent *ent_from_line(char *line){
  char **pieces;
  int piece_count =  ct_split(line, ':', 1, &pieces); 
  if(piece_count != 2){
    fprintf(stderr, "wrong number of pieces returned by split in cindex_to_tree: %d\n", piece_count);
    exit(123);
  }
  char *cid = pieces[0];
  char *fname = pieces[1];
  trimnl(fname);

  struct ent *cur = ent_init(fname, NULL);
  cur->cid = dupstr(cid);
  return cur;
}

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
#include <ftw.h>
#include "crowtree/tree.h"
#include "cams.h"

struct ignore {
  int len;
  char **pieces;
};

static struct ignore ign; 
struct ct_tree *present;

int print_fname(const char *_fpath, const struct stat *sb, int flags){
  struct ct_key_data kv = {NULL, NULL};
  char *fpath = (char *)_fpath;
  fpath++;
  fpath++;
  bool show = true;
  if(strlen(fpath) == 0 || !strncmp(".cams", fpath, strlen(".cams"))){
    show = false;
  }
  int i = 0;
  while(i < ign.len){
    if(!strncmp(ign.pieces[i], fpath, strlen(ign.pieces[i]))){
      show = false;
    }
    i++;
  }
  if(show){
    kv.key = dupstr(fpath);
    kv.data = NULL;
    ct_tree_set(present, &kv);
  }
  return 0;
}

struct ct_tree *slist(){
  present = ct_tree_alpha_init();
  char *ignore = ct_fread(".cams/ignore", 4096);
  trimnl(ignore);
  ign.len = ct_split(ignore, '\n', 0, &ign.pieces);
  ftw(".", print_fname, 512);
  return present;
}


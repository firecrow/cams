#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
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

int list(int argc, char **argv){
  char *cid;
  char *filestr;
  int count = 0;
  int qty = 0;
  int cols;
  char *buff;
  struct opts *opts = getopts(argc, argv);
  struct opt_value *optval;

  bool fit = (find_opt(opts, 'f', "fit")) ? true : false;
  if(fit){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    qty = w.ws_row-1;
    cols = w.ws_col;
    buff = dk_malloc(cols);
  }else{
    optval = find_opt(opts, 'q', "quantity");
    if(optval && optval->value){
      if(!strcmp(optval->value, "all")){
        qty = 0;
      }else{
        qty = atoi(optval->value);
      }
    }
  }

  if(opts->argc >= 2){
    cid = lookup_or_die(opts->argv[1]);
  }else{
    cid = get_current();
  }
  while(cid){
    struct commit *com = commit_init(cid);
    trimnl(com->message);
    filestr = ct_tree_alpha_join(cfiles(com->cid), ", ");
    if(fit){
      if(snprintf(buff, cols, "%09d: %s (%s)\n", com->time.tv_nsec, com->message, filestr) > cols){
        buff[cols-3] = buff[cols-2] = buff[cols-1] = '.';
        buff[cols] = '\n';
      }
      printf(buff);
    }else{
      printf( "%09d: %s (%s)\n", com->time.tv_nsec, com->message, filestr);
    }
    if(filestr != NULL){
      dk_free(filestr);
    }
    if(qty && ++count == qty){
      break;
    };
    cid = com->parent;
  }
  return 0;
}

long ts_delta(struct timespec *a, struct timespec *b){
  if(a->tv_sec != b->tv_sec){
    return b->tv_sec - a->tv_sec;
  }else{
    return (b->tv_nsec < a->tv_nsec ? 1 : -1);
  }
}

int status(int argc, char **argv){
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *staged = flist();
  struct ct_tree *untracked = slist();
  char *cid = get_current();
  struct ct_tree *cindex = cindex_to_tree(cid);
  staged->free = NULL;
  untracked->free = NULL;
  cindex->free = NULL;
  struct timespec cindex_time;
  struct stat file_stat;
  bool printed = false;

  if(staged->len){
    printf("-> staged files:\n");
    kv.key = NULL;
    while(!ct_tree_next(staged, &kv)){
      ct_tree_unset(cindex, &kv);
      ct_tree_unset(untracked, &kv);
    }
  }

  if(cindex->len){
    kv.key = NULL;
    while(!ct_tree_next(cindex, &kv)){
      ct_tree_unset(untracked, &kv);
      struct ent *cur = kv.data;
      parse_cid(cur->cid, &cindex_time, NULL);
      stat(cur->path, &file_stat);
      if(ts_delta(&cindex_time, &file_stat.st_mtim) > 0){
        if(!printed){
          printf("-> modified files:\n");
          printed = true;
        }
        printf("%s\n", kv.key);
      }
    }
  }

  if(untracked->len){
    printf("-> untracked files:\n");
    kv.key = NULL;
    while(!ct_tree_next(untracked, &kv)){
      printf("%s\n", kv.key);
    }
  }

  /*
  ct_tree_free(cindex);
  ct_tree_free(staged);
  dk_free(cid);
  */
}

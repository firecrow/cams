#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
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
    cid = opts->argv[1];
  }else{
    cid = get_current();
  }
  while(cid){
    struct commit *com = commit_init(cid);
    filestr = ct_tree_alpha_join(cfiles(com->cid), ", ");
    if(fit){
      if(snprintf(buff, cols, "%s: %s (%s)\n", com->cid, com->message, filestr) > cols){
        buff[cols-3] = buff[cols-2] = buff[cols-1] = '.';
        buff[cols] = '\n';
      }
      printf(buff);
    }else{
      printf( "%s: %s (%s)\n", com->cid, com->message, filestr);
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



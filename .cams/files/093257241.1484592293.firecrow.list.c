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
  int count = 0;
  int qty = 0;
  struct opts *opts = getopts(argc, argv);
  struct opt_value *optval;
  optval = find_opt(opts, 'q', "quantity");
  if(optval && optval->value){
    fprintf(stderr, "%s:%s:%d\n", optval->value, "fit", strcmp(optval->value, "fit"));
    if(!strcmp(optval->value, "all")){
      qty = 0;
    }else if(!strcmp(optval->value, "fit")){
      struct winsize w;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      qty = w.ws_row;
    }else{
      qty = atoi(optval->value);
    }
  }
  fprintf(stderr, "qty:%d\n", qty);

  if(opts->argc >= 3){
    cid = opts->argv[2];
  }else{
    cid = get_current();
  }
  while(cid){
    struct commit *com = commit_init(cid);
    char *filestr = ct_tree_alpha_join(cfiles(com->cid), ", ");
    printf("%s: %s (%s)\n", com->cid, com->message, filestr);
    dk_free(filestr);
    if(qty && ++count == qty){
      break;
    };
    cid = com->parent;
  }
  return 0;
}



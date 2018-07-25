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
#include "cams.h"

int list(int argc, char **argv){
  char *cid;
  int count = 0;
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int qty = w.ws_row;
  if(argc >= 3){
    cid = argv[2];
  }else{
    cid = get_current();
  }
  while(cid){
    struct commit *com = commit_init(cid);
    char *filestr = ct_tree_alpha_join(cfiles(com->cid), ", ");
    printf("%s: %s (%s)\n", com->cid, com->message, filestr);
    dk_free(filestr);
    if(qty && ++count == qty-10){
      break;
    };
    cid = com->parent;
  }
  return 0;
}



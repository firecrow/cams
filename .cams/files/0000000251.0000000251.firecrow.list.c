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

int list(int argc, char **argv){
  int id;
  int count = 0;
  int start = (argc >= 3) ? atoi(argv[2]) : 0;
  int qty = (argc == 4) ? atoi(argv[3]) : 0;
  if(start == 0){
    id = next_commit_id()-1;
  }else{
    id = start;
  }
  while(id > 0){
    struct commit *com = commit_init(id);
    char *filestr = ct_tree_alpha_join(com->files, ", ");
    printf("%s: %s (%s)\n", com->hex, com->message, filestr);
    dk_free(filestr);
    if(qty && ++count == qty){
      break;
    };
    id--;
  }
  return 0;
}



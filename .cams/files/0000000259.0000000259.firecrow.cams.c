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
  FILE *name = dk_open(".cams/name", "w+");
  fprintf(name, "%s", getenv("USER"));
  fclose(name);
  make_next(1);
  return 0;
}

int debug_fnc(int argc, char **argv){
  char *str = "hi:there";
  char **pieces;
  printf("count:%d\n", ct_split(str, ':', 0, &pieces));
  printf("%s = %s\n", pieces[0], pieces[1]);
}

void checkout(int16_t from){
}

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
    {"debug",  debug_fnc},
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

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
#include "crowopt/opt.h"
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
  struct ct_subp subp;
  subp.cmd = "mkdir";
  char *_argv[] = {
    "mkdir", 
    ".cams", 
    ".cams/files", 
    ".cams/stage", 
    ".cams/stage/files", 
    ".cams/millis", 
    NULL
  };
  subp.argv = _argv;
  int ret = ct_subp(&subp, 0);
  if(ret){
    fprintf(stderr, "error creating directories during init");
  }

  FILE *name = dk_open(".cams/name", "w+");
  fprintf(name, "%s", getenv("USER"));
  fclose(name);
  return 0;
}

int debug_fnc(int argc, char **argv){
  char buff[4096];
  struct ct_subp subp;
  subp.cmd = "ls";
  char *_argv[] = {"ls", "-1", "/usr/psrc", NULL};
  subp.argv = _argv;
  int ret = ct_subp(&subp, CT_USE_STDOUT);
  while(fgets(buff, 4096, subp.stdout) != NULL){
    trimnl(buff);
    fprintf(stderr, "here it is:'%s'\n", buff);
  }
}

void checkout(char *from){
  ;
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
    {"add",  add},
    {"unadd",  unadd},
    {"rm",  rm},
    {"commit",  commit},
    {"list",  list},
    {"status",  status},
    {"diff",  diff},
    {"push",  push},
    {"show",  show_commit},
    {"debug",  debug_fnc},
    NULL
  };

  ret = run_cmd(argc, argv, cmds);
  if(ret != -1){
    if(count = dk_count()){
      /*
      fprintf(stderr, "\033[34merror memory count not zero for %s: %d\n\033[0m", argv[1], count);
      return 123;
      */
    }
    return ret;
  }
  fprintf(stderr, "command not found\n");
  return 1;
}

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <crowtils.h>
#include <fcntl.h>
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
  ct_subp(&subp);
  if(subp.ret){
    fprintf(stderr, "error creating directories during init");
  }

  FILE *name = dk_open(".cams/name", "w+");
  fprintf(name, "%s", getenv("USER"));
  fclose(name);
  return 0;
}

int debug_fnc(int argc, char **argv){
  /*
  int selres;
  fd_set rset;
  struct timeval tv = {0, 250000}; /* 0.250 second * /
  char buff[4096];
  struct ct_subp subp;
  subp.cmd = "./pipetest.py";
  char *_argv[] = {
    "./pipetest.py",
    NULL
  };
  subp.argv = _argv;
  subp.flags = CT_SUBP_ASYNC | CT_USE_STDOUT | CT_USE_STDERR;
  int l = 0;
  int exited = ct_subp(&subp);
  int flags = fcntl(subp.outs[0], F_GETFL, 0);
  fcntl(subp.outs[0], F_SETFL, flags | O_NONBLOCK);
  FD_ZERO(&rset);
  FD_SET(subp.outs[0], &rset);
  while(exited == 0 && (exited = ct_subpwait(&subp)) == 0){
    selres = select(subp.outs[0]+1, &rset, NULL, NULL, &tv);
    if(selres < 0){
       fprintf(stderr, "Error reading selec");
       break;
    }
    l = read(subp.outs[0], buff, 4096);
    if(!l){
      close(subp.outs[0]);
      break;
    }else if(l > 0){
      buff[l] = '\0';
      fprintf(stderr, buff);
    }
  }
  */
}

int checkout(int argc, char **argv){
  struct opts *opts = getopts(argc, argv);
  struct opt_value *ruri = find_opt(opts, 'r', "remote");
  if(ruri->value){
    struct remote r;
    parse_remote(&r, ruri->value);
    char *rlatest = latest_remote(&r);
    mkdir(rlatest, 0775);
    /* get the latest cindex */
    scp_down(&r, dk_fmtmem("%s/%s/cindex", r.path, rlatest), rlatest, NULL);
    /* make a tar of the files */
    /* pull it down, and unpack it */
    /* rename the artifacts */
    return 0;
  }else{
    fprintf(stderr, "Non remote checkin not yet supported.\n");
    exit(123);
  }
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
    {"checkout",  checkout},
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

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
  /*char *str = "firecrow@crowtils.net:342/usr/local/psrc/cams";*/
  /*char *str = "firecrow@crowtils.net/usr/local/psrc/cams";*/
  /*char *str = "firecrow@crowtils.net~/psrc/cams";*/
  /*char *str = "crowtils.net/psrc/cams";*/
  /*char *str = "crowtils.net";*/
  char *str = "crowtils.net:342";
  char *from = str;
  int offset = 0;
  enum REMOTE_PARSE_STATES state = CT_ROPEN;
  struct remote *r = dk_malloc(sizeof(struct remote));
  memset(r, 0, sizeof(struct remote));
  fprintf(stderr, "from:%s\n", from);
  while(1){
    if(state == CT_ROPEN){
      if(*(from+offset) == '@'){
        fprintf(stderr, "OPEN USER END:%d\n", offset);
        r->user = dupnstr(from, offset);
        from = from+(offset+1);
        offset = 0;
        state = CT_RHOST;
      }
    }
    if(state == CT_ROPEN || state == CT_RHOST){
      if(from[offset] == '/' || from[offset] == '~' || from[offset] == '\0'){
        fprintf(stderr, "HOST END\n");
        r->host = dupnstr(from, offset);
        from = from+offset;
        offset = 0;
        state = CT_RPATH;
      }else if(from[offset] == ':'){
        fprintf(stderr, "HOST END PORT\n");
        r->host = dupnstr(from, offset);
        from = from+(offset+1);
        offset = 0;
        state = CT_RPORT;
      }
    }else if(state == CT_RPORT){
      if(from[offset] < 48 || from[offset] > 57){ /* non number character */
        fprintf(stderr, "PORT END\n");
        r->port = atoi(dupnstr(from, offset));
        from = from+offset;
        offset = 0;
        state = CT_RPATH;
      }
    }else if(state == CT_RPATH){
      if(from[offset] == '\0'){
        fprintf(stderr, "%d\n", offset);
        fprintf(stderr, "PATH END\n");
        if(offset > 1){
          r->path = dupnstr(from, offset);
          from = from+(offset-1);
          offset = 0;
        }
        state = CT_RDONE;
        fprintf(stderr, "breaking\n");
        break;
      }
    }
    if(from[offset] == '\0' && state != CT_RDONE){
      /*
      fprintf(stderr, "Error parsing remote\n");
      */
      /*exit(123);*/
    }
    offset++;
  }
  fprintf(stderr, "user:%s, host:%s, port:%d, path:%s\n", r->user, r->host, r->port, r->path);
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

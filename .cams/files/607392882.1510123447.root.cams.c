#include "cams.h"

int usage(int argc, char **argv, struct intls *intls){
  char *command = argv[0];
  int l = strlen(command);
  while(*command == '.' || *command == '/'){
    command++;
  }

  printf("%s\n    help|init|add|commit|list|diff|push [args...]\n", command);
  return 1;
}

int init(int argc, char **argv, struct intls *intls){
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
  subp.flags = 0;
  ct_subp(&subp);
  if(subp.ret){
    fprintf(stderr, "error creating directories during init");
  }

  FILE *name = dk_open(".cams/name", "w+");
  fprintf(name, "%s", getenv("USER"));
  fclose(name);
  return 0;
}

void out(int idx, char *str, void *data){
  trimnl(str);
  fprintf(stderr, "=%d: '%s'\n", idx, str);  
}

void done(struct ct_subp *subp, void *data){
  fprintf(stderr, "done\n");
  fprintf(stderr, "%d\n", subp->ret);
}

int debug_fnc(int argc, char **argv, struct intls *intls){
  struct ct_subp subp;
  subp.cmd = "./pipetest.py";
  char *_argv[] = {
    "./pipetest.py",
    NULL
  };
  subp.argv = _argv;
  subp.flags = CT_SUBP_ASYNC | CT_USE_STDOUT | CT_USE_STDERR;
  subp_emitter(&subp, NULL, out, done);
  return 0;
}

int checkout(int argc, char **argv, struct intls *intls){
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

int run_cmd(int argc, char **argv, struct intls *intls, struct opt_cmd cmds[]){
  int i = 0;
  while(cmds[i].name != NULL){
    if(strlen(cmds[i].name) == strlen(argv[1]) 
        && !strncmp(cmds[i].name, argv[1], strlen(cmds[i].name))){
      return cmds[i].fnc(argc, argv, intls);
    }
    i++;
  }
  return -1;
}

#ifdef UNITTEST
int main(int argc, char **argv){
	fprintf(stdout, "poo\n");
}
#else
int main(int argc, char **argv){
  int ret;
  int count;
	struct intls intls;

  if(argc < 2){
    usage(argc, argv, &intls);
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

	init_intls(&intls);
  ret = run_cmd(argc, argv, &intls, cmds);
  if(ret != -1){
    return ret;
  }
  fprintf(stderr, "command not found\n");
  return 1;
}
#endif

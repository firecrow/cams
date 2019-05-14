#include "cams.h"

int usage(){
  printf("cams\n    help|init|add|commit|list|diff|push [args...]\n");
  return 1;
}

/*
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
*/

/*
void out(int idx, char *str, void *data){
  trimnl(str);
  fprintf(stderr, "=%d: '%s'\n", idx, str);  
}
*/

/*
void done(struct ct_subp *subp, void *data){
  fprintf(stderr, "done\n");
  fprintf(stderr, "%d\n", subp->ret);
}
*/

/*
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
*/

/*
int checkout(int argc, char **argv, struct intls *intls){
  struct opts *opts = getopts(argc, argv);
  struct opt_value *ruri = find_opt(opts, 'r', "remote");
  if(ruri->value){
    struct remote r;
    parse_remote(&r, ruri->value);
    char *rlatest = latest_remote(&r);
    mkdir(rlatest, 0775);
    / * get the latest cindex * /
    scp_down(&r, dk_fmtmem("%s/%s/cindex", r.path, rlatest), rlatest, NULL);
    / * make a tar of the files * /
    / * pull it down, and unpack it * /
    / * rename the artifacts * /
    return 0;
  }else{
    fprintf(stderr, "Non remote checkin not yet supported.\n");
    exit(123);
  }
}
*/

char *op;
struct ct_tree *opkv;
struct ct_tree *ophandlers;
struct ct_tree *opflookup;
struct crray *opitems;
void arg_item_func(int idx, char *item){
  if(idx == 1){
    op = item;
  }else{
    /*opitems->add(opitems, item);*/
  }
}

void arg_flag_func(int idx, char flag){
}

void arg_word_func(int idx, char *word, char *value){
}

int main(int argc, char **argv){
  if(argc < 2){
    usage();
    return 1;
  }

  opitems = crray_str_init();
  ophandlers = ct_tree_alpha_init();
  struct ct_leaf _ophandlers[] = {
    {"help", 0, NULL},
    {"init", 0, NULL},
    {"add", 0, NULL},
    {"unadd", 0, NULL},
    {"rm", 0, NULL},
    {"commit", 0, NULL},
    {"checkout", 0, NULL},
    {"list", 0, list},
    {"status", 0, status},
    {"diff", 0, NULL},
    {"push", 0, NULL},
    {"show", 0, NULL},
    {"debug", 0, NULL},
    NULL
  };
  ct_tree_add_bulk(ophandlers, _ophandlers);

  opflookup = ct_tree_idx_init();
  struct ct_leaf _oflookup[] = {
    {NULL, 'f', "force"},
    {NULL, 'n', "number"},
    NULL
  };
  ct_tree_add_bulk(opflookup, _oflookup);

  crowarg_parse(argc, argv, NULL, arg_flag_func, arg_word_func, arg_item_func);
  
  struct ct_leaf opt_leaf;
  opt_leaf.key = op;
  if(ct_tree_get(ophandlers, &opt_leaf) == CT_NOT_FOUND){
    printf("op not found: %s\n", op);
    usage();
    return (1);
  }

  void (*handler)() = opt_leaf.data; 
  handler();

  return (0);
}

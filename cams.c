#include "cams.h"

int usage(){
  printf("cams\n    help|init|add|commit|list|diff|push [args...]\n");
  return 1;
}

void arg_item_func(int idx, char *item){
  if(idx == 0)
    return;

  if(idx == 1){
    op = item;
  }else{
    opitems->add(opitems, item);
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
    {"add", 0, add},
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

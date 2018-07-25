#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"
#include "tree.h"

int cmp_alpha(void *data_a, void *data_b){
  return strcmp(data_a, data_b);
}

int cmp_int(void *data_a, void *data_b){
  return *(int *)data_a - *(int *)data_b;
}

void free_node(struct ct_node *node){
  dk_free(node);
}

void free_node_int(struct ct_node *node){
  dk_free(node->key);
  dk_free(node->data);
  dk_free(node);
}

void pnode(struct ct_node *node){
  printf(node->key);
  printf("\n");
}

struct ptree_state {
  int level;
};

bool open[32];
void print_tree(
    struct ptree_state *state,
    struct ct_node *node, 
    void (*out)(struct ct_node *node)
  ){
  int show_level;
  int i;
  if(node->parent){
    i = 1;
    while(i < state->level){
      printf(open[i] ? "|" : " ");
      printf("  ");
      i++;
    }
    printf("|\n");
  }
  i = 1;
  while(i < state->level){
    printf(open[i] ? "|" : " ");
    printf("  ");
    i++;
  }
  if(node->parent){
    printf(node->is_gt ? "<" : ">");
    printf("--");
  }
  out(node);
  if(node->lt){
    state->level++;
    if(node->gt){
      open[state->level] = true;
    }
    print_tree(state, node->lt, out);
    state->level--;
  }
  if(node->gt){
    state->level++;
    open[state->level] = false;
    print_tree(state, node->gt, out);
    state->level--;
  }
}

int main(int argc, char **argv){
  struct ct_tree *tree;
  void *key;
  void **data;
  int *iptr;
  int i;
  if(argc == 1){
    printf("usage: treeprint [-i|-a] keys...\n");
    return 1;
  }
  argc--;
  argv++;

  bool is_alpha = false;
  if(!strcmp(argv[0], "-a")){
    is_alpha = true;
    argc--;
    argv++;
    tree = ct_tree_init(cmp_alpha, free_node);
  }else{
    tree = ct_tree_init(cmp_int, free_node_int);
  }

  i = 0;
  while(i < argc){
    if(is_alpha){
      key = argv[i];
    }else{
      iptr = dk_malloc(sizeof(int));
      *iptr = atoi(argv[i]);
      key = iptr;
    }
    ct_tree_op(tree, key, &key, CT_SET);
    i++;
  }

  memset(open, 0, sizeof(bool)*32);

  struct ptree_state *state = malloc(sizeof(struct ptree_state));
  state->level = 0;
  print_tree(state, tree->root, pnode);
  free(state);

}

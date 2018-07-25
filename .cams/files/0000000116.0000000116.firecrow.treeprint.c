#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"
#include "tree.h"


int cmp(void *data_a, void *data_b){
  return strcmp(data_a, data_b);
}

void free_node(struct ct_node *node){
  /*
  dk_free(node->key);
  dk_free(node->data);
  */
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
  if(node->parent){
    show_level = state->level;
    while(--show_level > 0){
      printf(open[show_level] ? "|" : " ");
      printf("  ");
    }
    printf("|\n");
  }
  show_level = state->level;
  while(--show_level > 0){
    printf(open[show_level] ? "|" : " ");
    printf("  ");
  }
  if(node->parent){
    printf(node->is_gt ? "<" : ">");
    printf("--");
  }
  out(node);
  if(node->lt){
    state->level++;
    open[state->level] = true;
    print_tree(state, node->lt, out);
    open[state->level] = false;
    state->level--;
  }
  if(node->gt){
    state->level++;
    print_tree(state, node->gt, out);
    state->level--;
  }
}

int main(int argc, char **argv){

  enum ct_status status;

  struct ct_tree *tree = ct_tree_init(cmp, free_node);

  char *keyC = "c";
  void *dataC = "carrot";
  status = ct_tree_op(tree, keyC, &dataC, CT_SET);

  char *keyB = "b";
  void *dataB = "bannana";
  status = ct_tree_op(tree, keyB, &dataB, CT_SET);

  char *keyA = "a";
  void *dataA = "apple";
  status = ct_tree_op(tree, keyA, &dataA, CT_SET);

  char *keyE = "e";
  void *dataE = "delta";
  status = ct_tree_op(tree, keyE, &dataE, CT_SET);

  char *keyF = "f";
  void *dataF = "delta";
  status = ct_tree_op(tree, keyF, &dataF, CT_SET);

  char *keyD = "d";
  void *dataD = "delta";
  status = ct_tree_op(tree, keyD, &dataD, CT_SET);

  char *keyG = "g";
  void *dataG = "delta";
  status = ct_tree_op(tree, keyG, &dataG, CT_SET);

  struct ptree_state *state = malloc(sizeof(struct ptree_state));
  state->level = 0;
  print_tree(state, tree->root, pnode);
  free(state);
}

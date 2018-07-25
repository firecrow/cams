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
  if(node->parent){
    printf(node->is_gt ? "<" : ">");
  }
  printf(node->key);
  printf("\n");
}

struct ptree_state {
  int level;
};

void print_tree(
    struct ptree_state *state,
    struct ct_node *node, 
    void (*out)(struct ct_node *node)
  ){
  int show_level = state->level;
  while(show_level--){
    printf("-");
  }
  out(node);
  if(node->lt){
    state->level++;
    print_tree(state, node->lt, out);
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


  char *key_o = "a";
  void *data_o;
  status = ct_tree_op(tree, key_o, &data_o, CT_GET);
  printf("result: %s\n", data_o);

  char *key3 = "c";
  void *data3 = "carrot";
  status = ct_tree_op(tree, key3, &data3, CT_SET);

  char *key2 = "b";
  void *data2 = "bannana";
  status = ct_tree_op(tree, key2, &data2, CT_SET);

  char *key = "a";
  void *data = "apple";
  status = ct_tree_op(tree, key, &data, CT_SET);

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

  char *key_o3 = "c";
  void *data_o3;
  status = ct_tree_op(tree, key_o3, &data_o3, CT_GET);
  printf("result: %s\n", data_o3);

  struct ptree_state *state = malloc(sizeof(struct ptree_state));
  print_tree(state, tree->root, pnode);
  free(state);

  /*
  char *x = fmtmem("%s->%s", "one", "two");
  printf("%d: %s\n", strlen(x), x);
  dk_free(x);
  if(dk_count() == 0){
    puts("all clean\n");
  }else{
    printf("oops dirty %s", dk_count);
  }
  */
}

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

int main(int argc, char **argv){

  enum ct_status status;

  struct ct_tree *tree = ct_tree_init(cmp, free_node);
  char *key = "a";
  void *data = "apple";
  status = ct_tree_op(tree, key, &data, CT_SET);

  char *key2 = "a";
  void *data2;
  status = ct_tree_op(tree, key2, &data2, CT_GET);
  printf("result: %s\n", data2);

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

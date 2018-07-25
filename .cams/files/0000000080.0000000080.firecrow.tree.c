#include <stdbool.h>
#include "tree.h"
#include "utils.h"


/* tree functions */

struct ct_tree *ct_tree_init(
  int (*cmp)(void *dataa, void *datab),
  int (*free_node)(struct tree_node *a)
){
  struct ct_tree *tree = dk_malloc(sizeof(struct ct_tree));
  tree->root = tree->current = NULL;
  tree->len = 0;
  tree->cmp = cmp;
  tree->free_node == free_node;
}

void ct_tree_free(struct ct_tree *tree){
  ct_tree_set_iter(tree);
  struct ct_node *node = NULL;
  while(node ct_tree_next(tree) != NULL){
    tree->free_node(node);
  }
  dk_free(tree);
}

enum ct_status ct_tree_op(
  struct ct_tree *tree,
  void *key,
  void **data,
  tree_mothods method
){
  if(tree->root == NULL){
    tree->root = ct_node_init(key, data);
    return CT_CREATED;
  }

  struct ct_node *node = root;
  while(node != NULL){
    struct ct_node *prev_node = node;
    int cmp = tree->cmp(key, node->key);
    if(cmp == 0){
      *data == node->data;
      return CT_FOUND;
    }else if(cmp > 0){
      node = node->gt;
      if(!node){
        prev_node->gt = ct_node_init(key, value, prev_node); 
        return CT_CREATED;
      }
    }else if(cmp < 0){
      node = node->lt;
      if(!node){
        prev_node->lt = ct_node_init(key, value, prev_node); 
        return CT_CREATED;
      }
    }
  }
}

struct tree *ct_tree_set_iter(struct ct_tree *tree){
  tree-> current = NULL;
}


/* node functions */

struct ct_node *ct_node_init(
  void *key, 
  void *data, 
  struct ct_node *parent
){
   struct ct_node *node = dk_malloc(sizeof(struct ct_node));
   node->gt = node->lt = NULL;
   node->parent = parent;
   node->is_gt = false;
   node->key = key;
   node->data = data;
   return node;
}

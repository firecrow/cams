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

/* 
 * complexity to manage
 *
 * comparisons: eq, gt, lt 
 * operations: set, get, unset
 * conditions: root is null
 * results: FOUND, NOT_FOUND, CREATED, MODIFIED, ERROR
 *
 */

enum ct_status ct_tree_op(
  void *key, 
  void **data,
  enum ct_tree_methods method
){
  struct ct_tree_node *node;
  struct ct_tree_node *parent;
  bool *is_gt;
  enum ct_status status = _ct_find_place(
    NULL, 
    &node,
    &parent,
    &is_gt
  );
  if(status != CT_ERROR){
    if(method == CT_GET && status == CT_FOUND){
      *data = node->data;
      return CT_FOUND;
    }else if(method == CT_SET){
      if(status == NOT_FOUND){
        node->data = *data;
        return CT_MODIFIED;
      }else{
        node = ct_node_init(key, *data, parent);
        *is_gt ? parent->gt = node : parent->lt = node;
        return CT_CREATED
      }
    }else if(method == CT_UNSET){
      /* to fill in */
    }
  }
  return CT_ERROR;
}

enum ct_status _ct_find_place(
  struct ct_node *current, 
  struct ct_node **node, 
  struct ct_node **parent,
  bool **is_gt
){
  *parent *node = NULL;
  while(current != NULL){
    int cmp = tree->cmp(key, current->key);
    if(cmp == 0){
      *node = current;
      return CT_FOUND;
    }else if(cmp < 0){
      *parent = current;
      *is_gt = false;
      current = current->lt;
    }else if(cmp > 0){
      *parent = current;
      *is_gt = true;
      current = current->gt;
    }
  }
  return CT_NOT_FOUND;
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
   node->is_gt = tree->cmp(key, parent->key) > 0;
   node->key = key;
   node->data = data;
   return node;
}

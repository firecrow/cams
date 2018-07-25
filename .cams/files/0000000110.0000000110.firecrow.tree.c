#include <stdbool.h>
#include <stdio.h>
#include "tree.h"
#include "utils.h"


/* tree functions */

struct ct_tree *ct_tree_init(
  int (*cmp)(void *data_a, void *data_b),
  void (*free_node)(struct ct_node *a)
){
  struct ct_tree *tree = dk_malloc(sizeof(struct ct_tree));
  tree->root = NULL;
  tree->len = 0;
  tree->cmp = cmp;
  tree->free_node = free_node;
  return tree;
}

void ct_tree_free(struct ct_tree *tree){
  struct ct_node *node = NULL;
  /*
  while(node = ct_tree_next(tree) != NULL){
    tree->free_node(node);
  }
  */
  dk_free(tree);
}

/* 
 * complexity to manage
 *
 * comparisons: eq, gt, lt 
 * operations: set, get, unset
 * conditions: root is null
 * results: CT_FOUND, CT_NOT_FOUND, CT_CREATED, CT_MODIFIED, CT_ERROR
 *
 */

enum ct_status ct_tree_op(
  struct ct_tree *tree,
  void *key, 
  void **data,
  enum ct_tree_methods method
){
  struct ct_node *node;
  struct ct_node *parent;
  bool *is_gt;
  enum ct_status status = _ct_find_place(
    tree,
    key,
    NULL,
    &node,
    &parent,
    is_gt
  );
  if(status != CT_ERROR){
    if(method == CT_GET && status == CT_FOUND){
      *data = node->data;
      return CT_FOUND;
    }else if(method == CT_SET){
      if(status == CT_FOUND){
        node->data = *data;
        return CT_MODIFIED;
      }else{
        node = ct_node_init(key, *data, parent, *is_gt);
        if(!parent){
          tree->root = node;
        }else{
          if(*is_gt){ 
            parent->gt = node;
          }else{ 
            parent->lt = node;
          }
        }
        return CT_CREATED;
      }
    }else if(method == CT_UNSET){
      /* to fill in */
    }
  }
  return CT_ERROR;
}

enum ct_status _ct_find_place(
  struct ct_tree *tree,
  void *key,
  struct ct_node *current, 
  struct ct_node **node, 
  struct ct_node **parent,
  bool *is_gt
){
  *parent = NULL;
  *node = NULL;
  if(!current){
    current = tree->root;
  }
  while(current != NULL){
    int cmp = tree->cmp(key, current->key);
    if(cmp == 0){
      *node = current;
      *parent = current->parent;
      return CT_FOUND;
    }else{
      *parent = current;
      if(cmp < 0){
        *is_gt = false;
        current = current->lt;
      }else{
        *is_gt = true;
        current = current->gt;
      }
    }
  }
  return CT_NOT_FOUND;
}

/* node functions */

struct ct_node *ct_node_init(
  void *key, 
  void *data, 
  struct ct_node *parent,
  bool is_gt
){
   struct ct_node *node = dk_malloc(sizeof(struct ct_node));
   node->gt = node->lt = NULL;
   node->parent = parent;
   node->is_gt = is_gt;
   node->key = key;
   node->data = data;
   return node;
}

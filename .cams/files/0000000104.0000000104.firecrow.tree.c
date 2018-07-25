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
  return _ct_run_op(
    tree,
    key,
    data,
    method,
    tree->root,
    NULL,
  );
}

/* 
 * complexity to manage
 *
 * comparisons: 0, gt, lt 
 * operations: set, get, unset
 * conditions: root is null
 * results: FOUND, NOT_FOUND, CREATED, MODIFIED, ERROR
 *
 */
enum ct_status _ct_run_op(
  struct ct_tree *tree,
  void *key,
  void **data,
  tree_mothods method,
  struct ct_node *current,
  struct ct_node *node
){
  struct ct_node *prev;
  while(current != NULL){
    struct ct_node *prev = current;
    int cmp = tree->cmp(key, current->key);
    if(cmp == 0){
      if(method == CT_GET){
        *data == current->data;
        return CT_FOUND;
      }else if(method == CT_SET){
        current->data = *data;
        return CT_MODIFIED;
      }else if(method == CT_UNSET){
        if(current->is_gt){
          current->parent->gt = NULL;
        }else{
          current->parent->lt = NULL;
        }
        if(current->gt){
          _ct_run_op(
            tree,
            key,
            data,
            CT_SET,
            current,
            current->gt,
          );
        }
        if(current->lt){
          _ct_run_op(
            tree,
            key,
            data,
            CT_SET,
            current,
            current->lt,
          );
        }
        tree->free_node(current);
      }
    }else if(cmp > 0){
      node = node->gt;
        if(!node){
          if(method == CT_SET){
            prev_node->gt = node || ct_node_init(key, value, prev_node); 
            return CT_CREATED;
          }else{
            return CT_NOT_FOUND;
          }
        }
      }
    }else if(cmp < 0){
      node = node->lt;
      if(!node){
        if(method == CT_SET){
          prev_node->lt = node || ct_node_init(key, value, prev_node); 
          return CT_CREATED;
        }else{
          return CT_NOT_FOUND;
        }
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
   node->is_gt = tree->cmp(key, parent->key) > 0;
   node->key = key;
   node->data = data;
   return node;
}

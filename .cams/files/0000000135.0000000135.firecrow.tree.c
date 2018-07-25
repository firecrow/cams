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

enum ct_status ct_tree_op_existing(
  struct ct_tree *tree,
  struct ct_node *node,
  enum ct_tree_methods method
){
  return _ct_run_op(
    tree,
    NULL,
    NULL,
    node,
    method
  );
}

enum ct_status ct_tree_op(
  struct ct_tree *tree,
  void *key,
  void **data,
  enum ct_tree_methods method
){
  return _ct_run_op(
    tree,
    key,
    data,
    NULL,
    method
  );
}

enum ct_status _ct_run_op(
  struct ct_tree *tree,
  void *key,
  void **data,
  struct ct_node *existing,
  enum ct_tree_methods method
){
  struct ct_node *node;
  struct ct_node *found_node;
  struct ct_node *node_inner;
  struct ct_node *parent;
  enum ct_ineq ineq = CT_LESS;
  enum ct_status status;
  if(existing){
    key = existing->key;
  }

  status = _ct_find_place(
    tree,
    key,
    NULL,
    &found_node,
    &parent,
    &ineq
  );
  if(status != CT_ERROR){
    if(method == CT_GET && status == CT_FOUND){
      *data = found_node->data;
      return CT_FOUND;
    }else if(method == CT_SET){
      if(status == CT_FOUND){
        found_node->data = *data;
        return CT_MODIFIED;
      }else{
        if(existing){
          node = existing;
        }else {
          node = ct_node_init(key, *data, parent, ineq);
        }
        if(!tree->root){
          tree->root = node;
          node->is_red = false;
        }else if(parent){
          parent->children[ineq] = node;
        }else{
          return CT_ERROR;
        }
        balance(tree, node);
        return CT_CREATED;
      }
    }else if(method == CT_UNSET){
      if(status == CT_FOUND){
          found_node->parent->children[ineq] = NULL;
        if(found_node->children[CT_LESS]){
          return ct_tree_op_existing(
            tree,
            found_node->children[CT_LESS],
            CT_SET
          );
        }
        if(found_node->children[CT_MORE]){
          return ct_tree_op_existing(
            tree,
            found_node->children[CT_MORE],
            CT_SET
          );
        }
        return CT_MODIFIED;
      }else{
        return CT_NOT_FOUND;
      }
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
  enum ct_ineq *ineq 
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
      *ineq = cmp < 0;
      current = current->children[*ineq];
    }
  }
  return CT_NOT_FOUND;
}

/* node functions */

struct ct_node *ct_node_init(
  void *key, 
  void *data, 
  struct ct_node *parent,
  enum ct_ineq ineq 
){
   struct ct_node *node = dk_malloc(sizeof(struct ct_node));
   node->children[CT_MORE] = NULL;
   node->children[CT_LESS] = NULL;
   node->parent = parent;
   node->ineq = ineq;
   node->key = key;
   node->data = data;
   node->is_red = true;
   return node;
}

enum ct_status balance(struct ct_tree *tree, struct ct_node *node){
  if(node->is_red && node->parent && node->parent->is_red){
    struct ct_node *grampa;
    struct ct_node *great_grampa;
    struct ct_node *uncle;
    struct ct_node *parent;

    parent = node->parent;
    enum ct_ineq parent_was_ineq = parent->ineq;
    if(!parent){
      return CT_NOOP;
    }
    grampa = parent->parent;
    if(!grampa){
      return CT_NOOP;
    }
    great_grampa = grampa->parent;
    if(!great_grampa){
      return CT_NOOP;
    }
    uncle = grampa->children[!parent_was_ineq];

    if(uncle && uncle->is_red){
      uncle->is_red = false;
      parent->is_red = false;
    }else{
      if(node->ineq != parent->ineq){
        swap(parent, node);
        swap(grampa, node);
        parent->is_red = true;
        grampa->is_red = true;
        node->is_red = false;
      }else{
        swap(grampa, parent);
        parent->is_red = false;
        grampa->is_red = true;
      }
    }
  }
  return CT_BALANCED;
}

enum ct_status swap(struct ct_node *parent, struct ct_node *child){
   enum ct_ineq ineq = child->ineq;
   /* 
    * move the child of the child (baby) from the to the child
    * to be a child of the parent, where the child connection 
    * used to be
    */
   struct ct_node *baby = child->children[!ineq];
   if(baby){
     parent->children[ineq] = baby; 
     baby->parent = parent;
     baby->ineq = ineq;
   }
   /* 
    * move the child to where the parent used to be
    * move the parent to the opposite ineq of 
    * where the child used to be
    */
   child->children[!ineq] = parent;
   child->parent = parent->parent;
   child->ineq = parent->ineq;
   parent->parent->children[parent->ineq] = child;
   parent->parent = child;
   parent->ineq = !ineq;
   parent->children[ineq] = NULL;

   return CT_MODIFIED;
}

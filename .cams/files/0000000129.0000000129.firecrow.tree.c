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
  bool is_gt = false;
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
    &is_gt
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
          node = ct_node_init(key, *data, parent, is_gt);
        }
        if(!tree->root){
          tree->root = node;
          node->is_red = false;
        }else if(parent){
          if(is_gt){ 
            parent->gt = node;
          }else{ 
            parent->lt = node;
          }
        }else{
          return CT_ERROR;
        }
        balance(tree, node);
        return CT_CREATED;
      }
    }else if(method == CT_UNSET){
      if(status == CT_FOUND){
        if(is_gt){
          found_node->parent->gt = NULL;
        }else{
          found_node->parent->lt = NULL;
        }
        if(found_node->lt){
          return ct_tree_op_existing(
            tree,
            found_node->lt,
            CT_SET
          );
        }
        if(found_node->gt){
          return ct_tree_op_existing(
            tree,
            found_node->gt,
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
   node->gt = NULL;
   node->lt = NULL;
   node->parent = parent;
   node->is_gt = is_gt;
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

    bool parent_was_gt;
    parent = node->parent;
    parent_was_gt = parent->is_gt;
    grampa = parent->parent;
    if(!grampa){
      return CT_NOOP;
    }
    great_grampa = grampa->parent;
    if(!great_grampa){
      return CT_NOOP;
    }
    if(grampa){
      if(parent->is_gt){
        uncle = grampa->lt;
      }else{
        uncle = grampa->gt;
      }
    }

    if(uncle && uncle->is_red){
      uncle->is_red = false;
      parent->is_red = false;
    }else{
      if(node->is_gt != parent->is_gt){
        swap(parent, node);
        rotate(grampa, parent->is_gt);
        parent->is_red = true;
        grampa->is_red = true;
      }else{
        rotate(grampa, parent->is_gt);
        parent->is_red = false;
        grampa->is_red = true;
      }
    }
  }
  return CT_BALANCED;
}

void swap(struct ct_node *parent, struct ct_node *child){
  /* 
   * swap the parent to the child position 
   * move the appropriate children under the new node
   */
   child->parent = parent->parent;
   parent->is_gt = !parent->is_gt;
   if(child->is_gt){
     child->gt = pareht;
     if(parent->lt){
       parent->lt->parent = child;
       child->lt = parent->lt;
     }
   }else{
     child->lt = pareht;
     if(parent->gt){
       parent->gt->parent = child;
       child->gt = parent->gt;
     }
   }
   child->is_gt = parent->is_gt;
}

void rotate(struct ct_node *grampa, bool is_right){

  great_grampa->children[grampa->gender] = parent;
  parent->is_gt = grampa->gender;
  parent->parent = great_grampa;
  grampa->parent = parent;
  enum ct_genders gender = node->gender;

  parent->children[CT_NOT_GD(gender)] = grampa;
  grampa->gender = CT_NOT_GD(gender);

  grampa->[parent->gender] = NULL;
}

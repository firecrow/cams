/* depends
 * stdbool.h
 */

enum ct_tree_methods {
  CT_GET = 1,
  CT_SET,
  CT_UNSET,
  CT_NEXT
};

enum ct_genders {
  CT_GREATER = 0,
  CT_LESS
};

enum ct_status {
  CT_ERROR = 0,
  CT_FOUND,
  CT_NOT_FOUND,
  CT_CREATED,
  CT_MODIFIED,
  CT_BALANCED,
  CT_NOOP
};

#define CT_NOT_GD(x) x == CT_GREATER ? CT_LESS : CT_GREATER;

struct ct_position {
  struct ct_node *node;
  struct ct_node *parent;
  enum ct_genders gender;
  enum ct_status status;
};

struct ct_tree {
  struct ct_node *root;  
  size_t len;
  int (*cmp)(void *data_a, void *data_b);
  void (*free_node)(struct ct_node *a);
};

struct ct_node {
  struct ct_node *parent;
  struct ct_node children[2];
  void *key;
  void *data;
  bool is_gt;
  bool is_red;
};

/* tree functions */
struct ct_tree *ct_tree_init(
  int (*cmp)(void *data_a, void *data_b),
  void (*free_node)(struct ct_node *a)
);

void ct_tree_free();

enum ct_status ct_tree_op(
  struct ct_tree *tree,
  void *key, 
  void **data,
  enum ct_tree_methods method
);

enum ct_status ct_tree_op_existing(
  struct ct_tree *tree,
  struct ct_node *node,
  enum ct_tree_methods method
);


/* node functions */

struct ct_node *ct_node_init(
  void *key, 
  void *data, 
  struct ct_node *parent,
  bool is_gt
);


/* internal functions */

enum ct_status _ct_run_op(
  struct ct_tree *tree,
  void *key,
  void **data,
  struct ct_node *existing,
  enum ct_tree_methods method
);

enum ct_status _ct_find_place(
  struct ct_tree *tree,
  void *key,
  struct ct_node *current, 
  struct ct_node **node, 
  struct ct_node **parent,
  bool *is_gt
);

void swap(struct ct_node *parent, struct ct_node *child);
void rotate(struct ct_node *grampa, bool is_right);

enum ct_status balance(struct ct_tree *tree, struct ct_node *node);


struct ct_node {
   struct ct_node *lt;
   struct ct_node *gt;
   bool ct_node is_gt;
   void *key;
   void *data;
};

enum tree_methods {
  CT_GET = 1,
  CT_SET,
  CT_UNSET
  CT_NEXT
};

enum ct_status {
  CT_NOOP = 1,
  CT_FOUND,
  CT_NOT_FOUND,
  CT_CREATED,
  CT_ERROR,
};

struct ct_tree {
  struct tree_node *root;  
  struct ct_node *current;
  size_t len;
  int (*cmp)(void *dataa, void *datab),
  int (*free_node)(struct tree_node *a);
};

/* tree functions */
struct ct_tree *ct_tree_init(
  int (*cmp)(void *dataa, void *datab),
  int (*free_node)(struct tree_node *a)
);
void ct_tree_free();

struct ct_node *ct_tree_op(
  struct ct_tree *tree,
  void *key,
  void **data,
  tree_mothods method
);

/* node functions */
struct ct_node *ct_node_init(
  void *key, 
  void *data, 
  struct ct_node *parent
);

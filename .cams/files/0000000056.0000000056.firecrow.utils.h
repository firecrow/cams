struct tree_node {
   struct tree_node *lt; 
   struct tree_node *gt; 
   struct tree_node *parent; 
   char *key;
   void *data;
};

struct tree {
  struct tree_node *root;  
  int (*cmp)(struct tree_node *a, struct tree_node *b);
};

char *fmtmem(char *fmt, ...);

int dkpipe(char *cmd, char **args, int in[2], int out[2]);

void *dk_malloc(size_t size);
void dk_free(void *mem);

int dk_count();

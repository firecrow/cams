struct tree_node {
   struct tree_node *lt; 
   struct tree_node *gt; 
   struct tree_node *parent; 
   char *key;
   void *data;
}

struct tree {
  struct tree_node *root;  
  int (*cmp)(struct tree_node *a, struct tree_node *b);
}

char *fmtmem(char *fmt, arg...){
  int l;
  char c[1];
  char *str;
  va_list va1;
  va_list va2;
  va_start(va1, arg);
  va_copy(va2, va1);
  l = vsnprintf(c, 1,fmt, va1);
  str = malloc(sizeof(char)*(l+1));
  vsnprintf(str, l+1,fmt, va2);
  return str; 	 
}

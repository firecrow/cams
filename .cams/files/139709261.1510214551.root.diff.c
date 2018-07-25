#include "cams.h"

static struct ct_key_data *cindex_tree_merge(void *key, void *a, void *b){

  struct ent *cur_a = (struct ent *)a;
  struct ent *cur_b = (struct ent *)b;
  struct ct_key_data *kv = dk_malloc(sizeof(struct ct_key_data));
  if(!strcmp(cur_a->cid, cur_b->cid)){
    /*
    ent_free(cur_a);
    ent_free(cur_b);
    */
    kv->key = NULL;
    kv->data = NULL;
    return kv;
  }
  struct ent *cur = ent_init(dupstr(key), NULL);
  cur->cid = dupstr(cur_a->cid);
  cur->prior = dupstr(cur_b->cid);
  kv->key = cur->path;
  kv->data = cur;
  
  /*
  ent_free(cur_a);
  ent_free(cur_b);
  */
  return kv;
}

static struct ct_tree *diff_list_staged(){
  struct ct_tree *files = flist();
  struct ct_tree *from_files =  cindex_to_tree(get_current());
  struct ct_key_data kv = {NULL, NULL};
  struct ent *cur;
  struct ent *prior;
  while(!ct_tree_next(files, &kv)){
    cur = (struct ent *)kv.data;
    if(!ct_tree_get(from_files, &kv)){
      prior = (struct ent *)kv.data;
      cur->prior = prior->cid;
    }
  }
  return files;
}

static struct ct_tree *diff_list(char *from, char *to){
  struct ct_tree *from_files = cindex_to_tree(from);
  from_files->free = NULL;
  struct ct_tree *to_files = cindex_to_tree(to);
  to_files->free = NULL;

  struct ct_tree *files = ent_tree_init();
  struct ct_key_data kv = {NULL, NULL};
  struct ent *cur;

  struct ct_tree_xresult *xres = ct_tree_intersection(from_files, to_files, cindex_tree_merge);

  ct_tree_free(from_files);
  ct_tree_free(to_files);

  if(xres->a->len){
    while(!ct_tree_next(xres->a, &kv)){
      struct ent *cur_a = (struct ent *)kv.data;
      ct_tree_set(files,  &kv);
    }
  }
  if(xres->b->len){
    kv.key = NULL;
    while(!ct_tree_next(xres->b, &kv)){
      cur = (struct ent *)kv.data;
      cur->prior = cur->cid;
      cur->cid = NULL;
      ct_tree_set(files,  &kv);
    }
  }
  if(xres->both->len){
    kv.key = NULL;
    while(!ct_tree_next(xres->both, &kv)){
      ct_tree_set(files,  &kv);
    }
  }
  xres->a->free = NULL;
  xres->b->free = NULL;
  xres->both->free = NULL;
  ct_tree_free(xres->a);
  ct_tree_free(xres->b);
  ct_tree_free(xres->both);
  dk_free(xres);

  return files;
}

int diff(int argc, char **argv, struct intls *intls){
  char *cid = get_current();
  struct stbuckets *stb = gen_stbuckets(cid);
  struct ct_key_data kv = {NULL, NULL};
  char *cur_path;

  char *cmd = "diff";
  char *args[] = {
    "diff",
    NULL,
    NULL,
    NULL
  };

  if(stb->staged->len){
    kv.key = NULL;
    while(!ct_tree_next(stb->staged, &kv)){

    }
  }

  if(stb->removed->len){
    kv.key = NULL;
    while(!ct_tree_next(stb->removed, &kv)){

    }
  }

  if(stb->modified->len){
    kv.key = NULL;
    while(!ct_tree_next(stb->modified, &kv)){
      struct ent *cur = (struct ent *)kv.data;
      cur_path = dk_fmtmem(".cams/files/%s.%s", cur->cid, cur->spath);
      printf("----------> [%s] <----------\n", cur->path);
      args[1] = cur_path;
      args[2] = cur->path;
      ct_exec(cmd, args);
      free(cur_path);
    }
  }
  return 0;
}

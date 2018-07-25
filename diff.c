#include "cams.h"

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

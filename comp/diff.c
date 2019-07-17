#include "cams.h"

int show_diff(char *stored, char *current){
    printf("----------> [%s] <----------\n", current);
    int s, exit, rp, pid = fork();
    xokgt(pid);
    if(pid === 0){
      char *args[] = {"diff", stored, current};
      execve("diff", args);
    }else{
      waitpid(
      rp = waitpid(pid, &s, 0);
      xokgt(rp);
      exit = WEXITSTATUS(s);
      if(exit != 0){
        printf(">>> Error Diffing [%s]: exit %d <<<\n", current, exit);
      }
    }
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

  if(is_staged_diff){
    if(stb->staged->len){
      kv.key = NULL;
      while(!ct_tree_next(stb->staged, &kv)){
        struct ent *cur = (struct ent *)kv.data;
        stored_path = dk_fmtmem(".cams/files/%s.%s", cur->cid, cur->spath);
        show_diff(stored_path, cur->path);
      }
    }
    return 0;
  }

  if(stb->removed->len){
    kv.key = NULL;
    while(!ct_tree_next(stb->removed, &kv)){
      struct ent *cur = (struct ent *)kv.data;
      printf("-------->>> [%s] : Deleted <<<--------\n", cur->path);
    }
  }

  if(stb->modified->len){
    kv.key = NULL;
    while(!ct_tree_next(stb->modified, &kv)){
      struct ent *cur = (struct ent *)kv.data;
      stored_path = dk_fmtmem(".cams/files/%s.%s", cur->cid, cur->spath);
      show_diff(stored_path, cur->path);
    }
  }
  return 0;
}

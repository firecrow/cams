#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <crowtils.h>
#include "crowtree/tree.h"
#include "cams.h"

char *dupstr(char *str){
  size_t len = strlen(str);
  char *dest = dk_malloc(len+1);
  memcpy(dest, str, len+1);
  return dest;
}

char *dupnstr(char *str, int len){
  char *dest = dk_malloc(len+1);
  memcpy(dest, str, len);
  dest[len] = '\0';
  return dest;
}

int flen(char *path){
  FILE *file = dk_open(path, "r");
  fseek(file, 0, SEEK_END);
  int len = ftell(file);
  fseek(file, 0, SEEK_SET);
  fclose(file);
}

int fexists(char *path){
  return (access(path, F_OK) != -1);
}

bool feq(char *path_a, char *path_b){
  char buff_a[1025];
  char buff_b[1025];
  FILE *a = dk_open(path_a, "r");
  FILE *b = dk_open(path_b, "r");
  fseek(a, 0, SEEK_END);
  fseek(a, 0, SEEK_END);
  if(ftell(a) != ftell(b)){
    return false;
  }
  rewind(a);
  rewind(b);
  while(fread(buff_a, 1, 1024, a) != 0){
    fread(buff_b, 1, 1024, b);
    if(strncmp(buff_a, buff_b, 1024)){
      return false;
    }
  }
  return true;
}

char *ct_fread(char *path, int size){
  if(!fexists(path)){
    return NULL;
  }
  FILE *file = dk_open(path, "r");
  char *str = dk_malloc(size);
  int l = fread(str, 1, size, file);
  str[l] = '\0';
  return str;
}

char *hexstr(char *bytes, int len){
  char *ret = dk_malloc(sizeof(char)*(len*2+1));
  char *b;
  char *s = ret;
  while(len-- > 0){
    b = bytes+len;
    snprintf(s, 3, "%02x", *b);
    s+=2;
  }
  return ret;
}

int localize_fname(char *fname){
  int count = 0;
  char *p = fname;
  while(*fname != '\0'){
    if(*fname == '.' && *fname+1 == '.'){
      fname += 2;
    }
    *p++ = *fname++;
  }
  *fname = '\0';
  return count;
}

int ct_split(char *str, char c, int limit, char ***ret){
  int count = 0;
  int len;
  int i;
  char *current = str;
  int curlen = 0;
  char *new;
  char *ptr = str;
  char *lptr = str;
  struct ct_tree *tree = ct_tree_int_init();
  struct ct_key_data kv = {NULL, NULL};
  char **_ret;
  while(true){
    if(*ptr == '\0' || (*ptr == c && (!limit || (count < limit)))){
      new = dk_malloc(curlen+1);
      memcpy(new, lptr, curlen);
      new[curlen] = '\0';
      kv.key = (void *) count+1;/* this has to be base 1 because of NULL checks */
      kv.data = new;
      ct_tree_set(tree, &kv);
      count++;
      if(*ptr == c){
        lptr += curlen+1;
        ptr = lptr;
        curlen = 0;
      }else{
        break;
      }
    }else{
      curlen++;
      ptr++;
    }
  }
  len = tree->len;
  _ret = dk_malloc(sizeof(char *)*(len+1)); 
  _ret[len] = NULL;
  i = 0;
  kv.key = NULL;
  while(!ct_tree_next(tree, &kv)){
    _ret[i++] = kv.data;
  }
  *ret = _ret;
  tree->free = NULL;
  ct_tree_free(tree);
  return len;
}

void trimnl(char *str){
  int target = strlen(str)-1;
  if(str[target] == '\n'){
    str[target] = '\0';
  }
}

int ct_subp(struct ct_subp *subp){
  int status;

  if(subp->flags & CT_USE_STDIN){
    pipe(subp->ins);
    subp->stdin = fdopen(subp->ins[1], "w");
  }
  if(subp->flags & CT_USE_STDOUT){
    pipe(subp->outs);
    subp->stdout = fdopen(subp->outs[0], "r");
  }
  if(subp->flags & CT_USE_STDERR){
    pipe(subp->errs);
    subp->stderr = fdopen(subp->errs[0], "r");
  }

  int pid = fork();
  if(pid == -1){
    fprintf(stderr, "forking error\n");
    exit(123);
  }else if(pid != 0){
    subp->pid = pid;
    if(subp->flags & CT_USE_STDIN) close(subp->ins[0]);
    if(subp->flags & CT_USE_STDOUT) close(subp->outs[1]);
    if(subp->flags & CT_USE_STDERR) close(subp->errs[1]);
    if(subp->flags & CT_SUBP_ASYNC){
      return ct_subpwait(subp);
    }else{
      waitpid(subp->pid, &status, 0);
      subp->ret = WEXITSTATUS(status);
      return 1;
    }
  }else{
    if(subp->flags & CT_USE_STDIN) dup2(subp->ins[0], 0);
    if(subp->flags & CT_USE_STDOUT) dup2(subp->outs[1], 1);
    if(subp->flags & CT_USE_STDERR) dup2(subp->errs[1], 2);

    if(subp->flags & CT_USE_STDIN) close(subp->ins[0]);
    if(subp->flags & CT_USE_STDOUT) close(subp->outs[1]);
    if(subp->flags & CT_USE_STDERR) close(subp->errs[1]);

    execvp(subp->cmd, subp->argv);
    exit(7);
  }
}

int ct_subpwait(struct ct_subp *subp){
  int exited;
  int status;
  exited = waitpid(subp->pid, &status, WNOHANG);
  if(exited != 0){
    if(WIFEXITED(status)){
      subp->ret = WEXITSTATUS(status);
      return 1;
    }
  }
  return exited;
}

struct ct_strbuff *ct_strbuff_init(){
  struct ct_strbuff *buff = dk_malloc(sizeof(struct ct_strbuff));
  buff->len = 0;
  buff->size = 64;
  buff->str = dk_malloc(buff->size);
}

struct ct_strbuff *ct_strbuff_free(struct ct_strbuff *buff){
  dk_free(buff->str);
  dk_free(buff);
}

int ct_strbuff_push(struct ct_strbuff *buff, char *str, size_t len){
  int newsize = buff->len+len+1;
  int bsize = buff->size;
  if(newsize > bsize){
    while(bsize < newsize){
      bsize *= 2;
    }
    buff->str = realloc(buff->str, bsize); 
  }
  memcpy(buff->str+buff->len, str, len);
  buff->str[buff->len+len] = '\0';
  buff->size = bsize;
  buff->len += len;
  return 0;
}

int ct_strbuff_shift(struct ct_strbuff *buff, size_t len, char **dest){
  if(len > buff->len){
    return 1;
  }
  *dest = dk_malloc(len+1);
  memcpy(*dest, buff->str, len);
  (*dest)[len] = '\0';
  int remaining_len = buff->len-len;
  memmove(buff->str, buff->str+len, remaining_len);
  buff->str[remaining_len] = '\0';
  buff->len = remaining_len;
  return 0;
}



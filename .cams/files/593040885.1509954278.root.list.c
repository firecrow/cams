#include "cams.h"

int list(int argc, char **argv, struct intls *intls){
  char *cid;
  char *filestr;
  char *cfilestr;
  char *rmfilestr;
  int count = 0;
  int qty = 0;
  int cols;
  char *buff;
  struct opts *opts = getopts(argc, argv);
  struct opt_value *optval;

  bool fit = (find_opt(opts, 'f', "fit")) ? true : false;
  if(fit){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    qty = w.ws_row-1;
    cols = w.ws_col;
    cols += 9*2;/*color code characters*/
    buff = dk_malloc(cols);
  }else{
    optval = find_opt(opts, 'q', "quantity");
    if(optval && optval->value){
      if(!strcmp(optval->value, "all")){
        qty = 0;
      }else{
        qty = atoi(optval->value);
      }
    }
  }

  if(opts->argc > 2){
    cid = lookup_or_die(opts->argv[1]);
  }else{
    cid = get_current();
  }
  while(cid){
    char *idxfname = dk_fmtmem(".cams/%s/cindex", cid);
    FILE *idxf = dk_open(idxfname, "r");
    char *hash = dk_malloc(64+1);
    int ret = ct_filehash(idxf, hash);
    hash[16] = '\0';
    struct commit *com = commit_init(cid);
    trimnl(com->message);
    struct ct_tree *ctree = cfiles(com->cid);
    cfilestr = ct_tree_alpha_join(ctree, ", ");
    struct ct_tree *rmtree = rmlist(com->cid);
    rmfilestr = dk_fmtmem("-%s", ct_tree_alpha_join(rmtree, ", -"));
    if(ctree->len && rmtree->len){
      filestr = dk_fmtmem("%s, %s", cfilestr, rmfilestr);
    }else if(rmtree->len){
      filestr = rmfilestr; 
    }else{
      filestr = cfilestr; 
    }
    if(fit){
      if(snprintf(buff, cols, "\033[35m%s\033[0m|\033[33m%09ld\033[0m: %s (%s)\n", hash, com->time.tv_nsec, com->message, filestr) > cols){
        buff[cols-3] = buff[cols-2] = buff[cols-1] = '.';
        buff[cols] = '\n';
      }
      printf("%s", buff);
    }else{
      /*printf( "%s|%09ld: %s (%s)\n", hash, com->time.tv_nsec, com->message, filestr);*/
      printf( "%09ld: %s (%s)\n", com->time.tv_nsec, com->message, filestr);
    }
    if(filestr != NULL){
      dk_free(filestr);
    }
    if(qty && ++count == qty){
      break;
    };
    cid = com->parent;
  }
  return 0;
}

long ts_delta(struct timespec *a, struct timespec *b){
  if(a->tv_sec != b->tv_sec){
    return b->tv_sec - a->tv_sec;
  }else{
    return (b->tv_nsec < a->tv_nsec ? 1 : -1);
  }
}

/*
int
is_modified (struct ent *cur, struct tm *cid_time )
{
  struct stat stg_stat;
  struct stat cur_stat;
  struct tm *compare_time = NULL;
  struct tm *cur_time;
  if(stat (cur->path, &cur_stat) != 0) {
    exit(errno);
  }
  cur_time = cur_stat.mtim;
  char *staged_path = dk_fmtmem(".cams/stage/files/%s", cur->path);
  if (ct_fexists(staged_path) {
      if(stat (staged_path, &stg_stat) != 0) {
        exit(errno);
      }
      comapre_time = stg_stat.mtim;
    }
  }
  if(compare_time == NULL){
    compare_time = cid_time;
  }
  if (ts_delta(compare_time, cur_time) > 0){
    return true;
  }

  // has compare and returns
}
*/


int status(int argc, char **argv, struct intls *intls){
  struct ct_key_data kv = {NULL, NULL};
  struct ct_tree *staged = flist();
  struct ct_tree *removed = rmlist(NULL);
  struct ct_tree *untracked = slist();
  char *cid = get_current();
  struct ct_tree *cindex = cindex_to_tree(cid);
  staged->free = NULL;
  untracked->free = NULL;
  cindex->free = NULL;
  struct timespec cindex_time;
  struct stat file_stat;
  struct stat staged_stat;
  bool printed = false;
  bool show_mod;
  char *staged_path;
  struct stat ost;
  struct stat wst;

  if(staged->len){
    printf("\033[34m--- staged files ---\033[0m\n");
    kv.key = NULL;
    while(!ct_tree_next(staged, &kv)){
      printf("%s\n", kv.key);
      ct_tree_unset(untracked, &kv);
    }
  }

  if(removed->len){
    printf("\033[34m--- staged for removal ---\033[0m\n");
    kv.key = NULL;
    while(!ct_tree_next(removed, &kv)){
      printf("%s\n", kv.key);
      ct_tree_unset(untracked, &kv);
    }
  }

  if(cindex->len){
    kv.key = NULL;
    while(!ct_tree_next(cindex, &kv)){
      ct_tree_unset(untracked, &kv);
      struct ent *cur = kv.data;
      parse_cid(cur->cid, &cindex_time, NULL);
      if (stat(cur->path, &file_stat) != 0 ){
        fprintf(stderr, "error with stat for current item '%s' %d\n", cur->path, errno); 
      };
      staged_path = dk_fmtmem(".cams/stage/files/%s", cur->path);
      if(stat(staged_path, &staged_stat) != -1){
        show_mod = ts_delta(&staged_stat.st_mtim, &file_stat.st_mtim) > 0;
      }else{
        show_mod = ts_delta(&cindex_time, &file_stat.st_ctim) > 0;
      }
      /*
      fprintf(stderr, "%s\n", cur->path);
      fprintf(stderr, "wks:%ld %ld\n", file_stat.st_mtim.tv_sec, file_stat.st_mtim.tv_nsec);
      fprintf(stderr, "cid:%ld %ld\n", cindex_time.tv_sec, cindex_time.tv_nsec);
      fprintf(stderr, "<>:%ld\n", file_stat.st_mtime);
      fprintf(stderr, "----------------------");
      */
      if(show_mod){
        if(!printed){
          printf("\033[34m--- modified files ---\033[0m\n");
          printed = true;
        }
        printf("%s\n", kv.key);
      }
    }
  }

  if(untracked->len){
    printf("\033[34m--- untracked files ---\033[0m\n");
    /*
    stat(ofname, &cst);
    stat(ofname, &wst);
    orig_atime = cst.st_atime;
    working_mtime = wst.mtime;
    if(working_mtime < orig_atime){

    }
    */
    kv.key = NULL;
    while(!ct_tree_next(untracked, &kv)){
      printf("%s\n", kv.key);
    }
  }

  /*
  ct_tree_free(cindex);
  ct_tree_free(staged);
  dk_free(cid);
  */
  return 0;
}

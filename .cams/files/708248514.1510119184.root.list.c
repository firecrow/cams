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

bool
is_modified (struct ent *cur)
{
  struct stat stg_stat;
  struct stat cur_stat;
  struct timespec cid_time;
  char *staged_path;
  char *compare_path;
  int show_mod = 0;
  char cur_hash[64+1];
  char wrk_hash[64+1];
  bool is_staged = true; 
  bool is_mod;
  FILE *cur_file;
  FILE *wrk_file;
  FILE *compare;
  int wcc;
  bool ret;

  bzero(cur_hash, 64+1);
  bzero(wrk_hash, 64+1);
  bzero(&stg_stat, sizeof(struct stat));
  bzero(&cur_stat, sizeof(struct stat));
  bzero(&cid_time, sizeof(struct timespec));

  staged_path = dk_fmtmem(".cams/stage/files/%s", cur->path);
  parse_cid(cur->cid, &cid_time, NULL);
  if (stat(cur->path, &cur_stat) != 0 ){
    fprintf(stderr, "error with stat for current item '%s' %d\n", cur->path, errno); 
    return (errno);
  }
  if (stat(staged_path, &stg_stat) != 0) {
    is_staged = false;
  }
  if (is_staged) {
    is_mod = ts_delta(&stg_stat.st_mtim, &cur_stat.st_mtim) > 0;
  }else{
    is_mod = ts_delta(&cid_time, &cur_stat.st_ctim) > 0;
  }

  if(is_mod){
    if (!ct_fexists(cur->path)) {
      wrk_file = dk_open(cur->path, "r");
      ct_filehash(wrk_file, wrk_hash);
    }
    if(is_staged){
      compare_path = staged_path;
    }else{
      compare_path = dk_fmtmem(".cams/files/%s.%s", cur->cid, cur->spath);
    }
    if (!ct_fexists(compare_path)) {
      cur_file = dk_open(compare_path, "r");
      int wcc = ct_filehash(cur_file, cur_hash);
      ret = ct_fcompare(wrk_file, wcc, cur_hash) != 0;
      fclose(cur_file);
      fclose(wrk_file);
      return ret;
    }
    return true;
  }
  return false;
}


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
      if(is_modified(cur)){
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

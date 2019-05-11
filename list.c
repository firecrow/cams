int list(){
  char *cid;
  char *filestr;
  char *cfilestr;
  char *rmfilestr;
  int count = 0;
  int qty = 0;
  int cols;
  char *buff;

  /* bool fit = (find_opt(opts, 'f', "fit")) ? true : false;*/
  bool fit = true;
  if(fit){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    qty = w.ws_row-1;
    cols = w.ws_col;
    cols += 9*2;/*color code characters*/
    buff = dk_malloc(cols);
  }else{
    qty = 10;
  }

  cid = get_current();
  while(cid){
    struct commit *com = commit_init(cid);
    struct tm *_t = localtime(&(com->time.tv_sec));
    char tbuff[256];
    strftime(tbuff, 256, "%a %Y-%m-%d %H:%M", _t); 

    char *time = ctime(&(com->time.tv_sec));
    trimnl(time);

    struct ct_tree *ctree = cfiles(com->cid);
    char *cfilestr = cfiles_join(ctree);

    printf("cid:%s %ld/%ld %s %s: %s\n", cid, com->time.tv_sec, com->time.tv_nsec, tbuff, com->name, com->message);
    printf("cfiles: %s\n", cfilestr);

    if(qty && (count+=2) == qty){
      break;
    };
    cid = com->parent;
  }
  return 0;
}

struct stbuckets * gen_stbuckets(char *cid){
  struct ct_tree *untracked = slist();
  struct ct_tree *staged = flist();
  struct ct_tree *cindex = cindex_to_tree(cid);
  struct ct_tree *removed = ct_tree_alpha_init();
  struct ct_tree *modified = ct_tree_alpha_init();
  struct ct_leaf kv = {NULL, NULL};

  struct stbuckets *buckets = dk_malloc(sizeof(struct stbuckets));

  if(staged->len){
    kv.key = NULL;
    while(!ct_tree_next(staged, &kv)){
      ct_tree_unset(untracked, &kv);
    }
  }

  if(cindex->len){
    kv.key = NULL;
    while(!ct_tree_next(cindex, &kv)){
      ct_tree_unset(untracked, &kv);
      struct ent *cur = kv.data;
      if (ct_fexists(cur->path)) {
        kv.key = cur->path;
        kv.data = cur;
        ct_tree_set(removed, &kv);
      }else if (is_modified(cur)) {
        kv.key = cur->path;
        kv.data = cur;
        ct_tree_set(modified, &kv);
      }
    }
  }
  buckets->staged = staged;
  buckets->removed = removed;
  buckets->modified = modified;
  buckets->untracked = untracked;
  return buckets;
}

int status(int argc, char **argv, struct intls *intls){
  char *cid = get_current();
  struct stbuckets *stb = gen_stbuckets(cid);
  struct ct_leaf kv = {NULL, NULL};


  if(stb->staged->len){
    printf("\033[34m--- staged files ---\033[0m\n");
    kv.key = NULL;
    while(!ct_tree_next(stb->staged, &kv)){
      printf("%s\n", kv.key);
    }
  }

  if(stb->removed->len){
    printf("\033[34m--- staged for removal ---\033[0m\n");
    kv.key = NULL;
    while(!ct_tree_next(stb->removed, &kv)){
      printf("%s\n", kv.key);
    }
  }

  if(stb->modified->len){
    printf("\033[34m--- modified ---\033[0m\n");
    kv.key = NULL;
    while(!ct_tree_next(stb->modified, &kv)){
      printf("%s\n", kv.key);
    }
  }

  if(stb->untracked->len){
    printf("\033[34m--- untracked files ---\033[0m\n");
    kv.key = NULL;
    while(!ct_tree_next(stb->untracked, &kv)){
      printf("%s\n", kv.key);
    }
  }

  return 0;
}

struct commit {
  char *cid;
  char *parent;
  struct timespec time;
  char *name;
  char *message;
};

char *get_current(){
  return ct_fread(".cams/current");
}

void parse_cid(char *_cid, struct commit *com){
  char *cid = dupstr(_cid);
  int i = 0;
  char *p = cid;
  char *pp = cid;
  int in_nsec = 1;

  
  while(*p != '\0'){
    if(*p == '.'){
      if(in_nsec){
        *p = '\0';
        com->time.tv_nsec = atoi(pp);
        pp = p+1;
        in_nsec = 0;
      }else{
        *p = '\0';
        com->time.tv_sec = atoi(pp);
        pp = p+1;
        com->name = dk_fmtmem("%s", pp);
      }
    }
    p++;
  }
}

struct commit *commit_init(char *cid){
  struct commit *com = dk_malloc(sizeof(struct commit));
  bzero(com, sizeof(struct commit));
  char *basedir = dk_fmtmem(".cams/%s", cid);
  char *path;

  com->cid = cid;
  parse_cid(cid, com);

  path = dk_fmtmem("%s/%s", basedir, "parent");
  if(!ct_fexists(path)){
    com->parent = ct_fread(path);
  }

  path = dk_fmtmem("%s/%s", basedir, "message");
  com->message = ct_fread(path);

  dk_free(basedir);
  return com;
}

/*
int show_commit(int argc, char **argv, struct intls *intls){
  char *cid = argv[2];
  struct commit *com = commit_init(cid);
  struct ct_leaf kv = {NULL, 0, NULL};
  while(!ct_tree_next(cfiles(com->cid), &kv)){
    struct ent *cur = (struct ent *)kv.data;
    fprintf(stderr, "  %s:%s\n", kv.key, cur->cid);
  }
  fprintf(stderr, "all:\n");
  kv.key = NULL;
  while(!ct_tree_next(cindex_to_tree(com->cid), &kv)){
    struct ent *cur = (struct ent *)kv.data;
    fprintf(stderr, "  %s:%s\n", kv.key, cur->cid);
  }
  fprintf(stderr, "removed:\n");
  kv.key = NULL;
  while(!ct_tree_next(rmlist(com->cid), &kv)){
    fprintf(stderr, "  %s\n", kv.key);
  }
  return 0;
}
*/

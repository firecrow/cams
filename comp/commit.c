char *get_current(){
  return ct_fread(".cams/current");
}

void parse_cid(char *_cid, struct timespec *time, char **name){
  char *cid = dupstr(_cid);
  int i = 0;
  char *p = cid;
  char *pp = cid;
  int in_nsec = 1;

  
  while(*p != '\0'){
    if(*p == '.'){
      if(in_nsec){
        *p = '\0';
        time->tv_nsec = atoi(pp);
        pp = p+1;
        in_nsec = 0;
      }else{
        *p = '\0';
        time->tv_sec = atoi(pp);
        pp = p+1;
        if(name){
          *name = dk_fmtmem("%s", pp);
        }
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
  parse_cid(cid, &(com->time), &(com->name));

  path = dk_fmtmem("%s/%s", basedir, "parent");
  if(!ct_fexists(path)){
    com->parent = ct_fread(path);
  }

  path = dk_fmtmem("%s/%s", basedir, "message");
  com->message = ct_fread(path);

  dk_free(basedir);
  return com;
}

char *san_fname(char *path, bool san){
  char *name = dk_malloc(sizeof(char)*(strlen(path)+1));
  char *ptr = name;
  while(*path != '\0'){
    if(san){
      if(*path == '/'){
        *ptr = '+';
      }else if(*path == '+'){
        *path = 26;
      }else{
        *ptr = *path;
      }
    }else{
      if(*path == '+'){
        *ptr = '/';
      }else if(*path == 26){
        *ptr = '+';
      }else{
        *ptr = *path;
      }
    }
    ptr++;
    path++;
  }
  *ptr = '\0';
  return name;
}

/*
int show_commit(int argc, char **argv){
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

struct ct_tree *cindex_to_tree(char *cid){
  struct ct_tree *tree = ct_tree_alpha_init();
  if(!cid){
    return tree;
  }
  char buff[1024];
  int len;
  struct ent *cur;
  struct ct_leaf kv = {NULL, 0, NULL};
  struct ct_leaf ckv = {NULL, 0, NULL};
  char *path = dk_fmtmem(".cams/%s/cindex", cid);
  FILE *file = dk_open(path, "r");
  while(fgets(buff, 1024, file) != NULL){
    cur = ent_from_line(buff);
    parse_cid(cur->cid, &cur->time, NULL);
    kv.key = cur->path;
    kv.data = cur;
    ct_tree_set(tree, &kv);
  }
  fclose(file);
  return tree;
}

int cfiles_filter(struct ct_leaf *kv, void *data){
  char *cid = (char *)data;
  struct ent *cur = (struct ent *)kv->data;  
  if(!strcmp(cur->cid, cid)){
    return true;
  }
  return false;
}

struct ct_tree *cfiles(char *cid){
  struct ct_tree *tree = cindex_to_tree(cid);
  struct ct_tree *dest = ct_tree_filter(tree, cfiles_filter, cid);
  tree->free = NULL; 
  ct_tree_free(tree);
  return dest;
}

char *cfiles_join(struct ct_tree *cfiles){
  struct ct_leaf leaf;
  bzero(&leaf, sizeof(struct ct_leaf));
  struct crowbuff *buff = crowbuff_init();
  int first = 1;
  char sep[] = ", ";
  while(!ct_tree_next(cfiles, &leaf)){
    if(leaf.key == NULL)
      break;
    if(first)
      first = 0;
    else
      buff->push(buff, sep, strlen(sep));
    struct ent *cur = (struct ent *)leaf.data;
    buff->push(buff, cur->path, strlen(cur->path));
  }
  char *content = buff->content;
  free(buff);
  return content;
}

struct ct_tree *flist(){
  DIR *d;
  struct dirent *dp;
  if((d = opendir(".cams/stage/files")) == NULL){
    fprintf(stderr, "unable to open stage/files");
    exit(123);
  }
  struct ent *cur;
  struct ct_leaf kv = {NULL,0, NULL};
  struct ct_tree *tree = ct_tree_alpha_init();
  while((dp = readdir(d)) != NULL){
    if(!strncmp(".", dp->d_name, 1) || !strncmp("..", dp->d_name, 2))
      continue;
    cur = ent_init(NULL, dk_fmtmem(dp->d_name));
    kv.key = cur->path;
    kv.data = cur;
    ct_tree_set(tree, &kv);
  }
  return tree;
}

void add(){
  int i;
  char *path, dest[1024];
  for(i=0;i<opitems->length;i++){
    opitems->get(opitems, i, (void **)&path); 
    snprintf(dest, 1024, ".cams/stage/files/%s", san_fname(path, true));
    printf("staging '%s' in '%s'\n", path, dest);
    ct_fcopy(path, dest);
  }
}

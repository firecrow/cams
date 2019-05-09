struct ent {
  char *path;
  char *spath;
  char *cid;
  char *prior;
  struct timespec time;
};

char *ent_tostr(struct ent *cur);
void ct_free_ent_node(void *key, void *data);
struct ct_tree *ent_tree_init();
char *gen_path(struct ent* cur, bool current);
struct ent *ent_init(char *path, char *spath);
void ent_free(struct ent *cur);
struct ent *ent_from_line(char *line);


char *ent_tostr(struct ent *cur){
  return dk_fmtmem("<ent path:%s, spath:%s, cid:%s, prior:%s>", cur->path, cur->spath, cur->cid, cur->prior);
}

void ct_free_ent_node(void *key, void *data){
  struct ent *cur = (struct ent *)data;
  ent_free(cur);
}

struct ct_tree *ent_tree_init(){
  return ct_tree_init(ct_cmp_alpha, NULL, NULL);
}

char *gen_path(struct ent* cur, bool current){
  return dk_fmtmem("%s.%s", (current ? cur->cid : cur->prior), cur->spath);
}

struct ent *ent_init(char *path, char *spath){
  if(!path && !spath){
    fprintf(stderr, "either path or spath must be specified\n");
    return NULL;
  }
  struct ent *cur = dk_malloc(sizeof(struct ent));
  if(!path){
    path = san_fname(spath, false);
  }
  if(!spath){
    spath = san_fname(path, true);
  }
  cur->path = path;
  cur->spath = spath;
  cur->cid = NULL;
  cur->prior = NULL;
  return cur;
}

void ent_free(struct ent *cur){
  if(cur->path){
    dk_free(cur->path);
  }
  if(cur->spath){
    dk_free(cur->spath);
  }
  if(cur->cid){
    dk_free(cur->cid);
  }
  if(cur->prior){
    dk_free(cur->prior);
  }
  dk_free(cur);
}

struct ent *ent_from_line(char *line){
  char **pieces;
  struct crray *arr = crray_str_init();
  ct_split(line, ':', arr); 
  if(arr->length != 2){
    fprintf(stderr, "wrong number of pieces returned by split in cindex_to_tree: %d\n", arr->length);
    exit(123);
  }
  char *cid;
  arr->get(arr, 0, (void **)&cid);
  char *fname;
  arr->get(arr, 1, (void **)&fname);
  trimnl(fname);

  struct ent *cur = ent_init(fname, NULL);
  cur->cid = dupstr(cid);
  return cur;
}

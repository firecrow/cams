struct crray *ign; 
struct ct_tree *present;

int process(const char *_fpath, const struct stat *sb, int flags){
  struct ct_leaf kv = {NULL, 0, NULL};
  char *fpath = (char *)_fpath;
  fpath += 2;
  bool show = true;
  if(strlen(fpath) == 0 || !strncmp(".cams", fpath, strlen(".cams"))){
    show = false;
  }
  int i = 0;
  while(i < ign->length){
    char *g;
    ign->get(ign, i, (void **)&g);
    if(!strncmp(g, fpath, strlen(g))){
      show = false;
    }
    i++;
  }
  if(show){
    kv.key = dupstr(fpath);
    kv.data = NULL;
    ct_tree_set(present, &kv);
  }
  return 0;
}

struct ct_tree *slist(){
  present = ct_tree_alpha_init();
  char *ignore;
  ign = crray_str_init();
  if(!ct_fexists(".cams/ignore")){
    ignore = ct_fread(".cams/ignore");
    trimnl(ignore);
    ct_split(ignore, '\n', ign);
  }
  ftw(".", process, 512);
  return present;
}


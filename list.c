int list(){
  char *cid;
  char *filestr;
  char *cfilestr;
  char *rmfilestr;
  int count = 0;
  int qty = 0;
  int cols;
  char *buff;
  /*
  struct opts *opts = getopts(argc, argv);
  struct opt_value *optval;
  */

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
    /*
    optval = find_opt(opts, 'q', "quantity");
    if(optval && optval->value){
      if(!strcmp(optval->value, "all")){
        qty = 0;
      }else{
        qty = atoi(optval->value);
      }
    }
    */
  }

  /*
  if(opts->argc > 2){
    cid = lookup_or_die(opts->argv[1]);
  }else{
    cid = get_current();
  }
  */
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



    /*
    char *idxfname = dk_fmtmem(".cams/%s/cindex", cid);
    FILE *idxf = dk_open(idxfname, "r");
    char *hash = dk_malloc(64+1);
    int ret = ct_filehash(idxf, hash);
    hash[16] = '\0';
    struct commit *com = commit_init(cid);
    trimnl(com->message);
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
      / *printf( "%s|%09ld: %s (%s)\n", hash, com->time.tv_nsec, com->message, filestr);* /
      printf( "%09ld: %s (%s)\n", com->time.tv_nsec, com->message, filestr);
    }
    if(filestr != NULL){
      dk_free(filestr);
    }
    */
  }
  return 0;
}

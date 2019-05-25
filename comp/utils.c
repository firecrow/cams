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
  return len;
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
  fseek(b, 0, SEEK_END);
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


void trimnl(char *str){
  int target = strlen(str)-1;
  if(str[target] == '\n'){
    str[target] = '\0';
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
      if(subp->flags & CT_USE_STDOUT){
        int flags = fcntl(subp->outs[0], F_GETFL, 0);
        fcntl(subp->outs[0], F_SETFL, flags | O_NONBLOCK);
      }
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

struct ct_strbuff *ct_strbuff_init(){
  struct ct_strbuff *buff = dk_malloc(sizeof(struct ct_strbuff));
  buff->len = 0;
  buff->size = 64;
  buff->str = dk_malloc(buff->size);
  return buff;
}

void ct_strbuff_free(struct ct_strbuff *buff){
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

int ct_split(char *_str, char c, struct crray *arr){

  int found = 0;
  char *str = dupstr(_str);
  char *p = str;
  char *last = p;
  int i = 0;
  char *out;
  char *item;
  while(*p != '\0'){
    if(*p == c){
      *p = '\0';
      item = dupstr(last);
      arr->add(arr, item);
      arr->get(arr, i, (void **)&out);
      last = p+1;
      found++;
    }
    p++;
  }
  if(last != p){
    arr->add(arr, dupstr(last));
    last = p+1;
  }
  return found;
}

/* 1 is not match 0 is match */
int ct_fcompare(char *apath, int alen, char *bpath, int blen){
  char ba[CT_FCOMPARE_SIZE];
  char bb[CT_FCOMPARE_SIZE];
  FILE *a, *b;
  int la, lb, r, end;
  struct stat astat;
  struct stat bstat;

  if(!alen){
    xok(stat(apath, &astat));
    alen = astat.st_size;
  }
  if(!blen){
    xok(stat(bpath, &astat));
    blen = bstat.st_size;
  }
  if(alen != blen)
    return (1);

  xokptr(a = fopen(apath, "r"));
  xokptr(b = fopen(bpath, "r"));

  la = lb = CT_FCOMPARE_SIZE;
  while(!end){
    do {
      r = fread(&ba, 1, CT_FCOMPARE_SIZE, a);
      la -= r;
      if(!r)
        end = 1;
    } while (!end && la < CT_FCOMPARE_SIZE);
    do {
      r = fread(&bb, 1, CT_FCOMPARE_SIZE, b);
      lb -= r;
      if(!r)
        end = 1;
    } while (!end && lb < CT_FCOMPARE_SIZE);
    if(la != lb)
      return (1);

    if(strncmp(ba, bb, CT_FCOMPARE_SIZE - la)){
      return 1;
    }
    la = lb = CT_FCOMPARE_SIZE;
    r = 0;
  }
  return 0;
}

int ct_fcopy(char *a, char *b){
  char bb[CT_FCOMPARE_SIZE];
  int c, r;
  FILE *fa = fopen(a, "r");
  FILE *fb = fopen(b, "w");
  while(1){
    r = fread(bb, 1, CT_FCOMPARE_SIZE, fa);
    if(!r) 
      break;
    c += r;
    fwrite(&bb, 1, r, fb);
  }
  return c;
}

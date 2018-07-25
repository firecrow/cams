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

static struct ct_tree *find_range(char *cid, char *needle);
static void parse_remote(struct remote *r, char *uri);

int push(int argc, char **argv){
  struct remote r;
  r.name = (argc == 3) ? argv[2] : NULL;
  remote_by_name(&r, r.name);
  printf("querying %s for latest commit\n", r.name);
  char *rlatest = latest_remote(&r);
  char *cid = get_current();
  if(rlatest && !strcmp(rlatest, cid)){
    printf("%s is up to date at commit %s\n", r.name, cid);
    return 0;
  }
  struct commit *com = commit_init(cid);

  char *cmd;
  cmd = dk_fmtmem("ls %s 1>/dev/null", r.path);
  if(system(ssh_cmd(&r, cmd))){
    fprintf(stderr, "directory not found: %s\n", r.path);
  }

  /* gathering files */
  struct ct_tree *files = ct_tree_alpha_init();
  struct ct_key_data kv = {NULL, NULL};
  struct ct_key_data rkv = {NULL, NULL};
  struct ct_tree *range = find_range(cid, rlatest); 
  char *current;
  while(!ct_tree_next(range, &kv)){
    /* TODO: add in millis file */
    current = kv.data;
    rkv.key = current;
    ct_tree_set(files, &rkv);
    gather_files_for_push(files, current);
  }

  char *filestr = ct_tree_alpha_join(files, " ");
  char *packet_name = dk_fmtmem("%s.batch.tgz", r.name);
  cmd = dk_fmtmem("cd .cams && tar -czf %s %s", packet_name, filestr);
  if(system(cmd)){
    fprintf(stderr, "error creating tar\n");
    exit(123);
  }

  char * path = dk_fmtmem(".cams/%s", packet_name);
  scp_batch(&r, path);

  char *remote_path = dk_fmtmem("%s/%s", r.path, packet_name);
  cmd = dk_fmtmem("\"tar -xzf %s -C %s && rm %s\"", remote_path, r.path, remote_path);
  if(system(ssh_cmd(&r, cmd))){
    fprintf(stderr, "unable to unpack: %s\n", packet_name);
  }

  cmd = dk_fmtmem("\"echo -n '%s' > %s/current\"", cid, r.path);
  if(system(ssh_cmd(&r, cmd))){
    fprintf(stderr, "unable to update rmote `current`\n", packet_name);
  }
  char *newlatest = latest_remote(&r);
  rlatest[9] = '\0';
  newlatest[9] = '\0';
  printf("pushed %d commits %s..%s to %s\n", range->len, rlatest, newlatest, r.name);
}

void remote_by_name(struct remote *r, char *name){
  char b[1024];
  char *uri;
  FILE * rfile = fopen(".cams/remotes", "r");
  if(!rfile){
    /* bad things have happened */
  }
  while(fgets(b, 1023, rfile) != 0){
    if(b[0] == '#')
      continue;
    /* validate that the line is the expected name
     * if name is specified
     * if name is not specified, use the first line found
     */
    if(name != NULL && strncmp(name, b, strlen(name)))
      continue;
    uri = b+(strlen(name)+1);
    trimnl(uri);
    parse_remote(r, uri);
    r->name = name;
    return;
  }
  fprintf(stderr, "error: no remote found for: %s\n", name);
  exit(123);
}

void parse_remote(struct remote *r, char *uri){
  char *from = uri;
  int offset = 0;
  enum REMOTE_PARSE_STATES state = CT_ROPEN;
  fprintf(stderr, "from:%s\n", from);
  r->port = 22;/* default port */
  while(1){
    if(state == CT_ROPEN){
      if(*(from+offset) == '@'){
        r->user = dupnstr(from, offset);
        from = from+(offset+1);
        offset = 0;
        state = CT_RHOST;
      }
    }
    if(state == CT_ROPEN || state == CT_RHOST){
      if(from[offset] == '/' || from[offset] == '~' || from[offset] == '\0'){
        r->host = dupnstr(from, offset);
        from = from+offset;
        offset = 0;
        state = CT_RPATH;
      }else if(from[offset] == ':'){
        r->host = dupnstr(from, offset);
        from = from+(offset+1);
        offset = 0;
        state = CT_RPORT;
      }
    }else if(state == CT_RPORT){
      if(from[offset] < 48 || from[offset] > 57){ /* non number character */
        r->port = atoi(dupnstr(from, offset));
        from = from+offset;
        offset = 0;
        state = CT_RPATH;
      }
    }else if(state == CT_RPATH){
      if(from[offset] == '\0'){
        if(offset > 1){
          r->path = dupnstr(from, offset);
          from = from+(offset-1);
          offset = 0;
        }
        state = CT_RDONE;
        break;
      }
    }
    if(from[offset] == '\0' && state != CT_RDONE){
      fprintf(stderr, "Error parsing remote\n");
      exit(123);
    }
    offset++;
  }
}


void remote_free(struct remote *r){

}

char *latest_remote(struct remote *r){
  char cmd[1024];
  char b[1024];
  FILE *out;
  int lt;
  if(snprintf(cmd, 1024, "cat %s/current 2>/dev/null", r->path) > 1023){
    fprintf(stderr, "cmd too long\n");
    exit(123);
  }
  out = popen(ssh_cmd(r, cmd), "r");
  fgets(b, 4096, out);
  if(!strlen(b)){
    return NULL;
  }else{
    return dk_fmtmem("%s", b);
  }
}

void init_remote(char *name){
  struct remote r;
  remote_by_name(&r, name);
  /* ssh to see if the directory exists */
  /* create it if it doesn't*/
}

char *ssh_cmd(struct remote *r, char *shell_cmd){
  char c[] = "c";
  size_t l = snprintf(c, 1, "ssh -p %d %s@%s %s", r->port, r->user, r->host, shell_cmd);
  char *cmd = malloc(sizeof(char)*(l+1));
  if(!malloc);
  snprintf(cmd, l+1, "ssh -p %d %s@%s %s", r->port, r->user, r->host, shell_cmd);
  return cmd;
}

void scp_batch(struct remote *r, char *path){
  char *cmd = dk_fmtmem("scp -r -P %d %s %s@%s:%s", r->port, path, r->user, r->host, r->path);
  if(system(cmd)){
    fprintf(stderr, "error with scp\n");
    exit(123);
  }
}

void gather_files_for_push(struct ct_tree *tree, char *cid){
  struct ct_tree *files = cfiles(cid);
  struct ct_key_data kv = {NULL, NULL};
  struct ct_key_data kv_dest = {NULL, NULL};
  struct ent *cur;
  while(ct_tree_next(files, &kv) != CT_NOT_FOUND){
    cur = (struct ent *)kv.data;  
    kv_dest.key = dk_fmtmem("files/%s", gen_path(cur, true));
    ct_tree_set(tree, &kv_dest);
  }
}

struct ct_tree *find_range(char *cid, char *needle){
  struct ct_tree *tree = ct_array_init();
  struct commit *com;
  while(cid && (!needle || strcmp(cid, needle))){
    ct_array_push(tree, cid);
    cid = commit_init(cid)->parent;
  }
  return tree;
}

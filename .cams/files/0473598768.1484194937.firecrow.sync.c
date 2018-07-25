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

int push(int argc, char **argv){
  struct remote r;
  char *cid = get_current();
  struct commit *com = commit_init(cid);
  int exists;
  char *rlatest;
  struct commit rcomm;
  r.name = (argc == 3) ? argv[2] : NULL;
  parse_remote(&r, r.name);
  struct ct_tree *files = ct_tree_alpha_init();
  struct ct_key_data kv;
  char *cmd;
  char *packet_name;
  char *filestr;
  char *path;
  cmd = dk_fmtmem("ls %s 1>/dev/null", r.path);
  char *remote_path;
  if(system(ssh_cmd(&r, cmd))){
    fprintf(stderr, "directory not found: %s\n", r.path);
  }
  printf("querying %s for latest commit\n", r.name);
  rlatest = latest_remote(&r);
  struct commit *rcom = commit_init(rlatest);
  printf("pushing %s..%s\n", rlatest, cid);
  if(com_before(rcom, com)){
    printf("%s is up to date at %s\n", r.name, cid);
  /* handle ahead case here*/
  }else{
    while(cid){
      struct commit *com = commit_init(cid);
      kv.key = com->cid;
      ct_tree_set(files, &kv);
      gather_files_for_push(files, cid);
      cid = com->parent;
    }

    filestr = ct_tree_alpha_join(files, " ");
    packet_name = dk_fmtmem("%s.batch.tgz", r.name);
    cmd = dk_fmtmem("cd .cams && tar -czf %s %s", packet_name, filestr);
    printf("packing commits and files\n");
    if(system(cmd)){
      fprintf(stderr, "error creating tar\n");
      exit(123);
    }
    printf("pushing packet\n");
    path = dk_fmtmem(".cams/%s", packet_name);
    scp_batch(&r, path);
    remote_path = dk_fmtmem("%s/%s", r.path, packet_name);
    cmd = dk_fmtmem("\"tar -xzf %s -C %s && rm %s\"", remote_path, r.path, remote_path);
    if(system(ssh_cmd(&r, cmd))){
      fprintf(stderr, "unable to unpack: %s\n", packet_name);
    }
    printf("pushed %d..%d to %s\n", rlatest, cid, r.name);
  }
}

void parse_remote(struct remote *r, char *name){
  /*open remotes file*/
  char b[1024];
  char *cur_remote;
  char *value;
  size_t l;
  int field = 0;
  int s_pos = 0;
  int e_pos = 0;
  FILE * rfile = fopen(".cams/remotes", "r");
  if(!rfile){
    /* bad things have happened */
  }
  while(l = fgets(b, 1023, rfile) != 0){
    if(b[0] == '#')
      continue;
    /* validate that the line is the expected name
     * if name is specified
     * if name is not specified, use the first line found
     */
    if(name != NULL && strncmp(name, b, strlen(name)))
      continue;
    while(field < 5){
      while(
        b[e_pos] != ' ' 
        && b[e_pos] != '\0' 
        && b[e_pos] != '\n'
      ){ e_pos++; };
      value = substr(b, s_pos, e_pos);
      switch(field){
        case 0:
          r->path = value;
          break;
        case 1:
           r->user = value;
           break;
        case 2:
          r->host = value;
          break;
        case 3:
          r->port = atoi(value);
          break;
        case 4:
          r->path = value;
          break;
      }
      s_pos = ++e_pos;
      field++;
    }
    if(field < 5){
      fprintf(stderr, "Oops misconfigured remote: %s", b);
      exit(123);
    }else{
      return;
    }
  }
  fprintf(stderr, "error: no remote found for: %s\n", name);
  exit(123);
}


void remote_free(struct remote *r){

}

char *latest_remote(struct remote *r){
  char cmd[1024];
  char b[1024];
  FILE *out;
  int lt;
  if(snprintf(cmd, 1024, "cat %s/current", r->path) > 1023){
    fprintf(stderr, "cmd too long\n");
    exit(123);
  }
  out = popen(ssh_cmd(r, cmd), "r");
  fgets(b, 4096, out);
  return dk_fmtmem("%s", b);
}

void init_remote(char *name){
  struct remote r;
  parse_remote(&r, name);
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

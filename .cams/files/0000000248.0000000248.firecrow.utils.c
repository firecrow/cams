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

char *dupstr(char *str){
  size_t len = strlen(str);
  char *dest = dk_malloc(len+1);
  memcpy(dest, str, len+1);
  return dest;
}

int flen(char *path){
  FILE *file = dk_open(path, "r");
  fseek(file, 0, SEEK_END);
  int len = ftell(file);
  fseek(file, 0, SEEK_SET);
  fclose(file);
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
  fseek(a, 0, SEEK_END);
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

char *ct_fread(char *path, int size){
  FILE *file = dk_open(path, "r");
  char *str = dk_malloc(size);
  int l = fread(str, 1, size, file);
  str[l] = '\0';
  return str;
}

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

static size_t _dk_count = 0;

int dk_count(){
  return _dk_count;
}

char *fmtmem(char *fmt, ...){
  int l;
  char c[1];
  char *str;
  va_list va1;
  va_list va2;
  va_start(va1, fmt);
  va_copy(va2, va1);
  l = vsnprintf(c, 1,fmt, va1);
  str = dk_malloc(sizeof(char)*(l+1));
  vsnprintf(str, l+1,fmt, va2);
  return str; 	 
}

int dkpipe(char *cmd, char **args, int in[2], int out[2]){
  ;
}

void *dk_malloc(size_t size){
  _dk_count++;
  return malloc(size);
}

void dk_free(void *mem){
  _dk_count--;
  free(mem);
}

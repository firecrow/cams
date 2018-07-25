#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
int main(int argc, char **argv){
  char *x = fmtmem("%s->%s", "one", "two");
  printf("%d: %s\n", strlen(x), x);
  dk_free(x);
  if(dk_count() == 0){
    puts("all clean\n");
  }else{
    printf("oops dirty %s", dk_count);
  }
}

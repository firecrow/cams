#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include "../../crowx/crowx.c"
#include "../../crowtils/crowtils.c"
#include "../../crray/crray.c"
#include "../comp/utils.h"
#include "../comp/utils.c"

int main(){
  /* dupstr */
  char astr[] = "hi there this is a string\n";
  char *bstr = dupstr(astr);
  if(!strcmp(astr, bstr))
    printf("pass");
  else
    printf("fail");
  printf(" dupstr\n");

  char cstr[] = "01234567890\n";
  char expc[] = "01234567";
  char *dstr = dupnstr(cstr, 8);
  if(!strcmp(expc, dstr))
    printf("pass");
  else
    printf("fail %s", dstr);
  printf(" dupnstr first 8\n");

  int len = flen("test/fixtures/A.1958.txt"); 
  if(len == 1958)
    printf("pass");
  else
    printf("fail %d", len);
  printf(" flen\n");

  if(fexists("test/fixtures/alphabet.txt"))
    printf("pass");
  else
    printf("fail %d", len);
  printf(" fexists alphabet.txt\n");

  if(!fexists("notafile"))
    printf("pass");
  else
    printf("fail %d", len);
  printf(" fexists not a file\n");

  if(feq("test/fixtures/A.1958.txt", "test/fixtures/A.same.1958.txt"))
    printf("pass");
  else
    printf("fail %d", len);
  printf(" feq identical\n");

  if(!feq("test/fixtures/A.1958.txt", "test/fixtures/B.1958.txt"))
    printf("pass");
  else
    printf("fail %d", len);
  printf(" feq different same size\n");

  if(!feq("test/fixtures/A.1958.txt", "test/fixtures/C.3370.txt"))
    printf("pass");
  else
    printf("fail %d", len);
  printf(" feq different sizes\n");

  /*
   * localize_fname
   * trimnl
   * ct_split
   * ct_fcompare
   * ct_fcopy
  */
}

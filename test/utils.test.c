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

  char withnl[] = "hi this has a new line\n";
  char withoutnl[] = "hi this has a new line";
  char nlcontrol[] = "hi this has a new line";

  trimnl(withnl);
  if(!strncmp(withnl, nlcontrol, 1024)) 
    printf("pass ");
  else
    printf("fail ");
  printf("trim nl with nl\n");  

  trimnl(withoutnl);
  if(!strncmp(withoutnl, nlcontrol, 1024)) 
    printf("pass ");
  else
    printf("fail ");
  printf("trim nl without nl\n");  

  struct crray *arr = crray_str_init();
  struct crray *arrb = crray_str_init();
  arrb->add(arrb, "hi");
  arrb->add(arrb, "there");
  arrb->add(arrb, "this");
  arrb->add(arrb, "is");
  arrb->add(arrb, "a");
  arrb->add(arrb, "thing");
  char scontent[] = "hi there this is a thing";
  ct_split(scontent, ' ', arr);

  /*show_str_arr(arr, "from split1");*/
  int r2 = crray_cmparr(arr, arrb);
  if(r2)
    printf("fail ");
  else
    printf("pass "); 
  printf("ct_split test with crray\n");      

  struct crray *arrc = crray_str_init();
  struct crray *arrcontb = crray_str_init();
  arrcontb->add(arrcontb, "hi");
  arrcontb->add(arrcontb, "there");
  char scontentb[] = "hi there ";
  ct_split(scontentb, ' ', arrc);

  int r1 = crray_cmparr(arrc, arrcontb);
  if(r1)
    printf("fail ");
  else
    printf("pass "); 
  printf("ct_split test with crray and following seperator\n");      

  int lenb = flen("test/fixtures/A.1958.txt"); 
  if(!ct_fcompare("test/fixtures/A.1958.txt", 0, "test/fixtures/A.same.1958.txt", 0))
    printf("pass");
  else
    printf("fail");
  printf(" ct_fcompare identical\n");

  if(ct_fcompare("test/fixtures/A.1958.txt", 0, "test/fixtures/B.1958.txt", 0))
    printf("pass");
  else
    printf("fail");
  printf(" ct_fcompare different same size\n");

  if(ct_fcompare("test/fixtures/A.1958.txt", 0, "test/fixtures/C.3370.txt", 0))
    printf("pass");
  else
    printf("fail %d", len);
  printf(" ct_fcompare different sizes\n");

  if(fexists("./test/fixtures/A.1958.dup.txt")){
    unlink("./test/fixtures/A.1958.dup.txt");
  }
  ct_fcopy("./test/fixtures/A.1958.txt", "./test/fixtures/A.1958.dup.txt");
  if(ct_fcompare("./test/fixtures/A.1958.txt", 0, "./test/fixtures/A.1958.dup.txt", 0))
    printf("pass");
  else
    printf("fail");
  printf(" ct_fcopy\n");
}

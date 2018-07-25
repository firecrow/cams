#include <unistd.h>
#include <suite.h>
#include "cams.h"

int ent_create(struct suite_set *set, void *data){
  char *msg = dupstr("basic init");
	int result = 0;
  return next(set, result, msg, data);
}

int main(){
  struct suite_set *basic_set = set_init("ent", 1, ent_create);
  start(basic_set, NULL);
	return 0;
}

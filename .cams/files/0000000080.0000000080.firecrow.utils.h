char *dk_fmtmem(char *fmt, ...);
FILE *dk_open(char *fname, char *type);

int dk_pipe(char *cmd, char **args, int in[2], int out[2]);

void *dk_malloc(size_t size);
void dk_free(void *mem);

int dk_count();

char *substr(char *src, size_t s, size_t e);


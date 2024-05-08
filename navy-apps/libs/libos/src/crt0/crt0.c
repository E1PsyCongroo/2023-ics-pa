#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = *((int*)args);
  char **argv = (char**)((int*)args + 1);
  char **envp = (char**)argv + argc + 1;
  environ = envp;
  printf("argc: %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  printf("envp:\n");
  for (char **p = envp; *p; p++) {
    printf("%s\n", *p);
  }
  exit(main(argc, argv, envp));
  assert(0);
}

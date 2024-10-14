#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
  for (char **e = argv; *e != NULL; e++) {
    printf("%s\n", *e);
  }

  for (char **e = envp; *e != NULL; e++) {
    printf("%s\n", *e);
  }
}

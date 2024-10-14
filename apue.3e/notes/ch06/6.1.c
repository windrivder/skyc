#include <stdio.h>
#include <sys/utsname.h>

int main(int argc, char *argv[]) {
  struct utsname *name;
  if (uname(name) == -1) {
    perror("uname error");
  }

  printf("%s\n", name->sysname);
  printf("%s\n", name->machine);
  printf("%s\n", name->release);
  printf("%s\n", name->version);
  printf("%s\n", name->nodename);
}

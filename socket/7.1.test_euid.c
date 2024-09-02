#include <stdio.h>
#include <unistd.h>

int main() {
  uid_t uid = getuid();
  uid_t euid = geteuid();
  printf("userId is %d, effective userId is %d\n", uid, euid);
  return 0;
}

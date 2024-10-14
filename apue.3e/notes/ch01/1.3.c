#include <stdio.h>

int main(int argc, char *argv[]) {
  int a = 1;
  int b = 2;

  const int *p1 = &a;
  p1 = &b;
  printf("%d\n", *p1);

  int *const p = &b;
  *p = a;
  printf("%d\n", *p);
}

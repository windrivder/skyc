#include <stdio.h>

union MyUnion {
  int i;
  float f;
  char str[20];
};

int main() {
  union MyUnion data;

  data.i = 10;
  printf("data.i: %d\n", data.i);

  data.f = 20.5;
  printf("data.f: %f\n", data.f);

  printf("data.i after setting data.f: %d\n", data.i);

  return 0;
}

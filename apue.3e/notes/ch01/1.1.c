#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void check_ls() {
  DIR *dp1, *dp2;
  struct dirent *dirp1, *dirp2;

  const char *dir1 = ".";
  const char *dir2 = "..";

  if ((dp1 = opendir(dir1)) == NULL) {
    printf("can't open %s\n", dir1);
  }

  if ((dp2 = opendir(dir2)) == NULL) {
    printf("can't open %s\n", dir2);
  }

  while ((dirp1 = readdir(dp1)) != NULL && (dirp2 = readdir(dp1)) != NULL) {
    if (strcmp(dirp1->d_name, dirp2->d_name) != 0) {
      printf("%s and %s are different\n", dir1, dir2);
      closedir(dp1);
      closedir(dp2);
      return;
    }
  }

  closedir(dp1);
  closedir(dp2);
}

void check_pwd() {
  char path1[1024], path2[1024];

  const char *dir1 = ".";
  const char *dir2 = "..";

  realpath(dir1, path1);
  realpath(dir2, path2);

  if (strcmp(path1, path2) == 0) {
    printf("%s and %s absolute path are same\n", path1, path2);
  } else {
    printf("%s and %s absolute path aren't same\n", path1, path2);
  }
}

int main(int argc, char *argv[]) {
  check_ls();
  check_pwd();
}

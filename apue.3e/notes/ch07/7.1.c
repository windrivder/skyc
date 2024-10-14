#include <apue.h>
#include <unistd.h>

int globvar = 6;
char buf[] = "a write to stdout\n";

int main(int argc, char *argv[]) {
  int var;
  pid_t pid;

  var = 88;
  // 标准输出到终端设备 -> 行缓冲
  // 标准输出到文件     -> 全缓冲
  if (write(STDOUT_FILENO, buf, sizeof(buf) - 1) != sizeof(buf) - 1) {
    err_sys("write error");
  }
  printf("before fork\n");

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    globvar++;
    var++;
  } else {
    sleep(2);
  }

  printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar, var);
  exit(0);
}

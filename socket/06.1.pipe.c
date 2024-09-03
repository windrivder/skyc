#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> // 添加这个头文件来使用 wait()
#include <unistd.h>

int main(void) {
  int pipefd[2]; // 管道描述符
  pid_t pid;

  /* 创建管道 */
  if (pipe(pipefd) == -1) {
    perror("Pipe failed");
    return -1;
  }

  /* 创建子进程 */
  pid = fork();

  if (pid < 0) { // fork 进程失败
    perror("Fork failed");
    return -1;
  }

  if (pid > 0) {      // 父进程
    close(pipefd[0]); // 关闭读端
    const char *msg = "Hello from parent to child\n";
    if (write(pipefd[1], msg, strlen(msg) + 1) == -1) { // 向管道写入信息
      perror("Write failed");
      return -1;
    }
    close(pipefd[1]);

    // 等待子进程结束
    if (wait(NULL) == -1) {
      perror("Wait failed");
      return -1;
    }
  } else {            // 子进程
    close(pipefd[1]); // 关闭写端
    char buffer[100];
    ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
      perror("Read failed");
      return -1;
    }
    buffer[bytes_read] = '\0'; // null-terminate the string
    printf("Child received: %s", buffer);

    close(pipefd[0]);
  }

  return 0;
}

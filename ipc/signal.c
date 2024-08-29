#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 信号处理函数
void signal_handler(int signum) {
  printf("Caught signal %d\n", signum);
  printf("Cleaning up resources...\n");
  // 进行必要的清理工作
  exit(signum); // 退出程序
}

int main() {
  // 注册信号处理函数
  signal(SIGINT, signal_handler);

  // 主循环
  while (1) {
    printf("Running...\n");
    sleep(1); // 模拟长时间运行的任务
  }

  return 0;
}

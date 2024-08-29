#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define MSGKEY 1234 // 消息队列键值

// 消息结构体
struct message_buffer {
  long mtype;      // 消息类型
  char mtext[100]; // 消息文本
};

// 发送消息到消息队列
void send_message(int msqid, const char *message, long mtype) {
  struct message_buffer msg;
  msg.mtype = mtype;
  strncpy(msg.mtext, message, sizeof(msg.mtext) - 1);
  msg.mtext[sizeof(msg.mtext) - 1] = '\0'; // 确保字符串以空字符结尾

  if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
    perror("msgsnd");
    exit(EXIT_FAILURE);
  }
}

// 接收消息从消息队列
void receive_message(int msqid, long mtype) {
  struct message_buffer msg;
  if (msgrcv(msqid, &msg, sizeof(msg.mtext), mtype, 0) == -1) {
    perror("msgrcv");
    exit(EXIT_FAILURE);
  }
  printf("Received message: %s\n", msg.mtext);
}

int main() {
  int msqid; // 消息队列标识符

  // 创建消息队列
  msqid = msgget(MSGKEY, IPC_CREAT | 0666);
  if (msqid == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  // 父进程发送消息
  send_message(msqid, "Hello from parent!", 1);

  // 创建子进程
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    // 父进程等待子进程完成
    wait(NULL);
    // 删除消息队列
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
      perror("msgctl");
      exit(EXIT_FAILURE);
    }
  } else {
    // 子进程接收消息
    receive_message(msqid, 1);
  }

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE 1024 // 共享内存大小
#define SHM_KEY 12345 // 共享内存键值

// 写入数据到共享内存
void write_to_shm(int shmid) {
  char *shm = shmat(shmid, NULL, 0); // 附加共享内存
  if (shm == (void *)-1) {
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  // 写入一些测试数据
  strcpy(shm, "Hello from parent!");

  printf("Parent wrote: %s\n", shm);

  if (shmdt(shm) == -1) { // 分离共享内存
    perror("shmdt");
    exit(EXIT_FAILURE);
  }
}

// 从共享内存读取数据
void read_from_shm(int shmid) {
  char *shm = shmat(shmid, NULL, 0); // 附加共享内存
  if (shm == (void *)-1) {
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  printf("Child read: %s\n", shm);

  if (shmdt(shm) == -1) { // 分离共享内存
    perror("shmdt");
    exit(EXIT_FAILURE);
  }
}

int main() {
  int shmid; // 共享内存标识符

  // 创建共享内存段
  shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
  if (shmid == -1) {
    perror("shmget");
    exit(EXIT_FAILURE);
  }

  // 父进程写入数据
  write_to_shm(shmid);

  // 创建子进程
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    // 父进程等待子进程完成
    if (wait(NULL) == -1) {
      perror("Wait failed");
      return -1;
    }

    // 删除共享内存段
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
      perror("shmctl");
      exit(EXIT_FAILURE);
    }
  } else {
    // 子进程读取数据
    read_from_shm(shmid);
  }

  return 0;
}

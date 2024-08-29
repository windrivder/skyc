#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080

int main() {
  int client_socket;
  struct sockaddr_in server_addr;
  char buffer[1024] = {0};

  // 创建套接字
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // 设置服务器地址
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

  // 连接到服务器
  if (connect(client_socket, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) == -1) {
    perror("connect");
    close(client_socket);
    exit(EXIT_FAILURE);
  }

  // 向服务器发送消息
  const char *message = "Hello, server!";
  send(client_socket, message, strlen(message), 0);

  // 接收服务器的响应
  ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received == -1) {
    perror("recv");
    close(client_socket);
    exit(EXIT_FAILURE);
  }
  buffer[bytes_received] = '\0';
  printf("Received: %s\n", buffer);

  // 关闭客户端套接字
  close(client_socket);

  return 0;
}

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
  client_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (client_socket == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // 设置服务器地址
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

  // 向服务器发送消息
  const char *message = "Hello, server!";
  if (sendto(client_socket, message, strlen(message), 0,
             (const struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
    perror("sendto");
    close(client_socket);
    exit(EXIT_FAILURE);
  }

  // 接收服务器的响应
  socklen_t server_len = sizeof(server_addr);
  ssize_t bytes_received =
      recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0,
               (struct sockaddr *)&server_addr, &server_len);
  if (bytes_received == -1) {
    perror("recvfrom");
    close(client_socket);
    exit(EXIT_FAILURE);
  }
  buffer[bytes_received] = '\0';
  printf("Received: %s\n", buffer);

  // 关闭客户端套接字
  close(client_socket);

  return 0;
}

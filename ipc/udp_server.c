#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 8080

int main() {
  int server_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);
  char buffer[1024] = {0};

  // 创建套接字
  server_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_socket == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // 设置服务器地址
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // 绑定套接字到地址
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == -1) {
    perror("bind");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  // 接收客户端的数据
  ssize_t bytes_received =
      recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0,
               (struct sockaddr *)&client_addr, &client_len);
  if (bytes_received == -1) {
    perror("recvfrom");
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  buffer[bytes_received] = '\0';
  printf("Received: %s\n", buffer);

  // 向客户端发送响应
  const char *response = "Hello, client!";
  if (sendto(server_socket, response, strlen(response), 0,
             (const struct sockaddr *)&client_addr, client_len) == -1) {
    perror("sendto");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  // 关闭服务器套接字
  close(server_socket);

  return 0;
}

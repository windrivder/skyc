#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <libgen.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// function to print usage and exit
static void print_usage_and_exit(const char *program_name) {
  fprintf(stderr, "Usage: %s ip_address port_number recv_buffer_size\n",
          program_name);
  exit(EXIT_FAILURE);
}

// function to create and configure a listening socket
static int create_and_bind_socket(const char *ip, int port, int recvbuf) {
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Failed to create socket");
    return -1;
  }

  int len = sizeof(recvbuf);
  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf, sizeof(recvbuf));
  getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf, (socklen_t *)&len);
  printf("the receive buffer size after settting is %d\n", recvbuf);

  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(port);

  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Failed to bind socket");
    close(sock);
    return -1;
  }

  return sock;
}

// function to start listening on the socket
static void start_listening(int sock, int backlog) {
  if (listen(sock, backlog) < 0) {
    perror("Failed to listen on socket");
    close(sock);
    exit(EXIT_FAILURE);
  }
}

// function to handle incoming connections
static void handle_connection(int sock) {
  struct sockaddr_in client;
  socklen_t client_addrlength = sizeof(client);
  int connfd = accept(sock, (struct sockaddr *)&client, &client_addrlength);
  if (connfd < 0) {
    perror("Failed to accept connection");
    return;
  }

  char remote[INET_ADDRSTRLEN];
  printf("Connected with IP: %s and Port: %d\n",
         inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN),
         ntohs(client.sin_port));

  char buffer[BUFFER_SIZE];
  memset(buffer, '\0', BUFFER_SIZE);

  int ret;
  while (1) {
    int i = recv(connfd, buffer, BUFFER_SIZE - 1, 0);
    if (i <= 0) {
      break;
    }
    ret += i;
  }
  printf("Got %d bytes of data '%s'\n", ret, buffer);

  close(connfd);
}

int main(int argc, char *argv[]) {
  // check command line arguments
  if (argc <= 3) {
    print_usage_and_exit(basename(argv[0]));
  }

  // parse command line arguments
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  int recvbuf = atoi(argv[3]);

  // create and bind the socket
  int sock = create_and_bind_socket(ip, port, recvbuf);
  if (sock < 0) {
    return EXIT_FAILURE;
  }

  // start listening on the socket
  start_listening(sock, 5);

  // handle incoming connections
  handle_connection(sock);

  // clean up resources
  close(sock);
  return EXIT_SUCCESS;
}

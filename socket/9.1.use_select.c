#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <libgen.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// function to print usage and exit
static void print_usage_and_exit(const char *program_name) {
  fprintf(stderr, "Usage: %s ip_address port_number\n", program_name);
  exit(EXIT_FAILURE);
}

// function to create and configure a listening socket
static int create_and_bind_socket(const char *ip, int port) {
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Failed to create socket");
    return -1;
  }

  int reuse = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

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

  char remote_addr[INET_ADDRSTRLEN];
  printf("connected with ip: %s and port: %d\n",
         inet_ntop(AF_INET, &client.sin_addr, remote_addr, INET_ADDRSTRLEN),
         ntohs(client.sin_port));

  char buf[1024];
  fd_set read_fds;
  fd_set exception_fds;

  FD_ZERO(&read_fds);
  FD_ZERO(&exception_fds);

  int nReuseAddr = 1;
  setsockopt(connfd, SOL_SOCKET, SO_OOBINLINE, &nReuseAddr, sizeof(nReuseAddr));
  while (1) {
    memset(buf, '\0', sizeof(buf));
    FD_SET(connfd, &read_fds);
    FD_SET(connfd, &exception_fds);

    int ret = select(connfd + 1, &read_fds, NULL, &exception_fds, NULL);
    printf("select one\n");
    if (ret < 0) {
      printf("selection failure\n");
      break;
    }

    if (FD_ISSET(connfd, &read_fds)) {
      ret = recv(connfd, buf, sizeof(buf), 0);
      if (ret <= 0) {
        break;
      }
      printf("get %d bytes of normal data: %s\n", ret, buf);
    } else if (FD_ISSET(connfd, &exception_fds)) {
      ret = recv(connfd, buf, sizeof(buf) - 1, MSG_OOB);
      if (ret <= 0) {
        break;
      }
      printf("get %d bytes of oob data: %s\n", ret, buf);
    }
  }

  close(connfd);
}

int main(int argc, char *argv[]) {
  // check command line arguments
  if (argc < 3) {
    print_usage_and_exit(basename(argv[0]));
  }

  // parse command line arguments
  const char *ip = argv[1];
  int port = atoi(argv[2]);

  // create and bind the socket
  int sock = create_and_bind_socket(ip, port);
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

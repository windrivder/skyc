#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
static const char *status_line[2] = {"200 OK", "500 Internal server error"};

// function to print usage and exit
static void print_usage_and_exit(const char *program_name) {
  fprintf(stderr, "Usage: %s ip_address port_number filename\n", program_name);
  exit(EXIT_FAILURE);
}

// function to create and configure a listening socket
static int create_and_bind_socket(const char *ip, int port) {
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Failed to create socket");
    return -1;
  }

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
static void handle_connection(int sock, const char *filename) {
  struct sockaddr_in client;
  socklen_t client_addrlength = sizeof(client);
  int connfd = accept(sock, (struct sockaddr *)&client, &client_addrlength);
  if (connfd < 0) {
    perror("Failed to accept connection");
    return;
  }

  char header_buf[BUFFER_SIZE];
  memset(header_buf, '\0', BUFFER_SIZE);

  char *file_buf;
  struct stat file_stat;
  int valid = 1;
  int len = 0; // header_buf used

  if (stat(filename, &file_stat) < 0) { // filename not exist
    valid = 0;
  } else if (S_ISDIR(file_stat.st_mode)) {
    valid = 0;
  } else if (file_stat.st_mode & S_IROTH) { // read permission
    int fd = open(filename, O_RDONLY);
    char file_buf[file_stat.st_size + 1];
    memset(file_buf, '\0', file_stat.st_size + 1);
    if (read(fd, file_buf, file_stat.st_size) < 0) {
      valid = 0;
    }
  } else {
    valid = 0;
  }

  printf("valid: %d\n", valid);
  if (valid) {
    int ret = snprintf(header_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1",
                       status_line[0]);
    len += ret;
    ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len,
                   "Content-Length: %d\r\n", (int)file_stat.st_size);
    len += ret;
    ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");
    struct iovec iv[2];
    iv[0].iov_base = header_buf;
    iv[0].iov_len = strlen(header_buf);
    iv[1].iov_base = file_buf;
    iv[1].iov_len = file_stat.st_size;
    ret = writev(connfd, iv, 2);
  } else {
    int ret = snprintf(header_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1",
                       status_line[1]);
    len += ret;
    ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");
    send(connfd, header_buf, strlen(header_buf), 0);
  }

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
  const char *filename = argv[3];

  // create and bind the socket
  int sock = create_and_bind_socket(ip, port);
  if (sock < 0) {
    return EXIT_FAILURE;
  }

  // start listening on the socket
  start_listening(sock, 5);

  // handle incoming connections
  handle_connection(sock, filename);

  // clean up resources
  close(sock);
  return EXIT_SUCCESS;
}

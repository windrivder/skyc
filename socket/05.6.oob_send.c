#include <arpa/inet.h>
#include <assert.h>
#include <libgen.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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

  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(port);

  if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Failed to connection");
    close(sock);
    return -1;
  }

  return sock;
}

// function to send oob data
static void send_oob_data(int sock) {
  printf("send oob data out\n");
  const char *oob_data = "hello world";
  const char *normal_data = "hello c";
  send(sock, normal_data, strlen(normal_data), 0);
  send(sock, oob_data, strlen(oob_data), MSG_OOB);
  send(sock, normal_data, strlen(normal_data), 0);
}

int main(int argc, char *argv[]) {
  // check command line arguments
  if (argc != 3) {
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

  // send oob data
  send_oob_data(sock);

  // clean up resources
  close(sock);
  return EXIT_SUCCESS;
}

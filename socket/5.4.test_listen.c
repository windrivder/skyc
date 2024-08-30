#include <arpa/inet.h>
#include <assert.h>
#include <libgen.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// global flag to indicate termination request
static volatile sig_atomic_t stop = 0;

// signal handler function
static void handle_term(int sig) { stop = 1; }

// function to print usage and exit
static void print_usage_and_exit(const char *program_name) {
  fprintf(stderr, "Usage: %s ip_address port_number backlog\n", program_name);
  exit(EXIT_FAILURE);
}

// function to create and configure a listening socket
static int create_and_bind(const char *ip, int port) {
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

int main(int argc, char *argv[]) {
  // check command line arguments
  if (argc != 4) {
    print_usage_and_exit(basename(argv[0]));
  }

  // parse command line arguments
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  int backlog = atoi(argv[3]);

  // validate port number and backlog
  if (port <= 0 || port > 65535 || backlog <= 0) {
    fprintf(stderr, "Invalid port number or backlog.\n");
    print_usage_and_exit(basename(argv[0]));
  }

  // register signal handler for sigterm
  signal(SIGTERM, handle_term);

  // create and bind the socket
  int sock = create_and_bind(ip, port);
  if (sock < 0) {
    exit(EXIT_FAILURE);
  }

  // start listening on the socket
  start_listening(sock, backlog);

  // main loop to wait for termination signal
  while (!stop) {
    usleep(1000000); // sleep for 1 second
  }

  // clean up resources
  close(sock);
  return EXIT_SUCCESS;
}

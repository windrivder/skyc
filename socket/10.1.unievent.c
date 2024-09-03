#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_EVENT_NUMBER 1024
static int pipefd[2];

int setnonblocking(int fd) {
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}

void addfd(int epollfd, int fd, int enable_et) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN;
  if (enable_et) {
    event.events |= EPOLLET;
  }
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  setnonblocking(fd);
}

void sig_handler(int sig) {
  int save_errno = errno;
  int msg = sig;
  send(pipefd[1], (char *)&msg, 1, 0);
  errno = save_errno;
}

void addsig(int sig) {
  struct sigaction sa;
  memset(&sa, '\0', sizeof(sa));
  sa.sa_handler = sig_handler;
  sa.sa_flags |= SA_RESTART;
  sigfillset(&sa.sa_mask);
  assert(sigaction(sig, &sa, NULL) != -1);
}

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
  struct epoll_event events[MAX_EVENT_NUMBER];
  int epollfd = epoll_create(5);
  assert(epollfd != -1);
  addfd(epollfd, sock, 0);

  // create new socket
  int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
  assert(ret != -1);
  setnonblocking(pipefd[1]);
  addfd(epollfd, pipefd[0], 0);

  // add all the interesting signals here
  addsig(SIGHUP);
  addsig(SIGCHLD);
  addsig(SIGTERM);
  addsig(SIGINT);

  int stop_server = 0;
  while (!stop_server) {
    int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    if ((number < 0) && (errno != EINTR)) {
      printf("epoll failure\n");
      break;
    }

    for (int i = 0; i < number; i++) {
      int sockfd = events[i].data.fd;
      // new client connection
      if (sockfd == sock) {
        struct sockaddr_in client_address;
        socklen_t client_addrlength = sizeof(client_address);
        int connfd = accept(sock, (struct sockaddr *)&client_address,
                            &client_addrlength);
        addfd(epollfd, connfd, 0);

        // new signal
      } else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)) {
        int sig;
        char signals[1024];
        ret = recv(pipefd[0], signals, sizeof(signals), 0);
        if (ret == -1) {
          continue;
        } else if (ret == 0) {
          continue;
        } else {
          for (int i = 0; i < ret; ++i) {
            printf("I caugh the signal %d\n", signals[i]);
            switch (signals[i]) {
            case SIGCHLD:
            case SIGHUP: {
              continue;
            }
            case SIGTERM:
            case SIGINT:
              stop_server = 1;
            }
          }
        }
      }
    }
  }

  printf("close fds\n");
  close(pipefd[1]);
  close(pipefd[0]);
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

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 1024

struct fds {
  int epollfd;
  int sockfd;
};

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

void reset_oneshot(int epollfd, int fd) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void *worker(void *arg) {
  int sockfd = ((struct fds *)arg)->sockfd;
  int epollfd = ((struct fds *)arg)->epollfd;
  printf("start new thread to receive data on fd: %d\n", sockfd);

  char buf[BUFFER_SIZE];
  memset(buf, '\0', BUFFER_SIZE);
  while (1) {
    int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
    if (ret == 0) {
      close(sockfd);
      printf("foreiner closed the connection\n");
      break;
    } else if (ret < 0) {
      if (errno == EAGAIN) {
        reset_oneshot(epollfd, sockfd);
        printf("read later\n");
        break;
      }
    } else {
      printf("get content: %s\n", buf);
      sleep(5);
    }
  }
  printf("end thread receiving data on fd: %d\n", sockfd);
  return NULL;
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

  while (1) {
    int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    if (ret < 0) {
      printf("epoll failure\n");
      break;
    }

    for (int i = 0; i < ret; i++) {
      int sockfd = events[i].data.fd;
      if (sockfd == sock) {
        struct sockaddr_in client_address;
        socklen_t client_addrlength = sizeof(client_address);
        int connfd = accept(sock, (struct sockaddr *)&client_address,
                            &client_addrlength);
        addfd(epollfd, connfd, 1);
      } else if (events[i].events & EPOLLIN) {
        pthread_t thread;
        struct fds fds_for_new_worker;
        fds_for_new_worker.epollfd = epollfd;
        fds_for_new_worker.sockfd = sockfd;
        pthread_create(&thread, NULL, worker, (void *)&fds_for_new_worker);
      } else {
        printf("something else happened\n");
      }
    }
  }
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

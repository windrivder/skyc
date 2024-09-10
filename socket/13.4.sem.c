#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t semaphore;

void *thread_one_func(void *arg) {
  printf("Thread One is running and will post to semaphore.\n");
  if (sem_post(&semaphore) != 0) {
    perror("sem_post");
    pthread_exit(NULL);
  }
  printf("Thread One posted to semaphore.\n");
  pthread_exit(NULL);
}

void *thread_two_func(void *arg) {
  printf("Thread Two is running and will wait for semaphore.\n");
  if (sem_wait(&semaphore) != 0) {
    perror("sem_wait");
    pthread_exit(NULL);
  }
  printf("Thread Two waited on semaphore.\n");
  pthread_exit(NULL);
}

int main() {
  if (sem_init(&semaphore, 0, 0) != 0) {
    perror("sem_init");
    return EXIT_FAILURE;
  }

  pthread_t thread_one, thread_two;
  if (pthread_create(&thread_one, NULL, thread_one_func, NULL) != 0) {
    perror("pthread_create Thread One");
    sem_destroy(&semaphore);
    return EXIT_FAILURE;
  }
  if (pthread_create(&thread_two, NULL, thread_two_func, NULL) != 0) {
    perror("pthread_create Thread Two");
    sem_destroy(&semaphore);
    return EXIT_FAILURE;
  }

  pthread_join(thread_one, NULL);
  pthread_join(thread_two, NULL);

  if (sem_destroy(&semaphore) != 0) {
    perror("sem_destroy");
    return EXIT_FAILURE;
  }

  printf("Main thread exiting.\n");
  return EXIT_SUCCESS;
}

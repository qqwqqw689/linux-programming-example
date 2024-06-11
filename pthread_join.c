#include <pthread.h>
#include <stdlib.h> // malloc
#include <string.h> // strcpy
#include <stdio.h> // perror

void *thread(void *arg) {
  char *ret;
  printf("thread() entered with argument '%s'\n", (char*)arg);
  if ((ret = (char*) malloc(20)) == NULL) {
    perror("malloc() error");
    exit(2);
  }
  strcpy(ret, "This is a test");
  pthread_exit(ret);
  // noreturn void pthread_exit(void *retval);
  // The pthread_exit() function terminates the calling thread and
  // returns a value via retval that (if the thread is joinable) is
  // available to another thread in the same process that calls pthread_join(3).
}

int main() {
  pthread_t thid;
  void *ret;

  if (pthread_create(&thid, NULL, thread, "thread 1") != 0) {
    perror("pthread_create() error");
    exit(1);
  }

  if (pthread_join(thid, &ret) != 0) {
    perror("pthread_create() error");
    exit(3);
  }

  printf("thread exited with '%s'\n", (char*)ret);
}

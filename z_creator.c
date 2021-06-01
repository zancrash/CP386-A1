#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int pid = fork();

  if (pid < 0) {
    fprintf(stderr, "An error occurred during the creation of the child process.");
    exit(1);
  } else if (pid == 0) {
    // child
    exit(0);
  } else {
    // parent
    sleep(60);  // this results in a zombie process
    printf("completed.");
  }

  return 0;
}
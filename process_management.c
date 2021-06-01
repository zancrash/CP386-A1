#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>

int main() {

  char * line = NULL;

  int pid = fork();

  if (pid<0){
    exit(1);
  } else if (pid == 0){
    FILE *fp = fopen("sample_in.txt", "r");
    fscanf(fp, "%s", line);
  }

  return 0;
}
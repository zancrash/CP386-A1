#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>


int main() {

  char * line = NULL;
  #define SHM_NAME "PROC_SHM"
  #define MEM_SPACE 1048576 

  //create shared memory area
  //int shm_fd = shm_open(SHM_NAME , 0_RDWR | 0_CREAT, S_IRUSR | S_IWUSR);
  int shm_fd = shm_open(SHM_NAME, 0_CREAT | 0_RDWR, 0666);

  //check for error
  if(shm_fd < 0){
    perror("Shared memory failed to initialize.");
    exit(-1)
  } else{
    printf("memory space created");
  }

  //set size of memory space
  ftruncate(shm_fd, MEM_SPACE);

  int pid = fork();

  if (pid < 0){

    exit(1);

  } else if (pid == 0){
    //in child process:
    
    FILE *fp = fopen("sample_in.txt", "r");

    //if file is empty...
    if(fp==NULL){
      printf("file not opened.")
      exit(-1);
    }

    //read the file
    int file_read = fread(memory, 1, MEM_SPACE-1, fp);

    if (file_read == 0){
      exit(-1);
    }

    //null character
    memory[file_read] = '\0';

    //fscanf(fp, "%s", line);

    //exit
    exit(0);
  }

  return 0;
}
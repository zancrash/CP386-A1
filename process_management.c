#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void writeOutput(char* command, char* output) {
  FILE* fp = fopen("output.txt", "a");  //append to end of file

  fprintf(fp, "The output of: %s : is\n", command);
  fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);

  fclose(fp);
}

#define MAX_LINE_LEN 60

int main() {
  int SIZE = 4096;          // size in bytes of shared memory object
  char* name = "OS";        // named of shared memory object
  int shm_fd;               // shared memory file descriptor
  void* mem_ptr;            // pointer to shared memory object
  char line[MAX_LINE_LEN];  // string to represent a single command
  //char commands_arr[5];     // array that holds the commands read from shared memory, will set up to be dynamic later
  //int command_lengths[5];  //temporary, for testing currently
  //int count = 0;           //temporary, for testing currently

  // create shared memory
  shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

  // setup size of shared memory
  ftruncate(shm_fd, SIZE);

  // map the shared memory
  mem_ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

  pid_t pid = fork();  // first child for reading file/writing to shared memory

  if (pid < 0) {
    fprintf(stderr, "An error occurred while forking.");
    exit(1);
  } else if (pid == 0) {
    // child
    FILE* fp = fopen("sample_in.txt", "r");

    // read from file and write each line to shared memory
    while (fgets(line, MAX_LINE_LEN - 1, fp) != NULL) {
      // printf("line looks like: %s\n", line);
      sprintf(mem_ptr, "%s", line);

      if (line[strlen(line) - 1] == '\n') {
        mem_ptr += strlen(line) - 2;  // account for end of line
        //command_lengths[count] = strlen(line) - 2;
      } else {
        mem_ptr += strlen(line);
        //command_lengths[count] = strlen(line);
      }
      //count++;
    }
    fclose(fp);
    close(shm_fd);  // close shared memory

  } else {
    // parent
    wait(NULL);

    // open shared memory
    shm_fd = shm_open(name, O_RDONLY, 0666);

    // map the shared memory
    mem_ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    // read from shared memory
    printf("%s", (char*)mem_ptr);

    // remove shared memory object
    shm_unlink(name);
    kill(pid, SIGKILL);  // end child process after done writing to memory
  }

  // split the shared memory string into separate commands and add them to the array (eventually dynamic)
  // for (int i = 0; i < 5; i++) {
  //   char temp[MAX_LINE_LEN];
  //   //strncpy(temp, line, command_lenghts[i]);
  //   //commands_arr[i] = line;
  // }

  // for (int i = 0; i < 5; i++) {
  //   printf("\ncommand length here is: %d\n", command_lengths[i]);
  // }

  return 0;
}

char* line = NULL;
// #define SHM_NAME "PROC_SHM"
// #define MEM_SPACE 1048576

// //create shared memory area
// //int shm_fd = shm_open(SHM_NAME , 0_RDWR | 0_CREAT, S_IRUSR | S_IWUSR);
// int shm_fd = shm_open(SHM_NAME, 0_CREAT | 0_RDWR, 0666);

// //check for error
// if(shm_fd < 0){
//   perror("Shared memory failed to initialize.");
//   exit(-1)
// } else{
//   printf("memory space created");
// }

// //set size of memory space
// ftruncate(shm_fd, MEM_SPACE);

// int pid = fork();

// if (pid < 0){

//   exit(1);

// } else if (pid == 0){
//   //in child process:

//   FILE *fp = fopen("sample_in.txt", "r");

//   //if file is empty...
//   if(fp==NULL){
//     printf("file not opened.")
//     exit(-1);
//   }

//   //read the file
//   int file_read = fread(memory, 1, MEM_SPACE-1, fp);

//   if (file_read == 0){
//     exit(-1);
//   }

//   //null character
//   memory[file_read] = '\0';

//   //fscanf(fp, "%s", line);

//   //exit
//   exit(0);
// }

// return 0;
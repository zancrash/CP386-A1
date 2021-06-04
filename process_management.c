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
  int SIZE = 4096;                     // size in bytes of shared memory object
  char* name = "OS";                   // named of shared memory object
  int shm_fd;                          // shared memory file descriptor
  void* mem_ptr;                       // pointer to shared memory object
  char line[MAX_LINE_LEN];             // string to represent a single command
  char commands_arr[5][MAX_LINE_LEN];  // array that holds the commands read from shared memory, will set up to be dynamic later

  int count = 0;  //temporary, for testing currently

  pid_t pid;
  pid = fork();  // first child for reading file/writing to shared memory

  if (pid < 0) {
    fprintf(stderr, "An error occurred while forking.");
    exit(1);
  } else if (pid == 0) {
    // child
    FILE* fp = fopen("sample_in.txt", "r");

    // create shared memory
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

    // setup size of shared memory
    ftruncate(shm_fd, SIZE);

    // map the shared memory
    mem_ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // read from file and write each line to shared memory
    while (fgets(line, MAX_LINE_LEN - 1, fp) != NULL) {
      // printf("line looks like: %s\n", line);
      sprintf(mem_ptr, "%s", line);
      mem_ptr += strlen(line);
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

    // print shared memory result
    // printf("%s", (char*)mem_ptr);

    // remove shared memory object
    shm_unlink(name);
    kill(pid, SIGKILL);  // end child process after done writing to memory

    // split the read memory string into separate command strings
    count = 0;
    char* token;
    const char delimiter[2] = "\n";
    char shared_memory_result[MAX_LINE_LEN];
    strcpy(shared_memory_result, mem_ptr);
    token = strtok(shared_memory_result, delimiter);  // initial starting token (1st command)

    // each token represents a different linux command string, and is added to the array.
    while (token != NULL) {
      // printf("\n\ntoken is: %s\n", token);
      strcpy(commands_arr[count], token);
      count++;
      token = strtok(NULL, delimiter);  // move to next token
    }
    // ================== testing ==================== //
    /*
    printf("\nSplitting completed.\n");
    for (int i = 0; i < 5; i++) {
      printf("Command at index %d is: %s\n", i, commands_arr[i]);
    }
    */

    // ================== testing =================== //
  }

  return 0;
}

// char* line = NULL;
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
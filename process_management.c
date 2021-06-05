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

typedef struct command_info {
  char full_command[MAX_LINE_LEN];  // entire command with flags
  char flags[5][10];                //make dynamic later
  char command[MAX_LINE_LEN];       // only the command
  int num_flags;
} COMMAND_INFO;

int main() {
  int SIZE = 4096;               // size in bytes of shared memory object
  char* name = "OS";             // named of shared memory object
  int shm_fd;                    // shared memory file descriptor
  void* mem_ptr;                 // pointer to shared memory object
  char line[MAX_LINE_LEN];       // string to represent a single command
  COMMAND_INFO commands_arr[5];  // array that holds the commands read from shared memory, will set up to be dynamic later

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
      strcpy(commands_arr[count].full_command, token);
      strcpy(commands_arr[count].command, token);
      count++;
      token = strtok(NULL, delimiter);  // move to next token
    }

    pid_t pid_2 = fork();

    if (pid_2 < 0) {
      fprintf(stderr, "An error occurred while forking.");
      exit(1);
    } else if (pid_2 == 0) {
      // child
      count = 0;
      char* token;
      const char delimiter[2] = " ";  // split on blank spaces

      for (int i = 0; i < 5; i++) {
        // will change this to not be hardcoded when the dynamic array is setup properly
        if (i != 4) {
          commands_arr[i].full_command[strlen(commands_arr[i].full_command) - 1] = '\0';
          commands_arr[i].command[strlen(commands_arr[i].command) - 1] = '\0';  //remove newline character on each command string as it interferes with the execution
        }

        token = strtok(commands_arr[i].command, delimiter);  // first token
        while (token != NULL) {
          //printf("\ntoken is: %s\n", token);
          strcpy(commands_arr[i].flags[count], token);  // add flags to flag array
          token = strtok(NULL, delimiter);              // next token
          count++;
        }
        commands_arr[i].num_flags = count;
        count = 0;

        printf("Command at index %d is: %s\n", i, commands_arr[i].full_command);

        //char* terminator = NULL;  // needed for execvp call (last arg must be null)
        //strcpy(commands_arr[i].flags[count + 1], terminator);

        //system(commands_arr[i].full_command);
      }
      //char* terminator = NULL;  // needed for execvp call (last arg must be null)

      // printf("first command is: %s\n", commands_arr[0].flags[0]);
      // //strcpy(argv[0], commands_arr[0].flags[0]);
      // argv[0] = commands_arr[0].flags[0];
      // argv[1] = commands_arr[0].flags[1];
      // argv[2] = NULL;
      // execvp(argv[0], argv);

      for (int i = 0; i < 5; i++) {
        char* argv[5];
        printf("num_flags for the command is: %d\n", commands_arr[i].num_flags);
        count = 0;
        while (count != commands_arr[i].num_flags) {
          argv[count] = commands_arr[i].flags[count];
          count++;
        }
        argv[commands_arr[i].num_flags] = NULL;
        // execvp(argv[0], argv);
        //system(commands_arr[i].full_command);
      }
      //printf("argv[0] is: %s\n", argv[0]);
      //execvp(commands_arr[0].flags[0], argv);
      //execvp(commands_arr->flags[0], commands_arr->flags);
      // for (int i = 0; i < 5; i++) {
      // }
      // printf("testing execvp... \n");
      // char* cmd = "pwd";
      // char* argv[3];
      // argv[0] = "pwd";
      // argv[1] = NULL;
      // argv[1] = "-l";
      // argv[2] = "-a";
      // argv[3] = "-F";
      // argv[4] = NULL;

      //execvp(cmd, argv);
    } else {
      wait(NULL);
      // parent
    }
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
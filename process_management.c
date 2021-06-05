/*
----------------------------------------------------
 Project: 201764720_yourLaurierID_a01_q02
 File:    process_management.c
 Authors:  Martin Klasninovski | Muhammad Ali
 Laurier IDs: 201764720 | 191651560
----------------------------------------------------
*/

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

//function to parse and execute command
void execute_cmd(char* const args[], int writepipe) {
  if (dup2(writepipe, STDOUT_FILENO) == -1) {
    perror("error dup2");
    exit(-1);
  }

  execvp(args[0], args);
}

#define MAX_LINE_LEN 60

typedef struct command_info {
  char full_command[MAX_LINE_LEN];  // entire command with flags
  char flags[5][10];                //make dynamic later
  char command[MAX_LINE_LEN];       // only the command
  int num_flags;
} COMMAND_INFO;

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    printf("No input file given. Please try again.");
  } else {
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
      FILE* fp = fopen(argv[1], "r");  //argv[1] holds the input file to read from

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
        }

        for (int i = 0; i < 5; i++) {
          char* args[5];
          printf("num_flags for the command is: %d\n", commands_arr[i].num_flags);
          count = 0;
          while (count != commands_arr[i].num_flags) {
            args[count] = commands_arr[i].flags[count];
            count++;
          }
          args[commands_arr[i].num_flags] = NULL;  //execvp requires NULL final argument to execute properly
          // execvp(argv[0], argv);
          system(commands_arr[i].full_command);
        }

      } else {
        wait(NULL);
        // parent

        //initialize buffer to store output
        char* buffer = malloc(sizeof(char) * BUFFER);

        //Ignore this, was trying something...
        /*
        for (int i = 0; i<5; i++){
          char* cmd = strdup(commands_arr[i]);
        }*/

        //create pipe
        int pipefd[2];

        //check for error
        if (pipe[pipefd] == -1) {
          perror("Failed to create pipe.");
          exit(-1);
        }

        if (fork() == 0) {
          //close read-end of pipe
          close(pipefd[0]);

          //execute command here
          execute_cmd(argv, pipefd[1]);

          exit(0);
        }

        //close write-end of pipe
        close(pipefd[1]);
        wait(NULL);

        //get pipe output
        int read_n = read(pipefd[0], buffer, BUFFER - 1);

        //close pipe
        close(pipefd[0]);

        buffer[read_n] = '\0';

        //write to output.txt here?
        writeOutput()
      }
    }

    free(buffer);

    return 0;
  }
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
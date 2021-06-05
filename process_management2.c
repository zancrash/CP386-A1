/*
----------------------------------------------------
 Project: 201764720_yourLaurierID_a01_q02
 File:    process_management.c
 Authors:  Martin Klasninovski | yourName
 Laurier IDs: 201764720 | yourLaurierID
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

#define MAX_LINE_LEN 60

typedef struct command_info {
  char full_command[MAX_LINE_LEN];  // entire command with flags
  char flags[5][10];                //make dynamic later
  char command[MAX_LINE_LEN];       // only the command
  int num_flags;
} COMMAND_INFO;

void writeOutput(char* command, char* output) {
  FILE* fp = fopen("output.txt", "a");  //append to end of file

  fprintf(fp, "The output of: %s : is\n", command);
  fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);

  fclose(fp);
}

void read_from_and_write_to_pipe(char* args[], COMMAND_INFO cmd_info) {
  int fd[2];

  // fd[0] - read from, fd[1] - write from
  if (pipe(fd) == -1) {
    fprintf(stderr, "An error occured while opening the pipe \n");
    exit(-1);
  }  // create pipe

  pid_t pid_2 = fork();

  if (pid_2 < 0) {
    fprintf(stderr, "An error occurred while forking.");
    exit(1);
  } else if (pid_2 == 0) {
    // child
    close(fd[0]);                // not reading, so close this end
    dup2(fd[1], STDOUT_FILENO);  // redirect output to write pipe
    close(fd[1]);                // close when done writing.

    execvp(args[0], args);  // output from execution gets redirected
  }
  // parent
  int status;
  wait(&status);
  char buff[1024];
  memset(buff, 0, strlen(buff));  //zero out the buffer

  close(fd[1]);  // not writing, so close this end.

  if (read(fd[0], buff, sizeof(buff) - 1) == -1) {
    printf("An error occurred while reading from the pipe.\n");
    exit(-1);
  }
  close(fd[0]);  // close when done reading.

  // for testing
  // printf("Command is: %s\n", cmd_info.full_command);
  // printf("Output is: %s\n", buff);

  writeOutput(cmd_info.full_command, buff);
}

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

    pid_t pid = fork();  // first child for reading file/writing to shared memory

    if (pid < 0) {
      fprintf(stderr, "An error occurred while forking.");
      exit(1);
    } else if (pid == 0) {
      // child
      FILE* fp = fopen(argv[1], "r");  //argv[1] holds the input file to read from

      shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);             // create shared memory
      ftruncate(shm_fd, SIZE);                                     // setup size of shared memory
      mem_ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);  // map the shared memory

      // read from file and write each line to shared memory
      while (fgets(line, MAX_LINE_LEN - 1, fp) != NULL) {
        // printf("line looks like: %s\n", line);
        sprintf(mem_ptr, "%s", line);
        mem_ptr += strlen(line);
      }
      fclose(fp);
      close(shm_fd);  // close shared memory
      exit(0);        // end child process
    }
    // parent
    wait(NULL);

    shm_fd = shm_open(name, O_RDONLY, 0666);                    // open shared memory
    mem_ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);  // map the shared memory
    close(shm_fd);                                              // close shared memory
    shm_unlink(name);                                           // remove shared memory object

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

    // split each command string into separate pieces (so flags can be identified)
    count = 0;
    char* tokens;
    const char delimiters[2] = " ";  // split on blank spaces

    for (int i = 0; i < 5; i++) {
      // will change this to not be hardcoded when the dynamic array is setup properly
      if (i != 4) {
        commands_arr[i].full_command[strlen(commands_arr[i].full_command) - 1] = '\0';
        commands_arr[i].command[strlen(commands_arr[i].command) - 1] = '\0';  //remove newline character on each command string as it interferes with the execution
      }

      tokens = strtok(commands_arr[i].command, delimiters);  // first token
      while (tokens != NULL) {
        strcpy(commands_arr[i].flags[count], tokens);  // add flags to flag array
        tokens = strtok(NULL, delimiters);             // next token
        count++;
      }
      commands_arr[i].num_flags = count;  // set the total # of flags for each command
      count = 0;
    }

    // iteratively call on helper function to fork and perform execvp() calls
    for (int i = 0; i < 5; i++) {
      char* args[5];
      count = 0;
      while (count != commands_arr[i].num_flags) {
        args[count] = commands_arr[i].flags[count];
        count++;
      }
      args[commands_arr[i].num_flags] = NULL;  //execvp requires NULL

      read_from_and_write_to_pipe(args, commands_arr[i]);
      //writeOutput()?
    }

    return 0;
  }
}
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

void writeOutput(char* command, char* output, int opened) {
  FILE* fp;
  if (!opened) {
    fp = fopen("output.txt", "w");
  } else {
    fp = fopen("output.txt", "a");
  }
  //append to end of file
  fprintf(fp, "The output of: %s : is\n", command);
  fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);

  fclose(fp);
}
//char* buff_copy 3rd param if using massive buffer method
void read_from_and_write_to_pipe(char* args[], COMMAND_INFO cmd_info, int opened) {
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

  memset(buff, 0, sizeof(buff));  //zero out the buffer

  close(fd[1]);  // not writing, so close this end.

  // int bytes_read = read(fd[0], buff, sizeof(buff));
  if (read(fd[0], buff, sizeof(buff)) == -1) {
    printf("An error occurred while reading from the pipe.\n");
    exit(-1);
  }

  //">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n"
  close(fd[0]);  // close when done reading.
  //"The output of: %s : is\n"
  // strcpy(buff_copy, "The output of: ");
  // strcat(buff_copy, cmd_info.full_command);
  // strcat(buff_copy, " : is\n>>>>>>>>>>>>>>>\n");
  // strcat(buff_copy, buff);
  // strcat(buff_copy, "<<<<<<<<<<<<<<<\n");
  // strcat(buff_copy, )

  // for testing
  // printf("Command is: %s\n", cmd_info.full_command);
  // printf("Output is: %s\n", buff);

  writeOutput(cmd_info.full_command, buff, opened);
}

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    printf("No input file given. Please try again.");
  }
  int MEM_SIZE = 4096;                                                   // size in bytes of shared memory object
  char* shared_mem_name = "OS";                                          // named of shared memory object
  int shm_fd;                                                            // shared memory file descriptor
  void* mem_ptr;                                                         // pointer to shared memory object
  char line[MAX_LINE_LEN];                                               // string to represent a single command
  int arr_size = 1;                                                      // size of dynamic array at start is 1
  int num_elements = 0;                                                  // total # of elements in array at start
  COMMAND_INFO* commands_arr = malloc(arr_size * sizeof(COMMAND_INFO));  // array that holds the commands read from shared memory, will set up to be dynamic later

  int count = 0;  //temporary, for testing currently

  pid_t pid = fork();  // first child for reading file/writing to shared memory

  if (pid < 0) {
    fprintf(stderr, "An error occurred while forking.");
    exit(1);
  } else if (pid == 0) {
    // child
    FILE* fp = fopen(argv[1], "r");  //argv[1] holds the input file to read from

    shm_fd = shm_open(shared_mem_name, O_CREAT | O_RDWR, 0666);      // create shared memory
    ftruncate(shm_fd, MEM_SIZE);                                     // setup size of shared memory
    mem_ptr = mmap(0, MEM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);  // map the shared memory

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

  shm_fd = shm_open(shared_mem_name, O_RDONLY, 0666);             // open shared memory
  mem_ptr = mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);  // map the shared memory

  close(shm_fd);                // close shared memory
  shm_unlink(shared_mem_name);  // remove shared memory object

  // split the read memory string into separate command strings
  char* token;
  const char delimiter[2] = "\n";
  char shared_memory_result[MAX_LINE_LEN];
  strcpy(shared_memory_result, mem_ptr);
  token = strtok(shared_memory_result, delimiter);  // initial starting token (1st command)

  // each token represents a different linux command string, and is added to the array.
  while (token != NULL) {
    // printf("\n\ntoken is: %s\n", token);
    if (num_elements == arr_size) {                                           // check if there is space in array
      arr_size *= 2;                                                          // double the size of array
      commands_arr = realloc(commands_arr, arr_size * sizeof(COMMAND_INFO));  // allocate more memory for elements.
    }

    strcpy(commands_arr[num_elements].full_command, token);
    strcpy(commands_arr[num_elements].command, token);
    num_elements++;
    token = strtok(NULL, delimiter);  // move to next token
  }

  // split each command string into separate pieces (so flags can be identified)
  count = 0;
  char* tokens;
  const char delimiters[2] = " ";  // split on blank spaces

  for (int i = 0; i < num_elements; i++) {
    // will change this to not be hardcoded when the dynamic array is setup properly
    if (i != num_elements - 1) {
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
  //char BUFFER[1024 * num_elements];
  int opened = 0;
  for (int i = 0; i < num_elements; i++) {
    char* args[num_elements];
    count = 0;
    while (count != commands_arr[i].num_flags) {
      args[count] = commands_arr[i].flags[count];
      count++;
    }
    if (i != 0) {
      opened = 1;
    } else {
      opened = 0;
    }
    args[commands_arr[i].num_flags] = NULL;  //execvp requires NULL

    read_from_and_write_to_pipe(args, commands_arr[i], opened);
    // char buff2[2048];
    // memset(buff2, 0, sizeof(buff2));

    // read_from_and_write_to_pipe(args, commands_arr[i], buff2);
    // //printf("buffer looks like: %s\n", buff2);
    // strcat(BUFFER, buff2);
  }

  // writeOutput()

  // //testing
  // count = 0;
  // char* tokenz;
  // const char delimiterz[2] = "\n";  // split on blank spaces
  // tokenz = strtok(BUFFER, delimiterz);

  // while (tokenz != NULL) {
  //   printf("%s\n", tokenz);
  //   tokenz = strtok(NULL, delimiterz);
  // }

  free(commands_arr);

  return 0;
}
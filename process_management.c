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


#define SIZE 4096
#define SHM "PROC_SHM"
#define BUFFER_SIZE 6024

void writeOutput(char* command, char* output) {
  FILE* fp = fopen("output.txt", "a");  //append to end of file

  fprintf(fp, "The output of: %s : is\n", command);
  fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);

  fclose(fp);
}

int main(int argc, char* argv[]){

    if (argc <= 1) {
      printf("No input file given. Please try again.");
      exit(-1);
    }

    //char* name = "OS";             // named of shared memory object
    int shm_fd;                    // shared memory file descriptor
    //void* mem_ptr;                 // pointer to shared memory object
    //char *mem_ptr

    shm_fd = shm_open(SHM, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    //shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);             // create shared memory
    ftruncate(shm_fd, SIZE);                                     // setup size of shared memory
    char *mem_ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    //mem_ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);  // map the shared memory

    pid_t pid = fork();

    // read files within child process...
    if (pid < 0){
      fprintf(stderr, "An error occurred while forking.");
      exit(1);
    } else if(pid == 0){

      //CHILD

      FILE *fp = fopen(argv[1], "r");

      //read contents
      int line_read = fread(mem_ptr, 1, SIZE-1, fp);

      if(line_read == 0){
        exit(-1);
      }
      mem_ptr[line_read] = '\0';
      exit(0);
    }

    // PARENT

    //parse lines...
    //char *mem;
    int lines = 0;
    int x = 0;
    while (mem_ptr[x] != '\0') {
        //if new line...
        if (mem_ptr[x] == '\n'){
            lines++;
            mem_ptr[x] = '\0';
        }
        x++;
    }
    
    if(x > 0 && mem_ptr[x-1] == '\n'){
        mem_ptr[x-1] = '\0';
        lines--;
    }
    lines += 1;


//----------------------------------
    //command array
    char** commands_arr = malloc(sizeof(char*)*lines);

    //copy commands
    int offset = 0;
    for(int i=0; i<lines; i++){
        commands_arr[i] = strdup(mem_ptr + offset);

        // update offset var, add 1 for null char
        offset += strlen(commands_arr[i]) + 1;
    }
    

    munmap(mem_ptr , SIZE); //unmap virtual mem
    close(shm_fd); //close shared mem

    //buffer to store output
    char* buffer = malloc(sizeof(char)*BUFFER_SIZE);

    //iterate for each child
    for(int i=0; i<lines; i++){

        char* cmd_current = strdup(commands_arr[i]);

        //get no. of args
        int argc = 0;
        int len = strlen(cmd_current);
        for(int i = 0; i < len; i++){
          if(cmd_current[i] == ' '){
            cmd_current[i] = '\0';
            argc++;
          } 
        }

        argc = argc+1;

        //create a new arr
        char* cmd_array[argc + 1];
        //set pointer
        int arg_offset = 0;
        for(int i = 0; i < argc; i++){
            cmd_array[i] = cmd_current + arg_offset;
            arg_offset += strlen(cmd_array[i]) + 1;
        }

        //null
        cmd_array[argc] = NULL;

        //create pipe
        int pipefd[2];
        if(pipe(pipefd) == -1){
            perror("Failed to initiate pipe.");
            exit(-1);
        }

        //fork
        pid_t pid_2 = fork();

        if(pid_2 == 0){
            //CHILD:

            //close read end
            close(pipefd[0]);
            //execute cmd
            if(dup2(pipefd[1], STDOUT_FILENO) == -1){
              exit(-1);
            }
            execvp(cmd_array[0] , cmd_array);
            exit(0);
        }

        //PARENT:

        //close write end
        close(pipefd[1]);

        wait(NULL);

        //pipe output
        int read_line = read(pipefd[0], buffer, BUFFER_SIZE - 1);

        //close pipe
        close(pipefd[0]);

        //set NULL
        buffer[read_line] = '\0';

        //output to file
        writeOutput(commands_arr[i], buffer);
        
        //free cmd
        free(cmd_current);
    }

    free(buffer);
    for(int i = 0; i < lines; i++){
        free(commands_arr[i]);
    }
    free(commands_arr);
    return 0;
}
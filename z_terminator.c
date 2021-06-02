#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  // call on z_creator so that it runs in the background
  system("./z_creator &");

  // identify the zombie process with <ps -l>, process with a Z state are zombies
  printf("List of processes are as follows: \n");
  system("ps -l");

  // obtain the pid of the zombie process and redirect the value to a text file to be read from
  system("ps -l | grep -w Z | tr -s ' '| cut -d ' ' -f 5 > zombie_ppid.txt");
  printf("\n");

  // get zombie ppid from output file and concatenate it to kill command
  char kill_cmd[16] = "kill -9 ";
  char line[8];
  FILE *fp = fopen("zombie_ppid.txt", "r");

  if (fgets(line, 7, fp) != NULL) {
    printf("The zombie's PPID is: %s\n", line);
    strcat(kill_cmd, line);

    // kill the zombie process parent using [kill -9 <ppid>]
    system(kill_cmd);

    // print the updated list of all the other processes and their statuses
    printf("List of processes after removing the zombie process: \n");
    system("ps -l");
    system("rm zombie_ppid.txt");  // remove unnecessary file once done
  } else {
    printf("No zombie process found.");
  }

  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  // 1. call on z_creator so that it runs in the background
  char *args[] = {"./z_creator &", NULL};
  //execv(args[0], args);
  system(args[0]);

  // 2. identify the zombie process with <ps -l>, process with a Z state are zombies
  printf("List of processes are as follows: \n");
  system("ps -l");

  // 3. obtain the pid of the zombie process
  system("ps -l | grep -w Z | tr -s ' '| cut -d ' ' -f 5 > zombie_ppid.txt");
  printf("\n");

  // note: there is probably a better (easier?) way of going about this, should also add error checks if sticking with this.
  // get zombie ppid from output file and concatenate it to kill command
  char kill_cmd[15] = "kill -9 ";
  char line[10];
  FILE *fp = fopen("zombie_ppid.txt", "r");

  fscanf(fp, "%s", line);
  printf("The zombie's PPID is: %s\n\n", line);
  strcat(kill_cmd, line);

  // 4. kill the zombie process parent using [kill -9 <ppid>]
  system(kill_cmd);

  // 5. print the updated list of all the other processes and their statuses
  printf("List of processes after removing the zombie process: \n");
  system("ps -l");
  system("rm zombie_ppid.txt");  // remove unnecessary file once done

  return 0;
}
/*
----------------------------------------------------
 Project: 201764720_yourLaurierID_a01_q12
 File:    z_terminator.c
 Authors:  Martin Klasninovski | Muhammad Ali
 Laurier IDs: 201764720 | 191651560
----------------------------------------------------
*/

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

  // obtain the ppid of the zombie process and kill it
  system("kill -9 $(ps -l | grep -w Z | tr -s ' '| cut -d ' ' -f 5)");
  printf("\n");

  // print the updated list of all the other processes and their statuses
  printf("List of processes after removing the zombie process: \n");
  system("ps -l");

  return 0;
}
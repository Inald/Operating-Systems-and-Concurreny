#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "coursework.h"
#include "linkedlist.h"

// Implementation of the FCFS scheduling algorithm (by Michael and Inald)
// OSC Coursework - Part One - Task 1a
struct element** createProcesses(struct element** processList)
{
  int i;
  struct process* newProcess;
  for(i = 0; i < NUMBER_OF_JOBS; i++)
  {
    newProcess = generateProcess(); 
    newProcess += i*sizeof(struct element**);
    //printf("GENERATED PROCESS - ID: %x, Priority:%d\n", newProcess -> iProcessId, newProcess ->iPriority);
    addLast(NULL, (struct element **)newProcess, (struct element **)newProcess);      
    printf("Address of list start: %x, Address of process: %x, ID: %d\n", &processList, &newProcess, newProcess -> iProcessId);
  }
  
  return processList;
}

int main()
{
  pid_t pid;
  int pBurst, nBurst, response, turnAround, i;
  double avgResponse, avgTurnAround;
  struct element** processList = createProcessses(processList);
  //Create a pointer that points to a pointer of elements
  printf("Process ID = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %d, Turn Around Time = %d\n", pid, pBurst, nBurst, response, turnAround);
  printf("Average response time = %f\nAverage turn around time = %f\n", avgResponse, avgTurnAround);
  return 0;
}


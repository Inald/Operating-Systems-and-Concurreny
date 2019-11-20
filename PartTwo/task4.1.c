#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"

//Semaphores for sync, delaying consumer
//Counting semaphore represents number of free elements (number of jobs not filled)
sem_t sSync, sEmpty, sFull;
int produced = 0, consumed = 0, dAverageResponseTime, dAverageTurnAroundTime, count2 = 0;
//Create head and tail pointers to pointers for the linked list
// struct element *ptrH = NULL, *ptrT = NULL;
// struct element **head = &ptrH, **tail = &ptrT;
struct element *headArray[MAX_PRIORITY], *tailArray[MAX_PRIORITY];
double calcAverage(int sum, int n)
{
  return ((double)sum/(double)n);
}

void printSems()
{
    int sema, semb;
    sem_getvalue(&sFull, &sema);
    sem_getvalue(&sEmpty, &semb);
    printf("Full = %d, Empty = %d\n", sema, semb);
}
struct process * processJob(int iConsumerId, struct process * pProcess, struct timeval oStartTime, struct timeval oEndTime)
{
	int iResponseTime;
	int iTurnAroundTime;
	if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime > 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
		dAverageResponseTime += iResponseTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2	 ? "FCFS" : "RR",pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime);
		return pProcess;
	} else if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
		dAverageResponseTime += iResponseTime;
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
		dAverageTurnAroundTime += iTurnAroundTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	} else if(pProcess->iPreviousBurstTime != pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime > 0)
	{
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime);
		return pProcess;
	} else if(pProcess->iPreviousBurstTime != pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
		dAverageTurnAroundTime += iTurnAroundTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	}
}

 void * consumerFunc(void *id)
 {
     int count,cID = (*(int *) id), currentPriority = 0;
     struct process *firstProcess, *processRun; 
     struct timeval start, end;
     while(consumed < MAX_NUMBER_OF_JOBS)
     {
        sem_wait(&sFull);
        if(consumed == MAX_NUMBER_OF_JOBS){
            break;
        }
        sem_wait(&sSync);
        for(int i = 0; i < MAX_PRIORITY; i++){
            if(headArray[i]){
                currentPriority = i;
                break;
            }
        }
        firstProcess = removeFirst(&headArray[currentPriority], &tailArray[currentPriority]);
        sem_post(&sSync);
        //check for values of time
        start = firstProcess -> oTimeCreated;
        end = firstProcess -> oMostRecentTime; 
        runJob(firstProcess, &start, &end);
        sem_wait(&sSync);
        processRun = processJob(cID, firstProcess, start, end);
        if(processRun){
            addLast(processRun, &headArray[currentPriority], &tailArray[currentPriority]);
            sem_post(&sFull);
        }else{
            consumed++;
            sem_post(&sEmpty);
        }
        //printf("consumer = %d\n", consumed);
        sem_post(&sSync);
        
     }
     if(count2 < 2){
        sem_wait(&sSync);
        sem_post(&sFull);
        count2++;
        sem_post(&sSync);
     }
     printf("Consumer finished = %d\n", cID);
}


void * producerFunc()
{
    struct process *newProcess;
    int count, pID = 1;
    
    while(produced < MAX_NUMBER_OF_JOBS)
    {
        sem_wait(&sEmpty);
        sem_wait(&sSync);
        newProcess = generateProcess();
        addLast(newProcess, &headArray[newProcess -> iPriority], &tailArray[newProcess -> iPriority]);
        produced++;
        sem_post(&sSync);
        sem_post(&sFull);
        
    }
}

int main(int argc, char **argv)
{
    pthread_t consumer[NUMBER_OF_CONSUMERS], producer;
    int finalSync, finalEmpty, finalFull, id = 0;
    sem_init(&sSync, 0 , 1);
    sem_init(&sEmpty, 0 , MAX_BUFFER_SIZE);
    sem_init(&sFull, 0, 0);
    pthread_create(&producer, NULL, producerFunc, NULL);

    for(int i = 0; i < NUMBER_OF_CONSUMERS; i++){
        pthread_create(&consumer[i], NULL, consumerFunc, &id);
        id++;
    }

    pthread_join(producer, NULL);
    pthread_join(consumer[0], NULL);
    pthread_join(consumer[1], NULL);
    pthread_join(consumer[2], NULL);
    //Final values of semapores
    sem_getvalue(&sSync, &finalSync);
    sem_getvalue(&sEmpty, &finalEmpty);
    sem_getvalue(&sFull, &finalFull);
    printf("sSync = %d, sEmpty = %d, sFull = %d\n", finalSync, finalEmpty, finalFull);
    printf("Average Reponse Time = %f\n", calcAverage(dAverageResponseTime, NUMBER_OF_JOBS));
    printf("Average Turnaround Time = %f\n", calcAverage(dAverageTurnAroundTime, NUMBER_OF_JOBS));
    return 0;
}

/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include "semaphore.h"

//Create Semaphore Data Structure
Semaphore *createSemaphore(int num)
{
  Semaphore *sem = malloc(sizeof(Semaphore));
  if (sem == NULL)
  {
    errExit("Semaphore malloc failed!");
  }
  sem->count = num;
  sem->id = semget(IPC_PRIVATE, num, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  if (sem->id == -1)
  {
    errExit("(createSemaphore)semget failed");
  }

  return sem;
}

//Destroy Semaphore Data Structure
void destroySemaphore(Semaphore *sem)
{
  if (semctl(sem->id, 0, IPC_RMID, NULL) == -1)
    errExit("semctl IPC_RMID failed");
  free(sem);
}

//Set Sempahores values
void setAllSemaphore(Semaphore *sem, int init_vals[])
{
  //unsigned short devices_sem_init_val[] = {0, 0, 0, 0, 0, 0};
  //printf("Addr InitVals %d\n", init_vals);

  union semun arg;
  arg.array = (unsigned short *)init_vals;

  if (semctl(sem->id, 0, SETALL, arg))
    errExit("(setAllSemaphore)semctl failed");
}

//P(sem[num])
void waitSemaphore(Semaphore *sem, int num)
{
  semOp(sem->id, (unsigned short)num, -1);
}

//V(sem[num])
void signalSemaphore(Semaphore *sem, int num)
{
  semOp(sem->id, (unsigned short)num, 1);
}

void semOp(int semid, unsigned short sem_num, short sem_op)
{
  struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

  if (semop(semid, &sop, 1) == -1)
    errExit("semop failed");
}

/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.
#pragma once
#ifndef _SEMAPHORE_HH
#define _SEMAPHORE_HH

#include "defines.h"
#include "utils.h"

struct Semaphore
{
  int id;
  int count;
};
typedef struct Semaphore Semaphore;

// definition of the union semun
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

Semaphore *createSemaphore(int num);
void destroySemaphore(Semaphore *sem);
void setAllSemaphore(Semaphore *sem, int init_vals[]);
void waitSemaphore(Semaphore *sem, int num);
void signalSemaphore(Semaphore *sem, int num);

/* errsemOpExit is a support function to manipulate a semaphore's value
 * of a semaphore set. semid is a semaphore set identifier, sem_num is the
 * index of a semaphore in the set, sem_op is the operation performed on sem_num
 */
void semOp(int semid, unsigned short sem_num, short sem_op);

#endif

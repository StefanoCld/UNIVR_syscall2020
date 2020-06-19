/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include "fifo.h"

//Create FIFO Data Structure
Fifo *createFifo(char *fifo_path)
{
  Fifo *fifo = malloc(sizeof(Fifo));

  if (fifo == NULL)
  {
    errExit("Fifo malloc failed!");
  }

  fifo->fifo_path = fifo_path;

  int res = mkfifo(fifo->fifo_path, S_IRUSR | S_IWUSR);

  if (res == -1)
  {
    errExit("mkfifo failed!");
  }

  fifo->fifo_fd = open(fifo->fifo_path, IPC_CREAT | O_RDONLY | O_NONBLOCK);
  
  if (fifo->fifo_fd == -1)
  {
    printf("[ERROR] fifo path: %s\n", fifo->fifo_path);
    errExit("FIFO creation failed!");
  }

  return fifo;
}

void destroyFifo(Fifo *fifo)
{
  // Close the FIFO
  if (close(fifo->fifo_fd) != 0)
    errExit("FIFOclose failed");

  // Remove the FIFO
  if (unlink(fifo->fifo_path) != 0)
    errExit("FIFOunlink failed");
}

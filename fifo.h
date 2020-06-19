/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once
#ifndef _FIFO_HH
#define _FIFO_HH

#include "defines.h"
#include "utils.h"

//FIFO Data Structure
struct Fifo{
  char *fifo_path;
  int fifo_fd;
};
typedef struct Fifo Fifo;

Fifo* createFifo(char *fifo_path);
void destroyFifo(Fifo *fifo);

#endif
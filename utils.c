/// @file err_exit.c
/// @brief Contiene l'implementazione della funzione di stampa degli errori.

#include "utils.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

void errExit(const char *msg)
{
    perror(msg);
    exit(1);
}

void clearScreen(){
  printf("\e[1;1H\e[2J");
}

void waitForKey(){
  char nl;
  scanf("%c",&nl);
}
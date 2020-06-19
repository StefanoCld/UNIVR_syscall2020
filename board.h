#pragma once

#ifndef _BOARD_HH
#define _BOARD_HH

#include "defines.h"
#include "semaphore.h"
#include "shared_memory.h"

//Grid Data Structure
struct BoardGrid{
  pid_t cells[BOARD_GRID_SIZE][BOARD_GRID_SIZE];
};
typedef struct BoardGrid BoardGrid;

//Board Data Structure
struct Board{  
  MutexSharedMemory* board_mem;
  int dev_num;
};
typedef struct Board Board;

Board* createBoard(int dev_num);
void destroyBoard(Board* board);
void initBoard( Board* board );
void findDeviceBoard(Board *board, int x, int y, double max_distance, pid_t *devices_in_range);
void printBoard(Board *board);
void startTurnBoard(Board *board);
void waitTurnBoard(Board *board, int turn);
void endTurnBoard(Board *board, int turn);
void moveFromToBoard(Board *board, pid_t pid, int src_x, int src_y, int dst_x, int dst_y);


#endif
#pragma once

#include "defines.h"
#include "board.h"
#include "ack_table.h"
#include "position_file.h"
#include "message.h"
#include "fifo.h"

#ifndef _DEVICE_HH
#define _DEVICE_HH 

//Device Data Structure
struct Device{
  int num;
  int x;
  int y;
  int pos_count;
  int turn_num;

  pid_t pid;
  Board *board;
  AckTable *ack_table;
  PositionFileRow *pos;
  MessageList msg_list; 
  Fifo *fifo;
};
typedef struct Device Device;

Device* createDevice(int num, Board* board, AckTable* ack_table, PositionFileRow *pos, int pos_count);

void destroyDevice(Device *device);
int forkDevice(Device *device);

#endif
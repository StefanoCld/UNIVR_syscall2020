#pragma once
#ifndef _ACK_TABLE_HH
#define _ACK_TABLE_HH

#include "defines.h"
#include "shared_memory.h"
#include "message.h"

//Data structure of a single AckTable row
struct AckTableRow{
  pid_t pid_sender;
  pid_t pid_receiver;
  int message_id;
  time_t timestamp;
};
typedef struct AckTableRow AckTableRow;

//Data structure of the entire AckTable
struct AckTableList{
  AckTableRow rows[ACK_TABLE_SIZE];
};
typedef struct AckTableList AckTableList;

//Ack Manager Data structure
struct AckTable{
  pid_t pid; 
  MutexSharedMemory *ack_mem;
  int size;
  int dev_num;
  key_t msg_queue_key;
  int msg_queue_id;
};
typedef struct AckTable AckTable;

//Ack Table-Manager methods
AckTable* createAckTable(key_t msg_queue_key, int num_rows, int dev_num);
void destroyAckTable(AckTable* ack_table);

void initAckTable(AckTable *ack_table, int sem_value);
int forkAckTableManager(AckTable *ack_table);

//Ack Table Mutex Methods
void requestAckTable(AckTable *ack_table);
void releaseAckTable(AckTable *ack_table);

void printAckTable();
int searchForMsgInAckTable(AckTable *ack_table, int message_id, pid_t *devices_pid);
void addAckTableRow(AckTable *ack_table, AckTableRow *new_row);
void submitCompletedAck(AckTable *ack_table);

#endif
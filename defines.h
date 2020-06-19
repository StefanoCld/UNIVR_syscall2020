/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#ifndef _DEFINES_HH
#define _DEFINES_HH

//utils
#include <stdlib.h>  
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

//System call
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/msg.h> 
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

//Utility
#define FALSE (1==0)
#define TRUE  (1==1)

#define PRJ_ID 42

//Server
#define SERVER_RUN_FILE "output/server.run"
#define SERVER_DEBUG 0
#define EXIT_SIGNAL SIGTERM

//Devices
#define DEVICE_NUM 5
#define DEVICE_QUEUE_PATH "/tmp/dev_fifo.%d"
#define POS_ROW_LENGTH 21 
#define LOC_MSG_LEN 10

//Board
#define BOARD_GRID_SIZE 10
#define BOARD_SHM_PATH "/tmp/board_mem"
#define BOARD_SHM_PRJ PRJ_ID

//Ack Manager
#define ACK_TABLE_SIZE 100
#define ACK_TABLE_SHM_PATH "/tmp/ack_table"
#define ACK_TABLE_SHM_PRJ PRJ_ID

//Client
#define CLIENT_OUT_PATH "./output/out_message_id_%ld.txt"
#define MSG_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define MSG_LENGTH 256

#endif

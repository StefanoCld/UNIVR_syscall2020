
#pragma once

#ifndef _MESSAGE_HH
#define _MESSAGE_HH

#include "defines.h"
#include "utils.h"

//Message Data Structure
struct Message{
  pid_t pid_sender;
  pid_t pid_receiver;
  int message_id;
  char message[MSG_LENGTH];
  double max_distance;
};
typedef struct Message Message;

//Device internal message list Data Structure
struct MessageList{
  Message msg;
  struct MessageList *next;
};
typedef struct MessageList MessageList;

//MessageAck Data Structure (Ack_Manager -> Client)
struct MessageAck{
  pid_t pid_sender;
  pid_t pid_receiver;
  time_t timestamp;
};
typedef struct MessageAck MessageAck;

//MessageAck list Data Structure (Ack_Manager -> Client)
struct MessageAckList{
  long type;
  MessageAck rows[5];
};
typedef struct MessageAckList MessageAckList;


void waitAckQueue(key_t msg_queue_key, int pid, MessageAckList *msgAckList);
void saveMessageAck(MessageAckList *msgAckList, char* msg);

void sendMessage(Message *msg);

#endif

/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include "ack_table.h"
#include "message.h"

#include <sys/stat.h> 
#include <sys/msg.h> 

int main(int argc, char * argv[]) { 
  
  //Check number of arguments
  if(argc < 2){
    printf("\nUsage:\n $ %s <msg_queue_key> \n", argv[0]);
    exit(1);
  }

  //Read msg_queue's key
  key_t msg_queue_key = atoi(argv[1]);
  if(msg_queue_key <= 0){
    printf("The message_queue_key must be greater than zero!\n");
    exit(1);
  }

  Message msg;
  msg.pid_sender = getpid();
  
  //Receiver PID
  printf("Pid receiver\n");
  scanf(" %d", &msg.pid_receiver);
  
  //Message ID
  printf("Message ID\n");
  scanf(" %d", &msg.message_id);

  //Clean from unwanted chars
  int c;
  do{
    c = getchar();
  }
  while( c != EOF && c != '\n');

  //Message
  printf("Message\n");
  fgets(msg.message, MSG_LENGTH, stdin);

  //Max distance
  printf("Max distance\n");
  scanf(" %lf", &msg.max_distance);
  if(msg.max_distance < 1 || msg.max_distance > 10){
    errExit("Max distance value must range between 1 and 10\n");
  }

  //Send the message on the device's FIFO
  printf("\nSending message\n");
  printf("PID Sender:\t\t%d\n", msg.pid_sender);
  printf("PID Receiver:\t\t%d\n", msg.pid_receiver);
  printf("Message ID:\t\t%d\n", msg.message_id);
  printf("Max distance:\t\t%1.lf\n", msg.max_distance);
  printf("Message:\t\t%s\n", msg.message);
  
  sendMessage(&msg);
  
  printf("Wait for ACK message\n");

  //Wait for ack message from the Message Queue
  MessageAckList msgAckList;
	waitAckQueue(msg_queue_key, msg.message_id, &msgAckList);

	//Create a txt file, with the info contained in ackMsg
  saveMessageAck(&msgAckList, msg.message);

  return 0;
}



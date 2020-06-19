#include "message.h"

//Send msg to the receiver related FIFO
void sendMessage(Message *msg){
  char fifo_path[255];
  sprintf(fifo_path, DEVICE_QUEUE_PATH, (int)msg->pid_receiver); 

  int fd = open(fifo_path, O_WRONLY);
  
  if (fd == -1){ 
    errExit("Open FIFO failed!");
  }

	if (write(fd, msg, sizeof(Message)) == -1){
		errExit("Write FIFO failed!");
	}

	if (close(fd) == -1){
		errExit("Closing FIFO failed!");
	}
}

//Wait for the Ack complete message from the Message Queue
void waitAckQueue(key_t msg_queue_key, int message_id, MessageAckList *msgAckList){ 
	int mq_id = msgget(msg_queue_key, S_IRUSR | S_IWUSR);
  //printf("waitAckQueue get: %d\n",mq_id);
	
  //Blocking
  printf("Waiting for Ack messages\t");
  int res = msgrcv(mq_id, msgAckList, sizeof( MessageAckList ), (long)message_id, 0);
  printf("[ %s ]\n",(res == -1 ? "ERROR":"OK"));
  //DEBUG
  //printf("waitAckQueue rcv: %d\n",res);

}

//Create a txt file, with the info contained in ackMsg
void saveMessageAck(MessageAckList *msgAckList, char* msg){
  char txt_path[255];
  char txt_msg[512];
  char txt_time[12];

  sprintf(txt_path, CLIENT_OUT_PATH, msgAckList->type); 

  int txtfd = open(txt_path, O_WRONLY | O_CREAT | O_TRUNC | S_IRUSR, 0777);

	//Intestazione
	sprintf(txt_msg, "Messaggio %ld : %s \nLista acknowledgment: \n", msgAckList->type, msg);
	write(txtfd, txt_msg, strlen(txt_msg));

	for(int i = 0; i<5;i++){
    MessageAck msgAck = msgAckList->rows[i];
    struct tm *msgTime = localtime( &msgAck.timestamp );
    strftime(txt_time, 26, MSG_TIME_FORMAT, msgTime);
    sprintf(txt_msg, "%d, %d, %s\n", msgAck.pid_sender, msgAck.pid_receiver, txt_time); 
    write(txtfd, txt_msg, strlen(txt_msg));
  }
	
  close(txtfd);
}

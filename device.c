
#include "device.h"

//Private Methods
void runDevice(Device *device);
void initFifoDevice(Device *device);

void printMsgDevice(Device *device);
void submitDevice(Device *device);
void receiveDevice(Device *device);
void moveDevice(Device *device);

void initSignalsDevice();
void receiveSignalDevice(int signal);

int removeDevicesReceived(pid_t *devices_in_range, pid_t *devices_received);

//Create Device Data Structure
Device* createDevice(int num, Board* board, AckTable* ack_table, PositionFileRow *pos, int pos_count){
  Device *device = malloc(sizeof(Device));

  if(device == NULL){
		errExit("Device malloc failed!");
  }

  device->num = num;
  device->pid = -1;
  device->board = board;
  device->ack_table = ack_table;
  device->pos = pos;
  device->pos_count = pos_count;
  device->turn_num = 0;
  device->msg_list.next = NULL;

  return device;
}

//Fork from the Server Process
Device *forkedDevice;
int forkDevice(Device *device){
  pid_t pid = fork();

  //Failed
  if (pid < 0){ return FALSE; }
  
  //Parent
  if(pid > 0){ 
    device->pid = pid;
    return TRUE; 
  }
  
  forkedDevice = device;
  device->pid = getpid();
  
  runDevice(device);
  exit(0);
}

//Init Device's FIFO
void initFifoDevice(Device *device){
  char *fifo_path = malloc(sizeof(char)*255);

  if(fifo_path == NULL){
		errExit("Fifo_path malloc failed!");
  }

  //Create Fifo
  sprintf(fifo_path, DEVICE_QUEUE_PATH, device->pid);
  device->fifo = createFifo(fifo_path); 
}

//Device main code
void runDevice(Device *device){

  initSignalsDevice();

	//Creating local list of messages  
  initFifoDevice(device);
  
  //printf("\n\n");
  while(TRUE){
    waitTurnBoard(device->board, device->num);

    printf("[ %d ]", device->pid);
    
		// Submit, Receive, Move
    printMsgDevice(device);

    submitDevice(device);
    receiveDevice(device);
    moveDevice(device);
    
    printf(".\n");
    device->turn_num++;
    
    endTurnBoard(device->board, device->num);
		
  }

  exit(0);
}

//Print device message list
void printMsgDevice(Device *device){
  MessageList* current = device->msg_list.next;

  while(current != NULL){
    printf(" msg_id = [%d] ",current->msg.message_id);
    current = current->next;
  }
}

//Move device on the board
void moveDevice(Device *device){
  printf(" [Moving to ");
  int x = device->pos->coords[0];
  int y = device->pos->coords[1];
  
  moveFromToBoard(device->board, device->pid, device->x, device->y, x, y);
  printf(" (%d, %d)] ",x,y);
  device->x = x;
  device->y = y;
  
  device->pos = device->pos->next;
}

//Look for messages to receive
void receiveDevice(Device *device){
  //printf(" [Receiving] ");    
  size_t byte_read;
  do{    
    MessageList* msg_node = malloc(sizeof(MessageList));

	  if(msg_node == NULL){
		errExit("Msg_node device malloc failed!");
    }

    byte_read = read(device->fifo->fifo_fd, &(msg_node->msg), sizeof(Message));
    if (byte_read != sizeof(Message)){
      //free(msg_node);
      break;
    }

    //printf("%d -> msg(%d) ", msg_node->msg.pid_sender, msg_node->msg.message_id);
    msg_node->next= device->msg_list.next;
    device->msg_list.next=msg_node;

    //Aggiorna lista acks
    AckTableRow row_to_add; 
    
    row_to_add.pid_sender = msg_node->msg.pid_sender;
    row_to_add.pid_receiver = msg_node->msg.pid_receiver;
    row_to_add.message_id = msg_node->msg.message_id;
    row_to_add.timestamp = time(NULL);
    
    addAckTableRow(device->ack_table, &row_to_add);

  }while(byte_read == sizeof(Message));

}

//Try to send messages
void submitDevice(Device *device){
    //printf(" [S] ");
    pid_t devices_in_range[DEVICE_NUM];
    pid_t devices_received[DEVICE_NUM];

    Message msg;
    MessageList* to_remove = NULL;
    MessageList* prev = NULL;
    MessageList* current = &device->msg_list;
		
    //Loop for every message
    while(current->next != NULL){
      prev = current;
      current = current->next;
      msg = current->msg;
	
      //Get list of device's pid from Ack Table
      int ack_cnt = searchForMsgInAckTable(device->ack_table, msg.message_id, devices_received); 
      //printf(" msg(%d):%d ", msg.message_id, ack_cnt);

      //Remove message from list if AckTable is full (or empty)
      if ( ack_cnt <= 0 || ack_cnt >= DEVICE_NUM ){
        //REMOVE MSG
        //printf(" delete(%d) ", msg.message_id);
        to_remove = current;
        prev->next = current->next;
        current = prev;
        free(to_remove);
      }
      else //Message needs to be submitted
      {
        //Find devices pid in range for given message and distance
        findDeviceBoard(device->board, device->x, device->y, msg.max_distance, devices_in_range);

        /*  DEBUG
        printf("\nDevice in range: ");
        for(int i = 0; i < DEVICE_NUM; i++){
          printf(" %d -", devices_in_range[i]);
        }
        */

        //Removes from devices_in_range, the ones that already received the message ( devices_received )
        //int submit_cnt = removeDevicesReceived(devices_in_range, devices_received);
		removeDevicesReceived(devices_in_range, devices_received);		

        /*  DEBUG
        printf("\nDevice who has already received the msg: ");
        for(int i = 0; i < DEVICE_NUM; i++){
          printf(" %d -", devices_received[i]);
        }
        printf("\nDevices that need to receive the msg: ");
        for(int i = 0; i < DEVICE_NUM; i++){
          printf(" %d -", devices_in_range[i]);
        }
        printf("\n");
        */

        /*
        if(submit_cnt > 0){
          printf("-> ");
        }*/

        //Send Message to device_in_range:
        //Open other device's FIFO
        for(int j = 0; j < DEVICE_NUM; j++){
          if(devices_in_range[j] != 0){
            pid_t pid_receiver = devices_in_range[j];
            //printf(" %d ",(int)pid_receiver);

            msg.pid_sender = device->pid;
            msg.pid_receiver = pid_receiver;
            sendMessage(&msg);

            //Remove message from queue
            //printf(" delete(%d) ", msg.message_id);
            to_remove = current;
            prev->next = current->next;
            current = prev;
            free(to_remove);

            break;
          }
        }
      }
		}
}

//Remove devices who already received the message
int removeDevicesReceived(pid_t *devices_in_range, pid_t *devices_received){
  int cnt = 0;

  for(int i = 0; i < DEVICE_NUM; i++){
    for(int j = 0; j < DEVICE_NUM; j++){
      if(devices_in_range[i] == devices_received[j]){ 
        devices_in_range[i] = 0; 
      }
    }
    if (devices_in_range[i] != 0){ 
      cnt++; 
    }
  }	
  return cnt;
}

//Init signal mask
void initSignalsDevice(){
		signal(EXIT_SIGNAL, receiveSignalDevice);

		sigset_t mySet;

		//Initialize mySet to contain all signals
		sigfillset(&mySet);

		//Remove SIGINT from mySet
		sigdelset(&mySet, EXIT_SIGNAL);

		//Blocking all signals but SIGTERM
		sigprocmask(SIG_SETMASK, &mySet, 0);
		
}

//Destroy device when EXIT_SIGNAL is received
void receiveSignalDevice(int signal){
  if (signal == EXIT_SIGNAL){
    printf("[ %d ] TERMINATING Device %d \n",getpid(),forkedDevice->num);
	  destroyDevice(forkedDevice);
    exit(0);
  }
}

void destroyDevice(Device *device){
  destroyFifo(device->fifo);
  free(device);
}

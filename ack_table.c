#include "ack_table.h"

//Private functions
void runAckTableManager(AckTable *ack_table);
void submitCompletedAck(AckTable *ack_table);
void receiveAckManager(int signal);
void initSignalsAckManager();

//Create and return (pointer to) the AckTable data structure
AckTable* createAckTable(key_t msg_queue_key, int num_rows, int dev_num){
  AckTable *ack_table = malloc(sizeof(AckTable));

  if(ack_table == NULL){
		errExit("Ack_table malloc failed!");
  }

  ack_table->dev_num = dev_num;
  ack_table->size = num_rows;
  ack_table->pid = -1;
  ack_table->msg_queue_key = msg_queue_key;

  //Create Shared Memory and Semaphore structures
  ack_table->ack_mem = createMutexSharedMemory(sizeof(AckTableRow) * num_rows, 1);
  
	//Init Message Queue
  ack_table->msg_queue_id = msgget(ack_table->msg_queue_key, IPC_CREAT | S_IRUSR | S_IWUSR);

	if(ack_table->msg_queue_id == -1)
		errExit("Ack_table_Msgget failed!");

  return ack_table;
}

//Destroy AckTable structure
void destroyAckTable(AckTable* ack_table){
  //Destroy MutexSharedMemory
  destroyMutexSharedMemory(ack_table->ack_mem);

	//Destroy Message Queue
	if (msgctl(ack_table->msg_queue_id, IPC_RMID, NULL) == -1)
            errExit("msgctl failed");
	
  //Release memory
  free(ack_table);
}

//Init AckTable structure
void initAckTable(AckTable *ack_table, int sem_value ){
  //Init MutexSharedMemory and Semaphores values
  setAllSemaphore(ack_table->ack_mem->semaphore, (int[]){sem_value});
  
  //Cleanup AckTableList ( message_id = 0 => empty )
  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;

  for(int i = 0; i < ack_table->size ; i += 1){
    ack_list->rows[i].message_id = 0;
  }

}

AckTable *forkedAckTable;

//Fork from Server Process
int forkAckTableManager(AckTable *ack_table){
  //Cleanup printf buffer before forking
  fflush(stdout); 

  pid_t pid = fork();

  //Failed
  if(pid < 0){ 
		errExit("Ack_table_Fork failed!");
	}

  //Parent (Server)
  if(pid > 0){ 
    ack_table->pid = pid;
    return TRUE; 
  }

  //Child (Ack Manager)
  forkedAckTable = ack_table;
	
  ack_table->pid = getpid();

  runAckTableManager(ack_table);
  
  exit(0);
}

//AckManager main code
void runAckTableManager(AckTable *ack_table){
  //Initialize signal mask
	initSignalsAckManager();

  while(TRUE){
    submitCompletedAck(ack_table);
    sleep(5);
  }
}

//Mutually exclusive try to access the AckTable Shared Memory
void requestAckTable(AckTable *ack_table){
  waitSemaphore(ack_table->ack_mem->semaphore, 0);
}

//Release the AckTable structure Semaphore
void releaseAckTable(AckTable *ack_table){
  signalSemaphore(ack_table->ack_mem->semaphore, 0);
}

//Print AckTable
void printAckTable(AckTable *ack_table){
  requestAckTable(ack_table);

  printf("\n--------------- ACK msg_queue_id = %d -----------\n",ack_table->msg_queue_id);
  //Print ack_list, with relative slot
  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;
  AckTableRow *ack_row;

  for(int slot = 0; slot < ack_table->size ; slot += ack_table->dev_num){
    ack_row = &ack_list->rows[slot];
    
    if (ack_row->message_id != 0 ) {
      printf("%2d | %d => ",slot, ack_row->message_id );
      for (int i=0;i<DEVICE_NUM;i++){
        ack_row = &ack_list->rows[slot + i];
        if (ack_row->message_id != 0 ) {
          printf("%d ",ack_row->pid_receiver);
        }
      }
      printf("\n");
    }    
  }
  
  releaseAckTable(ack_table);
}

//Look for Acks in the AckTable
int searchForMsgInAckTable(AckTable *ack_table, int message_id, pid_t *devices_pid){
	int cnt = 0;

	for(int j = 0; j < DEVICE_NUM; j++){
		devices_pid[j] = 0;
	}

  requestAckTable(ack_table);

  AckTableList* ack_list = (AckTableList*)ack_table->ack_mem->mem->ptr;
  AckTableRow *ack_row;
  int slot;

  for(slot = 0; slot < ack_table->size ; slot += ack_table->dev_num){
    ack_row = &ack_list->rows[slot];
    if(ack_row->message_id == message_id){
      break;
    }
  }

  for(int i = 0; i < DEVICE_NUM; i++){
    ack_row = &ack_list->rows[i+slot];
    if (ack_row->message_id == message_id){
      devices_pid[i] = ack_row->pid_receiver;
      cnt++;
    }
  }
  
  releaseAckTable(ack_table);
  return cnt;
}

//Add a row to the AckTable
void addAckTableRow(AckTable *ack_table, AckTableRow *new_row){

  requestAckTable(ack_table);

  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;
  int free_slot = -1;
  int found_slot = -1;
  int slot_size = ack_table->dev_num;
	
  //Search end of each slot (table_size / slot_size)
  for(int slot = 0; slot < ack_table->size ; slot += slot_size ){

    //First row
    AckTableRow *row =  &ack_list->rows[slot];

    //Find free space
    if (free_slot == -1 && row->message_id == 0){ //I have found a free slot
      free_slot = slot;
      continue;
    }

    if (row->message_id == new_row->message_id){
      //found_slot = slot;
      for(found_slot = slot ; found_slot < slot+slot_size; found_slot++ ){
        if(ack_list->rows[found_slot].message_id == 0){
          //printf("oldAck(%d, %d, %d )",row->message_id, slot, found_slot-slot );
          ack_list->rows[found_slot].pid_sender = new_row->pid_sender;
          ack_list->rows[found_slot].pid_receiver = new_row->pid_receiver;
          ack_list->rows[found_slot].message_id = new_row->message_id;
          ack_list->rows[found_slot].timestamp = new_row->timestamp;
          break; //Message added 
        }
      }
      break; 
    }
  }

  if (found_slot == -1 && free_slot != -1){
    //printf("newAck(%d)", free_slot );
    ack_list->rows[free_slot].pid_sender = new_row->pid_sender;
    ack_list->rows[free_slot].pid_receiver = new_row->pid_receiver;
    ack_list->rows[free_slot].message_id = new_row->message_id;
    ack_list->rows[free_slot].timestamp = new_row->timestamp;
  }
  
  releaseAckTable(ack_table);
}

//Find messages with a complete AckList and sumbit them to the Message Queue
void submitCompletedAck(AckTable *ack_table){
  //Running the AckManager
  requestAckTable(ack_table);

  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;

  MessageAckList msgAckList;

  int slot_size = ack_table->dev_num;

  //Search end of each slot (table_size / slot_size)
  for(int slot = 0; slot < ack_table->size ; slot += slot_size ){
    
		//Checking last entry of the slot
    int slot_last = slot + (slot_size - 1);
    
    AckTableRow *last_ack_row = &ack_list->rows[slot_last];
    
    //Found a full slot (last is full)
    if(last_ack_row->message_id > 0){
      printf("ACK Manager cleanup & submit slot %d for msg id %d \n",slot, last_ack_row->message_id );  

      //Compose message
      msgAckList.type = last_ack_row->message_id;  
      for (int j = 0; j < slot_size ; j++ ){
        int idx = slot + j;
        AckTableRow* row = &ack_list->rows[idx];
        msgAckList.rows[j].pid_sender = row->pid_sender;
        msgAckList.rows[j].pid_receiver = row->pid_receiver;
        msgAckList.rows[j].timestamp = row->timestamp;

        //Cleanup row
        row->message_id = 0;
      }
      
			//Print the msgAcklist (about to be sent)
      for (int j = 0; j < slot_size ; j++ ){
        printf("[%d]Snd = %d ",j, msgAckList.rows[j].pid_sender);
        printf("Rcv = %d ",msgAckList.rows[j].pid_receiver);
        printf("Time = %d",(int)msgAckList.rows[j].timestamp);
				printf("\n");
      }

      //Submit message to the client
      size_t msg_size = sizeof(struct MessageAckList) - sizeof(long); 

      if( msgsnd(ack_table->msg_queue_id, &msgAckList, msg_size,0) == -1) {
        errExit("msgsnd failed");
      };
    }
  }

  releaseAckTable(ack_table);
}

//Init Signal Mask
void initSignalsAckManager(){
    //Install a new signal handler for EXIT_SIGNAL
		signal(EXIT_SIGNAL, receiveAckManager);

		sigset_t mySet;

		//Initialize mySet to contain all signals
		sigfillset(&mySet);

		//Remove SIGINT from mySet
		sigdelset(&mySet, EXIT_SIGNAL);

		//Blocking all signals but SIGTERM
		sigprocmask(SIG_SETMASK, &mySet, 0);		
}

//Destroy AckManager process when SIGTERM is received
void receiveAckManager(int signal){
  if (signal == EXIT_SIGNAL){
    printf("[ %d ] TERMINATING Ack Manager\n",getpid());
	  destroyAckTable(forkedAckTable);
    exit(0);
  }
}


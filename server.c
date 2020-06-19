/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

/*
DEFINE_NAME
StructName
functionName
var_name
*/

#include "defines.h"
#include "utils.h"
#include "ack_table.h"
#include "board.h"
#include "position_file.h"
#include "device.h"

void initSignals();
void receiveSignal(int signal);
void destroyServer();

pid_t ack_manager_pid;
PositionFile *pos_file;
AckTable *ack_table;
Board *board;
Device **device_list;

int main(int argc, char *argv[])
{

  //Check number of arguments
  printf("Reading args %*s", 10, "\t");
  if (argc != 3)
  {
    printf("Usage: %s msg_queue_key file_posizioni\n", argv[0]);
    exit(1);
  }

  //Read msg_queue's key
  key_t message_queue_key = atoi(argv[1]);
  if (message_queue_key <= 0)
  {
    printf("The message_queue_key must be greater than zero!\n");
    exit(1);
  }
  printf("[ OK ]\n");

  clearScreen();

  //Read the file_posizioni
  printf("Load positions %*s", 10, "\t");
  char *filename_pos = argv[2];
  pos_file = loadPositionFile(filename_pos);
  printf("[ OK ]\n");

  // ***** AckTable *****
  printf("AckTable setup %*s", 10, "\t");
  AckTable *ack_table = createAckTable(message_queue_key, ACK_TABLE_SIZE, DEVICE_NUM);
  initAckTable(ack_table, 1);
  forkAckTableManager(ack_table);
  ack_manager_pid = ack_table->pid;
  printf("[ OK ]\n");

  // ***** Board *****
  printf("Board setup %*s", 15, "\t");
  board = createBoard(DEVICE_NUM + 1);
  initBoard(board);
  printf("[ OK ]\n");

  // ***** Device *****
  printf("\nDevices setup\n");
  device_list = malloc(sizeof(Device *) * DEVICE_NUM);

  if (device_list == NULL)
  {
    errExit("Device_list malloc failed!");
  }

  for (int i = 0; i < DEVICE_NUM; i++)
  {
    PositionFileRow *device_row = pos_file->head[i].next;
    device_list[i] = createDevice(i, board, ack_table, device_row, pos_file->count);
    forkDevice(device_list[i]);
    printf("Device %d: PID = %d %*s[ OK ]\n", i, device_list[i]->pid, 5, "\t");
  }

  printf("Devices setup %*s[ OK ]\n", 11, "\t");

  // ***** Server *****
  printf("\nRunning server\n");
  initSignals();
  long tick = 0;

  do
  {
    if (SERVER_DEBUG == 1)
    {
      printf("\n[DEBUG MODE] press a key to continue ...\n");
      waitForKey();
    }

    else
    {
      sleep(2);
    }

    clearScreen();

    printf("Cycle: %ld\n", tick++);
    printf("Server pid: %d\n", getpid());
    printf("Ack_Manager pid: %d\n", ack_table->pid);
    printBoard(board);
    printAckTable(ack_table);

    startTurnBoard(board);            //Start device execution ( 0 -> 1 -> 2 -> 3 -> 4 )
    waitTurnBoard(board, DEVICE_NUM); //Wait for last semaphore ( 5 )

  } while (TRUE);

  return 0;
}

//Init Signal mask
void initSignals()
{
  //Set receiveSignal as the handler for EXIT_SIGNAL
  signal(EXIT_SIGNAL, receiveSignal);

  sigset_t mySet;

  //Initialize mySet to contain all signals
  sigfillset(&mySet);

  //Remove SIGINT from mySet
  sigdelset(&mySet, EXIT_SIGNAL);

  //Blocking all signals but SIGTERM
  sigprocmask(SIG_SETMASK, &mySet, 0);
}

void receiveSignal(int signal)
{

  if (signal == EXIT_SIGNAL)
  {
    printf("\n[ %d ] SERVER: Shutdown signal\n", getpid());
    printf("[ %d ] SERVER: propagate signal to children\n\n", getpid());

    kill(ack_manager_pid, EXIT_SIGNAL);

    for(int i = 0; i < DEVICE_NUM; i++){
      kill(device_list[i]->pid, EXIT_SIGNAL);
    }

    //Wait for children to die
    while (wait(NULL) > 0){};
    printf("\n[ %d ] SERVER: All children halted! \n", getpid());

    destroyServer();
    printf("[ %d ] SERVER: halted.\nGoodbye!\n", getpid());

    exit(0);
  }
}

void destroyServer()
{
  destroyBoard(board);
}

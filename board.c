#include <math.h>

#include "board.h"

//Create Board Data Structure
Board* createBoard(int dev_num){
  Board *board = malloc( sizeof(Board) );

  if(board == NULL){
		errExit("Board malloc failed!");
  }

  board->dev_num = dev_num;
  
  // Create Board related Shared Memory and Semaphores
  board->board_mem = createMutexSharedMemory( sizeof(BoardGrid), dev_num);

  return board;
}

void initBoard( Board* board ){
  // Init Semaphores
  int *values = malloc( sizeof(int)*board->dev_num);

	if(values == NULL){
		errExit("Board values malloc failed!");
  }

  for(int i = 0; i< board->dev_num; i++){
    values[i]=0;
  }

  setAllSemaphore(board->board_mem->semaphore, values);
  free(values);
}

//Destroy Board Data Structure
void destroyBoard(Board* board){
  destroyMutexSharedMemory(board->board_mem);
  free(board);
}

//Find devices in range, on the board, and put them into an array
void findDeviceBoard(Board *board, int x, int y, double max_distance, pid_t *devices_in_range){
  BoardGrid* grid = (BoardGrid*) board->board_mem->mem->ptr;
	
	int i = 0;
	for(i = 0; i < DEVICE_NUM; i++){
		devices_in_range[i] = 0;
	}

	i = 0;

  for(int row=0; row < BOARD_GRID_SIZE; row++){
    for(int col=0; col < BOARD_GRID_SIZE; col++){
      int cell_pid = grid->cells[row][col];
      if (cell_pid != 0){
		    //Distanza circolare
        double dist = sqrt(pow(row-x,2)+pow(col-y,2));

        if (dist <= max_distance){
          devices_in_range[i] = cell_pid;
					i++;
        }

      }      
    }
  }
}

//Move devices on the board, from a src(x,y) to a dst(x,y), and update the board values
void moveFromToBoard(Board *board, pid_t pid, int src_x, int src_y, int dst_x, int dst_y){
  BoardGrid* grid = (BoardGrid*) board->board_mem->mem->ptr;
  //printf("[ %d ] move (%d,%d) -> (%d, %d)\n", pid, src_x,src_y, dst_x,dst_y);
  //if(grid->cells[src_x][src_y] == pid){
    grid->cells[src_x][src_y] = 0;
    grid->cells[dst_x][dst_y] = pid;
  //} 
}

//Print Board in a nice way
void printBoard( Board *board ){
  BoardGrid* grid = (BoardGrid*) board->board_mem->mem->ptr;
	
  //Table header
  printf("     ");
  for(int col=0; col < BOARD_GRID_SIZE; col++){
    printf("|%5d",col);
  }
  printf("|\n");

  //Table body
  pid_t cell_pid;
  for(int row=0; row < BOARD_GRID_SIZE; row++){
    printf("%5d",row);
    for(int col=0; col < BOARD_GRID_SIZE; col++){
      cell_pid = grid->cells[row][col];
      if ( cell_pid !=0 ){
        printf("|%5d", cell_pid);  
      }else{
        printf("|     ");
      }
    }
    printf("|\n");
  }

}

// Queue Execution
void startTurnBoard(Board *board){
  signalSemaphore(board->board_mem->semaphore, 0);
}

void waitTurnBoard(Board *board, int turn){
  waitSemaphore(board->board_mem->semaphore, turn);
}

void endTurnBoard(Board *board, int turn){
  if ( turn >= board->dev_num ) { return; }
  signalSemaphore(board->board_mem->semaphore, turn+1);
}

/* ==========================================================================
 processes.c : implementation file
 ============================================================================ */

#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "processes.h"
#include "constants.h"
#include "output.h"
#include "synchronization.h"


/* --------------------------------------------------------------------------
 Keyboard hit detection.
 
 RETURNS:
 	-> 1 if a key has been pressed, 0 otherwise.
 ---------------------------------------------------------------------------- */
int kbhit(){
	struct termios oldt, newt;
	int ch;
	int oldf;
	
	//Changing the flags to make getchar() a non blocking operation
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	
	//Try to read of character (non-blocking)
	ch = getchar();
	
	//Resetting the flags to their old values
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	
	//If we did manage to read something
	if(ch != EOF)
	{
		//Put back the character on the input stream
		ungetc(ch, stdin);
		return 1;
	}
	
	return 0;
}


/* --------------------------------------------------------------------------
 This function determines if an intersection exists between two squares.
 
 ARGUMENTS:
 	-> a : the position structure of the first square
 	-> b : the position structure of the second square
 
 RETURNS:
 	-> 1 if an intersection exists between a and b, 0 otherwise.
 ---------------------------------------------------------------------------- */
int hasIntersection(position a, position b){
	int rc = 0;
	
	if(a.y < b.y + SQUARE_WIDTH && a.y + SQUARE_WIDTH > b.y &&
	   a.x < b.x + SQUARE_WIDTH && a.x + SQUARE_WIDTH > b.x)
		rc = 1;
	
	return rc;
}


/* --------------------------------------------------------------------------
 This function reads in shared memory the positions of the squares of
 identifier a_id and b_id and determines if square a is higher (and then
 lefter) in the grid than square b.
 
 ARGUMENTS:
 	-> segptr : a pointer to the shared memory
 	-> a_id : identifier of square a
 	-> b_id : identifier of square b
 
 RETURNS:
 	-> 1 if a is higher (and then lefter) in the grid than b, 0 otherwise.
 ---------------------------------------------------------------------------- */
static int is_higher(int* segptr, int a_id, int b_id){
	
	position a, b;
	a.x = rd_shm(segptr, 3 + 2 * a_id);
	a.y = rd_shm(segptr, 4 + 2 * a_id);
	b.x = rd_shm(segptr, 3 + 2 * b_id);
	b.y = rd_shm(segptr, 4 + 2 * b_id);
	
	if(a.x < b.x)
		return 1;
	else if(a.x > b.x)
		return 0;
	else{
		if(a.y < b.y)
			return 1;
		return 0;
	}
}


/* --------------------------------------------------------------------------
 This function reads in shared memory the positions of the squares of
 identifier a_id and b_id and determines if square a is lower (and then
 righter) in the grid than square b.
 
 ARGUMENTS:
 	-> segptr : a pointer to the shared memory
 	-> a_id : identifier of square a
 	-> b_id : identifier of square b
 
 RETURNS:
 	-> 1 if a is lower (and then righter) in the grid than b, 0 otherwise.
 ---------------------------------------------------------------------------- */
static int is_lower(int* segptr, int a_id, int b_id){
	
	position a, b;
	a.x = rd_shm(segptr, 3 + 2 * a_id);
	a.y = rd_shm(segptr, 4 + 2 * a_id);
	b.x = rd_shm(segptr, 3 + 2 * b_id);
	b.y = rd_shm(segptr, 4 + 2 * b_id);
	
	if(a.x > b.x)
		return 1;
	else if(a.x < b.x)
		return 0;
	else{
		if(a.y > b.y)
			return 1;
		return 0;
	}
}


/* --------------------------------------------------------------------------
 This function determines if the square of position a is out of bound.
 
 ARGUMENTS:
 	-> a : the position structure of the square to check
 
 RETURNS:
 	-> 1 if the square is out of bound, 0 otherwise.
 ---------------------------------------------------------------------------- */
static int is_out_of_bound(position a){
	
	if(a.x < 0 || a.x > SIZE_X - SQUARE_WIDTH ||
	   a.y < 0 || a.y > SIZE_Y - SQUARE_WIDTH)
		return 1;
	
	return 0;
}


/* --------------------------------------------------------------------------
 Implementation of a "rendez-vous". It actually implements two "rendez-vous"
 used consecutively by the process "worker".
 
 ARGUMENTS:
 	-> segptr : a pointer to the shared memory
 	-> semid : the identifier of the semaphore set
 	-> nb_workers : the number of active worker processes
 	-> nb_rdv : the numbe of the "rendez-vous" (1 for the first, 2 for the
 				second
 ---------------------------------------------------------------------------- */
void rendez_vous(int* segptr, int semid, int nb_workers, int nb_rdv){
	
	int nb_finish;
	
	// Reads and modifies a shared variable (with mutual exclusion)
	lock_sem(semid, 1 + nb_rdv);
	nb_finish = rd_shm(segptr, nb_rdv);
	nb_finish += 1;
	wr_shm(segptr, nb_rdv, nb_finish);
	unlock_sem(semid, 1 + nb_rdv);
	
	// Unlocks the waiting processes
	if(nb_finish == nb_workers){
		for(int i = 0; i < nb_workers - 1; i++)
			unlock_sem(semid, nb_rdv - 1);
		
		// Reset nb_finish to 0
		wr_shm(segptr, nb_rdv, 0);
	}
	// Waits the last worker process finishing its computations
	else{
		lock_sem(semid, nb_rdv - 1);
	}
}


/* --------------------------------------------------------------------------
 WORKER PROCESS : It represents a square in the grid. This function is
 responsible for computing the next position of the square and managing the
 collisions with other squares.
 
 ARGUMENTS:
 	-> segptr : a pointer to the shared memory
 	-> semid : the identifier of the semaphore set
 	-> square_data : a data structure containing the color and the speed of
					 the square
 	-> id : the identifier of the square
 	-> nb_workers : the number of active worker processes
 ---------------------------------------------------------------------------- */
void worker(int* segptr, int semid, int qid, square square_data, int id,
			 int nb_workers){
	
	position cur_pos;
	
	while(rd_shm(segptr, 0) == 0){
		
		lock_sem(semid, 7 + id);
	
		cur_pos.x = rd_shm(segptr, 3 + 2 * id);
		cur_pos.y = rd_shm(segptr, 4 + 2 * id);
		cur_pos.x += square_data.speedx;
		cur_pos.y += square_data.speedy;
		
		// Correcting out of bounds
		if(cur_pos.x > SIZE_X - SQUARE_WIDTH){
			cur_pos.x = SIZE_X - SQUARE_WIDTH;
			square_data.speedx = -1;
		}
		if(cur_pos.x < 0){
			cur_pos.x = 0;
			square_data.speedx = 1;
		}
		if(cur_pos.y > SIZE_Y - SQUARE_WIDTH){
			cur_pos.y = SIZE_Y - SQUARE_WIDTH;
			square_data.speedy = -1;
		}
		if(cur_pos.y < 0){
			cur_pos.y = 0;
			square_data.speedy = 1;
		}
		
		wr_shm(segptr, 3 + 2 * id, cur_pos.x);
		wr_shm(segptr, 4 + 2 * id, cur_pos.y);
		
		// First "rendez-vous"
		rendez_vous(segptr, semid, nb_workers, 1);
		
		// Checks intersection
		int nb_intersect = 0;
		int up_swap_sq_id[nb_workers - 1];
		int low_swap_sq_id[nb_workers - 1];
		int nb_up = 0;
		int nb_low = 0;
		position swap_sq;
		
		// Determines intersection squares
		for(int i = 0; i < nb_workers; i++){
			if(i != id){
				swap_sq.x = rd_shm(segptr, 3 + 2 * i);
				swap_sq.y = rd_shm(segptr, 4 + 2 * i);
				
				if(hasIntersection(swap_sq, cur_pos)){
					nb_intersect++;
					
					// The intersected square is higher than the current one
					if(swap_sq.x <= cur_pos.x){
						
						up_swap_sq_id[nb_up] = i;
						nb_up++;
						
						// Sorts intersected squares from higher to the closer
						if(nb_up > 1){
							for(int j = nb_up - 1; j > 0; j--){
								if(is_higher(segptr, up_swap_sq_id[j],
											 			up_swap_sq_id[j-1])){
									int tmp = up_swap_sq_id[j];
									up_swap_sq_id[j] = up_swap_sq_id[j-1];
									up_swap_sq_id[j-1] = tmp;
								}
								else{
									break;
								}
							}
						}
					}
					// The intersected square is lower than the current one
					if(swap_sq.x > cur_pos.x){
						
						low_swap_sq_id[nb_low] = i;
						nb_low++;
						
						// Sorts intersected squares from lower to the closer
						if(nb_low > 1){
							for(int j = nb_low - 1; j > 0; j--){
								if(is_lower(segptr, low_swap_sq_id[j],
														low_swap_sq_id[j-1])){
									int tmp = low_swap_sq_id[j];
									low_swap_sq_id[j] = low_swap_sq_id[j-1];
									low_swap_sq_id[j-1] = tmp;
								}
								else{
									break;
								}
							}
						}
					}
				}
			}
		}
		
		// Second "rendez-vous"
		rendez_vous(segptr, semid, nb_workers, 2);
		
		// Updates position if an intersection occurs
		if(nb_intersect != 0){
			
			cur_pos.x -= square_data.speedx;
			cur_pos.y -= square_data.speedy;
			
			if(is_out_of_bound(cur_pos) == 0){
				wr_shm(segptr, 3 + 2 * id, cur_pos.x);
				wr_shm(segptr, 4 + 2 * id, cur_pos.y);
			}
		}
		
		// Exchange of velocities between intersected squares
		while(nb_intersect > 0){
			struct syncmsgbuf snd_qbuf;
			struct syncmsgbuf rcv_qbuf;
			
			if(nb_low > 0){
				snd_msg(qid, &snd_qbuf, low_swap_sq_id[nb_low - 1] + 1,
																&square_data);
				nb_low--;
			}
			else{
				snd_msg(qid, &snd_qbuf, up_swap_sq_id[nb_up - 1] + 1,
																&square_data);
				nb_up--;
			}
			
			
			rcv_msg(qid, &rcv_qbuf, id + 1);
			square_data.speedx = rcv_qbuf.speed[0];
			square_data.speedy = rcv_qbuf.speed[1];
			
			nb_intersect--;
		}
		// Signals the master process for update
		unlock_sem(semid, 4);
	}
	// Signals the exit process for exit
	unlock_sem(semid, 5);
}


/* --------------------------------------------------------------------------
 EXIT PROCESS : This function is in charge of quitting properly the program.
 
 ARGUMENTS:
 	-> segptr : a pointer to the shared memory
 	-> semid : the identifier of the semaphore set
 	-> qid : the identifier of the message queue
 	-> shmid : the shared memory identifier
 	-> nb_workers : the number of active worker processes
 ---------------------------------------------------------------------------- */
void exit_proc(int* segptr, int semid, int qid, int shmid, int nb_workers){
	
	// Waits a key of the keyboard to be pressed
	while(kbhit() == 0)
		lock_sem(semid, 6);
	
	// Stops execution of other processes
	wr_shm(segptr, 0, 1);

	for(int i = 0; i < nb_workers + 1; i++)
		lock_sem(semid, 5);

	// Removes ipcs from the kernel
	rm_queue(qid);
	rm_shm(shmid);
	rm_sem(semid);
	
	printf("\n User pressed a key and stopped properly the program\n");
}


/* --------------------------------------------------------------------------
 MASTER PROCESS : This function is responsible for displaying the grid on
 the screen.
 
 ARGUMENTS:
 	-> segptr : a pointer to the shared memory
 	-> semid : the identifier of the semaphore set
 	-> nb_workers : the number of active worker processes
 	-> square_table : a table containing the information about the squares
					  (principally its color)
 ---------------------------------------------------------------------------- */
void master(int* segptr, int semid, int nb_workers, square* square_table){
	
	int table_of_pixels[SIZE_X][SIZE_Y];
	position pos_table[nb_workers];
	
	// Reads the positions of the squares in shared memory
	for(int i = 0; i < nb_workers; i++){
		pos_table[i].x = rd_shm(segptr, 3 + 2 * i);
		pos_table[i].y = rd_shm(segptr, 4 + 2 * i);
	}
	
	// Filling the table with zeroes
	for(int i = 0; i < SIZE_X; ++i){
		for(int j = 0; j < SIZE_Y; ++j){
			table_of_pixels[i][j] = 0;
		}
	}
	
	// Creates the squares in the pixels table
	for(int i = 0; i < nb_workers; i++){
		for(int j = 0; j < SQUARE_WIDTH; j++){
			for(int k = 0; k < SQUARE_WIDTH; k++){
				table_of_pixels[pos_table[i].x + j][pos_table[i].y + k]
				= square_table[i].color;
			}
		}
	}
	
	//Initializes SDL and the colours
	init_output();
	
	while(rd_shm(segptr, 0) == 0){
		
		unlock_sem(semid, 6);
		
		for(int i = 0; i < nb_workers; i++)
			lock_sem(semid, 4);
		
		// Deletes squares on the screen
		for(int i = 0; i < nb_workers; i++){
			for(int j = 0; j < SQUARE_WIDTH; j++){
				for(int k = 0; k < SQUARE_WIDTH; k++){
					table_of_pixels[pos_table[i].x + j][pos_table[i].y + k] = 0;
				}
			}
		}
		
		//Updating table of pixels with new positions
		for(int i = 0; i < nb_workers; i++){
			pos_table[i].x = rd_shm(segptr, 3 + 2 * i);
			pos_table[i].y = rd_shm(segptr, 4 + 2 * i);
			for(int j = 0; j < SQUARE_WIDTH; j++){
				for(int k = 0; k < SQUARE_WIDTH; k++){
					table_of_pixels[pos_table[i].x + j][pos_table[i].y + k]
					= square_table[i].color;
				}
			}
		}
		
		// Apply the change on SDL display
		update_output(table_of_pixels);
		
		for(int i = 0; i < nb_workers; i++)
			unlock_sem(semid, 7 + i);
	}
	unlock_sem(semid, 5);
}

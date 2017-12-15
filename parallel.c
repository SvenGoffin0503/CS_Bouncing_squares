//
//  parallel.c
//  Bouncing_squares
//
//  Created by Sven Goffin on 14/12/17.
//  Copyright © 2017 Sven Goffin. All rights reserved.
//

#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "parallel.h"
#include "constants.h"
#include "output.h"
#include "synchronization.h"



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


//Does two squares have an intersection?
int hasIntersection(position a, position b){
	int rc = 0;
	
	if(a.y < b.y + SQUARE_WIDTH && a.y + SQUARE_WIDTH > b.y &&
	   a.x < b.x + SQUARE_WIDTH && a.x + SQUARE_WIDTH > b.x)
		rc = 1;
	
	return rc;
}

// Returns 1 if a is higher (and then lefter) than b
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

// Returns 1 if a is lower (and then righter) than b
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

static int is_at_bound(position a){
	
	if(a.x == 0 || a.x == SIZE_X - SQUARE_WIDTH ||
	   a.y == 0 || a.y == SIZE_Y - SQUARE_WIDTH)
		return 1;
	
	return 0;
}

void rendez_vous(int* segptr, int semid, int nb_squares, int nb_rdv){
	
	lock_sem(semid, 1 + nb_rdv); // wait(mutex)
	int nb_finish = rd_shm(segptr, nb_rdv);
	nb_finish += 1;
	wr_shm(segptr, nb_rdv, nb_finish);
	unlock_sem(semid, 1 + nb_rdv); // signal(mutex)
	if(nb_finish == nb_squares){
		for(int i = 0; i < nb_squares - 1; i++)
			unlock_sem(semid, nb_rdv - 1);
		
		//lock_sem(semid, 1 + nb_rdv); // wait(mutex)
		wr_shm(segptr, nb_rdv, 0); // Reset nb_finish to 0
		//unlock_sem(semid, 1 + nb_rdv); // signal(mutex)
	}
	else{
		lock_sem(semid, nb_rdv - 1); // wait(Go_intersect)
	}
}


void squares(int* segptr, int semid, int qid, square square_data, int id,
			 int nb_squares){
	
	position cur_pos;
	
	while(rd_shm(segptr, 0) == 0){
		
		lock_sem(semid, 7 + id); // wait(Go_compute_pos[id])
	printf("Process %d\n", id);
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
	printf("Before RDV %d\n", id);
		rendez_vous(segptr, semid, nb_squares, 1);
	printf("After RDV %d\n", id);
		// Checks intersection
		int nb_intersect = 0;
		int up_swap_sq_id[nb_squares - 1];
		int low_swap_sq_id[nb_squares - 1];
		int nb_up = 0;
		int nb_low = 0;
		position up_swap_sq;
		position low_swap_sq;
		position swap_sq;
		
		// Initialization of swap squares' positions
		up_swap_sq.x = SIZE_X;
		up_swap_sq.y = SIZE_Y;
		low_swap_sq.x = -1;
		low_swap_sq.y = -1;
		
		// Determines intersection squares
		for(int i = 0; i < nb_squares; i++){
			if(i != id){
				swap_sq.x = rd_shm(segptr, 3 + 2 * i);
				swap_sq.y = rd_shm(segptr, 4 + 2 * i);
				
				if(hasIntersection(swap_sq, cur_pos)){
					nb_intersect++;
					if(swap_sq.x <= cur_pos.x){
						
						up_swap_sq_id[nb_up] = i;
						nb_up++;
						
						// Sorts up_swap_sq_id
						if(nb_up > 1){
							for(int j = nb_up - 1; j > 0; j--){
								if(is_higher(segptr, up_swap_sq_id[j], up_swap_sq_id[j-1])){
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
					if(swap_sq.x > cur_pos.x){
						
						low_swap_sq_id[nb_low] = i;
						nb_low++;
						
						// Sorts low_swap_sq_id
						if(nb_low > 1){
							for(int j = nb_low - 1; j > 0; j--){
								if(is_lower(segptr, low_swap_sq_id[j], low_swap_sq_id[j-1])){
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
		
	printf("Before RDV2 %d\n", id);
		rendez_vous(segptr, semid, nb_squares, 2);
		
		if(nb_intersect != 0){
			
			if(is_at_bound(cur_pos) == 0){
				cur_pos.x -= square_data.speedx;
				cur_pos.y -= square_data.speedy;
				wr_shm(segptr, 3 + 2 * id, cur_pos.x);
				wr_shm(segptr, 4 + 2 * id, cur_pos.y);
			}
		}
		
		while(nb_intersect > 0){
			struct syncmsgbuf snd_qbuf;
			struct syncmsgbuf rcv_qbuf;
			
			if(nb_low > 0){
				snd_msg(qid, &snd_qbuf, low_swap_sq_id[nb_low - 1] + 1, &square_data);
				nb_low--;
				printf("%d send to %d\n", id, low_swap_sq_id[nb_low - 1]);
			}
			else{
				snd_msg(qid, &snd_qbuf, up_swap_sq_id[nb_up - 1] + 1, &square_data);
				nb_up--;
				printf("%d send to %d\n", id, up_swap_sq_id[nb_up - 1]);
			}
			
			
			rcv_msg(qid, &rcv_qbuf, id + 1);
			square_data.speedx = rcv_qbuf.speed[0];
			square_data.speedy = rcv_qbuf.speed[1];
			
			nb_intersect--;
		}
		
		unlock_sem(semid, 4); // signal(can_disp)
	}
	unlock_sem(semid, 5); // signal(Go_exit)
}


void exit_proc(int* segptr, int semid, int qid, int shmid, int nb_squares){
	
	while(kbhit() == 0){
		printf("Controller is waiting!!!\n");
		lock_sem(semid, 6); // wait(KBHIT)
	}
	
	wr_shm(segptr, 0, 1);
	printf("Controller is in the starting blocks...\n");
	for(int i = 0; i < nb_squares + 1; i++)
		lock_sem(semid, 5); // wait(Go_exit)
	printf("Controller is in the place!!!\n");
	rm_queue(qid);
	rm_shm(shmid);
	rm_sem(semid);
}

void master(int* segptr, int semid, int nb_squares, square* square_table){
	
	int table_of_pixels[SIZE_X][SIZE_Y];    //Will store the states of the pixels
	position pos_table[nb_squares];
	
	// Reads the positions of the squares in shared memory
	for(int i = 0; i < nb_squares; i++){
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
	for(int i = 0; i < nb_squares; i++){
		for(int j = 0; j < SQUARE_WIDTH; j++){
			for(int k = 0; k < SQUARE_WIDTH; k++){
				table_of_pixels[pos_table[i].x + j][pos_table[i].y + k]
				= square_table[i].color;
			}
		}
	}
	
	//Initializes SDL and the colours
	init_output();
	printf("initialized\n");
	
	while(rd_shm(segptr, 0) == 0){
		
		unlock_sem(semid, 6); // signal(kbhit)
		
		for(int i = 0; i < nb_squares; i++)
			lock_sem(semid, 4); // wait(Can_display)
		
		printf("Dips\n");
		// Deletes squares on the screen
		for(int i = 0; i < nb_squares; i++){
			for(int j = 0; j < SQUARE_WIDTH; j++){
				for(int k = 0; k < SQUARE_WIDTH; k++){
					table_of_pixels[pos_table[i].x + j][pos_table[i].y + k] = 0;
				}
			}
		}
		
		//Updating table of pixels with new positions
		for(int i = 0; i < nb_squares; i++){
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
		
		for(int i = 0; i < nb_squares; i++)
			unlock_sem(semid, 7 + i); // Signal(Go_intersection)
	}
	unlock_sem(semid, 5);
}


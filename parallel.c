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


void squares(int* segptr, int semid, int id){
	
	int cnt = 0;
	int x, y;
	while(rd_shm(segptr, 0) == 0){
		lock_sem(semid, 5 + id);
		printf("square %d\n", id);
		x = rd_shm(segptr, 2 + 2 * id);
		y = rd_shm(segptr, 3 + 2 * id);
		
		if((cnt % 2) == 1){
			wr_shm(segptr, 2 + 2 * id, x + 1);
			wr_shm(segptr, 3 + 2 * id, y + 1);
		}
		else{
			wr_shm(segptr, 2 + 2 * id, x - 1);
			wr_shm(segptr, 3 + 2 * id, y - 1);
		}
		cnt ++;
		unlock_sem(semid, 2);
	}
	unlock_sem(semid, 3);
}


void exit_proc(int* segptr, int semid, int qid, int shmid, int nb_squares){
	
	while(kbhit() == 0){
		printf("Controller is waiting!!!\n");
		lock_sem(semid, 4);
	}
	
	wr_shm(segptr, 0, 1);
printf("Controller is in the starting blocks...\n");
	for(int i = 0; i < nb_squares; i++)
		lock_sem(semid, 3);
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
		pos_table[i].x = rd_shm(segptr, 2 + 2 * i);
		pos_table[i].y = rd_shm(segptr, 3 + 2 * i);
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
		
		unlock_sem(semid, 4); // signal(kbhit)
		
		for(int i = 0; i < nb_squares; i++)
			lock_sem(semid, 2); // wait(Can_display)
		
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
			pos_table[i].x = rd_shm(segptr, 2 + 2 * i);
			pos_table[i].y = rd_shm(segptr, 3 + 2 * i);
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
			unlock_sem(semid, 5 + i); // Signal(Go_intersection)
	}
}

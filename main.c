//
//  main.c
//  Bouncing_squares
//
//  Created by Sven Goffin on 9/12/17.
//  Copyright © 2017 Sven Goffin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>

#include "synchronization.h"
#include "output.h"
#include "constants.h"

#define SIZE 5
//Global variables
int table_of_pixels[SIZE_X][SIZE_Y];    //Will store the states of the pixels
int temporary_table[SIZE_X][SIZE_Y];    //Will be used to store state n+1

typedef struct square{
	int color;
	int speedx;
	int speedy;
} square;

typedef struct position{
	int x;
	int y;
} position;



//Does two squares have an intersection?
static int hasIntersection(position a, position b)
{
	int rc = 0;
	
	if(a.y < b.y + SQUARE_WIDTH && a.y + SQUARE_WIDTH > b.y &&
	   a.x < b.x + SQUARE_WIDTH && a.x + SQUARE_WIDTH > b.x)
		rc = 1;
	
	return rc;
}




static void initSquares(int nb_squares, square* table_square, int* segptr){
	
	position table_pos[nb_squares];
	
	for(int i = 0; i < nb_squares; i++){
		table_pos[i].x = (rand() % (SIZE_X - SQUARE_WIDTH + 1));
		table_pos[i].y = (rand() % (SIZE_Y - SQUARE_WIDTH + 1));
		for(int j = 0; j < i; j++){
			if(hasIntersection(table_pos[i], table_pos[j])){
				i--;
				break;
			}
		}
	}
	
	for(int i = 0; i < nb_squares; i++){
		table_square[i].color = (rand() % 5);
		table_square[i].speedx = (rand() % 3) - 1;
		table_square[i].speedy = (rand() % 3) - 1;
		wr_shm(segptr, 2 + 2 * i, table_pos[i].x);
		wr_shm(segptr, 3 + 2 * i, table_pos[i].y);
	}
}




int main(int argc, char* argv[]){
	
	int nb_squares;
	int nb_custom_squares;
	
	// Initializes the number of squares
	switch(argc){
		case 1:
			// Default initialization
			nb_squares = (rand() % 10) + 1;
			break;
		case 2:
			// Custom initialization
			nb_squares = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Invalid number of parameters : %d. Only the number"
					"of squares is expected", argc - 1);
			exit(1);
	}
	
	if(nb_squares > SQUARE_MAX){
		
		fprintf(stderr, "Too many squares! This must be smaller (or equal) "
				"than 10");
		exit(1);
	}
	
	/*--------------------------------------------------------------------------
	 // Initializes the number of custom squares
	 while(1 == 1){
	 printf("How many squares do you want to custom? "
	 "(only %d available)\n", nb_squares);
	 scanf("%d", &nb_custom_squares);
	 if(nb_custom_squares >= 0 && nb_custom_squares <= nb_squares)
	 break;
	 printf("Invalid number. Please enter a number between "
	 "0 and %d.\n", nb_squares);
	 }
	 
	 if(nb_custom_squares > 0){
	 printf("CUSTOM ZONE:\n");
	 for(int i = 0; i < nb_custom_squares; i++){
	 printf("\t Custom square %d:\n", i);
	 printf("\t\tEnter x and y positions of square %d:\n", i);
	 scanf("%d %d", &pos_table[i].x, &pos_table[i].y);
	 for(int j = 0; j < i; j++){
	 if(pos_table[i].x == pos_table[j].x){
	 printf("A FAIRE");
	 }
	 }
	 }
	 }
	 -------------------------------------------------------------------------*/
	
	
	square square_table[nb_squares];
	union semun semopts;
	pid_t pid;
	key_t key_shm = ftok(".", 'M');
	key_t key_sem = ftok(".", 'S');
	key_t key_msg = ftok(".", 'Q');
	int semnum = 3 + nb_squares;
	int shmid = open_shm(key_shm, ((2 * nb_squares) + 2) * sizeof(int));
	int semid = open_sem(key_sem, semnum);
	int qid = open_msgq(key_msg);
	int id, cnt;
	int* segptr;
	
	// Attach the shared memory segment into the current process
	if((segptr = (int*)shmat(shmid, 0, 0)) == (int*)-1){
		perror("shmat");
		exit(1);
	}

	
	// Semaphores' initialization
	for(int i = 0; i < nb_squares; i++){
		semopts.val = 0;
		semctl(semid, i, SETVAL, semopts);
	}
	
	semopts.val = 0;
	semctl(semid, nb_squares, SETVAL, semopts);
	semopts.val = 1;
	semctl(semid, nb_squares + 1, SETVAL, semopts);
	semopts.val = 1;
	semctl(semid, nb_squares + 2, SETVAL, semopts);
	
	// Initialization of shared variables
	wr_shm(segptr, 0, 0);
	wr_shm(segptr, 1, 0);
	
	// Randomly initializes the squares
	initSquares(nb_squares, square_table, segptr);
	
	// Creation of the different processes
	id = 0;
	
	for(cnt = 0; cnt <= nb_squares; cnt++){
		pid = fork();
		if(pid < 0){
			perror("Processes creation failed.");
			exit(1);
		}
		if(pid == 0){
			if(cnt == nb_squares){
				// Controller area
				
			}
			else{
				// Square area
				cnt = nb_squares;
			}
		}
		else{
			id++;
		}
	}
	
	if(pid != 0){
		// Graphic handler area
		
	}
	
	return 0;
}

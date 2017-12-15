//
//  main.c
//  Bouncing_squares
//
//  Created by Sven Goffin on 9/12/17.
//  Copyright © 2017 Sven Goffin. All rights reserved.
//

#include <stdio.h>
//#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>

#include "parallel.h"
#include "synchronization.h"
#include "output.h"
#include "constants.h"


static void initSquares(int nb_squares, int nb_custom, square* table_square,
						   position* table_pos, int* segptr){
	
	const int X_MAX = SIZE_X - SQUARE_WIDTH;
	const int Y_MAX = SIZE_Y - SQUARE_WIDTH;
	
	// Custom initialization of squares
	if(nb_custom > 0)
		printf("CUSTOM ZONE:\n");
	
	for(int i = 0; i < nb_custom; i++){
		
		printf("\t Custom square %d:\n", i + 1);
		
		// Coordinates initialization
		int valid_coord = 0;
		while(valid_coord == 0){
			printf("\n\t\t Enter x coordinate of square %d (between 0 and %d) :",
				   i + 1, X_MAX);
			scanf("%d", &(table_pos[i].x));
		
			printf("\t\t Enter y coordinate of square %d (between 0 and %d) :",
				   i + 1, Y_MAX);
			scanf("%d", &(table_pos[i].y));
		
			if(table_pos[i].x < 0 || table_pos[i].x > X_MAX ||
			   table_pos[i].y < 0 || table_pos[i].x > Y_MAX){
				printf("\n\t\t Invalid coordinates. They must be between 0 and"
					" %d for x and %d for y. Try again.\n",  X_MAX, Y_MAX);
				continue;
			}
		
			int intersection = 0;
			for(int j = 0; j < i; j++){
				if(hasIntersection(table_pos[i], table_pos[j])){
					printf("\n\t\t Intersection occurs, you have to enter new "
						   "positions for square %d.\n", i + 1);
					intersection = 1;
					break;
				}
			}
		
			if(intersection == 1)
				continue;
			
			valid_coord = 1;
		}
		
		// Color initialization
		int valid_color = 0;
		while(valid_color == 0){
			printf("\n\t\t (1 = white, 2 = red, 3 = green, 4 = blue)\n");
			printf("\t\t Enter the color of square %d : ", i + 1);
			scanf("%d", &table_square[i].color);
			
			if(table_square[i].color < 1 || table_square[i].color > 4){
				printf("\n\t\t Invalid color. Only the values 1, 2, 3 and "
						"4 are allowed. Please, try again.\n");
			}
			else{
				valid_color = 1;
			}
		}
		
		// Speed initialization
		int valid_speed = 0;
		while(valid_speed == 0){
			printf("\n\t\t Enter the x velocity of the square %d "
				   "(Must be -1, 0 or 1) : ", i + 1);
			scanf("%d", &table_square[i].speedx);
			
			printf("\t\t Enter the y velocity of the square %d "
				   "(Must be -1, 0 or 1) : ", i + 1);
			scanf("%d", &table_square[i].speedy);
			
			if(table_square[i].speedx  > 1 || table_square[i].speedx  < -1 ||
			   table_square[i].speedy  > 1 || table_square[i].speedy  < -1 ){
				printf("\n\t\t Invalid velocity. It must be -1, 0 or 1. "
					   "Please, try again.\n");
			}
			else{
				valid_speed = 1;
			}
		}
	}
	
	// Random initialisation of squares
	int nb_rand = nb_squares - nb_custom;
	for(int i = 0; i < nb_rand; i++){
		table_pos[nb_custom + i].x = (rand() % (X_MAX + 1));
		table_pos[nb_custom + i].y = (rand() % (Y_MAX + 1));
		for(int j = 0; j < nb_custom + i; j++){
			if(hasIntersection(table_pos[nb_custom + i], table_pos[j])){
				i--;
				break;
			}
		}
	}
	
	for(int i = 0; i < nb_rand; i++){
		table_square[nb_custom + i].color = (rand() % 4) + 1;
		table_square[nb_custom + i].speedx = (rand() % 3) - 1;
		table_square[nb_custom + i].speedy = (rand() % 3) - 1;
	}
	
	// Writes positions in shared memory
	for(int i = 0; i < nb_squares; i++){
		wr_shm(segptr, 3 + 2 * i, table_pos[i].x);
		wr_shm(segptr, 4 + 2 * i, table_pos[i].y);
	}
}

int main(int argc, char* argv[]){
	
	int nb_squares;
	int nb_cust_squares;
	char custom_type;
	char type_nb_squares;
	const int squares_max = 10;
	
	// Initializes the number of squares
	printf("Do you want to choose the number of squares (write c) or do you"
		   " want a random number of squares (write r) ? \n");
	scanf("%s", &type_nb_squares);
	
	switch(type_nb_squares){
		case 'r':
			nb_squares = (rand() % 10) + 1;
			break;
		case 'c':
			printf("How many squares do you want? (Max %d) \n",  squares_max);
			scanf("%d", &nb_squares);
			
			if(nb_squares > squares_max){
				
				fprintf(stderr, "Too many squares! This must be smaller (or"
						"equal) than %d. \n", squares_max);
				exit(1);
			}
			
			break;
		default:
			fprintf(stderr, "Invalid character. Only the characters c or r "
					"are expected.\n");
			exit(1);
	}
	
	if(nb_squares > squares_max){
		
		fprintf(stderr, "Too many squares! This must be smaller (or equal) "
				"than %d.\n", squares_max);
		exit(1);
	}
	
	printf("Do you want to initialize your squares manually (write m) "
		   "or randomly (write r) ? \n");
	scanf("%s", &custom_type);
	
	
	position pos_table[nb_squares];
	square square_table[nb_squares];
	
	union semunion semopts;
	pid_t pid;
	key_t key_shm = ftok(".", 'M');
	key_t key_sem = ftok(".", 'S');
	key_t key_msg = ftok(".", 'Q');

rm_queue(1769472);
rm_shm(1507329);
rm_sem(2162688);
	
	int semnum = 7 + nb_squares;
	int shmid = open_shm(key_shm, ((2 * nb_squares) + 3) * sizeof(int));
	int semid = open_sem(key_sem, semnum);
	int qid = open_msgq(key_msg);
	int id, cnt;
	int* segptr;
	
	// Attach the shared memory segment into the current process
	if((segptr = (int*)shmat(shmid, 0, 0)) == (int*)-1){
		perror("shmat");
		rm_queue(qid);
		rm_shm(shmid);
		rm_sem(semid);
		exit(1);
	}
	
	switch(custom_type){
		case 'r':
			nb_cust_squares = 0;
			break;
		case 'm':
			printf("How many squares do you want to customize ? (Max %d) \n",
				   nb_squares);
			scanf("%d", &nb_cust_squares);
			
			if(nb_cust_squares > nb_squares){
				fprintf(stderr, "Too many squares to customize! This must be"
						"smaller (or equal) than %d \n", nb_squares);
				rm_queue(qid);
				rm_shm(shmid);
				rm_sem(semid);
				exit(1);
			}
			break;
		default:
			fprintf(stderr, "Invalid character. Only the characters m or r "
					"are expected");
			rm_queue(qid);
			rm_shm(shmid);
			rm_sem(semid);
			exit(1);
	}
	
	initSquares(nb_squares, nb_cust_squares, square_table, pos_table, segptr);
	
	// Semaphores' initialization
	semopts.val = 0;
	semctl(semid, 0, SETVAL, semopts); // Go_compute_position
	semopts.val = 0;
	semctl(semid, 1, SETVAL, semopts);
	semopts.val = 1;
	semctl(semid, 2, SETVAL, semopts); // Mutex 1
	semopts.val = 1;
	semctl(semid, 3, SETVAL, semopts); // Mutex 2
	semopts.val = nb_squares;
	semctl(semid, 4, SETVAL, semopts); // Can_display
	semopts.val = 0;
	semctl(semid, 5, SETVAL, semopts); // Go_exit
	semopts.val = 0;
	semctl(semid, 6, SETVAL, semopts); // KBHIT
	
	for(int i = 0; i < nb_squares; i++){
		semopts.val = 0;
		semctl(semid, 7 + i, SETVAL, semopts); // Go_intersection
	}
	
	
	// Initialization of shared variables
	wr_shm(segptr, 0, 0); // Stop
	wr_shm(segptr, 1, 0); // nb_finish_compute
	wr_shm(segptr, 2, 0); // nb_finish_intersect
	
	// Clear input buffer
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }
	
	// Creation of the different processes
	id = 0;
	
	for(cnt = 0; cnt <= nb_squares; cnt++){
		pid = fork();
		if(pid < 0){
			perror("Processes creation failed.");
			rm_queue(qid);
			rm_shm(shmid);
			rm_sem(semid);
			exit(1);
		}
		if(pid == 0){
			if(cnt == nb_squares){
				// User control area
				exit_proc(segptr, semid, qid, shmid, nb_squares);
			}
			else{
				// Square area
				cnt = nb_squares;
				squares(segptr, semid, qid, square_table[id], id, nb_squares);
			}
		}
		else{
			id++;
		}
	}
	
	if(pid != 0){
		// Master area
		master(segptr, semid, nb_squares, square_table);
	}
	
	return 0;
}

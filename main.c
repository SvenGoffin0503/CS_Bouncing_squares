//
//  main.c
//  Bouncing_squares
//
//  Created by Sven Goffin on 9/12/17.
//  Copyright © 2017 Sven Goffin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "constants.h"

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
int hasIntersection(position a, position b)
{
	int rc = 0;
	
	if(a.y < b.y+SQUARE_WIDTH && a.y+SQUARE_WIDTH > b.y &&
	   a.x < b.x+SQUARE_WIDTH && a.x+SQUARE_WIDTH > b.x)
		rc = 1;
	return rc;
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
				"than 10", argc - 1);
		exit(1);
	}
	
	position pos_table[nb_squares];
	square square_table[nb_squares];
	
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
					printf(")
				}
			}
			
		}
	}
	return 0;
}


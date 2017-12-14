//
//  parallel.h
//  Bouncing_squares
//
//  Created by Sven Goffin on 14/12/17.
//  Copyright © 2017 Sven Goffin. All rights reserved.
//

#ifndef parallel_h
#define parallel_h

typedef struct position{
	int x;
	int y;
} position;

typedef struct square{
	int color;
	int speedx;
	int speedy;
} square;

int kbhit(void);
void squares(int* segptr, int semid, int id);
void exit_proc(int* segptr, int semid, int qid, int shmid, int nb_squares);
void master(int* segptr, int semid, int nb_squares, square* square_table);

#endif /* parallel_h */

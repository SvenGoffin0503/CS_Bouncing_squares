//
//  main.c
//  Bouncing_squares
//
//  Created by Sven Goffin on 8/12/17.
//  Copyright © 2017 Sven Goffin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include "synchronization.h"

int main(int argc, const char * argv[]) {
	
	key_t key_sem;
	key_t key_q;
	int semid;
	int qid;
	union semun semopts;
	
	key_sem = ftok(".", 'S');
	key_q = ftok(".", 'Q');
	
	if((semid = semget(key_sem, 3, IPC_CREAT|0666)) == -1){
		fprintf(stderr,"Semaphore set already exists!");
	}
	
	
	semopts.val = 5;
	semctl(semid, 0, SETVAL, semopts);
	semopts.val = 5;
	semctl(semid, 1, SETVAL, semopts);
	semopts.val = 5;
	semctl(semid, 2, SETVAL, semopts);
	
	lock_sem(semid, 0);
	printf("try 0: OK\n");
	
	unlock_sem(semid, 0);
	rm_sem(semid);
	
	if((qid = msgget(key_q, IPC_CREAT|0660)) == -1){
		perror("msgget");
		exit(1);
	}
	
	return 0;
	
	
}

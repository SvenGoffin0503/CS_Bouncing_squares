//
//  semaphore.c
//  Bouncing_squares
//
//  Created by Sven Goffin on 8/12/17.
//  Copyright © 2017 Sven Goffin. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/msg.h>

#include "synchronization.h"

/* --------------------------------------------------------------------------
 Performs a wait opertion on the semaphore located at position "member" in
 the semaphore set of identifier "semid".
 
 ARGUMENTS: -> semid : identifier of the semaphore set
 -> member : index of the target semaphore in the set
 ---------------------------------------------------------------------------- */
void lock_sem(int semid, int member){
	
	errno = 0;
	struct sembuf sem_lock = { member, -1, 0 };
	
	// Attempt to wait the semaphore
	if(semop(semid, &sem_lock, 1) == -1){
		if(errno == EFBIG){
			// sem_num member is out of range
			fprintf(stderr, "Semaphore member %d is out of range\n", member);
			return;
		}
		else{
			fprintf(stderr, "Wait operation failed\n");
			exit(1);
		}
	}
}


/* --------------------------------------------------------------------------
 Performs a signal opertion on the semaphore located at position "member" in
 the semaphore set of identifier "semid".
 
 ARGUMENTS: -> semid : identifier of the semaphore set
 -> member : index of the target semaphore in the set
 ---------------------------------------------------------------------------- */
void unlock_sem(int semid, int member){
	
	errno = 0;
	struct sembuf sem_unlock = { member, 1, 0 };
	
	// Attempt to signal the semaphore
	if(semop(semid, &sem_unlock, 1) == -1){
		if(errno == EFBIG){
			// sem_num member is out of range
			fprintf(stderr, "Semaphore member %d is out of range\n", member);
			return;
		}
		else{
			fprintf(stderr, "Signal operation failed\n");
			exit(1);
		}
	}
}


/* --------------------------------------------------------------------------
 Marks the semaphore set of identifier "semid" for deletion.
 
 ARGUMENTS: -> semid : identifier of the semaphore set to remove
 ---------------------------------------------------------------------------- */
void rm_sem(int semid){
	
	semctl(semid, 0, IPC_RMID, 0);
}

void snd_msg(int qid, struct syncmsgbuf* qbuf, long type, char* text){
	
	qbuf->mtype = type;
	strcpy(qbuf->mtext, text);
	
	if(msgsnd(qid, (struct msgbuf*) qbuf, strlen(qbuf->mtext) + 1, 0) == -1){
		perror("msgsnd ");
		exit(1);
	}
}

void rcv_msg(int qid, struct syncmsgbuf* qbuf, long type){
	
	qbuf->mtype = type;
	msgrcv(qid, (struct msgbuf*) qbuf, MAX_SEND_SIZE, type, 0);
}

void rm_queue(int qid){
	
	msgctl(qid, IPC_RMID, 0);
}

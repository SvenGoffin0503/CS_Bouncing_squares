//
//  synchronization.h
//  Bouncing_squares
//
//  Created by Sven Goffin on 8/12/17.
//  Copyright © 2017 Sven Goffin. All rights reserved.
//

#ifndef synchronization_h
#define synchronization_h

#include <sys/sem.h>
#include <sys/msg.h>

#define MAX_SEND_SIZE 10 //To be modify

union semun{
	int val;                /* value for SETVAL */
	struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
	unsigned short *array;          /* array for GETALL & SETALL */
	struct seminfo *__buf;  /* buffer for IPC_INFO */
};

struct syncmsgbuf{
	long mtype;
	char mtext[MAX_SEND_SIZE];
};

/* Semaphore section */
int open_sem(key_t keyval, int numsems);
void lock_sem(int sid, int member);
void unlock_sem(int sid, int member);
void rm_sem(int semid);

/* Message queue section */
int open_msgq(key_t keyval);
void snd_msg(int qid, struct syncmsgbuf* qbuf, long type, char* text);
void rcv_msg(int qid, struct syncmsgbuf* qbuf, long type);
void rm_queue(int qid);

/* Shared memory section */
int open_shm(key_t keyval, int segsize);
void wr_shm(int* segptr, int index, int value);
int rd_shm(int* segptr, int index);
void rm_shm(int shmid);

#endif /* synchronization_h */

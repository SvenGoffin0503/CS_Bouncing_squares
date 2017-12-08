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

#define MAX_SEND_SIZE 2 //To be modify

union semun{
	int val;                /* value for SETVAL */
	struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
	ushort *array;          /* array for GETALL & SETALL */
	struct seminfo *__buf;  /* buffer for IPC_INFO */
};

struct syncmsgbuf{
	long mtype;
	char mtext[MAX_SEND_SIZE];
};

/* Semaphore implementation */
void lock_sem(int sid, int member);
void unlock_sem(int sid, int member);
void rm_sem(int semid);

/* Message queue implementation */
void snd_msg(int qid, struct syncmsgbuf* qbuf, long type, char* text);
void rcv_msg(int qid, struct syncmsgbuf* qbuf, long type);
void rm_queue(int qid);

#endif /* synchronization_h */

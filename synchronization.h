/* ==========================================================================
 synchronization.h : header file
 ============================================================================ */

#ifndef synchronization_h
#define synchronization_h

#include <sys/sem.h>
#include <sys/msg.h>

#include "processes.h"


/* --------------------------------------------------------------------------
 Redefinition of "union semun".
 ---------------------------------------------------------------------------- */
union semunion{
	int val;                /* value for SETVAL */
	struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
	unsigned short *array;  /* array for GETALL & SETALL */
	struct seminfo *__buf;  /* buffer for IPC_INFO */
};


/* --------------------------------------------------------------------------
 Data structure representing a message queue buffer.
 ---------------------------------------------------------------------------- */
struct syncmsgbuf{
	long mtype;
	int speed[2];
};


/* --------------------------------------------------------------------------
 SEMAPHORE SECTION
 ---------------------------------------------------------------------------- */
// Creates a semaphore set
int open_sem(key_t keyval, int numsems);
// Wait operation
void lock_sem(int sid, int member);
// Signal operation
void unlock_sem(int sid, int member);
// Removes the semaphore set semid from the kernel
void rm_sem(int semid);


/* --------------------------------------------------------------------------
 MESSAGE QUEUE SECTION
 ---------------------------------------------------------------------------- */
// Creates a message queue
int open_msgq(key_t keyval);
// Sends a message to the message queue qid
void snd_msg(int qid, struct syncmsgbuf* qbuf, long type, square* swap_sq);
// Retrieves a message from the message queue qid
void rcv_msg(int qid, struct syncmsgbuf* qbuf, long type);
// Removes the message queue qid from the kernel
void rm_queue(int qid);


/* --------------------------------------------------------------------------
 SHARED MEMORY SECTION
 ---------------------------------------------------------------------------- */
// Creates a shared memory segment
int open_shm(key_t keyval, int segsize);
// Writes a value in shared memory
void wr_shm(int* segptr, int index, int value);
// Reads a value from shared memory
int rd_shm(int* segptr, int index);
// Removes the shared memory segment shmid from the kernel
void rm_shm(int shmid);


#endif /* synchronization_h */

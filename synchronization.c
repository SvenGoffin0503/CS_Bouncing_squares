/* ==========================================================================
 synchronization.c : implementation file
 ============================================================================ */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include "synchronization.h"
#include "processes.h"


/* --------------------------------------------------------------------------
 Creates a set of semaphores
 
 ARGUMENTS:
 	-> keyval : key value (generally returned by a call to ftok())
	-> numsems : the number of semaphores in the set.
 
 RETURNS:
	-> semid : identifier of the created semaphore set
 ---------------------------------------------------------------------------- */
int open_sem(key_t keyval, int numsems){
	
	int semid;
	
	if (!numsems){
		fprintf(stderr, "Invalid number of semaphores numsems %d \n", numsems);
		exit(-1);
	}
	
	if((semid = semget(keyval, numsems, IPC_CREAT|IPC_EXCL|0666)) == -1){
		fprintf(stderr, "Semaphore set already exists. \n");
		exit(-1);
	}
	
	return semid;
}


/* --------------------------------------------------------------------------
 Performs a wait opertion on the semaphore located at position "member" in
 the semaphore set of identifier "semid".
 
 ARGUMENTS:
	-> semid : identifier of the semaphore set
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
 
 ARGUMENTS:
	-> semid : identifier of the semaphore set
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
 
 ARGUMENTS:
	-> semid : identifier of the semaphore set to remove
 ---------------------------------------------------------------------------- */
void rm_sem(int semid){
	
	semctl(semid, 0, IPC_RMID, 0);
}


/* --------------------------------------------------------------------------
 Creates (or opens if already exists) a message queue.
 
 ARGUMENTS:
	-> keyval : key value (generally returned by a call to ftok())
 
 RETURNS:
 	-> qid : identifier of the created/opened message queue
 ---------------------------------------------------------------------------- */
int open_msgq(key_t keyval){
	
	int qid;
	
	if((qid = msgget( keyval, IPC_CREAT|0660)) == -1){
		perror("msgget");
		exit(1);
	}
	
	return qid;
}


/* --------------------------------------------------------------------------
 Delivers a message to the message queue of identifier "qid".
 
 ARGUMENTS:
	-> qid : identifier of the message queue
	-> qbuf : data structure containing all the information about the message
	-> type : type of the message to send
	-> swap_sq : a pointer to a data structure containing the square
 				 information to send
 ---------------------------------------------------------------------------- */
void snd_msg(int qid, struct syncmsgbuf* qbuf, long type, square* swap_sq){
	
	qbuf->mtype = type;
	qbuf->speed[0] = swap_sq->speedx;
	qbuf->speed[1] = swap_sq->speedy;
	
	int length = sizeof(struct syncmsgbuf) - sizeof(long);
	if(msgsnd(qid, (struct msgbuf*) qbuf, length, 0) == -1){
		perror("msgsnd ");
		exit(1);
	}
}


/* --------------------------------------------------------------------------
 Retrieves a message from the message queue of identifier "qid".
 
 ARGUMENTS:
	-> qid : identifier of the message queue
	-> qbuf : data structure used to store the retrieved message
	-> type : type of the message to retrieve
 ---------------------------------------------------------------------------- */
void rcv_msg(int qid, struct syncmsgbuf* qbuf, long type){
	
	qbuf->mtype = type;
	int length = sizeof(struct syncmsgbuf) - sizeof(long);
	if(msgrcv(qid, qbuf, length, type, 0) == -1){
		perror("msgrcv ");
		exit(1);
	}
}


/* --------------------------------------------------------------------------
 Removes the message queue of identifier "qid" from the kernel.
 
 ARGUMENTS:
	-> qid : identifier of the message queue
 ---------------------------------------------------------------------------- */
void rm_queue(int qid){
	
	msgctl(qid, IPC_RMID, 0);
}


/* --------------------------------------------------------------------------
 Creates (or opens if already exists) a shared memory segment.
 
 ARGUMENTS:
	-> keyval : key value (generally returned by a call to ftok())
	-> segsize : the size of the shared memory segment in bytes
 
 RETURNS:
	-> shmid : identifier of the created/opened shared memory segment
 ---------------------------------------------------------------------------- */
int open_shm(key_t keyval, int segsize){

	int shmid;

	if((shmid = shmget(keyval, segsize, IPC_CREAT|IPC_EXCL|0666)) == -1){
		if((shmid = shmget(keyval, segsize, 0)) == -1){
			perror("shmget");
			exit(1);
		}
	}
	
	return(shmid);
}


/* --------------------------------------------------------------------------
 Writes a value in the shared memory segment.
 
 ARGUMENTS:
	-> segptr : a pointer to the shared memory segment
	-> index : index in the shared memory segment of the shared memory cell
			   to write
	-> value : the value to write in shared memory
 ---------------------------------------------------------------------------- */
void wr_shm(int* segptr, int index, int value){
	
	segptr[index] = value;
}


/* --------------------------------------------------------------------------
 Reads a value from shared memory segment.
 
 ARGUMENTS:
	-> segptr : a pointer to the shared memory segment
	-> index : index of the shared memory cell to read
 ---------------------------------------------------------------------------- */
int rd_shm(int* segptr, int index){
	
	return segptr[index];
}


/* --------------------------------------------------------------------------
 Marks the shared memory segment of identifier "shmid" for removal.
 
 ARGUMENTS:
	-> shmid : identifier of the shared memory segment
 ---------------------------------------------------------------------------- */
void rm_shm(int shmid){
	
	shmctl(shmid, IPC_RMID, 0);
}

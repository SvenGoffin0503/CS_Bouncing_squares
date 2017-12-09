//
//  main_test.c
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
	key_t key_shm;
	int semid;
	int qid;
	int shmid;
	union semun semopts;
	
	key_sem = ftok(".", 'S');
	key_q = ftok(".", 'Q');
	key_shm = ftok(".", 'M');
	
	semid = open_sem(key_sem, 3);
	rm_sem(semid);
	
	qid = open_msgq(key_q);
	qid = open_msgq(key_q);
	
	struct syncmsgbuf send_qbuf;
	long type = 7;
	char* text = "01";
	
	snd_msg(qid, &send_qbuf, type, text);
	
	struct syncmsgbuf rcv_qbuf;
	rcv_msg(qid, &rcv_qbuf, type);
	rm_queue(qid);
	
	shmid = open_shm(key_shm, sizeof(int));
	shmid = open_shm(key_shm, sizeof(int));
	
	return 0;
	
	
}

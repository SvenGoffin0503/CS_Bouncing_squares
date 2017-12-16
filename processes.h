/* ==========================================================================
 processes.h : header file
 ============================================================================ */

#ifndef processes_h
#define processes_h


/* --------------------------------------------------------------------------
 Data structure representing the position of a square in the grid.
 ---------------------------------------------------------------------------- */
typedef struct position{
	int x;
	int y;
} position;


/* --------------------------------------------------------------------------
 Data structure representing a square.
 ---------------------------------------------------------------------------- */
typedef struct square{
	int color;
	int speedx;
	int speedy;
} square;


/* --------------------------------------------------------------------------
 Detects if a key of the keyboard is pressed.
 ---------------------------------------------------------------------------- */
int kbhit(void);


/* --------------------------------------------------------------------------
 Determines if an intersection exists between two squares.
 ---------------------------------------------------------------------------- */
int hasIntersection(position a, position b);


/* --------------------------------------------------------------------------
 Implementation of a "rendez-vous" for all worker processes.
 ---------------------------------------------------------------------------- */
void rendez_vous(int* segptr, int semid, int nb_workers, int nb_rdv);

/* --------------------------------------------------------------------------
 Worker process function : simulates a square.
 ---------------------------------------------------------------------------- */
void worker(int* segptr, int semid, int qid, square square_data, int id,
			 int nb_workers);


/* --------------------------------------------------------------------------
 Exit process function : quits properly the program.
 ---------------------------------------------------------------------------- */
void exit_proc(int* segptr, int semid, int qid, int shmid, int nb_workers);


/* --------------------------------------------------------------------------
 Master process function : manages the display window.
 ---------------------------------------------------------------------------- */
void master(int* segptr, int semid, int nb_workers, square* square_table);


#endif /* processes_h */

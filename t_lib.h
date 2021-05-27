/*
 * Author: Raj Trivedi
 * Partner Name: James Cooper
 * Date: May 15th, 2021
 *
 * This is the header file of the implementation of UD_Thread library
 *
 */

/*
 * types used by thread library
 */
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

/* Definition for a Message Node "messageNode" */
struct messageNode {
        char *message;             // copy of the message
        int len;                   // length of the message
        int sender;                // TID of sender thread
        int receiver;              // TID of receiver thread
        struct messageNode *next;  // pointer to next node
};

/* Definition for a Semaphore Type sem_t */
typedef struct{
        int count;        /* Value of semaphore */
        struct tcb *q;    /* Queue of threads waiting on semaphore */
}sem_t;


/* Definition for a Mailbox "mbox" */
typedef struct {
        struct messageNode *msg;       // message queue
        sem_t              *mbox_sem;  // used as lock
}mbox;

/* Definition for a Thread Control Block(TCB) of each thread */
/* This can be used to create a linked list of TCBs */
struct tcb {
	int                 thread_id;       /* Thread ID */
	int                 thread_priority; /* Priority of a thread to support priority scheduling just like MLFQ */
	ucontext_t         *thread_context;  /* Heap space for storing "ucontext_t" of this particular thread */
	struct messageNode *tmsg;            /* Message Queue for thread */
	sem_t              *counting;        /* Counting Semaphore */
	struct tcb         *next;            /* Pointer to next TCB in the linked list */
};

/*
 * Author: Raj Trivedi
 * Partner Name: James Cooper
 * Date: May 15th, 2021
 *
 * This is the source file of the implementation of UD_Thread library
 */

#include "t_lib.h"

struct tcb *running;                   // Running Queue
struct tcb *head;                      // Head of the Ready Queue
struct tcb *tail;                      // Tail of the Ready Queue
struct tcb* sp_tail;                   // Tail of the Semaphore Queue 
struct messageNode *message_tail;      // Tail of the Message Queue
struct messageNode *mbox_message_tail; // Tail of the Message Queue for Mailbox

/* Initialize a semaphore pointed by "sp" with the given "count" */
int sem_init(sem_t **sp, unsigned int count){

	/* Create a new semaphore pointed to by "sp" with a count value "count" */
	*sp = (sem_t *) malloc(sizeof(sem_t));
	(*sp)->count = count;
	(*sp)->q = NULL;

	return 0;
}

/* Perform a wait(P) operation on the semaphore pointed by "sp" */
void sem_wait(sem_t *sp){

	/* Inhibit interrupts */
	siginterrupt(SIGINT, 1);

	/* Decrement the semaphore count */
	sp->count--;
	
	/* Check if semaphore count is less than 0 */
        if(sp->count < 0){

		/* Place this process P in semaphore queue */
		// Check if semaphore queue is empty
		// If it is, then add process P as first process of semaphore queue
		if( !sp->q ){
			sp->q = (struct tcb *) malloc(sizeof(struct tcb));
			sp->q->thread_id = running->thread_id;
			sp->q->thread_priority = running->thread_priority;
			sp->q->thread_context = running->thread_context;
			sp->q->tmsg = running->tmsg;
			sp->q->counting = running->counting;

			sp->q->next = NULL;
			sp_tail = sp->q;

		}

		// If it not, then add process P after the "tail" of the semaphore queue
		else{
			struct tcb *tmp = (struct tcb *) malloc(sizeof(struct tcb));
                        tmp->thread_id = running->thread_id;
                        tmp->thread_priority = running->thread_priority;
                        tmp->thread_context = running->thread_context;
                        tmp->tmsg = running->tmsg;
                        tmp->counting = running->counting;			
			tmp->next = NULL;

			sp_tail->next = tmp;
			sp_tail = tmp;
		}
		

		/* Assign a new thread to the "running" queue */		
		running->thread_id = head->thread_id;
		running->thread_priority = head->thread_priority;
		running->thread_context = head->thread_context;
		running->tmsg = head->tmsg;
		running->counting = head->counting;

		/* Shift the head of ready queue to its next element */
		struct tcb* tmp = head->next;           
                free(head);
		head = tmp;
		
		/* Do context-switching */
                // Resume running the head of the ready queue after placing P in the semaphore queue 
		// We also save the contents of the process P that was placed in the semaphore queue
                swapcontext(sp_tail->thread_context, running->thread_context);
		
		/* Allow interrupts */
		siginterrupt(SIGINT, 0);
	}	

	else{
		/* Allow interrupts */
		siginterrupt(SIGINT, 0);
	}

}

/* Perform a post(V) operation on the semaphore pointed by "sp" */
void sem_signal(sem_t *sp){

        /* Inhibit interrupts */
	siginterrupt(SIGINT, 1);
        
	/* Increment the semaphore count */	
        sp->count++;

        if(sp->count <= 0){              

		/* Check if ready queue is not empty */
		// If it is not empty, then place the process P at the end of ready queue
		if(head){
			/* Place process P at the end of the ready queue */
			tail->next = (struct tcb *) malloc(sizeof(struct tcb));
			tail->next->thread_id = sp->q->thread_id;
			tail->next->thread_priority = sp->q->thread_priority;
			tail->next->thread_context = sp->q->thread_context;
			tail->next->tmsg = sp->q->tmsg;
			tail->next->counting = sp->q->counting;

			tail->next->next = NULL;
			tail = tail->next;
			
		}

		/* Else insert process at the head of the ready queue */
		else{

			/* Place process P at the head of the ready queue */
	      		head = (struct tcb *) malloc(sizeof(struct tcb));
	      		head->thread_id = sp->q->thread_id;
	      		head->thread_priority = sp->q->thread_priority;
	      		head->thread_context = sp->q->thread_context;
                        head->tmsg = sp->q->tmsg;
                        head->counting = sp->q->counting;			

	      		head->next = NULL;
	      		tail = head;
		}	


		/* Remove process P from the head of the semaphore queue */
		struct tcb *tmp = sp->q->next;                       
                free(sp->q);
                sp->q = tmp;		
        }

	/* Allow interrupts */
	siginterrupt(SIGINT, 0);
}

/* Destroy any state related to the semaphore pointed by "sp" */
void sem_destroy(sem_t **sp){

	/* Checks if there are any process in the semaphore queue */
	if( (*sp)->q ){
		struct tcb *tmp = (*sp)->q;
		while(tmp){
			/* Shift the pointer for head of semaphore queue */
			(*sp)->q = (*sp)->q->next;

			/* If there are queued TCBs, then move them to the end of ready queue */
                        tail->next = (struct tcb *) malloc(sizeof(struct tcb));
                        tail->next->thread_id = tmp->thread_id;
                        tail->next->thread_priority = tmp->thread_priority;
                        tail->next->thread_context = tmp->thread_context;
                        tail->next->next = NULL;
                        tail = tail->next;
			
			/* Free the current process that is in the semaphore queue */
                        free(tmp->thread_context->uc_stack.ss_sp);   
      			free(tmp->thread_context);			
                        free(tmp);
                        tmp = (*sp)->q;			
		}
	}

	/* Free the semaphore itself */
	free(*sp);
}

/* Create a mailbox pointed to by "mb" */
int mbox_create(mbox **mb){
	
	/* Create a mailbox pointed to by "mb" */
	*mb = (mbox *) malloc(sizeof(mbox));

	/* Assign values to each of mailbox fields for initialization */
	(*mb)->msg = NULL;
	sem_init( &( (*mb)->mbox_sem ) , 1);

	return 0;
}

/* Deposit a message "msg" of length "len" into the mailbox pointed by "mb" */
void mbox_deposit(mbox *mb, char *msg, int len){

	/* Check if there are ANY messages in the Message Queue of mailbox "mb" */
	// If there are NONE yet, add the first message into the mailbox "mb"
	if( mb->msg == NULL){

		mb->msg = (struct messageNode *) malloc(sizeof(struct messageNode));

		mb->msg->message = (char *) malloc(len + 1);
		
		strcpy(mb->msg->message, msg);
		mb->msg->len = len;
		mb->msg->sender = running->thread_id;
		mb->msg->next = NULL;
		
		mbox_message_tail = mb->msg;
	}

	// If there is ATLEAST ONE message, then add the message at the tail of the Message Queue
	else{
                struct messageNode *tmp = (struct messageNode *) malloc(sizeof(struct messageNode));

		tmp->message = (char *) malloc(len + 1);
                
		strcpy(tmp->message, msg);
                tmp->len = len;		
		tmp->sender = running->thread_id;
                tmp->next = NULL;
                
		mbox_message_tail -> next = tmp;
		mbox_message_tail = mbox_message_tail -> next;

	}
}

/* Withdraw the first message from the mailbox pointed by "mb" and store it into "msg" and its
 * length into "len" to access later
 *
 * Now, if there is NO message left in the mailbox "mb", then set the length of message "len" to 0
 *
 */
void mbox_withdraw(mbox *mb, char *msg, int *len){

	/* Check if there are ANY messages in the Message Queue pointed by mailbox "mb" */
	// If there are NONE left to withdraw, then set the "len" to 0
	if(mb->msg == NULL){
		printf("There are no messages to withdraw from the mailbox\n");
		*len = 0;
	}

	// Otherwise, store the message and its length into "msg" and "len" respectively
	else{
		strcpy(msg, mb->msg->message);
		*len = mb->msg->len;
		struct messageNode *tmp = mb->msg;
		mb->msg = mb->msg->next;

		free(tmp->message);
		free(tmp);
	}
}

/* Destroy BOTH the Message Queue and the Semaphore pointed by mailbox "mb" and mailbox "mb" itself */
int mbox_destroy(mbox **mb){

	/* Start by destroying Message Queue of mailbox "mb" */
	struct messageNode *tmp = (*mb)->msg;
	while(tmp){
		(*mb)->msg = (*mb)->msg->next;
		free(tmp->message);
		free(tmp);
		tmp = (*mb)->msg;
	}

	/* Destroy Semaphore of Mailbox "mb" */
	sem_destroy( &((*mb)->mbox_sem) );

	/* Now, destroy the mailbox "mb" itself */
	free(*mb);

	return 0;
}

/*
 * This is a helper function for send(...) that adds given message "msg" of length "len"
 * into the Message Queue of thread given as "thread"
 */
void addMessage(struct tcb *thread, char *msg, int len){

	/* Check if there are ANY messages in the Message Queue */
        // If there are NONE yet, add the first message into the Message Queue
        if( thread->tmsg == NULL){

                thread->tmsg = (struct messageNode *) malloc(sizeof(struct messageNode));

                thread->tmsg->message = (char *) malloc(len + 1);

                strcpy(thread->tmsg->message, msg);
                thread->tmsg->len = len;
                thread->tmsg->sender = running->thread_id;
                thread->tmsg->next = NULL;

                message_tail = thread->tmsg;
        }

        // If there is ATLEAST ONE message, then add the message at the tail of the Message Queue
        else{
                struct messageNode *tmp = (struct messageNode *) malloc(sizeof(struct messageNode));

                tmp->message = (char *) malloc(len + 1);

                strcpy(tmp->message, msg);
                tmp->len = len;
                tmp->sender = running->thread_id;
                tmp->next = NULL;

                message_tail -> next = tmp;
                message_tail = message_tail -> next;

        }

}

/*
 * This is a helper function for receive(...) that withdraws the first message from the Message Queue of
 * thread "thread" and stores its content into "msg" and its length into "len"
 *
 */
void removeFirstMessage(struct tcb *thread, char *msg, int *len){

	/* Check if there are ANY messages in the Message Queue */
        // If there are NONE left to withdraw, then set the "len" to 0
        if(thread->tmsg == NULL){
                *len = 0;
        }

        // Otherwise, store the message and its length into "msg" and "len" respectively
        else{
                strcpy(msg, thread->tmsg->message);
                *len = thread->tmsg->len;
                struct messageNode *tmp = thread->tmsg;
                thread->tmsg = thread->tmsg->next;

                free(tmp->message);
                free(tmp);
        }
	
}

/*
 * This is a helper function for receive(...) that withdraws the message from a sender whose Thread ID is given by
 * "tid" from the Message Queue of thread "thread". After withdrawing the sender's message, it stores its contents into "msg" 
 * and its length into "len"
 *
 */
void removeSenderMessage(struct tcb *thread, int *tid, char *msg, int *len){

	*len = 0; // This will mean that sender "tid"'s message is not there in the Message Queue of receiver's thread

        if(thread->tmsg != NULL){
                struct messageNode **indirect = &thread->tmsg;
                struct messageNode *tmp;

                while( (*indirect) != NULL ){
                        if( (*indirect)-> sender == (*tid) ){
                                tmp = *indirect;
                                *indirect = (*indirect)->next;

                                strcpy(msg, tmp->message);
                                *len = tmp->len;

                                free(tmp->message);
                                free(tmp);

                                return;
                        }

                        else{
                                indirect = &( (*indirect)->next );
                        }
                }
        }

}

/*
 * Frees all the dynamically allocated space for Message Queue of a thread
 */
void destroyMessageQueue(struct tcb *thread){
	struct messageNode *tmp = thread->tmsg;
	while(tmp){
		thread->tmsg = thread->tmsg->next;
		free(tmp->message);
		free(tmp);
		tmp = thread->tmsg;
	}
}

/* Send a message "msg" of length "len" to receiver's Message Queue whose Thread ID is given as "tid"
 *
 * Internally, sender just deposits his message for receiver into his Message Queue so that
 * receiver can withdraw it later 
 *
 * Helper method addMessage(...) is used to implement this functionality
 */
void send(int tid, char *msg, int len){
	struct tcb *tmp = head;

	while(tmp && tmp->thread_id != tid){
		tmp = tmp->next;
	}

	/* Deposit a message "msg" of "len" into the receiver Thread ID "tid"'s Message Queue */
	addMessage(tmp, msg, len);
}

/* Receive a message from sender whose Thread ID is given as "tid"
 *
 * Once received, store it into "msg" and its length into "len"
 *
 * Internally, receiver just checks his own Message Queue and withdraws a message, if there is one, 
 * that was intended to send to him
 *
 * Helper method removeFirstMessage(...) and removeSenderMessage(...) are used to implement
 * this functionality
 */
void receive(int *tid, char *msg, int *len){
	
	/* Receive message from ANY sender */
	if( (*tid) == 0){

		*tid = running->tmsg->sender;
		removeFirstMessage(running, msg, len);
	}

	/* Receive message from sender "tid" */
	else{
		removeSenderMessage(running, tid, msg, len);
	}
}

/**************************************   EXTRA CREDIT : RENDEZVOUS --- SYNCHRONOUS COMMUNICATIONS  ******************************************/

/* TEST CASES THAT ARE USED FOR THIS ARE GIVEN AS FOLLOWS:
 *    1. T9.alt(The alternate output for T9)
 *    2. T11.alt(The alternate output for T11)
 */    


/*
 * Block sender until his receiver receives his message
 * If there is a blocked receiver already, then wake him first before sending him message
 *
 * Counting semaphore can be used here so that sender can block himself until some receiver wakes him up
 */
void block_send(int tid, char *msg, int length) {

	/* Check if your receiver is blocked waiting for message */
	// If there is one, then wake him up
	if(running->counting != NULL)
		sem_signal(running->counting);

	/* Send message to receiver whose Thread ID is "tid" */
        send(tid, msg, length);

	/* Iterate over to receiver thread within "ready" queue */
	struct tcb *tmp = head;
	while(tmp && tmp->thread_id != tid ){
		tmp = tmp->next;
	}

	/* Block yourself on receiver's semaphore */
	sem_wait(tmp->counting);
}

/*
 * Blocks receiver if there is NO message from the sender whose Thread ID is "tid"
 * If there is a message from the sender's Thread ID "tid" in the receiver's Message Queue, then basically receiver just wakes up the 
 * sender who is waiting for reception
 *
 * In case if receiver is blocked, then sender wakes up the receiver at some point and sends the message to this receiver. Receiver 
 * then basically withdraw the sender's message from his own Message Queue and then wakes the sender up so that sender can continue
 * his task
 *
 * Counting semaphore can be used here again so that receiver can block himself until some sender wakes him up
 */
void block_receive(int *tid, char *msg, int *length){

	/* Receive message from a particular sender whose thread id is "tid" */
	receive(tid, msg, length);

	/* Check if there are any messages from the sender "tid" in the Message Queue */
	// If there are NO messages from the sender "tid" in the Message Queue, then simply block yourself until you receive one
	if( (*length) == 0){

		/* Iterate over to the particular sender with thread id "tid" within "ready" queue */
		struct tcb *tmp = head;		
		while(tmp && tmp->thread_id != (*tid) ){
			tmp = tmp->next;
		}		

		/* Block yourself on sender's semaphore until you get message from that sender */
		sem_wait(tmp->counting);

		// At this point, message is being received by its sender
	
		/* Withdraw the message sent by your sender from your Message Queue */
		receive(tid, msg, length);

		/* Wake your sender up as he might be waiting for you for reception */
		sem_signal(running->counting);
	}

	// Otherwise, wake the sender up that's waiting for you for reception
	else{
		sem_signal(running->counting);
	}
}

/*********************************************************************************************************************************************/



/* The calling thread volunarily relinquishes the CPU, and is placed at the end of the "ready" queue. 
   The first thread (IF THERE IS ONE) in the ready queue resumes execution. 
*/
void t_yield()
{
        /* Check if there are ANY threads in the "ready" queue */
        // If there are ANY, then start running the thread that is in the head of the "ready" queue 	
	// If there are NONE, then the "main" thread will get executed
	if(head){
		/* Place the calling thread and its corresponding mailbox at the end of "ready" queue */
	       	tail->next = (struct tcb *) malloc(sizeof(struct tcb));
	       	tail->next->thread_id = running->thread_id;
		tail->next->thread_priority = running->thread_priority;
		tail->next->thread_context = running->thread_context;
		tail->next->tmsg = running->tmsg;
		tail->next->counting = running->counting;
		tail->next->next = NULL;
		tail = tail->next;

		/* Replace the thread in "running" queue with the head of the "ready" queue */	     	
		running->thread_id = head->thread_id;
		running->thread_priority = head->thread_priority;
		running->thread_context = head->thread_context;
                running->tmsg = head->tmsg;
                running->counting = head->counting;
		running->next = NULL;
  
		/* Shift the head pointer of "ready" queue to its next thread and free the original head */	
		struct tcb *tmp = head->next;
		free(head);
		head = tmp;		

		/* Do context-switching */
		/* Save the PC for the thread that just got added from "running" queue */
		/* Start running thread that is currently in the "running" queue */		
		swapcontext(tail->thread_context, running->thread_context);
	}
}

/* Initialize the thread library by setting up the "running" and the "ready" queues, creating TCB of the "main" 
   thread, and inserting it into the running queue. 
*/
void t_init()
{

  /* "head" will be used from t_create(...) */
  /* Right now, just initializing it to NULL */	
  head = NULL;	

  /* Save the PC for the "main" thread */
  ucontext_t *tmp;
  tmp = (ucontext_t *) malloc(sizeof(ucontext_t));
  getcontext(tmp);    /* let tmp be the context of main() */

  /* TCB of the "main" thread created and inserted into "running" queue */
  running = (struct tcb *) malloc(sizeof(struct tcb));
  running->thread_id = 0;
  running->thread_priority = 1;
  running->thread_context = tmp;
  running->tmsg = NULL; // initializes message queue for "main" thread
  sem_init( &(running->counting), 0 ); // initializes counting semaphore for "main" thread
  running->next = NULL;
  
}

/* Create a thread with priority "pri", start function "func" with argument "thr_id" as the thread id. 
   Function "func" should be of type "void func(int)". TCB of the newly created thread is added to the end 
   of the "ready" queue; the parent thread calling t_create() continues its execution upon returning 
   from t_create(). 
*/
int t_create(void (*fct)(int), int id, int pri)
{
  /* Size of the stack that will be allocated for the newly created thread */	
  size_t sz = 0x10000; 

  /* Save the PC for the newly created thread */
  ucontext_t *uc;
  uc = (ucontext_t *) malloc(sizeof(ucontext_t));
  getcontext(uc);

  /* Attach a stack for newly created thread */
  uc->uc_stack.ss_sp = malloc(sz); 
  uc->uc_stack.ss_size = sz;
  uc->uc_stack.ss_flags = 0;

  /* When the current context "uc" terminates, then it will resume context pointed by "uc_link" */
  uc->uc_link = running->thread_context;

  /* Attach a function to execute for current context */
  makecontext(uc, (void(*)(void)) fct, 1, id);

  /* Check if head of the "ready" queue is empty or not */

  // If head is empty, then add the first thread to this queue i.e. HEAD OF THE "READY" QUEUE = TAIL OF THE "READY" QUEUE
  if(head == NULL){
	  head = (struct tcb *) malloc(sizeof(struct tcb));
	  head->thread_id = id;
	  head->thread_priority = pri;
	  head->thread_context = uc;	  
	  head->next = NULL;

	  tail = head;
	  
  }

  // If head is not empty, then just add the thread after the "tail" of the "ready" queue 
  else{
	  struct tcb *new_tcb = (struct tcb *) malloc(sizeof(struct tcb));
	  new_tcb->thread_id = id;
	  new_tcb->thread_priority = pri;
	  new_tcb->thread_context = uc;
	  new_tcb->next = NULL;

	  tail->next = new_tcb;
	  tail = new_tcb;
  }

  tail->tmsg = NULL; // initializes message queue for newly created thread
  sem_init( &(tail->counting), 0 ); // initializes counting semaphore for newly created thread

  return 0;

}

/* Terminate the calling thread by removing (and freeing) its TCB from the "running" queue, and 
   resuming execution of the thread in the head of the "ready" queue via setcontext(). Every thread 
   MUST end (semantically) with a call to t_terminate(). 
*/
void t_terminate()
{	
	/* Terminates the calling thread */
	free(running->thread_context->uc_stack.ss_sp);
	free(running->thread_context);	
	destroyMessageQueue(running);
	sem_destroy( &(running->counting) );
	free(running);

	/* Assign a new thread to the "running" queue */
        running = (struct tcb *) malloc(sizeof(struct tcb));
        running->thread_id = head->thread_id;
        running->thread_priority = head->thread_priority;
        running->thread_context = head->thread_context;
	running->tmsg = head->tmsg;
	running->counting = head->counting;	
        running->next = NULL;

        /* Shift the head pointer of "ready" queue to its next thread and free the original head */
	struct tcb *tmp = head->next;
        free(head);
	head = tmp;

	/* Resume execution with the head of the "ready" queue */
	setcontext(running->thread_context);
}

/* Shut down the thread library by freeing all the dynamically allocated memory. */
void t_shutdown()
{
	/* Start by freeing "running" queue */
	if(running){
		free(running->thread_context->uc_stack.ss_sp);
		free(running->thread_context);
		destroyMessageQueue(running);
		sem_destroy( &(running->counting) );	
		free(running);
	}

	/* Now free "ready" queue */
	if(head){
		struct tcb *tmp = head;
		while(tmp){
			head = head->next;
			free(tmp->thread_context->uc_stack.ss_sp);
			free(tmp->thread_context);		
			destroyMessageQueue(tmp);
			sem_destroy( &(tmp->counting) );				
			free(tmp);
			tmp = head;
		}
	}
}

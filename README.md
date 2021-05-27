# User-Level Thread Library
    - This project involved designing and implementing a user-level thread library which supports thread creation, thread scheduling via context-switching, thread      synchronization using Semaphores, and Inter-Thread Communication(ITC) mechanism using Message Passing and Mailbox functionality

    - This project utilizes high level C language to achieve all of the given functionality

# Compilation and Running Instructions
    - Because this is a thread library, this DOES NOT need to be compiled or run
    - However, you can download this thread library on your local machine, include "ud_thread.h" header file on the top of your application, and basically then use the APIs for my thread library

# APIs in my thread library
    - The APIs that you can use in your application for my thread library are given as follows:
         1. void t_init(); // Initializes the thread library                        
         2. void t_shutdown(); // Shuts down the thread library
         3. void t_create(void (*func)(int), int thr_id, int pri); // Creates a thread
         4. void t_terminate(); // Termination of a thread
         5. void t_yield(); // Can be used as context-switching between threads
         6. int sem_init(sem_t **sp, int sem_count); // Creates a new semaphore pointed to by "sp" with count value of "sem_count"
         7. void sem_wait(sem_t *sp); // Current thread does a wait(P) on the specified semaphore
         8. void sem_signal(sem_t *sp); // Current thread does a signal(V) on the specified semaphore
         9. void sem_destroy(sem_t **sp); // Destroys any state related to the specified semaphore
         10. int mbox_create(mbox **mb); // Creates a mailbox pointed to by "mb"
         11. void mbox_destroy(mbox **mb); // Destroys any state related to the mailbox pointed to by "mb"
         12. void mbox_deposit(mbox *mb, char *msg, int len); // Deposits message "msg" of length "len" into the mailbox pointed to by "mb"
         13. void mbox_withdraw(mbox *mb, char *msg, int *len); // Withdraws the first message from the mailbox pointed to by "mb" into "msg" and set the message's length in "len" accordingly.
         14. void send(int tid, char *msg, int len); // Sends a message to the thread whose tid is "tid". "msg" is the pointer to the start of the message, and "len" specifies the length of the message in bytes. 
         15. void receive(int *tid, char *msg, int *len); // If "tid" is 0, then receiver thread receives a message from ANY thread. Otherwise,receiver thread receives message from another thread whose "tid" is given in the parameter 
         16. void block_send(int tid, char *msg, int length); // Sender thread sends a message and waits for reception
         17. void block_receive(int *tid, char *msg, int *length); // Receiver thread waits for and recieves a message from sender thread whose tid is "tid"

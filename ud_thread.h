
/* Definition of a Semaphore type sem_t that can be used in application programs*/
typedef void sem_t;

/* Definition of a Mailbox "mbox" that can be used in application programs */
typedef void mbox;

/* 
 * thread library, semaphore function, mailbox , and message passing function 
 * prototypes
 */

/* Thread Library */
void t_create(void(*function)(int), int thread_id, int priority);
void t_yield(void);
void t_init(void);
void t_terminate(void);
void t_shutdown(void);

/* Semaphore function prototypes */
int  sem_init(sem_t **sp, unsigned int count);
void sem_wait(sem_t *sp);
void sem_signal(sem_t *sp);
void sem_destroy(sem_t **sp);

/* Mailbox function prototypes */
int mbox_create(mbox **mb);
void mbox_destroy(mbox **mb);
void mbox_deposit(mbox *mb, char *msg, int len);
void mbox_withdraw(mbox *mb, char *msg, int *len);

/* Message Passing function prototypes */
void send(int tid, char *msg, int len);
void receive(int *tid, char *msg, int *len);

/* EXTRA CREDIT : RENDEZVOUS --- SYNCHRONOUS COMMUNICATION FUNCTION PROTOTYPES */
void block_send(int tid, char *msg, int length);
void block_receive(int *tid, char *msg, int *length);

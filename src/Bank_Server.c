/**
 * Author: Brayton Rude (rude87@iastate.edu)
 * 
 * CPR E 308 Project 2 - Multithreaded Server
 *      Bank_Server.c contains code for the server that 
 *      manages bank accounts. 
 * 
 * Test File Project2Test_v2.c
 *      compile with: 
 *          gcc -o Project2Test -lpthread Project2Test_v2.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "Bank.h"

/*================================================================
 *                         CONSTANTS                             *
=================================================================*/
#define STR_MAX_SIZE 256
/*===============================================================*/

/*================================================================
 *                         STRUCTURES                            *
=================================================================*/
struct trans {      // Structure for a transaction pair
    int acc_id;     // Account ID
    int amount;     // amount to be added, could be positive or negative
};

struct request {                        // Structure for a request object    
    struct request * next;              // pointer to the next request in the list
    int request_id;                     // request ID assigned by the main thread
    int check_acc_id;                   // account ID for a CHECK request
    struct trans * transactions;        // array of transaction data
    int num_trans;                      // number of accounts in this transaction
    struct timeval starttime, endtime;  // starttime and endtime for TIME
};

struct queue {                      // Structure for a queue 
    struct request * head, * tail;  // head and tail of the list
    int num_jobs;                   // number of jobs currently in queue
};
/*===============================================================*/

/*================================================================
 *                      GLOBAL VARIABLES                         *
=================================================================*/
pthread_mutex_t q_mut;          // q_mut: mutex for accessing queue  
pthread_mutex_t * acc_mut;      // acc_mut: points to an array mutexs associated with every account
pthread_cond_t jobs_cv;         
struct queue Q;                 // Global Queue containing the requests
int clockOut = 0;               // Signifies to the workers that it is time to clock out
FILE *fp;                       // Pointer to output file
/*===============================================================*/

/*================================================================
 *                    FUNCTION DECLARATIONS                      *
 ================================================================*/
void program_loop(pthread_t * workersArray, int numWThreads, int numAccounts);
int end_request_protocol(pthread_t * workersArray, int numWThreads);
void* worker(void *);
int transaction_operation(struct request * job);
void add_request(struct request * r);
struct request * get_request();
void sortIDLeastToGreatest(struct trans * transactions, int num_trans);
/*===============================================================*/

/**
 * Main function of the server that handles server startup and initialization as well as a few exit protocols. 
 * 
 * Syntax to the launch the server program:
 *      $ appserver <# of worker threads> <# of accounts> <output file>;
 * 
 * @param argc - number of command line arguments
 * @param argv - array of command line arguments
 * @return int 
 */
int main(int argc, char *argv[]) {
    // Command Line Input Error Handling
    if (argc != 4) {
        printf("ERROR: Command line input invalid, required format:\n\t$ server <# of worker threads> <# of account> <output file>\n");
        return 0;
    }
    
    // Setting Up Output File
    fp = fopen(argv[3], "w");

    // Initializing Accounts
    int numAccounts = atoi(argv[2]);
    if (!initialize_accounts(numAccounts)) {
        printf("ERROR: Account creation failed.\n");
        return 0;
    }

    // Validate Worker Quantity
    int numWThreads = atoi(argv[1]);
    if (numWThreads < 1) {
        printf("ERROR: Invalid worker thread amount, must be at least 1.\n");
        return 0;
    }

    // Initialize queue Q
    Q.head = NULL;
    Q.tail = NULL;
    Q.num_jobs = 0;

    /*================================================================
     *                     THREAD INITIALIZATION                     *
     ================================================================*/ 
    // array of worker pthreads
    pthread_t workers_tid[numWThreads]; 
    int thread_index[numWThreads];

    pthread_cond_init(&jobs_cv, NULL);

    // Allocating enough space for all the locks
    acc_mut = malloc(sizeof(pthread_t) * numAccounts);  
    
    // initialized mutex for queue
    pthread_mutex_init(&q_mut, NULL); 
    int t;
    for (t = 0; t < numAccounts; t++) {
        // initializes mutex for every account
        pthread_mutex_init(&acc_mut[t], NULL); 
    }

    for (t = 0; t < numWThreads; t++) {
        thread_index[t] = t;
        // creates each worker thread
        pthread_create(&workers_tid[t], NULL, worker, (void *)&thread_index[t]);    
    }
    /*===============================================================*/

    // Enter program loop
    program_loop(workers_tid, numWThreads, numAccounts);

    // Program Termination
    free_accounts();
    fclose(fp);
    return 0;
}

/**
 * The event loop of the program, with each the loop the program takes in a request, parses the request, 
 * then builds the request based on the type of request. After each request is built, they are added to the 
 * request queue where they will wait to be chosen by a worker thread.
 * 
 * @param workersArray - pointer to the start of the worker thread array
 * @param numWThreads  - number worker threads in the thread array
 */
void program_loop(pthread_t * workersArray, int numWThreads, int numAccounts) {
    // Allocate space for input string
    char *userInput = malloc(STR_MAX_SIZE);     
    // Tells token where to split
    const char delim[2] = " ";       
    // Temporarly store input chunk           
    char * token;  
    // Used as the request ID                             
    int requestCount = 1;       
    // Loop Condition                
    int done = 0;                               

    // EVENT LOOP
    while(!done) {
        // Input indicator
        printf("> ");
        // Snags entire line from stdin
        fgets(userInput, STR_MAX_SIZE, stdin);  
        // Replaces newline char with a terminating char
        userInput[strlen(userInput) - 1] = '\0';
        // Gets first input chunk    
        token = strtok(userInput, delim);
        if (!strcmp(token, "END")) {
            // Begin Exit Protocol
            done = end_request_protocol(workersArray, numWThreads);
            // Exit the program loop function
            return;
        } else if (!strcmp(token, "CHECK")) {       
            // CHECK REQUEST PROTOCOL
            // Output indicator
            printf("< ");
            // Get Account ID to check
            token = strtok(NULL, delim);            
            if (token != NULL) {
                // Build Balance Check Request
                struct request bReq;
                bReq.next = NULL;
                bReq.request_id = requestCount;
                bReq.transactions = NULL;
                bReq.num_trans = -1;
                bReq.check_acc_id = atoi(token);
                
                // If valid ID is provided finsish building request and add it to the queue
                if (bReq.check_acc_id <= numAccounts && bReq.check_acc_id > 0) {
                    // Store current time as start time for request
                    gettimeofday(&bReq.starttime, NULL);

                    // Lock the queue
                    pthread_mutex_lock(&q_mut);
                    // Add Request to queue 
                    add_request(&bReq);
                    //pthread_cond_broadcast(&jobs_cv);

                    // unlock the queue
                    pthread_mutex_unlock(&q_mut);

                    // Console Response
                    printf("ID %d\n", requestCount);             
                    // Increment Request Count        
                    requestCount++;       
                } else {
                    // Error message
                    printf("INVALID REQUEST: The account provided with CHECK request does not exist.\n");   
                }               
            } else {   
                // ERROR
                printf("INVALID REQUEST: Balance Check was not provided an account ID.\n");
            }
        } else if (!strcmp(token, "TRANS")) {       
            // TRANSACTION REQUEST PROTOCOL
            // Output indicator
            printf("< ");
            // Stores request validity: 1 = valid, 0 = invalid
            int validRequest = 1;                   
            // Build Transaction Request
            struct request tReq;
            tReq.next = NULL;
            tReq.request_id = requestCount;
            tReq.check_acc_id = -1;
            gettimeofday(&tReq.starttime, NULL);
            // Allocating Space for up to 10 transaction pairs
            tReq.transactions = malloc(sizeof(struct trans) * 10);  
            // Set number of transactions
            tReq.num_trans = 0;                                    
            // Build Transaction Pairs
            int i;
            for (i = 0; i < 10; i++) {
                // Get Account ID
                token = strtok(NULL, delim);                        
                if (token != NULL) { 
                    // Assign token to account ID
                    tReq.transactions[i].acc_id = atoi(token);     
                    // Get amount value
                    token = strtok(NULL, delim);                    
                    if (token != NULL && tReq.transactions[i].acc_id <= numAccounts && tReq.transactions[i].acc_id > 0) {
                        // Assign token to amount
                        tReq.transactions[i].amount = atoi(token);  
                        // Increase Transaction count
                        tReq.num_trans++;                           
                    } else {
                        printf("INVALID REQUEST: an account within the Transaction Request was not provided a transaction amount or an invalid account number was provided.\n");
                        // Request Failed
                        validRequest = 0; 
                        // End loop
                        i = 10;             
                    }
                } else {
                    // End of transaction pairs
                    i = 10; // End loop
                    if (tReq.num_trans < 1) {
                        printf("INVALID REQUEST: no transaction pairs were provided with transaction request.\n");
                        validRequest = 0;   // Request failed
                    }
                }
            }
            // Add request and Increment Request Count
            if (validRequest) {
                // Lock the queue
                pthread_mutex_lock(&q_mut);
                // Add request to queue
                add_request(&tReq);
                //pthread_cond_broadcast(&jobs_cv);
                // unlock the queue
                pthread_mutex_unlock(&q_mut);
                // Console Response
                printf("ID %d\n", requestCount); 
                requestCount++;
            }
        } else {
            printf("INVALID REQUEST: no action taken.\n");
        }
        // Clear input string
        strcpy(userInput, "");
    }
}

/**
 * Handles the termination state of the server. The function will wait until all 
 * remaining requests in the request queue are handled. From there it signals to 
 * the workers that they are free to clock out, and then joins the main thread
 * with all of the worker threads to wait until each worker finishes.
 * 
 * @param workersArray 
 * @param numWThreads 
 * @return int* 
 */
int end_request_protocol(pthread_t * workersArray, int numWThreads) {
   // Wait for job queue to reach to zero
   while (Q.num_jobs != 0) {
       // wait
       // pthread_cond_broadcast(&jobs_cv);
    }

    printf("Job Count: %d", Q.num_jobs);

    // Signifies to workers, that they can finish
    clockOut = 1;

    // Join Threads to make main wait for worker threads before proceeding
    int t = 0;
    for (t = 0; t < numWThreads; t++) {
        pthread_join(workersArray[t], NULL);
    }
    // Return value of 1 to be assigned to program loop condition
    return 1;
}

/**
 * Handles the worker thread operations. Continues to loop until clockOut signals the loop to end. Worker will wait until a
 * job is available, and once the job is acquired the worker determines whether it is a CHECK request or a TRANS request. 
 * Worker then carrys out the job and repeats the process. 
 * 
 * @return void* 
 */
void* worker(void * arg) {
    while (clockOut == 0 || Q.num_jobs > 0) {
        // Pointer to worker's current task
        struct request * job = NULL;
        // Lock the queue
        pthread_mutex_lock(&q_mut);
        //while(Q.num_jobs == 0 && clockOut == 0) {
            //pthread_cond_wait(&jobs_cv, &q_mut);
        //}
        // Attempts to get a job, if NULL, there are no current jobs in the queue
        job = get_request();
        // Unlock the queue
        pthread_mutex_unlock(&q_mut);
          
        if (job != NULL && job->check_acc_id == -1) {
            // Perform Transaction operation
            // Sort Transactions by Account ID from least to greatest
            sortIDLeastToGreatest(job->transactions, job->num_trans);
            // Acquire Locks for each of the accounts
            int i;
            for (i = 0; i < job->num_trans; i++) {
                pthread_mutex_lock(&acc_mut[job->transactions[i].acc_id - 1]);
            }
            // Attempt operation
            int insufAccID = transaction_operation(job);
            // Relenquishe Locks for each count
            for (i = 0; i < job->num_trans; i++) {
                pthread_mutex_unlock(&acc_mut[job->transactions[i].acc_id - 1]);
            }
            // Get endtime
            gettimeofday(&job->endtime, NULL);
            // Print result to file
            if (insufAccID == -1) {
                // lock print file
                flockfile(fp);
                fprintf(fp, "%d OK TIME %ld.%06ld %ld.%06ld \tThread: %d\n", job->request_id, job->starttime.tv_sec, job->starttime.tv_usec, job->endtime.tv_sec, job->endtime.tv_usec, *((int *)arg));
                // unlock print file
                funlockfile(fp);
            } else {
                // lock print file
                flockfile(fp);
                fprintf(fp, "%d ISF %d TIME %ld.%06ld %ld.%06ld \tThread: %d\n", job->request_id, insufAccID, job->starttime.tv_sec, job->starttime.tv_usec, job->endtime.tv_sec, job->endtime.tv_usec, *((int *)arg));
                // unlock print file
                funlockfile(fp);
            }
        } 

        if (job != NULL && job->check_acc_id != -1) {
            // Perform Balance operation
            // Get lock associated account id
            pthread_mutex_lock(&acc_mut[job->check_acc_id - 1]);
            // Call read account and store result
            int balance = read_account(job->check_acc_id);
            // reliquishe the lock 
            pthread_mutex_unlock(&acc_mut[job->check_acc_id - 1]);
            // Get endtime
            gettimeofday(&job->endtime, NULL);
            // lock print file
            flockfile(fp);
            // Print result to file
            fprintf(fp, "%d BAL %d TIME %ld.%06ld %ld.%06ld \tThread: %d\n", job->request_id, balance, job->starttime.tv_sec, job->starttime.tv_usec, job->endtime.tv_sec, job->endtime.tv_usec, *((int *)arg));
            // unlock print file
            funlockfile(fp);
        }
    }
    exit(0);
}

/**
 * Performs a transaction operation on the provided request structure. If any account is incapable of carrying out the transaction
 * without suffficient funds, then all transactions in the structure are voided and keep their original balances.
 * 
 * @param job - structure containing request information.
 * @return int - returns -1 if transactions were sufficient, or returns account ID of the first account with insufficient funds.
 */
int transaction_operation(struct request * job) {
    // Gets value of ISF account if found    
    int firstISFAcc = -1;
    // Array of the balances corresponding to accounts
    int balanceArr[job->num_trans];

    int i;
    for (i = 0; i < job->num_trans && firstISFAcc == -1; i++) {
        // Get Account Balance
        balanceArr[i] = read_account(job->transactions[i].acc_id);
        // Perform Transaction
        balanceArr[i] = balanceArr[i] + job->transactions[i].amount;
        // Check if transaction is valid
        if(balanceArr[i] < 0) {
            // Transaction was not valid, store account ID
            firstISFAcc = job->transactions[i].acc_id;
        }
    }
    // Write new balances to accounts if all transactions are valid
    if (firstISFAcc == -1) {
        // Write new balances
        for (i = 0; i < job->num_trans; i++) {
            // Write new balance to the account
            write_account(job->transactions[i].acc_id, balanceArr[i]);
        }
    }
    // Return ID of the ISF account or -1 if all accounts performed transactions successfully
    return firstISFAcc;
}

/**
 * Adds a new job to the end of the job queue.
 * 
 * @param r - job of struct request type to be added to the queue
 */
void add_request(struct request * r) {
    // Skip if new request is NULL
    if (r != NULL) {
        // Check if queue is empty
        if (Q.num_jobs < 1) {
            // r will be the head and tail 
            Q.head = r;
            Q.tail = r;
        } else {
            // previous tail now points to new tail
            Q.tail->next = r;
            // tail gets new tail
            Q.tail = r;
        }
        // Increment job count
        Q.num_jobs++;
        // Return 1 for succesul request addition
        return;
    }
    printf("WARNING: Request to add was NULL, so it was not added to the job queue.\n");
}

/**
 * Removes the job at the front of the job queue and returns a pointer the removed job.
 * 
 * @return struct request* - the job from the front of the queue. Returns NULL if queue is empty.
 */
struct request * get_request() {  
    if (Q.num_jobs < 1) {
        // Queue is empty return NULL
        return NULL;
    } 
    // struct value to return
    struct request * task;
    // Task gets the first job in line
    task = Q.head;
    
    // Give queue pointer NULL if no more jobs exist
    if(Q.num_jobs == 1) {
        // Reassign head to NULL since queue would now be empty
        Q.head = NULL;
        // Reassign tail to NULL since queue would now be empty
        Q.tail = NULL;
    } else {
        // Head gets the next job in line
        Q.head = Q.head->next;
    }
    // Decrement job count
    Q.num_jobs--;
    // return the task
    return task;
}

/**
 * Takes a pointer to an array of trans type objects and sorts the elements from least to greatest based on account id.
 * 
 * @param transactions - array to be sorted
 * @param num_trans - number of elements in the array
 */
void sortIDLeastToGreatest(struct trans * transactions, int num_trans) {
    // Temporary trans object
    struct trans temp;
    int i, j;
    for (i = 0; i < num_trans; i++) {
        // Checking current i against every other object in the array
        for (j = i + 1; j < num_trans; j++) {
            // If true swap the two transactions
            if (transactions[i].acc_id > transactions[j].acc_id) {
                // swapping positions
                temp = transactions[j];
                transactions[j] = transactions[i];
                transactions[i] = temp;
            }
        }
    }
}
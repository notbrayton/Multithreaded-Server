/**
 * Author: Brayton Rude (rude87@iastate.edu)
 * 
 * CPR E 308 Project 2 - Multithreaded Server
 *      Bank_Server.c contains code for the server that 
 *      manages bank accounts. 
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
int add_request(struct request * r);
struct request * get_request();
/*===============================================================*/

/**
 * Main function of the server that handles server startup and initialization as well as a few exit protocols. 
 * 
 * Syntax to the launch the server program:
 *      $ server <# of worker threads> <# of accounts> <output file>;
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
        // creates each worker thread
        pthread_create(&workers_tid[t], NULL, worker, NULL);    
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
    char *userInput = malloc(STR_MAX_SIZE);     // Allocate space for input string
    const char delim[2] = " ";                  // Tells token where to split
    char * token;                               // Temporarly store input chunk
    int requestCount = 1;                       // Used as the request ID
    int done = 0;                               // Loop Condition

    // EVENT LOOP
    while(!done) {
        printf("> ");
        // Snags entire line from stdin
        fgets(userInput, STR_MAX_SIZE, stdin);  
        // Replaces newline char with a terminating char
        userInput[strlen(userInput) - 1] = '\0';
        // Gets first input chunk    
        token = strtok(userInput, delim);
        
        printf("< ");

        if (!strcmp(token, "END")) {
            // Begin Exit Protocol
            done = end_request_protocol(workersArray, numWThreads);
        } else if (!strcmp(token, "CHECK")) {       
            // CHECK REQUEST PROTOCOL
            // Get Account ID to check
            token = strtok(NULL, delim);            
            if (token != NULL) {
                // Build Balance Check Request
                struct request bReq;
                bReq.next = NULL;
                bReq.request_id = requestCount;
                bReq.check_acc_id = atoi(token);
                bReq.transactions = NULL;
                bReq.num_trans = -1;
                gettimeofday(&bReq.starttime, NULL);

                // Add Request to queue 
                add_request(&bReq);
                // Console Response
                printf("ID %d\n", requestCount);             
                // Increment Request Count        
                requestCount++;                         
            } else {   
                // ERROR
                printf("INVALID REQUEST: Balance Check was not provided an account ID.\n");
            }
        } else if (!strcmp(token, "TRANS")) {       
            // TRANSACTION REQUEST PROTOCOL
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
                token = strtok(NULL, delim);                        // Get Account ID
                if (token != NULL) { 
                    tReq.transactions[i].acc_id = atoi(token);      // Assign token to account ID
                    token = strtok(NULL, delim);                    // Get amount value
                    if (token != NULL && tReq.transactions[i].acc_id < numAccounts && tReq.transactions[i].acc_id >= 0) {
                        tReq.transactions[i].amount = atoi(token);  // Assign token to amount
                        tReq.num_trans++;                           // Increase Transaction count
                    } else {
                        printf("INVALID REQUEST: an account within the Transaction Request was not provided a transaction amount or an invalid account number was provided.\n");
                        validRequest = 0;   // Request Failed
                        i = 10;             // End loop
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
                add_request(&tReq);
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
    }
    
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
 * @brief 
 * 
 * @return void* 
 */
void* worker(void * arg) {
    while (!clockOut) {
        // Pointer to worker's current task
        struct request * job = NULL;

        //fprintf(fp, "Worker beginning job search...\n");

        // Waits until a job is available or it is time to clock out
        while (job == NULL) {
            // returns if clockOut is true and no jobs remain
            if (clockOut && Q.num_jobs == 0) {
                return NULL;
            }
            
            // Lock the queue
            pthread_mutex_lock(&q_mut);
            // Attempts to get a job, if NULL, there are no current jobs in the queue
            job = get_request();
            // Unlock the queue
            pthread_mutex_unlock(&q_mut);
        }

        //fprintf(fp, "Working on request %d...\n", job->request_id);

        if (job->check_acc_id == -1) {
            // Perform Transaction operation
            // Acquire Locks for each of the accounts
            int i;
            for (i = 0; i < job->num_trans; i++) {
                pthread_mutex_lock(&acc_mut[job->transactions[i].acc_id]);
            }

            // Perform Transactions
            int accountsSufficient = 1;
            int firstISFAcc;
            int balanceArr[job->num_trans];
            for (i = 0; i < job->num_trans && accountsSufficient == 1; i++) {
                // Get Account Balance
                balanceArr[i] = read_account(job->transactions[i].acc_id);
                // Perform Transaction
                balanceArr[i] = balanceArr[i] + job->transactions[i].amount;
                // Check if transaction is valid
                if(balanceArr[i] < 0) {
                    // Transaction was not valid, store account ID
                    firstISFAcc = job->transactions[i].acc_id;
                    // Set ISF trigger
                    accountsSufficient = 0;
                }
            }

            // Write new balances to accounts if all transactions are valid
            if (accountsSufficient) {
                // Write new balances
                for (i = 0; i < job->num_trans; i++) {
                    // Write new balance to the account
                    write_account(job->transactions[i].acc_id, balanceArr[i]);
                }
            }

            // Relenquishe Locks for each count
            for (i = 0; i < job->num_trans; i++) {
                pthread_mutex_unlock(&acc_mut[job->transactions[i].acc_id]);
            }

            // Get endtime
            gettimeofday(&job->endtime, NULL);
            // Print result to file
            if (accountsSufficient) {
                fprintf(fp, "%d OK TIME %ld.%06.ld %ld.%06.ld\n", job->request_id, job->starttime.tv_sec, job->starttime.tv_usec, job->endtime.tv_sec, job->endtime.tv_usec);
            } else {
                fprintf(fp, "%d ISF %d TIME %ld.%06.ld %ld.%06.ld\n", job->request_id, firstISFAcc, job->starttime.tv_sec, job->starttime.tv_usec, job->endtime.tv_sec, job->endtime.tv_usec);
            }
        } else {
            // Perform Balance operation
            // Get lock associated account id
            pthread_mutex_lock(&acc_mut[job->check_acc_id]);
            // Call read account and store result
            int balance = read_account(job->check_acc_id);
            // reliquishe the lock 
            pthread_mutex_unlock(&acc_mut[job->check_acc_id]);

            // Get endtime
            gettimeofday(&job->endtime, NULL);
            // Print result to file
            fprintf(fp, "%d BAL %d TIME %ld.%06.ld %ld.%06.ld\n", job->request_id, balance, job->starttime.tv_usec, job->endtime.tv_sec, job->endtime.tv_usec);
        }

        //fprintf(fp, "Request Finished, Jobs Remaining: %d\n", Q.num_jobs);
    }
    return NULL;
}

/**
 * @brief 
 * 
 * @param r 
 * @return int 
 */
int add_request(struct request * r) {
    // Skip if new request is NULL
    if (r != NULL) {
        // Lock the queue
        pthread_mutex_lock(&q_mut);

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

        //fprintf(fp, "Request added to queue. Current job count: %d\n", Q.num_jobs);

        // Unlock the queue
        pthread_mutex_unlock(&q_mut);

        // Return 1 for succesul request addition
        return 1;
    }

    printf("WARNING: Request to add was NULL, so it was not added to the job queue.\n");
}

/**
 * @brief 
 * 
 * @return struct request* 
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
    // Head gets the next job in line
    Q.head = Q.head->next;
    if(Q.num_jobs == 1) {
        // Reassign tail to NULL since queue would now be empty
        Q.tail = NULL;
    }
    // Decrement job count
    Q.num_jobs--;
    //fprintf(fp, "Request removed from queue. Current job count: %d\n", Q.num_jobs);

    // return the task
    return task;
}
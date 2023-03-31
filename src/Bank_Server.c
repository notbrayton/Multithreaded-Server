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

/*
    struct timeval time;
    gettimeofday(&time, NULL);
    printf("TIME %ld.%06.ld\n", time.tv_sec, time.tv_usec);
*/    

/*  INPUT:
        Balance Check
            CHECK <accountid>
        Transaction
            TRANS <acct1> <amount1> <acct2> <amount2> <acct3> <amount3> …
        Exit Program
            END
*/

// Constants
#define STR_MAX_SIZE 256

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

// Global Variables
pthread_mutex_t mut, q_mut;    
pthread_cond_t worker_cv;
struct queue Q;
int clockOut = 0;

// Function Declaration
void* worker();
int add_request(struct request * r);

int main(int argc, char *argv[]) {
    // Command Line Input Error Handling
    if (argc != 4) {
        printf("ERROR: Command line input invalid, required format:\n\t$ server <# of worker threads> <# of account> <output file>\n");
        return 0;
    }
    
    // Setting Up Output File
    FILE *fp;
    fp = fopen(argv[3], "w");

    // Initializing Accounts
    int numAccounts = atoi(argv[2]);
    if (!initialize_accounts(numAccounts)) {
        printf("ERROR: Account creation failed.\n");
        return 0;
    }

    // Creating Worker Threads
    int numWThreads = atoi(argv[1]);
    if (numWThreads < 1) {
        printf("ERROR: Invalid worker thread amount, must be at least 1.\n");
        return 0;
    }
    pthread_t workers_tid[numWThreads]; // array of worker pthreads
    pthread_mutex_init(&mut, NULL);     // initializes mutex
    pthread_cond_init(&worker_cv, NULL);     // initialized queue conditional variable
    int t;
    for (t = 0; t < numWThreads; t++) {
        pthread_create(&workers_tid[t], NULL, worker, NULL);    // creates each worker thread
    }

    // Initialize queue Q
    Q.head = NULL;
    Q.tail = NULL;
    Q.num_jobs = 0;

    //  BEGINNING PROGRAM LOOP
    char *userInput = malloc(STR_MAX_SIZE);         // Allocate space for input string
    int requestCount = 1;                           // Used as the request ID
    while(1) {
        // PARSING INPUT  
        fgets(userInput, STR_MAX_SIZE, stdin);      // Snags entire line from stdin
        userInput[strlen(userInput) - 1] = '\0';    // Replaceds newline char with a terminating char
        const char delim[2] = " ";                  // Tells token where to split
        char * token;                               // Temporarly store input chunk
        token = strtok(userInput, delim);           // Gets first input chunk

        if (!strcmp(token, "END")) {
            // Stop taking requests
            // Maybe join main thread with worker threads, so main will wait till all workers finish

            clockOut = 1;
            free_accounts();
            return 0;

        } else if (!strcmp(token, "CHECK")) {       // BALANCE CHECK
            token = strtok(NULL, delim);            // Get Account ID to check
            if (token != NULL) {
                // Build Balance Check Request
                struct request bReq;
                bReq.next = NULL;
                bReq.request_id = requestCount;
                bReq.check_acc_id = atoi(token);
                bReq.transactions = NULL;
                bReq.num_trans = -1;
                gettimeofday(&bReq.starttime, NULL);

                add_request(&bReq);                     // Add Request to queue 
                requestCount++;                         // Increment Request Count
            } else {   
                // ERROR
                printf("INVALID REQUEST: Balance Check was not provided an account ID.\n");
            }
        } else if (!strcmp(token, "TRANS")) {       // TRANSACTION
            int validRequest = 1;                   // Stores request validity: 1 = valid, 0 = invalid

            // Build Transaction Request
            struct request tReq;
            tReq.next = NULL;
            tReq.request_id = requestCount;
            tReq.check_acc_id = -1;
            gettimeofday(&tReq.starttime, NULL);
            tReq.transactions = malloc(sizeof(struct trans) * 10);  // Allocating Space for up to 10 transaction pairs
            tReq.num_trans = 0;                                     // Set number of transactions

            // Build Transaction Pairs
            int i;
            for (i = 0; i < 10; i++) {
                token = strtok(NULL, delim);                        // Get Account ID
                if (token != NULL) { 
                    tReq.transactions[i].acc_id = atoi(token);      // Assign token to account ID
                    token = strtok(NULL, delim);                    // Get amount value
                    if (token != NULL) {
                        tReq.transactions[i].amount = atoi(token);  // Assign token to amount
                        tReq.num_trans++;                           // Increase Transaction count
                    } else {
                        printf("INVALID REQUEST: an account within the Transaction Request was not provided a transaction amount.\n");
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
                requestCount++;
            }
        } else {
            printf("INVALID REQUEST: no action taken.\n");
        }
        
        // Clear input string
        strcpy(userInput, "");
    }
}

void* worker() {
    while (!clockOut) {
        if (!clockOut) {
            // Get q_mut
            // Wait for jobs
            // Get New Job
                // Copy the request at the front of the queue
                // Reassign Head to the next request
                // Relenquish q_mut

            // Determine Job Type
                // if check_acc_id == -1, then the job is a transaction
                // else it is a balance check

            // Execute Job Task

            // Repeat
        }
    }
    return NULL;
}

int add_request(struct request * r) {
    return -1;
}
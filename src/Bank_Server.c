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
pthread_mutex_t mut;    
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
    pthread_t workers_tid[numWThreads];
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
        fgets(userInput, STR_MAX_SIZE, stdin);      // Snags entire line from stdin
        userInput[strlen(userInput) - 1] = '\0';    // Replaceds newline char with a terminating char
        const char delim[2] = " ";                  // Tells token where to split
        char * token;                               // Temporarly store input chunk
        token = strtok(userInput, delim);           // Gets first input chunk

        if (!strcmp(token, "END")) {

            clockOut = 1;
            free_accounts();
            return 0;

        } else if (!strcmp(token, "CHECK")) {   // BALANCE CHECK
            token = strtok(NULL, delim);    // Get Account ID

            // Build Balance Check Request
            struct request br1;
            br1.next = NULL;
            br1.request_id = requestCount;
            br1.check_acc_id = atoi(token);
            br1.transactions = NULL;
            br1.num_trans = -1;
            gettimeofday(&br1.starttime, NULL);
            add_request(&br1);
            requestCount++;

        } else if (!strcmp(token, "TRANS")) {

            // TRANSACTION
            printf("Transaction Request\n");

        } else {
            printf("ERROR: Invalid Request, no action taken.\n");
        }
        
        // Clear input string
        strcpy(userInput, "");
    }
}

void* worker() {
    while (!clockOut) {
        if (!clockOut) {
            // Get New Job

            // Determine Job Type

            // Execute Job Task

            // Repeat
        }
    }
    return NULL;
}

int add_request(struct request * r) {
    return -1;
}
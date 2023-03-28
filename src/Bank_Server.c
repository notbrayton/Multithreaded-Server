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
#include <pthread.h>
#include "Bank.h"

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

// Function Declaration
void* dummy_thread(FILE * fp);

int main(int argc, char *argv[]) {
    // Command Line Input Error Handling
    if (argc != 4) {
        printf("ERROR: Command line input invalid, required format:\n\t$ server <# of worker threads> <# of account> <output file>\n");
        return 0;
    }

    int numWThreads = atoi(argv[1]);
    int numAccounts = atoi(argv[2]);
    char * outFName = argv[3];
    
    // Setting Up Output File
    printf("Creating Output File <%s>...\n", outFName);
    FILE *fp;
    fp = fopen(outFName, "w");
    fprintf(fp, "Hello, File!\n");
    fclose(fp);

    // Creating Worker Threads
    printf("Creating %d Worker Threads...\n", numWThreads);
    if (numWThreads < 1) {
        printf("ERROR: Invalid worker thread amount, must be at least 1.\n");
        return 0;
    } else {
        // Create Worker Threads
    }

    // Initializing Accounts
    printf("Creating %d Accounts...\n", numAccounts);
    if (!initialize_accounts(numAccounts)) {
        printf("ERROR: Account creation failed.\n");
        return 0;
    }

/*
    struct timeval time;
    gettimeofday(&time, NULL);
    printf("TIME %ld.%06.ld\n", time.tv_sec, time.tv_usec);
*/    
         
    // Begin Program Loop
        // Waits for request
        // Checks request's validity
        // Adds request to request queue
            // Unless the request is an END request

    // Creates a pthread variable
    pthread_t d1;
    
    // Creates a new thread with a storage location d1, and start routine dummy_thread
    pthread_create(&d1, NULL, dummy_thread, NULL);

    // Pauses the calling thread main()'s execution until the thread under d1 is terminated
    pthread_join(d1, NULL);

    return 0;
}

void* dummy_thread(FILE * fp) {
    fprintf(fp, "Hello from dummy thread.");
}
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

// Global Queue
struct queue Q;

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

    // Creating Worker Threads
    int numWThreads = atoi(argv[1]);
    if (numWThreads < 1) {
        printf("ERROR: Invalid worker thread amount, must be at least 1.\n");
        return 0;
    } else {
        // Create Worker Threads

        // Use the number of worker threads to set the size of a pthread array containing the worker threads
        // pthread_t workers_tid[numWorkers];
        // int thread_index[numWorkers];
    }

    // Initializing Accounts
    int numAccounts = atoi(argv[2]);
    if (!initialize_accounts(numAccounts)) {
        printf("ERROR: Account creation failed.\n");
        return 0;
    }

    // Initialize queue Q
    Q.head = NULL;
    Q.tail = NULL;
    Q.num_jobs = 0;

    char *userInput = malloc(STR_MAX_SIZE);       // Allocate space for input string
    int requestCount = 1;
    // PROGRAM LOOP
    while(1) {
        fgets(userInput, STR_MAX_SIZE, stdin);      // Snags entire line from stdin
        userInput[strlen(userInput) - 1] = '\0';
        const char delim[2] = " ";
        char * token;
        token = strtok(userInput, delim);

        if (!strcmp(token, "END")) {
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

        while(token != NULL) {
            printf(" %s\n", token);
            token = strtok(NULL, delim);
        }
        
        // Clear input string
        strcpy(userInput, "");
    }
}

void* worker() {
    // Do Worker Stuff Here PEPELAUGH
}

int add_request(struct request * r) {

}
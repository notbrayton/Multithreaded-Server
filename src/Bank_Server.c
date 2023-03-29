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

// Function Declaration
void* worker();

int main(int argc, char *argv[]) {
    // Command Line Input Error Handling
    if (argc != 4) {
        printf("ERROR: Command line input invalid, required format:\n\t$ server <# of worker threads> <# of account> <output file>\n");
        return 0;
    }
    
    // Setting Up Output File
    FILE *fp;
    fp = fopen(argv[3], "w");
    fprintf(fp, "Hello, File!\n");
    fclose(fp);

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

    /*  INPUT:
            Balance Check
                CHECK <accountid>
            Transaction
                TRANS <acct1> <amount1> <acct2> <amount2> <acct3> <amount3> …
            Exit Program
                END
    */
    char *cmd_line_input = malloc(STR_MAX_SIZE);       // Allocate space for input string
    while(1) {
        fgets(cmd_line_input, STR_MAX_SIZE, stdin);         // Snags entire line from stdin
        cmd_line_input[strlen(cmd_line_input) - 1] = '\0';  // Replaces newline character with terminating character in the input string
        int input_length = strlen(cmd_line_input);          // Stores the length of the current input string

        if(strcmp("END", cmd_line_input)) {
            free_accounts();
            return 0;
        }
    }
}

void* worker() {
    // Do Worker Stuff Here PEPELAUGH
}
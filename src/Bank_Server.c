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

int main(int argc, char *argv[]) {
    // Parse Command line input
    if (argc != 4) {
        printf("ERROR: Command line input invalid, required format:\n\t$ server <# of worker threads> <# of account> <output file>\n");
        return 0;
    }

    int numWThreads = atoi(argv[1]);
    int numAccounts = atoi(argv[2]);
    char * outFName = argv[3];

    printf("Creating %d Worker Threads...\n", numWThreads);
    printf("Creating %d Accounts...\n", numAccounts);
    printf("Creating Output File <%s.txt>...\n", outFName);

    // Initialize progam based on command line input
        // Setup output file
        // Create the fixed number of bank accounts
            // Utilize initialize_accounts(int n); to create accounts
        // Create specified number of worker threads
         
    // Begin Program Loop
        // Waits for request
        // Checks request's validity
        // Adds request to request queue
            // Unless the request is an END request

    return 0;
}
/**
 * Author: Brayton Rude (rude87@iastate.edu)
 * 
 * CPR E 308 Project 2 - Multithreaded Server
 *      Bank_Server.c contains code for the server that 
 *      manages bank accounts. 
*/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv) {
    // Parse Command line input
    if (argc != 4) {
        printf("ERROR: Command line input invalid, required format:\n\t$ appserver <# of worker threads> <# of account> <output file>\n");
        return 0;
    }

    

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
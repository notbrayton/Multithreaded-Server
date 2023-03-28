# Author: Brayton Rude (rude87@iastate.edu)

# the compiler: gcc for C program
CC = gcc

#compiler flags:
#	-g		adds debugging information to the executable file
#	-Wall	turns on almost all compiler warnings
CFLAGS = -g -Wall

# Typing 'make' in the terminal will invoke this call to Bank_Server
all: Bank_Server

# Creates an executable file for Shell using:
# 	- Bank_Server.c
Bank_Server: 
	$(CC) $(CFLAGS) -o appserver Bank_Server.c 

# Typing 'make clean' will invoke a call to this section.
# 'Shell' removes the executable file.
# '-.o' removes old object files.
# '*~' removes backup files.
clean:
	$(RM) Bank_Server *.o *~
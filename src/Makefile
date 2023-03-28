# Author: Brayton Rude (rude87@iastate.edu)

# the compiler: gcc for C program
CC = gcc

#compiler flags:
#	-g		adds debugging information to the executable file
#	-Wall	turns on almost all compiler warnings
CFLAGS = -Wall

# Typing 'make' in the terminal will invoke this call to Server
all: Server

# Creates an executable file for Shell using:
# 	- Bank_Server.c
Server: 
	$(CC) $(CFLAGS) -o server Bank_Server.c 

# Typing 'make clean' will invoke a call to this section.
# 'server' removes the executable file.
# '-.o' removes old object files.
# '*~' removes backup files.
clean:
	$(RM) server *.o *~
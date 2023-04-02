# Author: Brayton Rude (rude87@iastate.edu)

# the compiler: gcc for C program
CC = gcc

#compiler flags:
#	-g		adds debugging information to the executable file
#	-Wall	turns on almost all compiler warnings
CFLAGS = -Wall -lpthread

# Typing 'make' in the terminal will invoke this call to Server
all: Server

# Creates an executable file for Server using:
# 	- Bank_Server.o
#	- Bank.o
Server: Bank_Server.o Bank.o
	$(CC) $(CFLAGS) -o appserver Bank_Server.o Bank.o 

# Creates an object file for Bank_Server.c using:
#	- Bank_Serve.c
#	- Bank.h
Bank_Server.o: Bank_Server.c Bank.h
	$(CC) $(CFLAGS) -c Bank_Server.c

# Creates an object file Bank.o using:
#	- Bank.c
#	- Bank.h
Bank.o: Bank.c Bank.h
	$(CC) $(CFLAGS) -c Bank.c

# Typing 'make clean' will invoke a call to this section.
# 'appserver' removes the executable file.
# '-.o' removes old object files.
# '*~' removes backup files.
clean:
	$(RM) appserver *.o *~
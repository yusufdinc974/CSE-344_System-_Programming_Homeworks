# Makefile for Bank Simulator

CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
LDFLAGS = -pthread

all: BankServer BankClient BankServer_Enhanced

BankServer: server.c common.h
	$(CC) $(CFLAGS) -o BankServer server.c $(LDFLAGS)

BankServer_Enhanced: server.c common.h
	$(CC) $(CFLAGS) -DENHANCED -o BankServer_Enhanced server.c $(LDFLAGS)

BankClient: client.c common.h
	$(CC) $(CFLAGS) -o BankClient client.c $(LDFLAGS)

clean:
	rm -f BankServer BankServer_Enhanced BankClient client_*_fifo *~ *.fifo $(LOG_FILE)

test: all
	./test_script.sh

.PHONY: all clean test
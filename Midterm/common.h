/**
 * common.h - Shared definitions for the Bank Simulator
 */

 #ifndef COMMON_H
 #define COMMON_H
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 #include <signal.h>
 #include <time.h>
 #include <errno.h>
 #include <sys/mman.h>
 #include <semaphore.h>
 
 // Constants
 #define MAX_ACCOUNTS 1000
 #define MAX_BUFFER 256
 #define MAX_CLIENTS 20
 #define BANK_NAME "AdaBank"
 #define LOG_FILE "AdaBank.bankLog"
 
 // Account structure
 typedef struct {
     char account_id[20];
     int balance;
     int is_active;
 } Account;
 
 // Message types
 typedef enum {
     MSG_DEPOSIT,
     MSG_WITHDRAW,
     MSG_RESPONSE
 } MessageType;
 
 // Message structure for communication
 typedef struct {
     MessageType type;
     char account_id[20];
     int amount;
     int status;  // 0: success, negative: error
     pid_t client_pid;
 } Message;
 
 // Create client FIFO name based on PID
 static inline void client_fifo_name(char *buffer, pid_t pid) {
     sprintf(buffer, "client_%d_fifo", pid);
 }
 
 #endif /* COMMON_H */
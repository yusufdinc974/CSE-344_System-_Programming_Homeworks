/**
 * client.c - Client implementation for the Bank Simulator
 */

 #include "common.h"

 // Function prototypes
 void process_client_file(const char *filename, const char *server_fifo);
 void handle_client_request(char *line, int client_num, const char *server_fifo);
 void signal_handler(int sig);
 
 // Global variables
 volatile sig_atomic_t running = 1;
 
 int main(int argc, char *argv[]) {
     if (argc != 3) {
         fprintf(stderr, "Usage: %s <client_file> <server_fifo_name>\n", argv[0]);
         exit(EXIT_FAILURE);
     }
     
     // Set up signal handlers
     signal(SIGINT, signal_handler);
     signal(SIGTERM, signal_handler);
     
     // Process client file
     printf("Reading %s..\n", argv[1]);
     process_client_file(argv[1], argv[2]);
     
     printf("exiting..\n");
     return 0;
 }
 
 // Process client file and create clients
 void process_client_file(const char *filename, const char *server_fifo) {
     FILE *file = fopen(filename, "r");
     if (!file) {
         perror("Failed to open client file");
         exit(EXIT_FAILURE);
     }
     
     // Count the number of lines (client operations)
     int num_clients = 0;
     char line[MAX_BUFFER];
     
     while (fgets(line, MAX_BUFFER, file) && running) {
         if (strlen(line) > 1) {  // Skip empty lines
             num_clients++;
         }
     }
     
     printf("%d clients to connect.. creating clients..\n", num_clients);
     
     // Check if server FIFO exists
     struct stat st;
     if (stat(server_fifo, &st) == -1) {
         printf("Cannot connect %s...\n", server_fifo);
         fclose(file);
         exit(EXIT_FAILURE);
     }
     
     printf("Connected to Adabank..\n");
     
     // Tell the server how many clients are coming
     int server_fd = open(server_fifo, O_WRONLY);
     if (server_fd == -1) {
         perror("Failed to open server FIFO");
         fclose(file);
         exit(EXIT_FAILURE);
     }
     
     Message init_msg;
     init_msg.client_pid = getpid();
     init_msg.amount = num_clients;
     write(server_fd, &init_msg, sizeof(Message));
     
     // Rewind file to beginning
     rewind(file);
     
     // Process each client operation
     int client_num = 0;
     while (fgets(line, MAX_BUFFER, file) && running) {
         if (strlen(line) > 1) {  // Skip empty lines
             handle_client_request(line, ++client_num, server_fifo);
         }
     }
     
     // Wait for all child processes to complete
     int status;
     pid_t wpid;
     while ((wpid = wait(&status)) > 0) {
         printf("Client process %d completed\n", wpid);
     }
     
     // Clean up all client FIFOs
     for (int i = 1; i <= client_num; i++) {
         char client_fifo[MAX_BUFFER];
         client_fifo_name(client_fifo, getpid() * 100 + i);
         unlink(client_fifo);
     }
     
     close(server_fd);
     fclose(file);
 }
 
 // Handle a single client request
 void handle_client_request(char *line, int client_num, const char *server_fifo) {
     // Create unique client ID based on parent PID and client number
     pid_t client_pid = getpid() * 100 + client_num;
     
     // Create client FIFO
     char client_fifo[MAX_BUFFER];
     client_fifo_name(client_fifo, client_pid);
     
     if (mkfifo(client_fifo, 0666) == -1 && errno != EEXIST) {
         perror("Failed to create client FIFO");
         return;
     }
     
     // Fork a new process for this client
     pid_t pid = fork();
     
     if (pid < 0) {
         perror("Fork failed");
         unlink(client_fifo);
         return;
     }
     
     if (pid == 0) {
         // Child process
         
         // Parse line
         char account_id[20];
         char operation[20];
         int amount;
         
         if (sscanf(line, "%s %s %d", account_id, operation, &amount) != 3) {
             fprintf(stderr, "Invalid format in client file\n");
             unlink(client_fifo);
             exit(EXIT_FAILURE);
         }
         
         // Prepare message
         Message msg;
         msg.client_pid = client_pid;
         msg.amount = amount;
         
         if (strcmp(operation, "deposit") == 0) {
             msg.type = MSG_DEPOSIT;
         } else if (strcmp(operation, "withdraw") == 0) {
             msg.type = MSG_WITHDRAW;
         } else {
             fprintf(stderr, "Unknown operation: %s\n", operation);
             unlink(client_fifo);
             exit(EXIT_FAILURE);
         }
         
         // Handle account ID
         if (strcmp(account_id, "N") == 0) {
             strcpy(msg.account_id, "BankID_None");
         } else {
             strcpy(msg.account_id, account_id);
         }
         
         // Print operation
         if (msg.type == MSG_DEPOSIT) {
             printf("Client%d connected..depositing %d credits\n", client_num, amount);
         } else {
             printf("Client%d connected..withdrawing %d credits\n", client_num, amount);
         }
         
         // Open server FIFO
         int server_fd = open(server_fifo, O_WRONLY);
         if (server_fd == -1) {
             perror("Failed to open server FIFO");
             unlink(client_fifo);
             exit(EXIT_FAILURE);
         }
         
         // Send request to server
         write(server_fd, &msg, sizeof(Message));
         close(server_fd);
         
         // Open client FIFO to receive response
         int client_fd = open(client_fifo, O_RDONLY);
         if (client_fd == -1) {
             perror("Failed to open client FIFO for reading");
             unlink(client_fifo);
             exit(EXIT_FAILURE);
         }
         
         // Wait for response
         Message response;
         if (read(client_fd, &response, sizeof(Message)) > 0) {
             // Process response
             if (response.status == 0) {
                 if (response.type == MSG_WITHDRAW && strcmp(response.account_id, "BankID_None") != 0 && 
                     response.amount > 0) { // Account closed
                     printf("Client%d served.. account closed\n", client_num);
                 } else {
                     printf("Client%d served.. %s\n", client_num, response.account_id);
                 }
             } else {
                 printf("Client%d something went WRONG\n", client_num);
             }
         }
         
         close(client_fd);
         unlink(client_fifo);
         exit(EXIT_SUCCESS);
     }
     
     // Parent process continues without waiting
     // We'll collect all children at the end of the program
     // Don't unlink the FIFO here as child is still using it
 }
 
 // Signal handler
 void signal_handler(int sig) {
     (void)sig; // Prevent unused parameter warning
     running = 0;
 }
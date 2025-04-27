/**
 * server.c - Bank Server implementation
 */

 #include "common.h"

 // Global variables
 Account accounts[MAX_ACCOUNTS];
 int next_account_id = 1;
 volatile sig_atomic_t running = 1;
 char server_fifo[MAX_BUFFER];
 
 // Function prototypes
 void initialize_bank();
 void save_bank_log();
 void signal_handler(int sig);
 int find_account_by_id(const char *account_id);
 int create_new_account();
 
 // Basic implementation functions
 void handle_deposit(Message *msg);
 void handle_withdraw(Message *msg);
 void create_teller_basic(pid_t client_pid);
 
 // Enhanced implementation functions
 pid_t Teller(void* func, void* arg_func);
 int waitTeller(pid_t pid, int* status);
 void deposit(void* arg);
 void withdraw(void* arg);
 void create_teller_enhanced(pid_t client_pid);
 
 // Shared memory structure for teller-server communication
 typedef struct {
     Message request;
     Message response;
     sem_t request_sem;
     sem_t response_sem;
 } SharedMemory;
 
 SharedMemory *shared_mems[MAX_CLIENTS];
 int next_shared_mem = 0;
 
 int main(int argc, char *argv[]) {
     if (argc != 3) {
         fprintf(stderr, "Usage: %s BankName ServerFIFO_Name\n", argv[0]);
         exit(EXIT_FAILURE);
     }
 
     // Save server FIFO name
     strcpy(server_fifo, argv[2]);
 
     // Set up signal handlers
     signal(SIGINT, signal_handler);
     signal(SIGTERM, signal_handler);
 
     // Initialize the bank
     initialize_bank();
 
     printf("%s is active....\n", argv[1]);
 
     // Create server FIFO
     if (mkfifo(server_fifo, 0666) == -1 && errno != EEXIST) {
         perror("mkfifo failed");
         exit(EXIT_FAILURE);
     }
 
     int server_fd;
     
     while (running) {
         printf("Waiting for clients @%s...\n", server_fifo);
         
         server_fd = open(server_fifo, O_RDONLY);
         if (server_fd == -1) {
             perror("Failed to open server FIFO");
             break;
         }
 
         // Read client connections from FIFO
         Message msg;
         int client_count = 0;
         pid_t client_group = 0;
         
         while (running && read(server_fd, &msg, sizeof(Message)) > 0) {
             if (client_count == 0) {
                 client_group = msg.client_pid;
                 printf(" - Received %d clients from PID%d..\n", msg.amount, client_group);
                 client_count = msg.amount;
             } else {
                 // Create a teller for this client
                 #ifdef ENHANCED
                 create_teller_enhanced(msg.client_pid);
                 #else
                 create_teller_basic(msg.client_pid);
                 #endif
                 client_count--;
             }
         }
         
         close(server_fd);
     }
 
     // Clean up
     unlink(server_fifo);
     save_bank_log();
     printf("Removing ServerFIFO... Updating log file...\n");
     printf("Adabank says \"Bye\"...\n");
     
     return 0;
 }
 
 // Initialize the bank and load from log if exists
 void initialize_bank() {
     // Initialize all accounts as inactive
     for (int i = 0; i < MAX_ACCOUNTS; i++) {
         accounts[i].is_active = 0;
     }
 
     // Try to load from log file
     FILE *log = fopen(LOG_FILE, "r");
     if (log == NULL) {
         printf("No previous logs.. Creating the bank database\n");
         return;
     }
 
     char line[MAX_BUFFER];
     while (fgets(line, MAX_BUFFER, log)) {
         // Skip comments and empty lines
         if (line[0] == '#' || line[0] == '\n') {
             continue;
         }
         
         char account_id[20];
         int balance;
         
         // Parse line for account information
         // Format: BankID_XX D 300 W 300 0
         if (sscanf(line, "%s", account_id) == 1) {
             char *token = strtok(line + strlen(account_id), " ");
             balance = 0;
             
             // Find the last number on the line (final balance)
             while (token != NULL) {
                 if (sscanf(token, "%d", &balance) == 1) {
                     // This will keep overwriting until we get the last number
                 }
                 token = strtok(NULL, " ");
             }
             
             if (balance > 0) {
                 int idx = find_account_by_id(account_id);
                 if (idx == -1) {
                     // New account
                     int id_num;
                     if (sscanf(account_id, "BankID_%d", &id_num) == 1) {
                         if (id_num >= next_account_id) {
                             next_account_id = id_num + 1;
                         }
                     }
                     
                     idx = id_num;
                     strcpy(accounts[idx].account_id, account_id);
                 }
                 accounts[idx].balance = balance;
                 accounts[idx].is_active = 1;
             }
         }
     }
     
     fclose(log);
 }
 
 // Save bank log
 void save_bank_log() {
     FILE *log = fopen(LOG_FILE, "w");
     if (!log) {
         perror("Failed to open log file");
         return;
     }
     
     // Get current time
     time_t now = time(NULL);
     struct tm *tm_info = localtime(&now);
     char time_str[50];
     strftime(time_str, sizeof(time_str), "%H:%M %B %d %Y", tm_info);
     
     fprintf(log, "# Adabank Log file updated @%s \n", time_str);
     
     // Write active accounts
     for (int i = 1; i < next_account_id; i++) {
         if (accounts[i].is_active) {
             fprintf(log, "%s D %d %d\n", accounts[i].account_id, accounts[i].balance, accounts[i].balance);
         }
     }
     
     fprintf(log, "## end of log. \n");
     fclose(log);
 }
 
 // Signal handler
 void signal_handler(int sig) {
     (void)sig; // Prevent unused parameter warning
     printf("Signal received closing active Tellers\n");
     running = 0;
 }
 
 // Find account by ID
 int find_account_by_id(const char *account_id) {
     int id_num;
     if (sscanf(account_id, "BankID_%d", &id_num) != 1) {
         return -1;  // Invalid format
     }
     
     if (id_num < 1 || id_num >= MAX_ACCOUNTS || !accounts[id_num].is_active) {
         return -1;  // Not found or inactive
     }
     
     return id_num;
 }
 
 // Create a new account
 int create_new_account() {
     int id = next_account_id++;
     sprintf(accounts[id].account_id, "BankID_%d", id);
     accounts[id].balance = 0;
     accounts[id].is_active = 1;
     return id;
 }
 
 // Handle deposit request
 void handle_deposit(Message *msg) {
     int account_idx;
     
     if (strcmp(msg->account_id, "BankID_None") == 0) {
         // New client
         account_idx = create_new_account();
     } else {
         // Existing client
         account_idx = find_account_by_id(msg->account_id);
         if (account_idx == -1) {
             msg->status = -1;  // Account not found
             return;
         }
     }
     
     // Update balance
     accounts[account_idx].balance += msg->amount;
     
     // Update message
     strcpy(msg->account_id, accounts[account_idx].account_id);
     msg->status = 0;  // Success
     
     printf("Client%d deposited %d credits... updating log\n", msg->client_pid, msg->amount);
     save_bank_log();
 }
 
 // Handle withdraw request
 void handle_withdraw(Message *msg) {
     int account_idx = find_account_by_id(msg->account_id);
     
     if (account_idx == -1) {
         msg->status = -1;  // Account not found
         return;
     }
     
     if (accounts[account_idx].balance < msg->amount) {
         printf("Client%d withdraws %d credit.. operation not permitted. \n", 
                msg->client_pid, msg->amount);
         msg->status = -2;  // Insufficient funds
         return;
     }
     
     // Update balance
     accounts[account_idx].balance -= msg->amount;
     
     // Check if account should be closed
     if (accounts[account_idx].balance == 0) {
         accounts[account_idx].is_active = 0;
         printf("Client%d withdraws %d credits... updating log... Bye Client%d\n", 
                msg->client_pid, msg->amount, msg->client_pid);
     } else {
         printf("Client%d withdraws %d credits... updating log\n", 
                msg->client_pid, msg->amount);
     }
     
     // Update message
     msg->status = 0;  // Success
     save_bank_log();
 }
 
 // Create teller process (basic implementation using fork)
 void create_teller_basic(pid_t client_pid) {
     pid_t pid = fork();
     
     if (pid < 0) {
         perror("Fork failed");
         return;
     }
     
     if (pid == 0) {
         // Child process (Teller)
         char client_fifo[MAX_BUFFER];
         client_fifo_name(client_fifo, client_pid);
         
         // Wait for client FIFO to be created
         struct stat st;
         int retry = 0;
         while (stat(client_fifo, &st) == -1) {
             usleep(100000);  // 100ms
             if (++retry > 50) {  // 5 second timeout
                 exit(EXIT_FAILURE);
             }
         }
         
         // Open client FIFO
         int client_fd = open(client_fifo, O_RDWR);
         if (client_fd == -1) {
             perror("Failed to open client FIFO");
             exit(EXIT_FAILURE);
         }
         
         printf(" -- Teller %d is active serving Client%d...\n", getpid(), client_pid);
         
         // Read client request
         Message msg;
         if (read(client_fd, &msg, sizeof(Message)) > 0) {
             // Process request
             if (msg.type == MSG_DEPOSIT) {
                 handle_deposit(&msg);
             } else if (msg.type == MSG_WITHDRAW) {
                 handle_withdraw(&msg);
             }
             
             // Send response back to client
             write(client_fd, &msg, sizeof(Message));
         }
         
         close(client_fd);
         exit(EXIT_SUCCESS);
     }
     
     // Parent continues
 }
 
 // Custom teller creation function (enhanced implementation)
 pid_t Teller(void* func, void* arg_func) {
     // Create shared memory
     SharedMemory *shm = mmap(NULL, sizeof(SharedMemory), 
                             PROT_READ | PROT_WRITE, 
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
     
     if (shm == MAP_FAILED) {
         perror("mmap failed");
         return -1;
     }
     
     // Initialize semaphores
     sem_init(&shm->request_sem, 1, 0);
     sem_init(&shm->response_sem, 1, 0);
     
     // Store shared memory pointer
     shared_mems[next_shared_mem++] = shm;
     
     pid_t pid = fork();
     
     if (pid < 0) {
         perror("Fork failed");
         munmap(shm, sizeof(SharedMemory));
         return -1;
     }
     
     if (pid == 0) {
         // Child process (Teller)
         // Cast and call the function
         void (*teller_func)(void*) = func;
         teller_func(arg_func);
         exit(EXIT_SUCCESS);
     }
     
     return pid;
 }
 
 // Custom wait function for Teller
 int waitTeller(pid_t pid, int* status) {
     return waitpid(pid, status, 0);
 }
 
 // Deposit function for Teller
 void deposit(void* arg) {
     Message *msg = (Message*)arg;
     
     // Copy to shared memory
     SharedMemory *shm = shared_mems[next_shared_mem - 1];
     memcpy(&shm->request, msg, sizeof(Message));
     
     // Signal request is ready
     sem_post(&shm->request_sem);
     
     // Wait for response
     sem_wait(&shm->response_sem);
     
     // Copy response back
     memcpy(msg, &shm->response, sizeof(Message));
 }
 
 // Withdraw function for Teller
 void withdraw(void* arg) {
     Message *msg = (Message*)arg;
     
     // Copy to shared memory
     SharedMemory *shm = shared_mems[next_shared_mem - 1];
     memcpy(&shm->request, msg, sizeof(Message));
     
     // Signal request is ready
     sem_post(&shm->request_sem);
     
     // Wait for response
     sem_wait(&shm->response_sem);
     
     // Copy response back
     memcpy(msg, &shm->response, sizeof(Message));
 }
 
 // Create teller process (enhanced implementation)
 void create_teller_enhanced(pid_t client_pid) {
     char client_fifo[MAX_BUFFER];
     client_fifo_name(client_fifo, client_pid);
     
     // Wait for client FIFO to be created
     struct stat st;
     int retry = 0;
     while (stat(client_fifo, &st) == -1) {
         usleep(100000);  // 100ms
         if (++retry > 50) {  // 5 second timeout
             return;
         }
     }
     
     // Open client FIFO
     int client_fd = open(client_fifo, O_RDWR);
     if (client_fd == -1) {
         perror("Failed to open client FIFO");
         return;
     }
     
     // Read client request
     Message msg;
     if (read(client_fd, &msg, sizeof(Message)) <= 0) {
         close(client_fd);
         return;
     }
     
     // Create appropriate teller
     pid_t teller_pid;
     if (msg.type == MSG_DEPOSIT) {
         teller_pid = Teller(deposit, &msg);
     } else {
         teller_pid = Teller(withdraw, &msg);
     }
     
     printf(" -- Teller %d is active serving Client%d...\n", teller_pid, client_pid);
     
     // Wait for teller to process request
     SharedMemory *shm = shared_mems[next_shared_mem - 1];
     
     // Wait for request to be ready
     sem_wait(&shm->request_sem);
     
     // Process request
     if (shm->request.type == MSG_DEPOSIT) {
         handle_deposit(&shm->request);
     } else if (shm->request.type == MSG_WITHDRAW) {
         handle_withdraw(&shm->request);
     }
     
     // Copy response
     memcpy(&shm->response, &shm->request, sizeof(Message));
     
     // Signal response is ready
     sem_post(&shm->response_sem);
     
     // Wait for teller to complete
     int status;
     waitTeller(teller_pid, &status);
     
     // Send response to client
     write(client_fd, &msg, sizeof(Message));
     
     close(client_fd);
     
     // Clean up shared memory
     sem_destroy(&shm->request_sem);
     sem_destroy(&shm->response_sem);
     munmap(shm, sizeof(SharedMemory));
 }
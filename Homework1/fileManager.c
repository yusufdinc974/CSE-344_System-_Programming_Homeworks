#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

#define LOG_FILE "log.txt"
#define MAX_BUFFER 1024

// Function to log operations
void log_operation(const char *message) {
    int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        const char *error_msg = "Error opening log file\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return;
    }
    
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char timestamp[32];
    char full_log[MAX_BUFFER];
    
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", timeinfo);
    
    // Construct the full log message
    strcpy(full_log, timestamp);
    strcat(full_log, message);
    strcat(full_log, "\n");
    
    write(log_fd, full_log, strlen(full_log));
    close(log_fd);
}

// Function to write message to stdout
void write_message(const char *message) {
    write(STDOUT_FILENO, message, strlen(message));
}

// Function to create a directory
void create_directory(const char *dir_name) {
    struct stat st = {0};
    char log_message[MAX_BUFFER];
    
    // Check if directory already exists
    if (stat(dir_name, &st) == 0) {
        strcpy(log_message, "Error: Directory \"");
        strcat(log_message, dir_name);
        strcat(log_message, "\" already exists.");
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        return;
    }
    
    // Create directory with permissions 0755
    if (mkdir(dir_name, 0755) == 0) {
        strcpy(log_message, "Directory \"");
        strcat(log_message, dir_name);
        strcat(log_message, "\" created successfully.");
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
    } else {
        strcpy(log_message, "Error creating directory \"");
        strcat(log_message, dir_name);
        strcat(log_message, "\": ");
        strcat(log_message, strerror(errno));
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
    }
}

// Function to create a file with timestamp
void create_file(const char *file_name) {
    struct stat st = {0};
    char log_message[MAX_BUFFER];
    
    // Check if file already exists
    if (stat(file_name, &st) == 0) {
        strcpy(log_message, "Error: File \"");
        strcat(log_message, file_name);
        strcat(log_message, "\" already exists.");
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        return;
    }
    
    // Create file and write timestamp
    int fd = open(file_name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        strcpy(log_message, "Error creating file \"");
        strcat(log_message, file_name);
        strcat(log_message, "\": ");
        strcat(log_message, strerror(errno));
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        return;
    }
    
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char timestamp[100];
    int timestamp_len = strftime(timestamp, sizeof(timestamp), 
                               "Created on: %Y-%m-%d %H:%M:%S\n", timeinfo);
    
    write(fd, timestamp, timestamp_len);
    close(fd);
    
    strcpy(log_message, "File \"");
    strcat(log_message, file_name);
    strcat(log_message, "\" created successfully.");
    write_message(log_message);
    write_message("\n");
    log_operation(log_message);
}

// Function to list directory contents
void list_directory(const char *dir_name) {
    pid_t pid = fork();
    
    if (pid < 0) {
        write_message("Fork failed\n");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {  // Child process
        char log_message[MAX_BUFFER];
        DIR *dir;
        struct dirent *entry;
        
        dir = opendir(dir_name);
        if (dir == NULL) {
            strcpy(log_message, "Error: Directory \"");
            strcat(log_message, dir_name);
            strcat(log_message, "\" not found.");
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_FAILURE);
        }
        
        char dir_msg[MAX_BUFFER];
        strcpy(dir_msg, "Contents of directory \"");
        strcat(dir_msg, dir_name);
        strcat(dir_msg, "\":\n");
        write_message(dir_msg);
        
        int count = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                write_message("  ");
                write_message(entry->d_name);
                write_message("\n");
                count++;
            }
        }
        
        if (count == 0) {
            write_message("  (empty directory)\n");
        }
        
        closedir(dir);
        strcpy(log_message, "Listed contents of directory \"");
        strcat(log_message, dir_name);
        strcat(log_message, "\".");
        log_operation(log_message);
        exit(EXIT_SUCCESS);
    } else {  // Parent process
        int status;
        waitpid(pid, &status, 0);  // Wait for child process to complete
    }
}

// Function to list files by extension
void list_files_by_extension(const char *dir_name, const char *extension) {
    pid_t pid = fork();
    
    if (pid < 0) {
        write_message("Fork failed\n");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {  // Child process
        char log_message[MAX_BUFFER];
        DIR *dir;
        struct dirent *entry;
        
        dir = opendir(dir_name);
        if (dir == NULL) {
            strcpy(log_message, "Error: Directory \"");
            strcat(log_message, dir_name);
            strcat(log_message, "\" not found.");
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_FAILURE);
        }
        
        char ext_msg[MAX_BUFFER];
        strcpy(ext_msg, "Files with extension \"");
        strcat(ext_msg, extension);
        strcat(ext_msg, "\" in directory \"");
        strcat(ext_msg, dir_name);
        strcat(ext_msg, "\":\n");
        write_message(ext_msg);
        
        int count = 0;
        size_t ext_len = strlen(extension);
        
        while ((entry = readdir(dir)) != NULL) {
            char *filename = entry->d_name;
            size_t filename_len = strlen(filename);
            
            // Check if file ends with the extension
            if (filename_len > ext_len && 
                strcmp(filename + filename_len - ext_len, extension) == 0) {
                write_message("  ");
                write_message(filename);
                write_message("\n");
                count++;
            }
        }
        
        if (count == 0) {
            char not_found[MAX_BUFFER];
            strcpy(not_found, "No files with extension \"");
            strcat(not_found, extension);
            strcat(not_found, "\" found in \"");
            strcat(not_found, dir_name);
            strcat(not_found, "\".\n");
            write_message(not_found);
        }
        
        closedir(dir);
        strcpy(log_message, "Listed files with extension \"");
        strcat(log_message, extension);
        strcat(log_message, "\" in directory \"");
        strcat(log_message, dir_name);
        strcat(log_message, "\".");
        log_operation(log_message);
        exit(EXIT_SUCCESS);
    } else {  // Parent process
        int status;
        waitpid(pid, &status, 0);  // Wait for child process to complete
    }
}

// Function to read file content
void read_file(const char *file_name) {
    char log_message[MAX_BUFFER];
    struct stat st;
    
    // Check if file exists
    if (stat(file_name, &st) == -1) {
        strcpy(log_message, "Error: File \"");
        strcat(log_message, file_name);
        strcat(log_message, "\" not found.");
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        return;
    }
    
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        strcpy(log_message, "Error opening file \"");
        strcat(log_message, file_name);
        strcat(log_message, "\": ");
        strcat(log_message, strerror(errno));
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        return;
    }
    
    char file_msg[MAX_BUFFER];
    strcpy(file_msg, "Contents of file \"");
    strcat(file_msg, file_name);
    strcat(file_msg, "\":\n");
    write_message(file_msg);
    
    char buffer[MAX_BUFFER];
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the buffer
        write_message(buffer);
    }
    
    // Add a newline if the file doesn't end with one
    if (bytes_read > 0 && buffer[bytes_read - 1] != '\n') {
        write_message("\n");
    }
    
    close(fd);
    strcpy(log_message, "Read contents of file \"");
    strcat(log_message, file_name);
    strcat(log_message, "\".");
    log_operation(log_message);
}

// Function to append content to a file
void append_to_file(const char *file_name, const char *content) {
    char log_message[MAX_BUFFER];
    struct stat st;
    
    // Check if file exists
    if (stat(file_name, &st) == -1) {
        strcpy(log_message, "Error: File \"");
        strcat(log_message, file_name);
        strcat(log_message, "\" not found.");
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        return;
    }
    
    int fd = open(file_name, O_WRONLY | O_APPEND);
    if (fd == -1) {
        strcpy(log_message, "Error opening file \"");
        strcat(log_message, file_name);
        strcat(log_message, "\": ");
        strcat(log_message, strerror(errno));
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        return;
    }
    
    // Try to get an exclusive lock
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        strcpy(log_message, "Error: Cannot write to \"");
        strcat(log_message, file_name);
        strcat(log_message, "\". File is locked or read-only.");
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        close(fd);
        return;
    }
    
    // Write content
    if (write(fd, content, strlen(content)) == -1) {
        strcpy(log_message, "Error writing to file \"");
        strcat(log_message, file_name);
        strcat(log_message, "\": ");
        strcat(log_message, strerror(errno));
        write_message(log_message);
        write_message("\n");
        log_operation(log_message);
        flock(fd, LOCK_UN);  // Release the lock
        close(fd);
        return;
    }
    
    // Add a newline if needed
    if (content[strlen(content) - 1] != '\n') {
        write(fd, "\n", 1);
    }
    
    flock(fd, LOCK_UN);  // Release the lock
    close(fd);
    
    strcpy(log_message, "Successfully appended to file \"");
    strcat(log_message, file_name);
    strcat(log_message, "\".");
    write_message(log_message);
    write_message("\n");
    log_operation(log_message);
}

// Function to delete a file
void delete_file(const char *file_name) {
    pid_t pid = fork();
    
    if (pid < 0) {
        write_message("Fork failed\n");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {  // Child process
        char log_message[MAX_BUFFER];
        struct stat st = {0};
        
        // Check if file exists
        if (stat(file_name, &st) == -1) {
            strcpy(log_message, "Error: File \"");
            strcat(log_message, file_name);
            strcat(log_message, "\" not found.");
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_FAILURE);
        }
        
        // Delete file
        if (unlink(file_name) == 0) {
            strcpy(log_message, "File \"");
            strcat(log_message, file_name);
            strcat(log_message, "\" deleted successfully.");
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_SUCCESS);
        } else {
            strcpy(log_message, "Error deleting file \"");
            strcat(log_message, file_name);
            strcat(log_message, "\": ");
            strcat(log_message, strerror(errno));
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_FAILURE);
        }
    } else {  // Parent process
        int status;
        waitpid(pid, &status, 0);  // Wait for child process to complete
    }
}

// Function to delete a directory
void delete_directory(const char *dir_name) {
    pid_t pid = fork();
    
    if (pid < 0) {
        write_message("Fork failed\n");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {  // Child process
        char log_message[MAX_BUFFER];
        DIR *dir;
        struct dirent *entry;
        int is_empty = 1;
        
        dir = opendir(dir_name);
        if (dir == NULL) {
            strcpy(log_message, "Error: Directory \"");
            strcat(log_message, dir_name);
            strcat(log_message, "\" not found.");
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_FAILURE);
        }
        
        // Check if directory is empty
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                is_empty = 0;
                break;
            }
        }
        
        closedir(dir);
        
        if (!is_empty) {
            strcpy(log_message, "Error: Directory \"");
            strcat(log_message, dir_name);
            strcat(log_message, "\" is not empty.");
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_FAILURE);
        }
        
        // Delete the empty directory
        if (rmdir(dir_name) == 0) {
            strcpy(log_message, "Directory \"");
            strcat(log_message, dir_name);
            strcat(log_message, "\" deleted successfully.");
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_SUCCESS);
        } else {
            strcpy(log_message, "Error deleting directory \"");
            strcat(log_message, dir_name);
            strcat(log_message, "\": ");
            strcat(log_message, strerror(errno));
            write_message(log_message);
            write_message("\n");
            log_operation(log_message);
            exit(EXIT_FAILURE);
        }
    } else {  // Parent process
        int status;
        waitpid(pid, &status, 0);  // Wait for child process to complete
    }
}

// Function to show logs
void show_logs() {
    char log_message[MAX_BUFFER];
    struct stat st;
    
    // Check if log file exists
    if (stat(LOG_FILE, &st) == -1) {
        write_message("No logs found. Log file does not exist yet.\n");
        return;
    }
    
    int fd = open(LOG_FILE, O_RDONLY);
    if (fd == -1) {
        write_message("Error opening log file: ");
        write_message(strerror(errno));
        write_message("\n");
        return;
    }
    
    write_message("Operation Logs:\n");
    char buffer[MAX_BUFFER];
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the buffer
        write_message(buffer);
    }
    
    // Add a newline if the file doesn't end with one
    if (bytes_read > 0 && buffer[bytes_read - 1] != '\n') {
        write_message("\n");
    }
    
    close(fd);
    strcpy(log_message, "Displayed operation logs.");
    log_operation(log_message);
}

// Function to display help
void display_help() {
    write_message("Usage: fileManager <command> [arguments]\n");
    write_message("Commands:\n");
    write_message("  createDir \"folderName\"                      - Create a new directory\n");
    write_message("  createFile \"fileName\"                       - Create a new file\n");
    write_message("  listDir \"folderName\"                        - List all files in a directory\n");
    write_message("  listFilesByExtension \"folderName\" \".txt\"    - List files with specific extension\n");
    write_message("  readFile \"fileName\"                         - Read a file's content\n");
    write_message("  appendToFile \"fileName\" \"new content\"       - Append content to a file\n");
    write_message("  deleteFile \"fileName\"                       - Delete a file\n");
    write_message("  deleteDir \"folderName\"                      - Delete an empty directory\n");
    write_message("  showLogs                                    - Display operation logs\n");
}

int main(int argc, char *argv[]) {
    // If no arguments provided, display help
    if (argc == 1) {
        display_help();
        return 0;
    }
    
    // Check command
    if (strcmp(argv[1], "createDir") == 0) {
        if (argc != 3) {
            write_message("Error: createDir requires one argument.\n");
            return 1;
        }
        create_directory(argv[2]);
    }
    else if (strcmp(argv[1], "createFile") == 0) {
        if (argc != 3) {
            write_message("Error: createFile requires one argument.\n");
            return 1;
        }
        create_file(argv[2]);
    }
    else if (strcmp(argv[1], "listDir") == 0) {
        if (argc != 3) {
            write_message("Error: listDir requires one argument.\n");
            return 1;
        }
        list_directory(argv[2]);
    }
    else if (strcmp(argv[1], "listFilesByExtension") == 0) {
        if (argc != 4) {
            write_message("Error: listFilesByExtension requires two arguments.\n");
            return 1;
        }
        list_files_by_extension(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "readFile") == 0) {
        if (argc != 3) {
            write_message("Error: readFile requires one argument.\n");
            return 1;
        }
        read_file(argv[2]);
    }
    else if (strcmp(argv[1], "appendToFile") == 0) {
        if (argc != 4) {
            write_message("Error: appendToFile requires two arguments.\n");
            return 1;
        }
        append_to_file(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "deleteFile") == 0) {
        if (argc != 3) {
            write_message("Error: deleteFile requires one argument.\n");
            return 1;
        }
        delete_file(argv[2]);
    }
    else if (strcmp(argv[1], "deleteDir") == 0) {
        if (argc != 3) {
            write_message("Error: deleteDir requires one argument.\n");
            return 1;
        }
        delete_directory(argv[2]);
    }
    else if (strcmp(argv[1], "showLogs") == 0) {
        show_logs();
    }
    else {
        write_message("Unknown command: ");
        write_message(argv[1]);
        write_message("\n");
        display_help();
        return 1;
    }
    
    return 0;
}
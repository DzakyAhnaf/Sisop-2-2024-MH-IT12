#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <dirent.h>

#define MAX_PATH_LENGTH 256
#define MAX_BUFFER_SIZE 1024
#define LOG_FILE "virus.log"

// Function to replace a substring with another substring in a string
void replaceSubstring(char *str, const char *oldWord, const char *newWord) {
    char *pos, temp[MAX_BUFFER_SIZE];
    int index = 0;
    int oldWordLength = strlen(oldWord);
    int newWordLength = strlen(newWord);

    while ((pos = strstr(str, oldWord)) != NULL) {
        strcpy(temp, str);
        index = pos - str;
        str[index] = '\0';
        strcat(str, newWord);
        strcat(str, temp + index + oldWordLength);
    }
}

// Function to write log entry to virus.log
void writeLogEntry(const char *fileName) {
    FILE *file = fopen(LOG_FILE, "a");
    if (file != NULL) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[20];
        strftime(timestamp, 20, "%d-%m-%Y %H:%M:%S", tm_info);
        fprintf(file, "[%s] Suspicious string at %s successfully replaced!\n", timestamp, fileName);
        fclose(file);
    }
}

// Function to replace suspicious strings in a file and write log entry
void processFile(const char *filePath) {
    char buffer[MAX_BUFFER_SIZE];
    FILE *file = fopen(filePath, "r+");
    if (file == NULL) {
        syslog(LOG_ERR, "Error opening file: %s", filePath);
        return;
    }

    char *suspiciousStrings[] = {"m4LwAr3", "5pYw4R3", "R4nS0mWaR3"};
    char *replacementStrings[] = {"[MALWARE]", "[SPYWARE]", "[RANSOMWARE]"};

    while (fgets(buffer, sizeof(buffer), file)) {
        for (int i = 0; i < sizeof(suspiciousStrings) / sizeof(suspiciousStrings[0]); ++i) {
            replaceSubstring(buffer, suspiciousStrings[i], replacementStrings[i]);
            if (strstr(buffer, replacementStrings[i]) != NULL) {
                writeLogEntry(filePath);
            }
        }
        fseek(file, -strlen(buffer), SEEK_CUR);
        fputs(buffer, file);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <folder_path>\n", argv[0]);
        return 1;
    }

    // Daemonize the process
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid > 0) {
        // Parent process, exit
        return 0;
    }

    // Change the file mode mask
    umask(0);

    // Create a new session
    if (setsid() < 0) {
        perror("Setsid failed");
        return 1;
    }

    // Change the working directory to specified folder
    if (chdir(argv[1]) < 0) {
        perror("Chdir failed");
        return 1;
    }

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Open syslog for logging
    openlog(argv[0], LOG_PID, LOG_DAEMON);

    // Main daemon loop
    while (1) {
        // Process files in the directory
        struct dirent *dp;
        DIR *dir = opendir(".");
        if (dir != NULL) {
            while ((dp = readdir(dir)) != NULL) {
                if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                    processFile(dp->d_name);
                }
            }
            closedir(dir);
        } else {
            syslog(LOG_ERR, "Error opening directory: %s", argv[1]);
        }

        // Sleep for 15 seconds
        sleep(15);
    }

    // Close syslog
    closelog();

    return 0;
}

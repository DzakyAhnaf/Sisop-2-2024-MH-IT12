#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_APPS 10
#define MAX_NAME_LENGTH 50
#define MAX_FILE_LINE_LENGTH 100

struct App {
    char name[MAX_NAME_LENGTH];
    int numWindows;
    pid_t pid[MAX_APPS]; 
    int numPid; // Jumlah proses
};

void saveDataToFile(const struct App apps[], int numApps);
void openApps(struct App apps[], int numApps);
void saveDataFToFile(const struct App apps[], int numApps);
void openAppsFromFile(const char *filename, struct App apps[], int *numApps);
void readOData(struct App apps[], int *numApps);
void readconfdata(struct App apps[], int *numApps);
void killApps(struct App apps[], int numApps);
void killAllpid(const char* filename);
void mergepid(const char* srcFilename1, const char* srcFilename2, const char* destFilename);

int main(int argc, char *argv[]) {
    struct App apps[MAX_APPS];
    int numApps = 0;

    if (argc == 1) {
        printf("Usage: %s -o <app1> <num1> <app2> <num2> ... OR %s -f <filename> OR %s -k OR %s -k <filename>\n", argv[0], argv[0], argv[0], argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-o") == 0) {
        if (argc < 4 || argc % 2 != 0) {
            printf("Invalid number of arguments\n");
            return 1;
        }
        numApps = (argc - 2) / 2;
        for (int i = 0; i < numApps; i++) {
            strcpy(apps[i].name, argv[i * 2 + 2]);
            apps[i].numWindows = atoi(argv[i * 2 + 3]);
            apps[i].numPid = 0;
        }
        openApps(apps, numApps);
        saveDataToFile(apps, numApps);
    } else if (strcmp(argv[1], "-f") == 0) {
        if (argc != 3) {
            printf("Invalid number of arguments\n");
            return 1;
        }
        FILE *file = fopen("app_data.txt", "r");
        if(file != NULL) {
            openAppsFromFile(argv[2],apps, &numApps);
            saveDataFToFile(apps, numApps);
            mergepid("app_data.txt", "app_dataf.txt", "combine.txt");
        }
        else {
            openAppsFromFile(argv[2],apps, &numApps);
            saveDataFToFile(apps, numApps);
        }
        fclose(file);
    } else if (strcmp(argv[1], "-k") == 0) {
        if (argc == 2) { // ./setup -k
            FILE *file = fopen("app_data.txt", "r");
            FILE *file1 = fopen("app_dataf.txt", "r");
            FILE *file2 = fopen("combine.txt", "r");

            if (file == NULL && file1 == NULL) { // data -o ga ada dan data -f ga ada
                printf("Failed to open file\n");
                    return 1;
            } 

            else if (file != NULL && file1 == NULL) { // data -o ada data -f ga ada
                readOData(apps, &numApps);
                killApps(apps, numApps);
                fclose(file);
                remove("app_data.txt");
            }

            else if (file1 != NULL && file == NULL) { // data -f ada data -o ga ada
                readconfdata(apps, &numApps);
                killApps(apps, numApps);
                fclose(file1);
                remove("app_dataf.txt");
            }

            else { // data -f ada data -o ga ada
                killAllpid("combine.txt");
                fclose(file);
                fclose(file1);
                fclose(file2);
                remove("app_data.txt");
                remove("app_dataf.txt");
                remove("combine.txt");
            }

        } else if (argc == 3) { // ./setup -k file.conf
                readconfdata(apps, &numApps); 
                killApps(apps, numApps);
                remove("app_dataf.txt");
        } else {
            printf("Invalid option\n");
            return 1;
        }
    }
    return 0;
}

// data -o
void saveDataToFile(const struct App apps[], int numApps) {
    FILE *file = fopen("app_data.txt", "w");
    if (file == NULL) {
        printf("Failed to open file\n");
        return;
    }
    fprintf(file, "%d\n", numApps);
    for (int i = 0; i < numApps; i++) {
        for (int j = 0; j < apps[i].numPid; j++) {
            fprintf(file, "%d\n", apps[i].pid[j]);
        }
    }
    fclose(file);
}

void openApps(struct App apps[], int numApps) {
    for (int i = 0; i < numApps; i++) {
        printf("Opening %d windows of %s\n", apps[i].numWindows, apps[i].name);
        apps[i].numPid = 0;
        for (int j = 0; j < apps[i].numWindows; j++) {
            pid_t pid = fork();
            if (pid == 0) {
                execlp("open", "open", "-n", "-a", apps[i].name, NULL);
                perror("execlp");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                int status;
                pid_t childPid = waitpid(pid, &status, 0);
                if (childPid > 0) {
                    apps[i].pid[apps[i].numPid++] = childPid;
                    printf("PID = %d\n", childPid);
                } else {
                    perror("waitpid");
                }
            } else {
                perror("fork");
            }
        }
    }
}

// data -f
void saveDataFToFile(const struct App apps[], int numApps) {
    FILE *file = fopen("app_dataf.txt", "w");
    if (file == NULL) {
        printf("Failed to open file\n");
        return;
    }
    fprintf(file, "%d\n", numApps);
    for (int i = 0; i < numApps; i++) {
        for (int j = 0; j < apps[i].numPid; j++) {
            fprintf(file, "%d\n", apps[i].pid[j]);
        }
    }
    fclose(file);
}

// buka app dari file.conf
void openAppsFromFile(const char *filename, struct App apps[], int *numApps) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open file\n");
        return;
    }

    char line[MAX_FILE_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        char *appName = strtok(line, " ");
        char *numWindowsStr = strtok(NULL, " ");

        if (appName != NULL && numWindowsStr != NULL) {
            strcpy(apps[*numApps].name, appName);
            apps[*numApps].numWindows = atoi(numWindowsStr);
            apps[*numApps].numPid = 0;
            (*numApps)++;
        }
    }

    fclose(file);

    openApps(apps, *numApps);
    saveDataFToFile(apps, *numApps); // simpan data ke file app_dataf.txt
}

// baca data -o
void readOData(struct App apps[], int *numApps) {
    FILE *file = fopen("app_data.txt", "r");
    if (file == NULL) {
        printf("Failed to open file\n");
        return;
    }
    fscanf(file, "%d", numApps);
    for (int i = 0; i < *numApps; i++) {
        apps[i].numPid = 0;
        fscanf(file, "%d", &apps[i].numPid);
        for (int j = 0; j < apps[i].numPid; j++) {
            fscanf(file, "%d", &apps[i].pid[j]);
        }
    }
    fclose(file);
}

// baca data -f
void readconfdata(struct App apps[], int *numApps) {
    FILE *file = fopen("app_dataf.txt", "r");
    if (file == NULL) {
        printf("Failed to open file\n");
        return;
    }
    fscanf(file, "%d", numApps);
    for (int i = 0; i < *numApps; i++) {
        apps[i].numPid = 0;
        fscanf(file, "%d", &apps[i].numPid);
        for (int j = 0; j < apps[i].numPid; j++) {
            fscanf(file, "%d", &apps[i].pid[j]);
        }
    }
    fclose(file);
}

void killApps(struct App apps[], int numApps) {
    for (int i = 0; i < numApps; i++) {
        for (int j = 0; j < apps[i].numPid; j++) {
            printf("Killing process %d...\n", apps[i].pid[j]);
            int result = kill(apps[i].pid[j]+1, SIGTERM);
            if (result == 0) {
                printf("Process %d terminated successfully\n", apps[i].pid[j]);
            } else {
                printf("Failed to terminate process %d\n", apps[i].pid[j]);
            }
        }
    }
}

void killAllpid(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file to read PIDs");
        return;
    }

    int pid;
    while (fscanf(file, "%d", &pid) == 1) {
        if (pid > 0) {
            printf("Killing process %d...\n", pid);
            int result = kill(pid + 1, SIGTERM);
            if (result == 0) {
                printf("Process %d terminated successfully.\n", pid);
            } else {
                perror("Failed to terminate process");
            }
        }
    }
    fclose(file);
}

// gabungin pid dari 2 file.txt -o dan -f
void mergepid(const char* srcFilename1, const char* srcFilename2, const char* destFilename) {
    FILE *src1 = fopen(srcFilename1, "r");
    FILE *src2 = fopen(srcFilename2, "r");
    if (!src1 || !src2) {
        perror("Failed to open source files");
        if (src1) fclose(src1);
        if (src2) fclose(src2);
        return;
    }

    FILE *dest = fopen(destFilename, "w"); // Open for writing to reset file content
    if (!dest) {
        perror("Failed to open destination file");
        fclose(src1);
        fclose(src2);
        return;
    }

    int pid, numApps;
    // Skip the numApps line and read PIDs from the first file
    fscanf(src1, "%d", &numApps);  // Skip the number of apps
    while (fscanf(src1, "%d", &pid) == 1) {
        fprintf(dest, "%d\n", pid);
    }

    // Skip the numApps line and read PIDs from the second file
    fscanf(src2, "%d", &numApps);  // Skip the number of apps
    while (fscanf(src2, "%d", &pid) == 1) {
        fprintf(dest, "%d\n", pid);
    }

    fclose(src1);
    fclose(src2);
    fclose(dest);
}


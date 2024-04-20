#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>

char mode[10] = "default";

void run_as_daemon() {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void download_and_extract() {
    pid_t pid;
    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);

        execlp("wget", "wget", "-qO-", "https://drive.google.com/uc?export=download&id=1rUIZmp10lXLtCIH3LAZJzRPeRks3Crup", "|", "tar", "xvz", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        close(fd[1]);
        char buffer[1024];
        ssize_t n;

        while ((n = read(fd[0], buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, n);
        }

        close(fd[0]);
        waitpid(pid, NULL, 0);
    }
}

void dekripsi_rot19(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] >= 'a' && str[i] <= 's') {
            str[i] = str[i] + 19;
        } else if (str[i] >= 't' && str[i] <= 'z') {
            str[i] = str[i] - 7;
        }
    }
}

void rename_file(char *filename) {
    char new_filename[256];
    char *ext = strrchr(filename, '.');

    if (ext != NULL) {
        if (strcmp(ext, ".ts") == 0) {
            strcpy(new_filename, "helper.ts");
        } else if (strcmp(ext, ".py") == 0) {
            strcpy(new_filename, "calculator.py");
        } else if (strcmp(ext, ".go") == 0) {
            strcpy(new_filename, "server.go");
        } else {
            strcpy(new_filename, "renamed.file");
        }
    } else {
        strcpy(new_filename, "renamed.file");
    }

    rename(filename, new_filename);
}

void backup_file(char *filename) {
    char backup_path[256];
    sprintf(backup_path, "%s/%s", "library/backup", filename);
    rename(filename, backup_path);
}

void restore_file(char *filename) {
    char backup_path[256];
    sprintf(backup_path, "%s/%s", "library/backup", filename);
    rename(backup_path, filename);
}

void handle_signal(int sig) {
    if (sig == SIGRTMIN) {
        strcpy(mode, "default");
    } else if (sig == SIGUSR1) {
        strcpy(mode, "backup");
    } else if (sig == SIGUSR2) {
        strcpy(mode, "restore");
    }
}

int main(int argc, char *argv[]) {
    run_as_daemon();
    download_and_extract();
    
    signal(SIGRTMIN, handle_signal);
    signal(SIGUSR1, handle_signal);
    signal(SIGUSR2, handle_signal);

    if (argc > 1) {
        if (strcmp(argv[1], "-m") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Usage: %s -m <mode>\n", argv[0]);
                return 1;
            }
            strcpy(mode, argv[2]);
        } else {
            fprintf(stderr, "Invalid argument: %s\n", argv[1]);
            return 1;
        }
    }

    DIR *dir;
    struct dirent *entry;
    int file_count = 0;

    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        file_count++;
        if (file_count > 6) {
            char filename[256];
            strcpy(filename, entry->d_name);
            dekripsi_rot19(filename);

            if (strcmp(mode, "backup") == 0) {
                if (strstr(filename, "m0V3") != NULL) {
                    backup_file(entry->d_name);
                }
            } else if (strcmp(mode, "restore") == 0) {
                if (strstr(filename, "m0V3") != NULL) {
                    restore_file(entry->d_name);
                }
            } else {
                if (strstr(filename, "d3Let3") != NULL) {
                    remove(entry->d_name);
                } else if (strstr(filename, "r3N4mE") != NULL) {
                    rename_file(filename);
                }

                printf("%s\n", filename);
            }
        }
    }

    closedir(dir);

    return 0;
}

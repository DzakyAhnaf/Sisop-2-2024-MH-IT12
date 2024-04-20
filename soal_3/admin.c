#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>


//biar konsisten tidak dapat diinterupsi
volatile sig_atomic_t current_status = 0;  // 0 JALAN 1 GAGAL

void start_daemon();
void monitoring(const char *user);
void log_activity(const char* user, const char* pid, const char* process_name, const char* status);
void save_pid();
void stop_daemon(const char* user);
void update_status(const char *status_file);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s [-m|-s|-c|-a] <user>\n", argv[0]);
        return 1;
    }

    const char* user = argv[2];
    const char* status_file = "/home/vagrant/status_file"; 

    if (strcmp(argv[1], "-m") == 0) {
        start_daemon();
        save_pid();
        monitoring(user);
    } else if (strcmp(argv[1], "-s") == 0) {
        stop_daemon(user);
    } else if (strcmp(argv[1], "-c") == 0) {
        FILE* file = fopen(status_file, "w");
        if (file) {
            fprintf(file, "1"); // GAGAL
            fclose(file);
        }
    } else if (strcmp(argv[1], "-a") == 0) {
        FILE* file = fopen(status_file, "w");
        if (file) {
            fprintf(file, "0"); // JALAN
            fclose(file);
        }
    }

    closelog();
    return 0;
}

void start_daemon() {
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    if (chdir("/home/vagrant/shift2") < 0) { 
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void monitoring(const char *user) {
    const char* status_file = "/home/vagrant/status_file"; 
    while (1) {
        update_status(status_file);  // Update current status dari file
        char command[256];
        sprintf(command, "ps -u %s -o pid,comm", user);
        FILE *fp = popen(command, "r");
        if (!fp) {
            syslog(LOG_ERR, "Failed to run command.\n");
            exit(1);
        }

        char buffer[1024];
        fgets(buffer, sizeof(buffer), fp);  
        char *status = current_status ? "GAGAL" : "JALAN";
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            char *pid = strtok(buffer, " ");
            char *process_name = strtok(NULL, "\n");
            if (pid && process_name) {
                log_activity(user, pid, process_name, status);
            }
        }

        pclose(fp);
        sleep(5);
    }
}

void update_status(const char *status_file) {
    FILE* f = fopen(status_file, "r"); // baca status_file
    if (f) {
        fscanf(f, "%d", &current_status);  // nilai current_status == nilai dalem file status_file
        fclose(f);
    }
}

void log_activity(const char* user, const char* pid, const char* process_name, const char* status) {
    char filename[100];
    sprintf(filename, "%s.log", user);
    FILE* log_file = fopen(filename, "a");
    if (log_file == NULL) {
        syslog(LOG_ERR, "Failed to open log file.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *tm_struct = localtime(&now);
    fprintf(log_file, "[%02d:%02d:%04d]-[%02d:%02d:%02d]-%s-%s-%s\n",
            tm_struct->tm_mday, tm_struct->tm_mon + 1, tm_struct->tm_year + 1900,
            tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec,
            pid, process_name, status);
    fclose(log_file);
}

void save_pid() {
    FILE* file = fopen("admin.pid", "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Failed to write PID file.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%d", getpid());
    fclose(file);
}

void stop_daemon(const char* user) {
    char filename[50];
    sprintf(filename, "%s.log", user); 
    FILE* file = fopen("admin.pid", "r");
    if (file == NULL) {
        syslog(LOG_ERR, "Failed to read PID file.\n");
        return;
    }
    int pid;
    fscanf(file, "%d", &pid);
    fclose(file);
    if (pid > 0) {
        kill(pid, SIGTERM);
        remove("admin.pid");  // hapus file pid
        remove(filename);  // hapus file log
    }
}

# Laporan Resmi Praktikum Sistem Operasi 2024 Modul-2

---

### Anggota Kelompok IT 12 :

- Muhammad Dzaky Ahnaf (5027231039)
- Adlya Isriena Aftarisya (5027231066)
- Nisrina Atiqah Dwiputri Ridzki (5027231075)

## Soal 1

Dikerjakan oleh Muhammad Dzaky Ahnaf (5027231039)

---

### Deskripsi Soal

---

a. Program dapat menerima input path berupa ‘argv’ untuk mengatur folder dimana file akan dieksekusi

b. Program tersebut berfungsi untuk mereplace string dengan ketentuan sebagai berikut:
1. String m4LwAr3 direplace dengan string [MALWARE]
2. String 5pYw4R3 direplace dengan string [SPYWARE]
3. String R4nS0mWaR3 direplace dengan string [RANSOMWARE]

c. Program harus berjalan secara daemon, dan tidak diperbolehkan menggunakan command system() dalam pembuatan program

d. Program akan secara terus menerus berjalan di background dengan jeda 15 detik

e. Catat setiap penghapusan string yang dilakukan oleh program pada sebuah file bernama virus.log dengan format: [dd-mm-YYYY][HH:MM:SS] Suspicious string at <file_name> successfully replaced!

Contoh penggunaan: ./virus /home/user/virus

Contoh isi file sebelum program dijalankan:

![Selection_001](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/110287409/fd3aca21-4946-424d-93ab-7533808941cf)

Contoh isi file setelah setelah program dijalankan:

![Selection_002](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/110287409/814a0370-3036-4603-b289-fbe176756377)

---

### Kode Penyelesaian

---

```c
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

```


## Soal 2

Dikerjakan oleh Nisrina Atiqah Dwiputri Ridzki (5027231075)

---

### Deskripsi Soal

---

Paul adalah seorang mahasiswa semester 4 yang diterima magang di perusahaan XYZ. Pada hari pertama magang, ia diberi tugas oleh atasannya untuk membuat program manajemen file sederhana. Karena kurang terbiasa dengan bahasa C dan environment Linux, ia meminta bantuan kalian untuk mengembangkan program tersebut.

a. Atasannya meminta agar program tersebut dapat berjalan secara daemon dan dapat mengunduh serta melakukan unzip terhadap file berikut. Atasannya juga meminta program ini dibuat tanpa menggunakan command system()

b. Setelah ditelusuri, ternyata hanya 6 file teratas yang nama filenya tidak dienkripsi. Oleh karena itu, bantulah Paul untuk melakukan dekripsi terhadap nama file ke-7 hingga terakhir menggunakan algoritma ROT19

c. Setelah dekripsi selesai, akan terlihat bahwa setiap file memuat salah satu dari kode berikut: r3N4mE, d3Let3, dan m0V3. Untuk setiap file dengan nama yang memuat kode d3Let3, hapus file tersebut. Sementara itu, untuk setiap file dengan nama yang memuat kode r3N4mE, lakukan hal berikut:
    - Jika ekstensi file adalah “.ts”, rename filenya menjadi “helper.ts”
    - Jika ekstensi file adalah “.py”, rename filenya menjadi “calculator.py”
    - Jika ekstensi file adalah “.go”, rename filenya menjadi “server.go”
    - Jika file tidak memuat salah satu dari ekstensi diatas, rename filenya menjadi “renamed.file”

d. Atasan Paul juga meminta agar program ini dapat membackup dan merestore file. Oleh karena itu, bantulah Paul untuk membuat program ini menjadi 3 mode, yaitu:
    - default: program berjalan seperti biasa untuk me-rename dan menghapus file. Mode ini dieksekusi ketika program dijalankan tanpa argumen tambahan, yaitu dengan command ./management saja
    - backup: program memindahkan file dengan kode m0V3 ke sebuah folder bernama “backup”
    - restore: program mengembalikan file dengan kode m0V3 ke folder sebelum file tersebut dipindahkan
    - Contoh penggunaan: ./management -m backup

e. Terkadang, Paul perlu mengganti mode dari program ini tanpa menghentikannya terlebih dahulu. Oleh karena itu, bantulan Paul untuk mengintegrasikan kemampuan untuk mengganti mode ini dengan mengirim sinyal ke daemon, dengan ketentuan:
    - SIGRTMIN untuk mode default
    - SIGUSR1 untuk mode backup
    - SIGUSR2 untuk mode restore
    - Contoh penggunaan: kill -SIGUSR2 <pid_program>

f. Program yang telah dibuat ini tidak mungkin akan dijalankan secara terus-menerus karena akan membebani sistem. Maka dari itu, bantulah Paul untuk membuat program ini dapat dimatikan dengan aman dan efisien

g. Terakhir, program ini harus berjalan setiap detik dan mampu mencatat setiap peristiwa yang terjadi ke dalam file .log yang bernama “history.log” dengan ketentuan:
    - Format: [nama_user][HH:MM:SS] - <nama_file> - <action>
    nama_user adalah username yang melakukan action terhadap file
    - Format action untuk setiap kode:
        kode r3N4mE: Successfully renamed.
        kode d3Let3: Successfully deleted.
        mode backup: Successfully moved to backup.
        mode restore: Successfully restored from backup.
    - Contoh pesan log:
        - [paul][00:00:00] - r3N4mE.ts - Successfully renamed.
        - [paul][00:00:00] - m0V3.xk1 - Successfully restored from backup.

h. Berikut adalah struktur folder untuk pengerjaan nomor 2 :

    soal_2/
        ├── history.log
        ├── management.c
        └── library/
             └── backup/

---

### Kode Penyelesaian


---

```c
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

void downloadFile(const char *url, const char *output_path) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error forking");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execlp("wget", "wget", "--content-disposition","-O", output_path, url, NULL);
        perror("Error running wget");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Download success!.\n");
        } else {
            printf("Download fail...\n");
            exit(EXIT_FAILURE);
        }
    }
}

void unzipFile(const char *zip_file, const char *output_dir) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error forking");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execlp("unzip", "unzip", "-o", zip_file, "-d", output_dir, NULL);
        perror("Can't unzip");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Extraction success!\n");
        } else {
            printf("Extraction fail...\n");
            exit(EXIT_FAILURE);
        }
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
    const char *download_url = "https://drive.google.com/uc?export=download&id=1rUIZmp10lXLtCIH3LAZJzRPeRks3Crup";
    const char *zip_file = "/home/ubuntu/sisop2.2/library.zip";
    const char *output_dir = "/home/ubuntu/sisop2.2/";

    downloadFile(download_url, zip_file);
    unzipFile(zip_file, output_dir);    
    run_as_daemon();

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
        if (file_count < 6) {
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

```

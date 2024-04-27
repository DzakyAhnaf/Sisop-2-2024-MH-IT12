# Laporan Resmi Praktikum Sistem Operasi 2024 Modul-2
### Anggota Kelompok IT 12 :

- Muhammad Dzaky Ahnaf (5027231039)
- Adlya Isriena Aftarisya (5027231066)
- Nisrina Atiqah Dwiputri Ridzki (5027231075)

## Soal 1

Dikerjakan oleh Muhammad Dzaky Ahnaf (5027231039)
### Deskripsi Soal
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

### Kode Penyelesaian

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
---

### Penjelesan

---

1. Fungsi ```replaceSubstring```:
```c
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
```
Fungsi ```replaceSubstring``` digunakan untuk mengganti sebuah substring (```oldWord```) dengan substring baru (```newWord```) di dalam sebuah string (```str```). Fungsi ini bekerja dengan mencari kemunculan ```oldWord``` dalam ```str``` menggunakan ```strstr```. Jika ditemukan, fungsi akan membagi str menjadi dua bagian: bagian sebelum oldWord dan bagian setelah ```oldWord```. Kemudian, bagian sebelum ```oldWord``` digabungkan dengan ```newWord``` dan bagian setelah ```oldWord``` untuk membentuk string baru. Proses ini diulang menggunakan ```while``` loop sampai tidak ada lagi kemunculan ```oldWord``` dalam ```str```.

2. Fungsi ```writeLogEntry```:
```c
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
```
Fungsi ```writeLogEntry``` digunakan untuk menulis entri log ke dalam file ```virus.log```. Fungsi ini membuka file ```virus.log``` dalam mode "append" (```"a"```). Kemudian, fungsi akan mendapatkan waktu saat ini menggunakan ```time``` dan ```localtime```, dan memformat waktu tersebut ke dalam string ```timestamp``` dengan format ```"dd-mm-YYYY HH:MM:SS"```. Selanjutnya, ```fprintf()``` akan menulis entri log ke dalam file ```virus.log``` dengan format ```[timestamp] Suspicious string at <fileName> successfully replaced!```. Terakhir, ```fclose()``` akan menututup ```virus.log```.

3. Fungsi ```processFile```:
```c
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
```

Fungsi ```processFile``` digunakan untuk memproses sebuah file dengan mengganti string-string tertentu (```"m4LwAr3"```, ```"5pYw4R3"```, dan ```"R4nS0mWaR3"```) dengan string-string pengganti (```"[MALWARE]"```, ```"[SPYWARE]"```, dan ```"[RANSOMWARE]"```). Fungsi ini membuka file yang diberikan dalam mode read/write (```"r+"```). Kemudian, fungsi akan membaca setiap baris dari file menggunakan ```fgets```. Untuk setiap baris, fungsi akan memanggil ```replaceSubstring``` untuk mengganti string-string tersebut. Jika terjadi penggantian, fungsi ```writeLogEntry``` akan dipanggil untuk menulis entri log ke dalam file ```virus.log```. Setelah itu, baris yang telah diproses akan ditulis kembali ke dalam file menggunakan ```fputs```. Terakhir, file akan ditutup.

4. Fungsi ```main```:
```c
int main(int argc, char *argv[]) {
    // ...
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
Fungsi ```main``` adalah fungsi utama dari program ini. Fungsi ini menerima dua argumen: ```argc``` (jumlah argumen) dan ```argv```(array yang berisi argumen-argumen tersebut). Jika jumlah argumen tidak sesuai dengan yang diharapkan (hanya satu argumen selain nama program, yaitu ```<folder_path>```), program akan menampilkan cara penggunaan dan keluar.
Selanjutnya, program akan melakukan proses daemonisasi dengan melakukan ```fork``` dua kali. Proses anak akan menjadi proses daemon, sedangkan proses induk akan keluar. Proses daemon akan mengubah mode file menggunakan ```umask(0)```, membuat sesi baru menggunakan ```setsid()```, dan mengubah direktori kerja ke direktori yang diberikan (```argv[1]```).
Setelah itu, program akan menutup file deskriptor standar (```stdin```, ```stdout```, ```stderr```) dan membuka ```syslog``` untuk logging menggunakan ```openlog```.

Program ini akan terus berjalan dalam loop yang tidak terbatas (```while (1)```). Di dalam loop ini, program akan memproses semua file dalam direktori saat ini (```"."```) dengan menggunakan fungsi ```opendir``` dan ```readdir```. Program akan melewatkan file atau direktori dengan nama khusus (```"."``` dan ```".."```) dan memproses file lainnya dengan memanggil fungsi ```processFile```.
Setelah memproses semua file dalam direktori, program akan tertidur selama 15 detik dengan memanggil fungsi ```sleep(15)```. Setelah 15 detik, program akan kembali memproses semua file dalam direktori yang sama.

Setelah keluar dari loop utama (yang seharusnya tidak pernah terjadi karena loop tersebut adalah loop yang tidak terbatas), program akan menutup ```syslog``` dengan memanggil fungsi ```closelog()``` dan kemudian keluar dengan mengembalikan nilai ```0```.
Secara umum, program ini akan berjalan sebagai daemon di latar belakang dan secara berkala (setiap 15 detik) memproses semua file dalam direktori yang diberikan, mengganti string-string tertentu, dan menulis entri log ke dalam file ```virus.log```. Program ini akan terus berjalan hingga dihentikan secara manual.

### Dokumentasi Hasil Program

---
- Hasil file txt setelah program dijalankan

![image](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/110287409/92666fd5-2ec8-4633-9860-6a2d2ed69110)

- Hasil file ```virus.log```

![image](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/110287409/7b64225c-51d8-46e1-837d-8c9fba62ed15)

## Soal 2

Dikerjakan oleh Nisrina Atiqah Dwiputri Ridzki (5027231075)

### Deskripsi Soal

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

### Kode Penyelesaian

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
---

### Penjelasan

---

1. ```char mode[10] = "default";``` :
2. Fungsi ```void run_as_daemon() {``` : untuk menjalankan program sebagai daemon
3. Fungsi ```void downloadFile(const char *url, const char *output_path) {``` : untuk mendownload file zip dari URL yang diberikan 
4. Fungsi ```void unzipFile(const char *zip_file, const char *output_dir) {``` : untuk mengunzip atau mengekstrak file zip ke direktori yang ditentukan
5. Fungsi ```void dekripsi_rot19(char *str) {``` : untuk melakukan dekripsi terhadap nama file ke-7 hingga terakhir menggunakan algoritma ROT19
6. Fungsi ```void rename_file(char *filename) {``` : untuk merename file yang sudah di unzip dengan nama yang memuat kode d3Let3, hapus file tersebut. Sementara itu, untuk setiap file dengan nama yang memuat kode r3N4mE.
7. Fungsi ```
void backup_file(char *filename) {``` : untuk membuat salinan file ke direktori backup.
8. Fungsi ```void restore_file(char *filename) {``` : untuk mengembalikan file dari direktori backup
10. Fungsi ```void handle_signal(int sig) {``` : menangani sinyal untuk mengatur mode kerja program.
11. Fungsi ```int main(int argc, char *argv[]) {``` :
    - Deklarasi dan inisialisasi variabel-variabel yang diperlukan, seperti download_url, zip_file, dan output_dir.
    - Panggilan fungsi downloadFile(download_url, zip_file): Untuk mengunduh file dari URL yang telah ditentukan dan menyimpannya di lokasi yang ditentukan.
    - Panggilan fungsi unzipFile(zip_file, output_dir): Untuk mengekstrak file ZIP yang telah diunduh ke direktori output yang telah ditentukan.
    - Panggilan fungsi run_as_daemon(): Untuk menjalankan program sebagai daemon, menjauhkan diri dari terminal utama.
    - Penanganan sinyal dengan fungsi signal(SIGRTMIN, handle_signal), signal(SIGUSR1, handle_signal), dan signal(SIGUSR2, handle_signal): Untuk menetapkan fungsi penangan sinyal yang sesuai saat sinyal tertentu diterima.
    - Pemeriksaan argumen baris perintah: Untuk menentukan mode kerja program berdasarkan argumen yang diberikan. Jika argumen adalah -m, maka mode akan diatur sesuai dengan argumen berikutnya. Jika argumen tidak valid, program akan menampilkan pesan kesalahan.
    - Pemindaian direktori saat ini: Untuk memproses file-file dalam direktori saat ini. Program akan membuka direktori, membaca setiap entri, dan memproses file dengan memanggil fungsi-fungsi tertentu berdasarkan mode kerja yang telah ditetapkan.
    - Penutupan direktori setelah selesai pemindaian.Pengembalian nilai 0 untuk menunjukkan bahwa program telah berakhir dengan sukses.

### Revisi

### Kendala

- Kode masih belum tertuntaskan sampai selesai
- Tidak bisa mem-backup file

### Dokumentasi Hasil Program 

---

- ![1000066834](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/151147728/070fdc3e-4704-4e5a-9bdf-394eafacd133)

- ![1000066833](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/151147728/b47bbb0d-4b8f-4519-98da-488d82932012)
- ![1000066830](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/151147728/19a444bc-363f-4825-b7a3-bc09aaee5540)
- ![1000066831](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/151147728/84d24d69-e705-472d-96a1-b74c8a634960)



## Soal 3

Dikerjakan oleh Adlya Isriena Aftarisya (5027231066)

### Deskripsi Soal

Pak Heze adalah seorang admin yang baik. Beliau ingin membuat sebuah program admin yang dapat memantau para pengguna sistemnya. Bantulah Pak Heze untuk membuat program  tersebut!

### Catatan

<img width="513" alt="image" src="https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/164857172/408a2610-738a-40ec-9353-3e2f81b9e77e">

- stdio.h: menyediakan fungsi-fungsi standar input-output
- stdlib.h: berisi fungsi-fungsi standar yang digunakan dalam pemrograman C
- unistd.h: berisi deklarasi untuk fungsi-fungsi sistem POSIX (Portable Operating System Interface) seperti **`fork`**, **`exec`**, **`pipe`**, dan fungsi-fungsi lainnya yang berkaitan dengan sistem.
- sys/types.h: menyediakan deklarasi untuk berbagai tipe data sistem seperti **`pid_t`**, **`size_t`**, **`off_t`**
- sys/stat.h: berisi deklarasi untuk fungsi-fungsi yang berkaitan dengan status file dan struktur data **`struct stat`**
- signal.h: berisi deklarasi untuk fungsi-fungsi yang berkaitan dengan penanganan sinyal (signal handling) seperti **`signal`**, **`kill`**, **`sigaction`**, dan tipe data **`sig_atomic_t`**
- syslog.h: berisi fungsi-fungsi yang digunakan untuk menulis pesan ke sistem log
- string.h: menyediakan fungsi-fungsi yang berkaitan dengan manipulasi string seperti **`strcpy`**, **`strcat`**, **`strlen`**, dan fungsi-fungsi lainnya.
- fcntl.h: Library ini berisi deklarasi untuk fungsi-fungsi yang berkaitan dengan kontrol file dan deskriptor file, seperti **`open`**
- time.h: Library ini berisi deklarasi untuk fungsi-fungsi yang berkaitan dengan manipulasi waktu dan tanggal, seperti **`time`**, **`localtime`**, **`strftime`**, dan tipe data **`struct tm`**.
- volatile sig_atomic_t current_status = 0 menginstruksikan kompiler untuk tidak melakukan optimisasi yang dapat menyebabkan pembacaan atau penulisan ulang variabel dilewati.

### Pengerjaan

```c
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s [-m|-s|-c|-a] <user>\n", argv[0]);
        return 1;
    }
		.....
```

pertama-tama, fungsi main. Disini, program akan memeriksa argumen baris perintah untuk menentukan tindakan yang harus dilakukan.

- jika `argc < 3` program akan mengeluarkan output yang menunjukkan arahan argc yang benar

```c
	const char* user = argv[2];
  int kill_process = 0;
  const char* status_file = "/Users/tarisa/smt-2/sisop/soal4/status_file"; 
```

- user: pointer ke string yang menyimpan nama user yang diberikan dari argc[2]
- kill_process: menentukan apakah proses-proses yang berjalan oleh user tersebut harus dihentikan atau tidak. Nilai 0 menunjukkan bahwa proses-proses tidak akan dihentikan, sementara nilai 1 menunjukkan bahwa proses-proses akan dihentikan.
- status_file: pointer ke string yang menyimpan path ke file yang digunakan untuk menyimpan status program. File ini digunakan untuk memperbarui status apakah program sedang berjalan atau gagal. Isi dari file ini akan dibaca dan diperbarui oleh fungsi **`update_status()`**.

```c
if (strcmp(argv[1], "-m") == 0) {
        start_daemon();
        save_pid();
        monitoring(user, kill_process);
    } else if (strcmp(argv[1], "-s") == 0) {
        stop_daemon(user);
    } else if (strcmp(argv[1], "-c") == 0) {
        FILE* file = fopen(status_file, "w");
        if (file) {
            fprintf(file, "1"); // GAGAL
            fclose(file);
        }
        kill_process = 1;
        start_daemon();
        save_pid();
        monitoring(user, kill_process);
    } else if (strcmp(argv[1], "-a") == 0) {
        FILE* file = fopen(status_file, "w");
        if (file) {
            fprintf(file, "0"); // JALAN
            fclose(file);
        }
        start_daemon();
        save_pid();
    }

    closelog();
    return 0;
}
```

- **-m (start monitoring)**
    - jika `argv[1]` adalah `-m`, maka program akan memulai monitoring
    - `start_daemon()`: agar program berjalan secara daemon
    - `save_pid()`: menyimpan pid daemon ke file [`admin.pid`](http://admin.pid) agar dapat dimatikan nantinya
    - `monitoring(user, kill_process);` untuk memulai proses monitoring user yang telah ditentukan dari argumen perintah
- **-s (stop monitoring)**
    - jika `argv[1]` adalah `-s`, maka program akan menghentikan monitoring
    - `stop_daemon(user);` menghentikan program daemon yang sedang berjalan
- **-c (monitoring dan killing process)**
    - jika `argv[1]` adalah `-c`, maka program akan memulai monitoring dan menghentikan proses yang sedang berjalan
    - File **`status_file`** dibuka untuk menulis, dan status program diatur menjadi "1" yang menandakan mode "GAGAL"
    - **`kill_process`** diatur menjadi **`1`** untuk menunjukkan bahwa proses harus dihentikan
    - **`start_daemon()`** dan **`save_pid()`** dipanggil seperti pada mode monitoring biasa.
    - **`monitoring(user, kill_process)`** memulai pemantauan proses dengan opsi untuk menghentikan proses yang berjalan
- **-a (stop command -c)**
    - jika `argv[1]` adalah `-a`, maka program akan memulai monitoring tanpa menghentikan proses yang sedang berjalan
    - File **`status_file`** dibuka untuk menulis, dan status program diatur menjadi 0 yang menandakan mode "JALAN"
    - **`start_daemon()`** dan **`save_pid()`** dipanggil untuk memulai kembali proses daemon dan menyimpan PID-nya

```c
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
    if (chdir("/Users/tarisa/smt-2/sisop/soal4") < 0) { 
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
```

Fungsi `start_daemon()` untuk memulai program secara daemon sesuai dengan yang di modul dengan mengubah chdir dengan directory perngerjaan

```c
void monitoring(const char *user, int kill_process) {
    const char* status_file = "/Users/tarisa/smt-2/sisop/soal4/status_file"; 
    while (1) {
        update_status(status_file);  
        char command[256];
        snprintf(command, sizeof(command), "ps -u %s -o pid,comm | awk 'NR>1 {print $1, $2}'", user);
        FILE *fp = popen(command, "r");
        if (!fp) {
            syslog(LOG_ERR, "Failed to run command.\n");
            exit(1);
        }

        char pid[32]; // Menyimpan PID sebagai string
        char process_name[256]; // Menyimpan nama proses sebagai string
        char log_filename[100];
        char *status = current_status ? "GAGAL" : "JALAN";
        while (fscanf(fp, "%s %s", pid, process_name) != EOF) {
            log_activity(user, pid, process_name, status);
            snprintf(log_filename, sizeof(log_filename), "%s.log", user);
            FILE *log_file = fopen(log_filename, "a");
            if (log_file == NULL) {
                perror("Failed to open log file");
                exit(EXIT_FAILURE);
            }
            if (kill_process == 1) {
                int res = kill(atoi(pid), SIGTERM);
                if (res != 0) {
                    perror("kill");
                    fprintf(log_file, "GAGAL KILL SI %d-%s_\n", atoi(pid), process_name);
                    exit(EXIT_FAILURE);
                } 
            }
        }

        pclose(fp);
        sleep(5);
    }
}
```

Fungsi **`monitoring()`** berguna untuk memantau proses-proses yang berjalan pada user tertentu

- Program akan melakukan pemantauan secara terus-menerus dengan menggunakan loop **`while(1)`**
- Command **`ps`** dieksekusi untuk mendapatkan daftar proses yang sedang berjalan untuk user yang ditentukan. Hasil dari command **`ps`** diproses menggunakan fungsi **`popen()`** untuk dibaca dalam program, setiap baris output dari command **`ps`** diparsing untuk mendapatkan PID dan nama proses
- PID dan nama proses disimpan dalam variabel **`pid`** dan **`process_name`** berturut-turut
- **`log_activity()`** dipanggil untuk mencatat aktivitas proses ke dalam file log
- Jika variabel **`kill_process`** bernilai **`1`**, proses akan dihentikan menggunakan fungsi **`kill()`** dengan sinyal **`SIGTERM`.** Hasil dari pemanggilan fungsi **`kill()`** akan diperiksa, dan jika tidak berhasil, pesan kesalahan akan dicatat dalam file log.
- Fungsi **`pclose()`** dipanggil untuk menutup stream dari command **`ps`**
- Program akan sleep selama 5 detik sebelum memulai iterasi berikutnya dari loop

```c
void update_status(const char *status_file) {
    FILE* f = fopen(status_file, "r");
    if (f) {
        fscanf(f, "%d", &current_status);
        fclose(f);
    }
}
```

Fungsi **`update_status()`** digunakan untuk memperbarui status program (JALAN atau GAGAL) dari file yang ditentukan. Fungsi ini membaca status dari file dan memperbarui variabel **`current_status`** sesuai dengan nilai yang dibaca.

```c
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
```

Fungsi **`log_activity()`** berguna untuk mencatat aktivitas yang terjadi ke dalam file log. Fungsi ini mencatat waktu kejadian, pid proses, nama proses, dan status ke dalam file log.

```c
void save_pid() {
    FILE* file = fopen("admin.pid", "a");
    if (file == NULL) {
        perror("Failed to write PID file.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%d\n", getpid());
    fclose(file);
}
```

Fungsi **`save_pid()`** digunakan untuk menyimpan PID (Process ID) daemon dengan getpid() ke dalam file. Ini dilakukan agar daemon dapat dihentikan nantinya. 

```c
void stop_daemon(const char* user) {
    char filename[50];
    sprintf(filename, "%s.log", user); 
    FILE* file = fopen("admin.pid", "r");
    if (file == NULL) {
        syslog(LOG_ERR, "Failed to read PID file.\n");
        return;
    }
    int pid;
    while (fscanf(file, "%d", &pid) != EOF) { // Loop sampai EOF untuk membaca semua PID
        kill(pid, SIGTERM); // Mengirim sinyal SIGTERM ke PID
    }
    fclose(file);

    remove("admin.pid");  // hapus file pid
    remove(filename);  // hapus file log
    remove("status_file");
}
```

Fungsi **`stop_daemon`** bertujuan untuk menghentikan daemon yang sedang berjalan dan menghapus file-file yang berkaitan dengan program daemon. Fungsi akan membuka file [`admin.pid`](http://admin.pid) yang berisikan PID yang telah tersimpah dari fungsi `save_pid()` sebelumnya lalu menghentikan semua PID yang berada di dalamnya menggunakan `SIGTERM`

### Errors

-c tidak berhasil menghentikan proses yang berjalan karena lupa tambahin kill pid process

![image](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/164857172/2718124a-d2c9-4e9b-bef1-870a9d789f40)

sudah ditambahi kill PID process namun -c tetap gagal menghentikan process, solusinya switch user ke root `su -` 

![image](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/164857172/f57e5e5a-073b-4bbb-8d4d-4afef1e6536e)

![image](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/164857172/5081f2d0-e9da-43bc-8154-db179a354d8e)

## Soal 4

Dikerjakan oleh Adlya Isriena Aftarisya (5027231066)

### Deskripsi Soal

Salomo memiliki passion yang sangat dalam di bidang sistem operasi. Saat ini, dia ingin mengotomasi kegiatan-kegiatan yang ia lakukan agar dapat bekerja secara efisien. Bantulah Salomo untuk membuat program yang dapat mengotomasi kegiatan dia!

### Pengerjaan

```c
struct App {
    char name[50];
    int numWindows;
    pid_t pid[10]; 
    int numPid; 
};
```

- name: nama aplikasi
- numWindows: jumlah windows yang akan dibuka
- pid: array untuk menyimpan PID process yang sedang berjalan
- numPID: jumlah proses yang diluncurkan

```c
int main(int argc, char *argv[]) {
    struct App apps[10];
    int numApps = 0;

    if (argc == 1) {
        printf("Usage: %s -o <app1> <num1> <app2> <num2> ... OR %s -f <filename> OR %s -k OR %s -k <filename>\n", argv[0], argv[0], argv[0], argv[0]);
        return 1;
    }
		....
```

Fungsi main berguna untuk menjalankan program sesuai dengan argumen perintah yang diinput.

- variabel `numApps` akan diinisialisasi dengan 0 sebagai jumlah awal dari aplikasi yang dibuka
- jika argumen perintah yang diinput sama dengan 1 `(argc == 1)` program tidak akan berjalan dan memberikan output arahan argumen yang sesuai dengan program

```c
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

            else { // data -f ada data -o juga ada
                
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
```

- command `-o`
    - jika `argc < 4 || argc % 2 != 0` maka program tidak akan berjalan karena argumen perintah belum sesuai dengan arahan yang diberikan
    - jika argumen sesuai, variabel `numApps` akan dikalkulasi berdasarkan jumlah argumen setelah opsi **`-o`**
    - melakukan `for` loop untuk menentukan `app[i].name` , `app[i].numWindows` dari argv
    - `openApps(apps, numApps);` untuk membuka aplikasi dengan jumlah windowsnya sesuai dengan yang diberikan argumen
    - `saveDataToFile(apps, numApps);` untuk menyimpan PID dari setiap windows yang dibuka
- command `-f`
    - jika `argc != 3` maka program tidak dapat dijalankan karena argumen tidak sesuai dengan arahan yang diberikan
    - memeriksa apakah app_data.txt telah tersedia apa belum, jika belum program akan memanggil  fungsi `openAppsFromFile(argv[2],apps, &numApps);` dan `saveDataFToFile(apps, numApps);` namun jika file app_data.txt sudah tersedia program akan menjalankan fungsi seperti kondisi sebelumnya dan `mergepid("app_data.txt", "app_dataf.txt", "combine.txt");` untuk menggabungkan PID dari windows yang telah dibuka dari `-o` dan `-f`
- command `-k`
    - jika `argc == 2` maka program akan mengkill semua PID windows yang telah dibuka
        - jika PID dari `-o` dan `-f` tidak tersedia, program akan mengeluarkan output error dan `return 1`
        - jika PID dari `-o` tersedia dan `-f` tidak tersedia, program akan menjalankan fungsi  `readOdata(apps, &numApps);` dan `killApps(apps, numApps);` kemudian menghapus file app_data.txt
        - jika PID dari `-o` tidak tersedia dan `-f` tersedia, program akan menjalankan fungsi `readconfdata(apps, &numApps);` dan `killApps(apps, numApps);` kemudian menghapus file app_dataf.txt
        - jika PID dari `-o` tersedia dan `-f` tersedia, program akan menjalankan fungsi `killAllpid("combine.txt");` kemudian menghapus file semua file.txt yang tersedia
    - jika `argc == 3` maka program akan mengkill PID process yang dibuka dari file.conf dengan memanggil fungsi `readconfdata(apps, &numApps);` dan `killApps(apps, numApps);` kemudian menghapus app_dataf.txt

```c
void openApps(struct App apps[], int numApps) {
    for (int i = 0; i < numApps; i++) {
        printf("Opening %d windows of %s\n", apps[i].numWindows, apps[i].name);
        apps[i].numPid = 0;
        for (int j = 0; j < apps[i].numWindows; j++) {
            pid_t pid = fork();
            if () {
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
```

Fungsi **`openApps`** bertujuan untuk membuka aplikasi-aplikasi yang telah ditentukan sejumlah windownya dan menyimpan PID dari setiap proses yang dibuka ke dalam  `struct App` 

- fungsi menggunakan **`fork()`** untuk membuat proses baru untuk membuka aplikasi.
    - Jika `pid == 0` artinya proses baru (child process) berhasil dibuat. Pada child process ini, fungsi menggunakan **`execlp()`** untuk menjalankan perintah **`open -n -a <nama_aplikasi>`** di terminal. Opsi **`-n`** menandakan bahwa aplikasi akan dibuka dalam satu instance yang baru, sedangkan **`-a`** diikuti dengan nama aplikasi yang akan dibuka
    - `pid > 0` artinya ini adalah parent process. Parent process menunggu child process selesai menggunakan **`waitpid()`** untuk mendapatkan PID child process yang selesai
        - `childPid > 0` artinya proses child telah selesai. PID child tersebut disimpan ke dalam array **`pid`** pada struktur **`App`** menggunakan **`apps[i].pid[apps[i].numPid++]`**.
    - `pid < 0` artinya `fork()` gagal berjalan

```c
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
```

Fungsi `saveDataToFile()` berguna untuk menyimpan PID dari setiap windows yang dibuka dari command `-o` 

```c
void openAppsFromFile(const char *filename, struct App apps[], int *numApps) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open file\n");
        return;
    }

    char line[100];
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
```

Fungsi `openAppsFromFile()` berguna untuk membuka aplikasi dari file.conf

- program akan melakukan `while` loop untuk membaca nama aplikasi dan jumlah windows dari setiap aplikasi dan memasukkannya ke dalam variabel `struct app`
- `openApps()` akan membuka aplikasi beserta jumlah windowsnya dengan data yang telah didapatkan dari `while` loop sebelumnya
- `saveDataFToFile()` akan menyimpan PID setiap windowsnya ke dalam file app_dataf.txt

```c
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
```

Fungsi `saveDataFToFile()` berguna untuk menyimpan PID dari setiap windows yang dibuka dari command `-f` 

```c
void mergepid(const char* srcFilename1, const char* srcFilename2, const char* destFilename) {
    FILE *src1 = fopen(srcFilename1, "r");
    FILE *src2 = fopen(srcFilename2, "r");
    if (!src1 || !src2) {
        perror("Failed to open source files");
        if (src1) fclose(src1);
        if (src2) fclose(src2);
        return;
    }

    FILE *dest = fopen(destFilename, "w"); 
    if (!dest) {
        perror("Failed to open destination file");
        fclose(src1);
        fclose(src2);
        return;
    }

    int pid, numApps;
    fscanf(src1, "%d", &numApps);  
    while (fscanf(src1, "%d", &pid) == 1) {
        fprintf(dest, "%d\n", pid);
    }
    fscanf(src2, "%d", &numApps);  
    while (fscanf(src2, "%d", &pid) == 1) {
        fprintf(dest, "%d\n", pid);
    }

    fclose(src1);
    fclose(src2);
    fclose(dest);
}

```

Fungsi `mergepid()` digunakan jika telah menjalankan command `-o` dan `-f` sebelumnya. Fungsi ini akan menggabungkan PID yang ada pada masing-masing file.txt ke dalam file yang bernama combine.txt

```c
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
```

Fungsi `readOData()` digunakan untuk membaca file app_data.txt yang menyimpan PID yang telah disimpan dari command `-o` 

```c
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
```

Fungsi `readconfData()` digunakan untuk membaca file app_data.txt yang menyimpan PID yang telah disimpan dari command `-f` 

```c
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
```

Fungsi **`killApps`** bertujuan untuk menghentikan semua proses yang telah dibuka sebelumnya dengan mematikan setiap proses yang terkait dengan aplikasi-aplikasi yang disimpan dalam `struct app` 

```c
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
```

Fungsi `killAllpid()` digunakan untuk mengkill semua PID yang berada di dalam file yang berisikan PID gabungan dari aplikasi yang dibuka melalui command `-o` dan `-f` 

### Errors

command `-k` tidak berhasil berjalan karena nilai dari `numApps` tidak tersimpan dari eksekusi program sebelumnya. Solusi: masukkan nilai `numApps` ke file.txt

![image](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/164857172/4f6c2510-6353-4870-b6c8-b056c778e60c)

command `-k` tidak berhasil kill PID yang berada dari 2 data file jika fungsinya seperti ini. Solusi: gabungkan isi dari 2 file berisi PID lalu kill isi dari file gabungan tersebut.

![image](https://github.com/DzakyAhnaf/Sisop-2-2024-MH-IT12/assets/164857172/b31e1abf-dd5a-4109-bdb2-d591f1ebb85c)

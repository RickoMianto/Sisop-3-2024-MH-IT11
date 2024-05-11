# Soal 1
Dikerjakan oleh Ricko Mianto Jaya Saputra (5027231031)

Kode yang disajikan adalah adalah implementasi dari layanan pemrograman bahasa C yang berfungsi untuk mengelola dan memproses data terkait tempat sampah di Belobong dan tempat parkir di Osaka.
Ketiga kode tersebut adalah auth.c, rate.c, dan db.c. Setiap kode tersebut memiliki implementasi masing masing dalam proses autentikaasi, menilai, dan memindahkan file.

# Deskripsi Kode

## auth.c

Deskripsi : Kode ini bertanggung jawab untuk melakukan autentikasi file yang masuk ke dalam direktori new-data, memastikan bahwa file file tersebut adalah file CSV yang berisi data tentang tempat sampah di Belobong atau tempat parkir di Osaka. Jika bukan, maka file tersebut akan dihapus.

Berikut adalah kode saya:

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define DIR_PATH "./new-data"
#define PARKINGLOT_KEY 1234
#define TRASHCAN_KEY 4321
#define MAX_PATH 1024

void transfer_data_to_shared_memory(const char* filename, key_t key){
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    int shmid = shmget(key, file_size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Failed to get shared memory segment");
        return;
    }

    char *shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("Failed to attach shared memory segment");
        return;
    }

    size_t bytes_read = fread(shm_ptr, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("Failed to read file");
    }
    fclose(file);

    int shmid_filename = shmget(key + 1, strlen(filename) + 1, IPC_CREAT | 0666);
    char *shm_ptr_filename = (char *)shmat(shmid_filename, NULL, 0);
    strcpy(shm_ptr_filename, filename);
}

void process_directory_files() {
    DIR *dir = opendir(DIR_PATH);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char filepath[MAX_PATH];
        if (strlen(DIR_PATH) + strlen(entry->d_name) + 2 > MAX_PATH) {
            fprintf(stderr, "File path is too long\n");
            return;
        }
        snprintf(filepath, sizeof(filepath), "%s/%s", DIR_PATH, entry->d_name);

        if (strstr(entry->d_name, "parkinglot.csv")) {
            transfer_data_to_shared_memory(filepath, PARKINGLOT_KEY);
        } else if (strstr(entry->d_name, "trashcan.csv")) {
            transfer_data_to_shared_memory(filepath, TRASHCAN_KEY);
        } else {
            if (remove(filepath) == -1) {
                perror("Failed to remove file");
            }
        }
    }
    closedir(dir);
}

int main() {
    process_directory_files();
    return 0;
}
```

### Hasil Run

## rate.c

Deskripsi : Kode ini berfungsi untuk mengambil data CSV dari shared memory dan memberikan output berupa rating tertinggi dari tempat sampah di Belobong dan tempat parkir di Osaka.

Berikut adalah kode saya :

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <libgen.h>

#define PARKINGLOT_KEY 1234
#define TRASHCAN_KEY 4321

const char *extract_filename(const char *path) {
    const char *filename = strrchr(path, '/');
    if(!filename || filename == path) return path;
    return filename + 1;
}

char *access_shared_memory(key_t key) {
    int shmid = shmget(key, 0, 0666);
    if (shmid == -1) {
        perror("Failed to get shared memory segment");
        return NULL;
    }
    char *shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("Failed to attach shared memory segment");
        return NULL;
    }
    char *string = strdup(shm_ptr);
    shmdt(shm_ptr);
    return string;
}

void find_and_print_highest_rating(char *data, const char *type, const char *filename) {
    char *line = strtok(data, "\n");
    float highest_rating = -1;
    char highest_name[100] = "";

    while (line != NULL) {
        char name[100];
        float rating;
        sscanf(line, "%[^,], %f", name, &rating);
      
        if (rating > highest_rating) {
            highest_rating = rating;
            strcpy(highest_name, name);
        }
        line = strtok(NULL, "\n");
    }
    printf("Type: %s\nFilename: %s\n", type, filename);
    printf("------------------------------------\n");
    printf("Name: %s\nRating: %.1f\n", highest_name, highest_rating);
    printf("\n\n");
}

int main() {
    key_t keys[] = {PARKINGLOT_KEY, TRASHCAN_KEY};
    const char *types[] = {"Parking Lot", "Trash Can"};

    for (int i = 0; i < 2; i++) {
        char *filename = access_shared_memory(keys[i] + 1);
        if (filename == NULL) {
            continue;
        }
        char *data = access_shared_memory(keys[i]);
        if (data == NULL) {
            free(filename);
            continue;
        }
        find_and_print_highest_rating(data, types[i], filename);
        free(filename);
        free(data);
    }    
    return 0;
}
```

### Hasil Run

## db.c

Dekripsi : Kode ini berfungsi untuk memindahkan file yang telah lolos autentikasi dari direktori new-data ke folder microservices/database. dan setiap file yang dipindahkan ke direktori database akan dicatat dalam file db.log

Berikut adalah kode saya :

```C
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>

#define PARKINGLOT_KEY 1234
#define TRASHCAN_KEY 4321

const char *extract_filename(const char *path) {
    const char *filename = strrchr(path, '/');
    if(!filename || filename == path) return path;
    return filename + 1;
}

char *access_shared_memory(key_t key) {
    int shmid = shmget(key, 0, 0666);
    char *shm_ptr = (char *)shmat(shmid, NULL, 0);
    char *string = strdup(shm_ptr);
    shmdt(shm_ptr);
    return string;
}

void modify_path(char **path) {
    char *token;
    const char search[] = "new-data";
    const char replacement[] = "microservices/database";

    token = strstr(*path, search);
    if (token != NULL) {
        size_t length_before = token - *path;
        size_t length_after = strlen(token + strlen(search));
        char *new_path = (char *)malloc(length_before + strlen(replacement) + length_after + 1);
        strncpy(new_path, *path, length_before);
        new_path[length_before] = '\0';
        strcat(new_path, replacement);
        strcat(new_path, token + strlen(search));
        free(*path);
        *path = new_path;
    }
}

void log_file_movement(const char *old_path, const char *new_path, const char *type) {
    if (rename(old_path, new_path) == 0) {
        const char *basename = extract_filename(old_path);
        FILE *log_file = fopen("database/db.log", "a");
        if (log_file != NULL) {
            time_t raw_time;
            struct tm *time_info;
            time(&raw_time);
            time_info = localtime(&raw_time);
            fprintf(log_file, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] [%s]\n",
                    time_info->tm_mday, time_info->tm_mon + 1, time_info->tm_year + 1900,
                    time_info->tm_hour, time_info->tm_min, time_info->tm_sec, type, basename);
            fclose(log_file);
        }
    }
}

int main() {
    char *old_path_1 = access_shared_memory(PARKINGLOT_KEY + 1);
    char *new_path_1 = strdup(old_path_1);
    modify_path(&new_path_1);
    log_file_movement(old_path_1, new_path_1, "Parking Lot");
    free(old_path_1);
    free(new_path_1);

    char *old_path_2 = access_shared_memory(TRASHCAN_KEY + 1);
    char *new_path_2 = strdup(old_path_2);
    modify_path(&new_path_2);
    log_file_movement(old_path_2, new_path_2, "Trash Can");
    free(old_path_2);
    free(new_path_2);
    return 0;
}
```

### Hasil Run

# Kendala yang Dialami
pada soal ini ada beberapa kendala yang saya alami yaitu output pada rate.c di bagian filename masih belum sesuai dengan yang diminta soal dan pada db.c masih belum bisa memindahkan file dari new-data ke database.

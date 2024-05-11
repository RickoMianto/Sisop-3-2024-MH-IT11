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
![Screenshot_from_2024-05-11_20-27-34](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/150517828/15842f0e-6961-49f8-a3d3-a72612e2f762)

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
![Screenshot_from_2024-05-11_20-29-00](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/150517828/e5851ea8-8d7e-4d59-be4e-9cca495ff32f)

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
![Screenshot_from_2024-05-11_20-29-26](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/150517828/b3f91cfc-2c3a-4eeb-b406-55e53f4140f9)

# Kendala yang Dialami
pada soal ini ada beberapa kendala yang saya alami yaitu output pada rate.c di bagian filename masih belum sesuai dengan yang diminta soal dan pada db.c masih belum bisa memindahkan file dari new-data ke database.

# Soal 3
Dikerjakan oleh Raditya Hardian Santoso (5027231033)

Kode yang disajikan adalah adalah implementasi dari layanan pemrograman bahasa C yang berfungsi untuk membangun sistem yang memungkinkan engineer di paddock (server) untuk mengontrol pengaturan mobil F1 melalui komunikasi jarak jauh dengan driver (client). 

# Deskripsi Kode

## actions.c

Deskripsi : Kode ini berisi fungsi yang berisi logika pengaturan mobil f1 seperti ‘Gap’, ‘Fuel’, ‘Tire’, dan ‘TireChange’ untuk menghitung respons berdasarkan permintaan client.

Berikut adalah kode saya:

```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "actions.h"

char* Gap(float distance) {
    if (distance < 3.5)
        return "Gogogo";
    else if (distance >= 3.5 && distance <= 10)
        return "Push";
    else
        return "Stay out of trouble";
}

char* Fuel(int fuelPercentage) {
    if (fuelPercentage > 80)
        return "Push Push Push";
    else if (fuelPercentage >= 50 && fuelPercentage <= 80)
        return "You can go";
    else
        return "Conserve Fuel";
}

char* Tire(int tireUsage) {
    if (tireUsage > 80)
        return "Go Push Go Push";
    else if (tireUsage >= 50 && tireUsage <= 80)
        return "Good Tire Wear";
    else if (tireUsage > 30 && tireUsage < 50)
        return "Conserve Your Tire";
    else
        return "Box Box Box";
}

char* TireChange(char* currentTireType) {
    if (strcmp(currentTireType, "Soft") == 0)
        return "Mediums Ready";
    else if (strcmp(currentTireType, "Medium") == 0)
        return "Box for Softs";
    else
        return "Invalid Tire Type";
}
```
## paddock.c

Deskripsi : Kode ini berfungsi untuk server yang menerima permintaan dari client (driver.c). kode ini juga memproses perintah dari client serta mengirimkan respons kembali ke client.

Berikut adalah kode saya:

```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "actions.h" 

#define PORT 8080
#define LOG_FILE "race.log"

void log_to_file(const char *source, const char *command, const char *additional_info) {
    FILE *logfile = fopen(LOG_FILE, "a");
    if (logfile == NULL) {
        perror("Failed to open race.log");
        exit(EXIT_FAILURE);
    }

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", tm_info);

    fprintf(logfile, "[%s] [%s]: [%s] [%s]\n", source, timestamp, command, additional_info);

    fclose(logfile);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Paddock is waiting for connections...\n");

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        read(new_socket, buffer, 1024);
        printf("Received command from driver.c: %s\n", buffer);

        char response[1024];
        if (strncmp(buffer, "Fuel", 4) == 0) {
            int fuelPercentage;
            sscanf(buffer, "Fuel %d%%", &fuelPercentage);
            strcpy(response, Fuel(fuelPercentage));
        } else if (strncmp(buffer, "Gap", 3) == 0) {
            float gapDistance;
            sscanf(buffer, "Gap %f", &gapDistance);
            strcpy(response, Gap(gapDistance));
        } else if (strncmp(buffer, "Tire", 4) == 0) {
            int tireUsage;
            sscanf(buffer, "Tire %d", &tireUsage);
            strcpy(response, Tire(tireUsage));
        } else if (strncmp(buffer, "TireChange", 10) == 0) {
            char currentTireType[20];
            sscanf(buffer, "TireChange %s", currentTireType);
            strcpy(response, TireChange(currentTireType));
        }

        send(new_socket, response, strlen(response), 0);
        printf("Sent response to driver.c: %s\n", response);

        log_to_file("Paddock", buffer, response);

        memset(buffer, 0, sizeof(buffer));
        close(new_socket);
    }

    return 0;
}
```

## driver.c

Deskripsi : Kode ini berfungsi sebagai client untuk mengirim perintah kepada server (paddock.c). kode ini menerima input dari pengguna dan mengirimkannya ke server. Lalu menampikan respons dari server kepada pengguna.

Berikut adalah kode saya:

```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Enter command (e.g., Fuel 55%%, Gap 8.5): ");
    scanf(" %[^\n]", command); // Membaca input dari pengguna sampai newline

    send(sock, command, strlen(command), 0);
    printf("Command sent to paddock.c: %s\n", command);

    valread = read(sock, buffer, BUFFER_SIZE);
    printf("Response from paddock.c: %s\n", buffer);

    close(sock);

    return 0;
}
```
# Langkah-langkah

1. Taruh pada satu direktori yang sama, lalu compile masing-masing file (driver.c, actions.c, dan paddock.c) dengan cara :
gcc -c actions.c -o actions.o
gcc -c paddock.c -o paddock.o
gcc -c driver.c -o driver.o
gcc actions.o paddock.o -o paddock
gcc driver.o -o driver
2. Lalu muncul beberapa file baru hasil kompilasi ketiga file itu, yaitu driver.o, paddock.o, actions.o, dan actions.h
3. Lalu bisa jalankan dengan terminal terpisah yang membedakan client dan server.
Dengan server masukkan ./paddock dan server masukkan ./driver
Maka akan muncul seperti ini :
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/de5573e0-46cd-4e14-8fd6-4fd31bacd5ff)
5. Lalu masukkan input sesuai soal, yang isinya ada Gap, Fuel, Tire, TireChange dan value yang diinginkan.

# Hasil Run
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/b5673558-ffad-4364-a12e-47142548c084)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/ee7d53b3-c5a8-4184-8c87-0f4d51062083)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/eaabe824-b046-4d98-a8ed-3eeba9949ae9)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/cbcb0beb-6287-4165-80b4-4c4f70a51407)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/73cecf46-a6f6-4f6f-a508-cdd6c1097a00)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/602b669e-0b00-42ed-8d46-176553c56dd7)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/602b669e-0b00-42ed-8d46-176553c56dd7)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/ea8cea08-c2c7-43f5-a8cb-fc1e193db735)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/880750c9-c83d-469b-8699-41ecbfc512af)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/8bf9a646-86f2-456b-a7d4-e63db5e461e9)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/f23b4534-4f30-47e8-8324-4d48d9d16645)

Hasil race.log : 
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/137570361/9da51188-5a70-4322-8163-c4ea9e67b904)

# Kendala
Kendala yang masih saya alami : 
1. Belum menemukan cara agar bisa melakukan input terus menerus tanpa harus exit atau ctrl+c
2. Untuk input TireChange outputnya masih boxboxbox alias hasil output dari else tire biasa



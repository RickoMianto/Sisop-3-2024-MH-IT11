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

# SOAL 2
Dikerjakan oleh Rafi' Afnaan Fathurrahman (5027231040)
Pengerjaan soal ini menggunakan 1 file dudududu.c
## dudududu.c
Max Verstappen seorang pembalap F1 dan programer memiliki seorang adik bernama Min Verstappen (masih SD) sedang menghadapi tahap paling kelam dalam kehidupan yaitu perkalian matematika, Min meminta bantuan Max untuk membuat kalkulator perkalian sederhana (satu sampai sembilan). Sembari Max nguli dia menyuruh Min untuk belajar perkalian dari web (referensi) agar tidak bergantung pada kalkulator.
(Wajib menerapkan konsep pipes dan fork seperti yang dijelaskan di modul Sisop. Gunakan 2 pipes dengan diagram seperti di modul 3).
## subsoal a
Sesuai request dari adiknya Max ingin nama programnya dudududu.c. Sebelum program parent process dan child process, ada input dari user berupa 2 string. Contoh input: tiga tujuh. 
```c
char firstInput[1024], secondInput[1024];
scanf("%s %s", firstInput, secondInput);
while((getchar()) != '\n');
```
## subsoal b
Pada parent process, program akan mengubah input menjadi angka dan melakukan perkalian dari angka yang telah diubah. Contoh: tiga tujuh menjadi 21. 
```c
int main(int argc, char **argv) 
{
  // ...
  // Parent process
  else if (p > 0) 
  { 
    int num1 = strToInt(firstInput);
    int num2 = strToInt(secondInput);
    int result = operation(num1, argv, num2);

    close(fd1[0]);
    write(fd1[1], &result, sizeof(result));
    close(fd1[1]);

    wait(NULL);

    close(fd2[1]);
    char resultStr[1024];
    read(fd2[0], resultStr, sizeof(resultStr));

    // ...
  } 
}

int strToInt(char *input)
{
  if (strcmp(input, "satu") == 0) return 1;
  else if (strcmp(input, "dua") == 0) return 2;
  else if (strcmp(input, "tiga") == 0) return 3;
  else if (strcmp(input, "empat") == 0) return 4;
  else if (strcmp(input, "lima") == 0) return 5;
  else if (strcmp(input, "enam") == 0) return 6;
  else if (strcmp(input, "tujuh") == 0) return 7;
  else if (strcmp(input, "delapan") == 0) return 8;
  else if (strcmp(input, "sembilan") == 0) return 9;
  else exit(EXIT_FAILURE);
}

int operation(int firstInput, char **argv, int secondInput)
{
  int mode = 0;
  // ..

  switch(mode)
  {
    case 1:
      return firstInput * secondInput;
    case 2:
      return firstInput + secondInput;
    case 3:
      return firstInput - secondInput;
    case 4:
      return firstInput / secondInput;
    default:
      exit(EXIT_FAILURE);
  }
}
```
## subsoal c
Pada child process, program akan mengubah hasil angka yang telah diperoleh dari parent process menjadi kalimat. Contoh: `21` menjadi “dua puluh satu”.
```c
int main(int argc, char **argv) 
{
  // ...
  // child process 
  else
  { 
    int result;

    close(fd1[1]);
    read(fd1[0], &result, sizeof(result));
    close(fd1[0]);

    char *resultStr = intToStr(result);

    output(firstInput, argv, secondInput, resultStr);

    close(fd2[0]);
    write(fd2[1], resultStr, strlen(resultStr)+1);
    close(fd2[1]);

    exit(0);
  }
}
// ...

// if it's stupid, but it works, it ain't stupid
char * intToStr(int result)
{
  if (result == 1) return "satu";
  else if (result == 2) return "dua";
  else if (result == 3) return "tiga";
  else if (result == 4) return "empat";
  else if (result == 5) return "lima";
  // 6 - 99 ...
  else if (result == 100) return "seratus";
  else if (result == 0) return "nol";
  else return "ERROR";
}
```
## subsoal d
Max ingin membuat program kalkulator dapat melakukan penjumlahan, pengurangan, dan pembagian, maka pada program buatlah argumen untuk menjalankan program : 
- perkalian   : ./kalkulator -kali
- penjumlahan : ./kalkulator -tambah
- pengurangan : ./kalkulator -kurang
- pembagian   : ./kalkulator -bagi
Beberapa hari kemudian karena Max terpaksa keluar dari Australian Grand Prix 2024 membuat Max tidak bersemangat untuk melanjutkan programnya sehingga kalkulator yang dibuatnya cuma menampilkan hasil positif jika bernilai negatif maka program akan print “ERROR” serta cuma menampilkan bilangan bulat jika ada bilangan desimal maka dibulatkan ke bawah.
```c
int main(int argc, char **argv) 
{}
// ...
int operation(int firstInput, char **argv, int secondInput)
{
  int mode = 0;
  if (strcmp(argv[1], "-kali") == 0) mode = 1;
  else if (strcmp(argv[1], "-tambah") == 0) mode = 2;
  else if (strcmp(argv[1], "-kurang") == 0) mode = 3;
  else if (strcmp(argv[1], "-bagi") == 0) mode = 4;
  else exit(EXIT_FAILURE);

  switch(mode)
  {
    case 1:
      return firstInput * secondInput;
    case 2:
      return firstInput + secondInput;
    case 3:
      return firstInput - secondInput; // di dalam fungsi int to str, jika angka bukan 0 - 100 (negatif dsbg) akan menjadi ERROR
    case 4:
      return firstInput / secondInput; // bulat kebawah
    default:
      exit(EXIT_FAILURE);
  }
}
```
## subsoal e
Setelah diberi semangat, Max pun melanjutkan programnya dia ingin (pada child process) kalimat akan di print dengan contoh format : 
- perkalian	: “hasil perkalian tiga dan tujuh adalah dua puluh satu.”
- penjumlahan	: “hasil penjumlahan tiga dan tujuh adalah sepuluh.”
- pengurangan	: “hasil pengurangan tujuh dan tiga adalah empat.”
- pembagian	: “hasil pembagian tujuh dan tiga adalah dua.”
```c
int main(int argc, char **argc)
{
  // ...
  char firstInput[1024], secondInput[1024];
  // ...
  // child process 
  else
  { 
    int result;

    // ...

    char *resultStr = intToStr(result);

    output(firstInput, argv, secondInput, resultStr);

    // ...
  } 
}
// ...
void output(char *firstInput, char **argv, char *secondInput, char *result)
{
  int mode = 0;
  if (strcmp(argv[1], "-kali") == 0) mode = 1;
  else if (strcmp(argv[1], "-tambah") == 0) mode = 2;
  else if (strcmp(argv[1], "-kurang") == 0) mode = 3;
  else if (strcmp(argv[1], "-bagi") == 0) mode = 4;
  else exit(EXIT_FAILURE);

  switch(mode)
  {
    case 1:
      printf("hasil perkalian %s dan %s adalah %s\n", firstInput, secondInput, result);
      break;
    case 2:
      printf("hasil penjumlahan %s dan %s adalah %s\n", firstInput, secondInput, result);
      break;
    case 3:
      printf("hasil pengurangan %s dan %s adalah %s\n", firstInput, secondInput, result);
      break;
    case 4:
      printf("hasil pembagian %s dan %s adalah %s\n", firstInput, secondInput, result);
      break;
    default:
      exit(EXIT_FAILURE);
  }
}
```
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/74938346-c067-4ed0-b2ff-3796055c98d8)

## subsoal f
Max ingin hasil dari setiap perhitungan dicatat dalam sebuah log yang diberi nama histori.log. Pada parent process, lakukan pembuatan file log berdasarkan data yang dikirim dari child process. 
- Format: [date] [type] [message]
- Type: KALI, TAMBAH, KURANG, BAGI
- Ex:
  1. [10/03/24 00:29:47] [KALI] tujuh kali enam sama dengan empat puluh dua.
  2. [10/03/24 00:30:00] [TAMBAH] sembilan tambah sepuluh sama dengan sembilan belas.
  3. [10/03/24 00:30:12] [KURANG] ERROR pada pengurangan.
```c
int main(int argc, char **argv) 
{
	// Parent process 
	else if (p > 0) 
	{ 
    // ..

    FILE *log = fopen("history.log", "a");
    if (log != NULL)
    {
      if (strcmp(resultStr, "ERROR") == 0) fprintf(log, "[%02d/%02d/%02d %02d:%02d:%02d][%s] %s pada %s.\n", digitime->tm_mday, digitime->tm_mon + 1, digitime->tm_year + 1900, digitime->tm_hour, digitime->tm_min, digitime->tm_sec, opt, resultStr, whenError(argv));
      else fprintf(log, "[%02d/%02d/%02d %02d:%02d:%02d][%s] %s %s %s sama dengan %s.\n", digitime->tm_mday, digitime->tm_mon + 1, digitime->tm_year + 1900, digitime->tm_hour, digitime->tm_min, digitime->tm_sec, opt, firstInput, lowOpt, secondInput, resultStr);
      fclose(log);
    }
  } 
}
```
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/faf58d10-aa64-496d-afa9-5d1161f2dc67)


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

# SOAL 4
Dikerjakan oleh Rafi' Afnaan Fathurrahman (5027231040)
Pengerjaan soal ini menggunakan 2 file yaitu client.c dan server.c
## server.c dan client.c
Lewis Hamilton seorang wibu akut dan sering melewatkan beberapa episode yang karena sibuk menjadi asisten. Maka dari itu dia membuat list anime yang sedang ongoing (biar tidak lupa) dan yang completed (anime lama tapi pengen ditonton aja). Tapi setelah Lewis pikir-pikir malah kepikiran untuk membuat list anime. Jadi dia membuat file (harap diunduh) dan ingin menggunakan socket yang baru saja dipelajarinya untuk melakukan CRUD pada list animenya. 
## subsoal a
Client dan server terhubung melalui socket. 
```c
// server.c
int main(int argc, char const **argv) 
{
  // ...
  int servSock, cliSock;
  struct sockaddr_in servAddr, cliAddr;
  socklen_t addrSize;
  int addrLen = sizeof(addrSize);
  char buffer[MAX_BUFFER_SIZE];
  char response[MAX_BUFFER_SIZE];

  // ...

  servSock = socket(AF_INET, SOCK_STREAM, 0);
  if (servSock < 0) 
  {
    printf("Error creating socket\n");
    return -1;
  }

  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(PORT);
  servAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) 
  {
    printf("Error binding socket\n");
    return -1;
  }

  if (listen(servSock, 1) < 0) 
  {
    printf("Error listening\n");
    return -1;
  }

  while (1) 
  {
    addrSize = sizeof(cliAddr);
    cliSock = accept(servSock, (struct sockaddr *)&cliAddr, &addrSize);
    if (cliSock < 0) 
    {
      printf("Error accepting connection\n");
      continue;
    }

    while (1) 
    {
      // ...
    } close(cliSock);
  } close(servSock);
  return 0;
}

// client.c
int main(int argc, char const **argv) 
{
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in servAddr;
  char *hello = "Hello from client";
  char buffer[MAX_BUFFER_SIZE] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&servAddr, '0', sizeof(servAddr));

  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(PORT);

  if(inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr)<=0) 
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) 
  {
    printf("\nConnection Failed \n");
    return -1;
  }

  while (1) 
  {
    // ...
  }

  // ...
}
```
## subsoal b
client.c di dalam folder client dan server.c di dalam folder server
soal_4/
    ├── client/
    │   └── client.c
    └── server/
        └── server.c
## subsoal c
Client berfungsi sebagai pengirim pesan dan dapat menerima pesan dari server.
```c
// server.c
int main(int argc, char **argv)
{
  // ...
  read(cliSock, buffer, sizeof(buffer));
  // ...
  send(cliSock, response, strlen(response), 0);
  // ...
}

// client.c
int main(int argc, char **argv)
{
  // ...
  send(sock, buffer, strlen(buffer), 0);
  // ...
  read(sock, buffer, sizeof(buffer));
  // ...
}
```
## subsoal d
Server berfungsi sebagai penerima pesan dari client dan hanya menampilkan pesan perintah client saja.  
```c
// server.c
int main(int argc, char **argv)
{
  // ...
}

// client.c
int main(int argc, char **argv)
{
  // ...
  printf("You: ");
  memset(buffer, 0, sizeof(buffer));
  fgets(buffer, sizeof(buffer), stdin);
  buffer[strcspn(buffer, "\n")] = '\0';
  // ...
  printf("Server:\n%s\n", buffer);
  // ...
}
```
## subsoal e
Server digunakan untuk membaca myanimelist.csv. Dimana terjadi pengiriman data antara client ke server dan server ke client.
- Menampilkan seluruh judul
- Menampilkan judul berdasarkan genre
- Menampilkan judul berdasarkan hari
- Menampilkan status berdasarkan berdasarkan judul
- Menambahkan anime ke dalam file myanimelist.csv
- Melakukan edit anime berdasarkan judul
- Melakukan delete berdasarkan judul
- Selain command yang diberikan akan menampilkan tulisan “Invalid Command”
```c
// server.c
int main(int argc, char **argv)
{
  // ...
  downloadCSV()
  // ...
  if (strcmp(buffer, "exit") == 0) exit(0);
  processCommand(buffer, response);
  // ...
}

void downloadCSV()
{
  system("rm -f ../myanimelist.csv");
  system("wget --content-disposition --no-check-certificate \"https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50\" -P ../");
}

void processCommand(char *buffer, char *response)
{
  char command[MAX_BUFFER_SIZE];
  char token[MAX_BUFFER_SIZE];

  sscanf(buffer, "%s %[^\n]", command, token);
  memset(buffer, 0, sizeof(buffer));
  
  if (strcmp(command, "show") == 0) 
  {
    if (strlen(token) == 0) showAll(response);
    else show(token, response);
  }
  
  else if (strcmp(command, "add") == 0) add(token, response);
  else if (strcmp(command, "edit") == 0) edit(token, response);
  else if (strcmp(command, "delete") == 0) delete(token, response);
  else strcpy(response, "Invalid command");

  memset(command, 0, sizeof(command));
  memset(token, 0, sizeof(token));
}

void show(char *token, char *response)
{
  int typeToken = type(token);
  char command[MAX_BUFFER_SIZE];
  snprintf(command, MAX_BUFFER_SIZE, "awk -F, '$%d ~ \"%s\" {print NR \". \" $3}' ../myanimelist.csv", typeToken, token);

  FILE *fp = popen(command, "r");
  if (fp == NULL) 
  {
    printf("Error executing command\n");
    strcpy(response, "Error executing command\n");
    return;
  }

  char line[MAX_BUFFER_SIZE];
  memset(response, 0, sizeof(response));
  memset(command, 0, sizeof(command));
  memset(token, 0, sizeof(token));
  while (fgets(line, sizeof(line), fp) != NULL) { strcat(response, line); }
  pclose(fp);
}

int type(char *token)
{
  if (strcmp(token, "Senin") == 0 || strcmp(token, "Selasa") == 0 || strcmp(token, "Rabu") == 0 || strcmp(token, "Kamis") == 0 || strcmp(token, "Jumat") == 0 || strcmp(token, "Sabtu") == 0 || strcmp(token, "Minggu") == 0) return 1;
  if (strcmp(token, "Action") == 0 || strcmp(token, "Adventure") == 0 || strcmp(token, "Drama") == 0 || strcmp(token, "Slice of Life") == 0 || strcmp(token, "Comedy") == 0 || strcmp(token, "Romance") == 0 || strcmp(token, "Fantasy") == 0 || strcmp(token, "Science Fiction") == 0) return 2;
  if (strcmp(token, "ongoing") == 0 || strcmp(token, "completed") == 0) return 4;
  return 0;
}

void showAll(char *response)
{
  char command[MAX_BUFFER_SIZE];
  snprintf(command, MAX_BUFFER_SIZE, "awk -F, '{print NR \". \" $3}' ../myanimelist.csv");

  FILE *fp = popen(command, "r");
  if (fp == NULL) 
  {
    printf("Error executing command\n");
    strcpy(response, "Error executing command\n");
    return;
  }

  char line[MAX_BUFFER_SIZE];
  memset(response, 0, sizeof(response));
  memset(command, 0, sizeof(command));
  while (fgets(line, sizeof(line), fp) != NULL) { strcat(response, line); }
  pclose(fp);
}

void add(char *token, char *response)
{  
  FILE *csv = fopen("../myanimelist.csv", "a");
  if (csv != NULL)
  {
    fprintf(csv, "\n%s", token);
    fclose(csv);
  }

  strcpy(response, "Anime berhasil ditambahkan");

  // ...
}


void edit(char *token, char *response)
{
  char prev[MAX_BUFFER_SIZE];
  char new[MAX_BUFFER_SIZE];

  sscanf(token, "%[^,],%[^\n]", prev, new);

  char old[MAX_BUFFER_SIZE];
  snprintf(old, MAX_BUFFER_SIZE, "cat ../myanimelist.csv | grep '%s'", prev);

  FILE *fp1 = popen(old, "r");
  if (fp1 == NULL) 
  {
    printf("Error executing command\n");
    return;
  } fgets(old, sizeof(old), fp1);

  old[strcspn(old, "\n")] = '\0';

  char command[MAX_BUFFER_SIZE];
  snprintf(command, MAX_BUFFER_SIZE, "awk -F, '{$3 == \"%s\" ? $0 = \"%s\" : 1}1' ../myanimelist.csv > tmpfile && mv tmpfile ../myanimelist.csv", prev, new);

  FILE *fp2 = popen(command, "r");
  if (fp2 == NULL) 
  {
    printf("Error executing command\n");
    strcpy(response, "Error executing command\n");
    return;
  } 

  strcpy(response, "Anime berhasil diedit");

  // ...

  pclose(fp1);
  pclose(fp2);
}

void delete(char *token, char *response)
{
  char command[MAX_BUFFER_SIZE];
  snprintf(command, MAX_BUFFER_SIZE, "sed -i '/%s/d' ../myanimelist.csv", token);
  
  FILE *fp = popen(command, "r");
  if (fp == NULL) 
  {
    printf("Error executing command\n");
    strcpy(response, "Error executing command\n");
    return;
  }

  strcpy(response, "Anime berhasil dihapus");

  // ...

  pclose(fp);
}
```
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/1bc83ee1-e3f2-4502-9d5e-c4c401196aac)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/3a348110-f718-42ca-900a-fe1591ff024c)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/a87f94ad-674b-4766-b1f3-2485eb32287f)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/d0925a32-60b1-4bc7-8223-3d0347259d01)
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/7c8f8cd9-651d-40db-bb8a-54b2bbba39fd)

## subsoal f
Karena Lewis juga ingin track anime yang ditambah, diubah, dan dihapus. Maka dia membuat server dapat mencatat anime yang dihapus dalam sebuah log yang diberi nama change.log.
- Format: [date] [type] [massage]
- Type: ADD, EDIT, DEL
- Ex:
  1. [29/03/24] [ADD] Kanokari ditambahkan.
  2. [29/03/24] [EDIT] Kamis,Comedy,Kanokari,completed diubah menjadi Jumat,Action,Naruto,completed.
  3. [29/03/24] [DEL] Naruto berhasil dihapus.
```c
function_type function(type input /* ... */)
{
  FILE *log = fopen("../change.log", "a");
  if (log != NULL)
  {
    fprintf(log, "[%02d/%02d/%02d][TYPE] %s MESSAGE.\n", digitime->tm_mday, digitime->tm_mon + 1, digitime->tm_year + 1900, token);
    fclose(log);
  }
}
```
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/4e8c90f5-ec2b-4d0c-b94e-e67ae247921f)

## subsoal g
Koneksi antara client dan server tidak akan terputus jika ada kesalahan input dari client, cuma terputus jika user mengirim pesan “exit”. Program exit dilakukan pada sisi client.
```c
// client.c
int main(int argc, char **argv)
{
  // ...
  if (strcmp(buffer, "exit") == 0) exit(0);
  // ...
}
```
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/ef6c4f30-8d4f-4e73-a77d-0225eff31452)

## subsoal h
Hasil akhir:
soal_4/
    ├── change.log
    ├── client/
    │   └── client.c
    ├── myanimelist.csv
    └── server/
        └── server.c
![image](https://github.com/RickoMianto/Sisop-3-2024-MH-IT11/assets/143690594/4e1a4b87-5b15-4296-b303-a87808e45e09)

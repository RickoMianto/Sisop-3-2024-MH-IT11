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

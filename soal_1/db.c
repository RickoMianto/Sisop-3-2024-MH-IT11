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

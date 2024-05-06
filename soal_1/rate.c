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

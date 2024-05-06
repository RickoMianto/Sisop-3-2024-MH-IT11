#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

char* Gap(float distance);
char* Fuel(int fuelPercentage);
char* Tire(int tireUsage);
char* TireChange(char* currentTireType);

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

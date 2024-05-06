#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 4096

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
    printf("You: ");
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    send(sock, buffer, strlen(buffer), 0);

    if (strcmp(buffer, "exit") == 0) exit(0);
    memset(buffer, 0, sizeof(buffer));
    int bytesRcvd = read(sock, buffer, sizeof(buffer));
    if (bytesRcvd < 0) 
    {
      printf("Error receiving data\n");
      break;
    }

    printf("Server:\n%s\n", buffer);

  }
  printf("Exiting the client\n");
  close(sock);
  return 0;
}

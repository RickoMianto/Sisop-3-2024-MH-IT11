#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 4096

void  downloadCSV(), processCommand(char *buffer, char *response);

void show (char *token, char *response),  add  (char *token, char *response), edit  (char *token, char *response), delete(char *token, char *response), showAll(char *response);
int  type (char *token);

time_t runtime;
struct tm *digitime;

int main(int argc, char const **argv) 
{
  runtime = time(NULL);
  digitime = localtime(&runtime);

  int servSock, cliSock;
  struct sockaddr_in servAddr, cliAddr;
  socklen_t addrSize;
  int addrLen = sizeof(addrSize);
  char buffer[MAX_BUFFER_SIZE];
  char response[MAX_BUFFER_SIZE];

  downloadCSV();

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
      memset(buffer, 0, sizeof(buffer));
      memset(response, 0, sizeof(response));

      read(cliSock, buffer, sizeof(buffer));

      printf("received: %s\n", buffer);

      if (strcmp(buffer, "exit") == 0) 
      {
        printf("Closing connection\n");
        exit(0);
      }

      processCommand(buffer, response);
      send(cliSock, response, strlen(response), 0);
    } close(cliSock);
  } close(servSock);
  return 0;
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

  FILE *log = fopen("../change.log", "a");
  if (log != NULL)
  {
    fprintf(log, "[%02d/%02d/%02d][ADD] %s ditambahkan.\n", digitime->tm_mday, digitime->tm_mon + 1, digitime->tm_year + 1900, token);
    fclose(log);
  }
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

  FILE *log = fopen("../change.log", "a");
  if (log != NULL)
  {
    fprintf(log, "[%02d/%02d/%02d][EDIT] %s diubah menjadi %s.\n", digitime->tm_mday, digitime->tm_mon + 1, digitime->tm_year + 1900, old, new);
    fclose(log);
  }

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

  FILE *log = fopen("../change.log", "a");
  if (log != NULL)
  {
    fprintf(log, "[%02d/%02d/%02d][DEL] %s berhasil dihapus.\n", digitime->tm_mday, digitime->tm_mon + 1, digitime->tm_year + 1900, token);
    fclose(log);
  }

  pclose(fp);
}

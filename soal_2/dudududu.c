#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <string.h> 
#include <sys/wait.h> 
#include <time.h>
#include <ctype.h>

int strToInt(char *input), operation(int firstInput, char **argv, int secondInput);
char * intToStr(int result);
char * whenError(char **argv);
void output(char *firstInput, char **argv, char *secondInput, char *result);

int main(int argc, char const **argv) 
{ 
  char *opt, *lowOpt;
  if (argc != 2) exit(EXIT_FAILURE);
  if (strcmp(argv[1], "-kali") == 0) { opt = "KALI"; lowOpt = "kali"; }
  else if (strcmp(argv[1], "-tambah") == 0) { opt = "TAMBAH"; lowOpt = "tambah"; }
  else if (strcmp(argv[1], "-kurang") == 0) { opt = "KURANG"; lowOpt = "kurang"; }
  else if (strcmp(argv[1], "-bagi") == 0) { opt = "BAGI"; lowOpt = "bagi"; }
  else exit(EXIT_FAILURE);

  time_t runtime;
  struct tm *digitime;
  runtime = time(NULL);
  digitime = localtime(&runtime);

	int fd1[2]; 
	int fd2[2]; 

	char firstInput[1024], secondInput[1024];

	pid_t p; 

	if (pipe(fd1) == -1) 
	{ 
		fprintf(stderr, "Pipe Failed" ); 
		return 1; 
	} 

	if (pipe(fd2) == -1) 
	{ 
		fprintf(stderr, "Pipe Failed" ); 
		return 1; 
	} 

  scanf("%s %s", firstInput, secondInput);
  while((getchar()) != '\n');

	p = fork(); 

	if (p < 0) 
	{ 
		fprintf(stderr, "fork Failed" ); 
		return 1; 
	} 

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

    FILE *log = fopen("history.log", "a");
    if (log != NULL)
    {
      if (strcmp(resultStr, "ERROR") == 0) fprintf(log, "[%02d/%02d/%02d %02d:%02d:%02d][%s] %s pada %s.\n", digitime->tm_mday, digitime->tm_mon + 1, digitime->tm_year + 1900, digitime->tm_hour, digitime->tm_min, digitime->tm_sec, opt, resultStr, whenError(argv));
      else fprintf(log, "[%02d/%02d/%02d %02d:%02d:%02d][%s] %s %s %s sama dengan %s.\n", digitime->tm_mday, digitime->tm_mon + 1, digitime->tm_year + 1900, digitime->tm_hour, digitime->tm_min, digitime->tm_sec, opt, firstInput, lowOpt, secondInput, resultStr);
      fclose(log);
    }
  } 

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

char * whenError(char **argv)
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
      return "perkalian";
    case 2:
      return "penjumlahan";
    case 3:
      return "pengurangan";
    case 4:
      return "pembagian";
    default:
      exit(EXIT_FAILURE);
  }
}

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
      return firstInput - secondInput;
    case 4:
      return firstInput / secondInput;
    default:
      exit(EXIT_FAILURE);
  }
}

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

// if it's stupid, but it works, it ain't stupid
char * intToStr(int result)
{
  if (result == 1) return "satu";
  else if (result == 2) return "dua";
  else if (result == 3) return "tiga";
  else if (result == 4) return "empat";
  else if (result == 5) return "lima";
  else if (result == 6) return "enam";
  else if (result == 7) return "tujuh";
  else if (result == 8) return "delapan";
  else if (result == 9) return "sembilan";
  else if (result == 10) return "sepuluh";
  else if (result == 11) return "sebelas";
  else if (result == 12) return "dua belas";
  else if (result == 13) return "tiga belas";
  else if (result == 14) return "empat belas";
  else if (result == 15) return "lima belas";
  else if (result == 16) return "enam belas";
  else if (result == 17) return "tujuh belas";
  else if (result == 18) return "delapan belas";
  else if (result == 19) return "sembilan belas";
  else if (result == 20) return "dua puluh";
  else if (result == 21) return "dua puluh satu";
  else if (result == 22) return "dua puluh dua";
  else if (result == 23) return "dua puluh tiga";
  else if (result == 24) return "dua puluh empat";
  else if (result == 25) return "dua puluh lima";
  else if (result == 26) return "dua puluh enam";
  else if (result == 27) return "dua puluh tujuh";
  else if (result == 28) return "dua puluh delapan";
  else if (result == 29) return "dua puluh sembilan";
  else if (result == 30) return "tiga puluh";
  else if (result == 31) return "tiga puluh satu";
  else if (result == 32) return "tiga puluh dua";
  else if (result == 33) return "tiga puluh tiga";
  else if (result == 34) return "tiga puluh empat";
  else if (result == 35) return "tiga puluh lima";
  else if (result == 36) return "tiga puluh enam";
  else if (result == 37) return "tiga puluh tujuh";
  else if (result == 38) return "tiga puluh delapan";
  else if (result == 39) return "tiga puluh sembilan";
  else if (result == 40) return "empat puluh";
  else if (result == 41) return "empat puluh satu";
  else if (result == 42) return "empat puluh dua";
  else if (result == 43) return "empat puluh tiga";
  else if (result == 44) return "empat puluh empat";
  else if (result == 45) return "empat puluh lima";
  else if (result == 46) return "empat puluh enam";
  else if (result == 47) return "empat puluh tujuh";
  else if (result == 48) return "empat puluh delapan";
  else if (result == 49) return "empat puluh sembilan";
  else if (result == 50) return "lima puluh";
  else if (result == 51) return "lima puluh satu";
  else if (result == 52) return "lima puluh dua";
  else if (result == 53) return "lima puluh tiga";
  else if (result == 54) return "lima puluh empat";
  else if (result == 55) return "lima puluh lima";
  else if (result == 56) return "lima puluh enam";
  else if (result == 57) return "lima puluh tujuh";
  else if (result == 58) return "lima puluh delapan";
  else if (result == 59) return "lima puluh sembilan";
  else if (result == 60) return "enam puluh";
  else if (result == 61) return "enam puluh satu";
  else if (result == 62) return "enam puluh dua";
  else if (result == 63) return "enam puluh tiga";
  else if (result == 64) return "enam puluh empat";
  else if (result == 65) return "enam puluh lima";
  else if (result == 66) return "enam puluh enam";
  else if (result == 67) return "enam puluh tujuh";
  else if (result == 68) return "enam puluh delapan";
  else if (result == 69) return "enam puluh sembilan";
  else if (result == 70) return "tujuh puluh";
  else if (result == 71) return "tujuh puluh satu";
  else if (result == 72) return "tujuh puluh dua";
  else if (result == 73) return "tujuh puluh tiga";
  else if (result == 74) return "tujuh puluh empat";
  else if (result == 75) return "tujuh puluh lima";
  else if (result == 76) return "tujuh puluh enam";
  else if (result == 77) return "tujuh puluh tujuh";
  else if (result == 78) return "tujuh puluh delapan";
  else if (result == 79) return "tujuh puluh sembilan";
  else if (result == 80) return "delapan puluh";
  else if (result == 81) return "delapan puluh satu";
  else if (result == 82) return "delapan puluh dua";
  else if (result == 83) return "delapan puluh tiga";
  else if (result == 84) return "delapan puluh empat";
  else if (result == 85) return "delapan puluh lima";
  else if (result == 86) return "delapan puluh enam";
  else if (result == 87) return "delapan puluh tujuh";
  else if (result == 88) return "delapan puluh delapan";
  else if (result == 89) return "delapan puluh sembilan";
  else if (result == 90) return "sembilan puluh";
  else if (result == 91) return "sembilan puluh satu";
  else if (result == 92) return "sembilan puluh dua";
  else if (result == 93) return "sembilan puluh tiga";
  else if (result == 94) return "sembilan puluh empat";
  else if (result == 95) return "sembilan puluh lima";
  else if (result == 96) return "sembilan puluh enam";
  else if (result == 97) return "sembilan puluh tujuh";
  else if (result == 98) return "sembilan puluh delapan";
  else if (result == 99) return "sembilan puluh sembilan";
  else if (result == 100) return "seratus";
  else if (result == 0) return "nol";
  else return "ERROR";
}

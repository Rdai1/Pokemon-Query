/* Includes */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//infostruct for server
typedef struct Info{
	FILE *pokemon_csv_file;
	char *type;
	int mySocket;
	
}Info;

//method declarations 
void *read_pokemon(void *arg);
int isType(char *line, char *type, char *separator);

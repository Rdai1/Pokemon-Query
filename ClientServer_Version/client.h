/* Includes */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Defines */
#define MAXLENGTH 32

//pokemon structure
typedef struct PokemonType {
	int number;
	char name[MAXLENGTH];
	char type1[MAXLENGTH];
	char type2[MAXLENGTH];
	int total;
	int hp;
	int attack;
	int defense;
	int spAtk;
	int spDef;
	int speed;
	int generation;
	char legendary[MAXLENGTH];
} Pokemon;

//NodeType struct that is used for our pokemon linkList
typedef struct Node{
	Pokemon *data;
	struct Node *next;
} NodeType;

//NodeType struct that is used for our string linkList

typedef struct StringType {
  char fileName[MAXLENGTH];
  struct StringType *next;
} StringType;

//info struct for client has more attributes
typedef struct Info{
	FILE *fileToSave;
	NodeType volatile *pokemons;
	char *type;
	int numOfQueries;
	sem_t mutex;
	int mySocket;
	int exit;
	
}Info;

void *readPokemon(void *arg);
void line_to_pokemon(char *line, Pokemon **new_Pokemon, char *separator);
void *write_pokemon(void* arg);
void addPokemon(NodeType **list, Pokemon *pokemon);
void addString(StringType **list, char *fileName);
void addList(NodeType volatile **list1, NodeType **list2);
void printFileNames(StringType *strPtr);
void cleanup(NodeType *head);

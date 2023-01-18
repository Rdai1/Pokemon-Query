/* Includes */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

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

typedef struct Info{
	FILE *pokemon_csv_file;
	FILE *fileToSave;
	NodeType volatile *pokemons;
	char *type;
	int numPokemon;
	int numOfQueries;
	sem_t mutex;
}Info;

void *read_pokemon(void *arg);
void line_to_pokemon(char *line, Pokemon **new_Pokemon, char *separator);
int isType(char *line, char *type, char *separator);
void *write_pokemon(void* arg);
void addPokemon(NodeType **list, Pokemon *pokemon);
void addString(StringType **list, char *fileName);
void addList(NodeType volatile **list1, NodeType **list2);
void printFileNames(StringType *strPtr);
void cleanup(NodeType *head);

//main function
int main(){
	Info *variables = (Info *) malloc(sizeof(Info));
	char *fileName;
	char *nameTosave;
	char option;
	variables->pokemons = NULL;
	variables->numOfQueries = 0;
	pthread_t reading, saving;
	StringType *newFileNames =NULL;
	StringType *currFileName;
	StringType *newName;
	
	if (sem_init(&variables->mutex, 0, 1) < 0){
		printf("Error: On emaphore init.\n");
		exit(1);
	}
	
	printf("Enter then name of the file containing the Pokemon descriptions:\n");
	
	while (1){
		scanf("%ms",&fileName);
		variables->pokemon_csv_file = fopen(fileName, "r");
		
		if (!variables->pokemon_csv_file) {
	    		printf("Pokemon file is not found. Please enter the name of the file again.\n");
	  	}else{
	  		printf("File to be opened: %s\n", fileName);
	  		break; 
	  	}
	}
	int x = 0;
	while (x == 0){
		printf("\nOptions: \na. Type search\nb. Save results\nc. Exit the program\n");
		getchar();
		scanf("%c",&option);
		
		switch (option){
			case 'a':
				printf("Gamer has selected a. Type Search\nWhat type do you want to search for:\n");
				scanf("%ms",&(variables->type));
				printf("ENTERED: %s\n",variables->type);
				pthread_create(&reading, NULL, read_pokemon, (void *)variables);
				//read_pokemon(pokemon_csv_file, &pokemons, type);
				break;
			case 'b':
				printf("Gamer has selected b. Save results\n");
				printf("Enter the name of file to be saved to:\n");
				while(!variables->fileToSave){
					scanf("%ms",&nameTosave);
					variables->fileToSave = fopen(nameTosave, "w");
					if (!variables->fileToSave){
						printf("Unable to create the new file. Please enter the name of the file again.");
					}
					addString(&newName, nameTosave);
					newName->next = newFileNames;
    					newFileNames = newName;
					free(nameTosave);
				}
				pthread_create(&saving, NULL, write_pokemon, (void *)variables);
				break;
			case 'c':
				printf("Gamer has selected c. Exit the program\n");
				x++;
				pthread_cancel(reading);
				pthread_cancel(saving);
				break;
		}
	}
	printf("NAME OF NEW FILES THAT WERE CREATED: ");
	for (currFileName = newFileNames; currFileName != NULL;  currFileName = currFileName->next) {
    		printf("%s ", currFileName->fileName);
  	}
  	printf("\n");
  	printf("Number of qs: %d", variables->numOfQueries);
	cleanup((void *)variables->pokemons);
	free(variables);
	free(variables->type);
	free(fileName);
	//fclose(variables->pokemon_csv_file);
	//fclose(variables->fileToSave);
	return EXIT_SUCCESS;
}

/*
 * Function: read_pokemon
 * Description: Reads csv lines from a file, creates PokemonType structures,
 *              dymamically grows the array that holds them.
 * Parameters:
 *   pokemon_csv: the pointer to the file to be read
 *   pokemon_ptr_array: pointer to the array that holds PokemonType structures
 * Returns:
 *   the number of pokemon read in from the file (header line is discarded)
*/
//FILE* pokemons_csv, NodeType **pokemon, char *type
void *read_pokemon(void *arg) {
	Info *vars = (Info *)arg;
	int num_lines = 0;
	Pokemon *newPokemon;
	NodeType *newPokemonList = NULL;
	char *line;
	
	while (fscanf(vars->pokemon_csv_file, "%m[^\n]\n", &line) != EOF) {
		if (isType(line, vars->type, ",") == 1){
			if (num_lines > 0) { /* Skip the header */
				line_to_pokemon(line, &newPokemon, ",");
				addPokemon(&newPokemonList, newPokemon);
			}
		}
		num_lines++;
		free(line);
	}
	if (sem_wait(&vars->mutex) < 0){
		printf("Error: on semaphore wait.\n");
		exit(1);
	}
	addList(&vars->pokemons, &newPokemonList);
	if (sem_post(&vars->mutex) < 0){
		printf("Error: on semaphore post.\n");
		exit(1);
	}

	rewind(vars->pokemon_csv_file);
	vars->numOfQueries++;
}

void addList(NodeType volatile **list1, NodeType **list2){
	NodeType volatile *currNode, *prevNode;
	

	prevNode = NULL;
	currNode = *list1;
	
	while (currNode != NULL) {
		//printf("%s\n",currNode->data->name);
		prevNode = currNode;
		currNode = currNode->next;
	}
	
	if (prevNode == NULL){
		*list1 = *list2;
	}
	else{
		prevNode->next = *list2;
	}
}

/*
* Function: line_to_pokemon
* Description: converts a csv line to a PokemonType data structure
* Parameters:
*    line: the line to be converted to a StudentType
*    new_pokemon: the new StudentType to be populated with data
*    separator: the character to use as the sparator, ',' by default
*/

void line_to_pokemon(char *line, Pokemon **new_Pokemon, char *separator){
	*new_Pokemon = (Pokemon *) malloc(sizeof(Pokemon));
	(*new_Pokemon)->number = atoi(strsep(&line, separator));
	strcpy((*new_Pokemon)->name, strsep(&line, separator));
	strcpy((*new_Pokemon)->type1, strsep(&line, separator));
	strcpy((*new_Pokemon)->type2, strsep(&line, separator));
	(*new_Pokemon)->total = atoi(strsep(&line, separator));
	(*new_Pokemon)->hp = atoi(strsep(&line, separator));
	(*new_Pokemon)->attack = atoi(strsep(&line, separator));
	(*new_Pokemon)->defense = atoi(strsep(&line, separator));
	(*new_Pokemon)->spAtk = atoi(strsep(&line, separator));
	(*new_Pokemon)->spDef = atoi(strsep(&line, separator));
	(*new_Pokemon)->speed = atoi(strsep(&line, separator));
	(*new_Pokemon)->generation = atoi(strsep(&line, separator));
	strcpy((*new_Pokemon)->legendary, strsep(&line, separator));
}

/*
* Function: isType
* Description: finds if the pokemon has the same type given
* Parameters:
*    line: the line to be searched
*    type: the type that is wanted
*    separator: the character to use as the sparator, ',' by default
*Returns:
*	1 - type matches
*	0 - type doesn't match
*/
int isType(char *line, char *type, char *separator){
	char *temp = strdup(line); //dupped so orginal strings doesn't get affected
	char *temp2 = temp;
	strsep(&temp, separator);
	strsep(&temp, separator);
	//printf("Made it in Function");
	if (strcasecmp(strsep(&temp, separator),type) == 0){
		free(temp2);
		return 1;
	}
	free(temp2);
	return 0;
}

/*
* Function: write_pokemon
* Description:
*   writes the contents of the pokemon array to either the terminal, or a file
*   when writing to a file - no header is inserted, only the data are written
* Parameters:
*   pokemons_array: the array that holds the StudentType data structures in memory
*   number_of_students: the number of students held in the StudentType array
*/

void *write_pokemon(void *arg){
	Info *vars = (Info *)arg;
	NodeType *currNode = (void *)vars->pokemons;
	NodeType *prevNode = NULL;
	
	while (currNode != NULL){
		Pokemon *tempPokemon = currNode->data;
		
		fprintf(vars->fileToSave, "%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s\n", tempPokemon->number,tempPokemon->name,tempPokemon->type1,tempPokemon->type2,tempPokemon->total,tempPokemon->hp,tempPokemon->attack,tempPokemon->defense,tempPokemon->spAtk,tempPokemon->spDef,tempPokemon->speed,tempPokemon->generation,tempPokemon->legendary);
		
		prevNode = currNode;
    		currNode = currNode->next;
	
	}
	vars->fileToSave = NULL;
	vars->numOfQueries++;
}

/*
*Purpose: Add an intialzied pokemon to a linkedList
*Input: A linkedlist, a payload to add to list
*output/return: none
*
*if list is empty set head as newNode
*else loop until end of list and set the next node as newNode
*/
void addPokemon(NodeType **list, Pokemon *pokemon){
	NodeType *newNode;
	NodeType *currNode, *prevNode;
	
	newNode = (NodeType *) malloc(sizeof(NodeType));
	newNode->data = pokemon;
	newNode->next = NULL;
	
	prevNode = NULL;
	currNode = *list;
	
	while (currNode != NULL) {
		prevNode = currNode;
		currNode = currNode->next;
	}
	
	if (prevNode == NULL){
		*list = newNode;
	}
	else{
		prevNode->next = newNode;
	}
	//could remove
	//newNode->next = currNode;
}

void addString(StringType **list, char *fileName){
	*list = (StringType*) malloc(sizeof(StringType));
	strcpy((*list)->fileName,  fileName);
	(*list)->next = NULL;
}

/*
*Purpose: free dynamically allocated memory from linklist
*Input: The head node of a list
*
*Loop through list and free data and node
*/
void cleanup(NodeType *head){
	NodeType *currNode;
	NodeType *nextNode;
	
	currNode = head;
	
	//loops until the end of list amd free everything
	while (currNode != NULL){
		free(currNode->data);
		nextNode = currNode->next;
		free(currNode);
		currNode = nextNode;
	}
}

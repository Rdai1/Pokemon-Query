#include "client.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 80

void *readPokemon(void *arg);
void printStuList(NodeType volatile *list);

int main(){
	//int mySocket;
	struct sockaddr_in addr;
	int i, bytesRcv;
	
	//buffers
	char buffer[80];
	char option;
	char *type;
	char *nameToSave;
	char *endClient = "done";
	
	//declaring struct and threads
	Info *variables = (Info *) malloc(sizeof(Info));
	variables->pokemons = NULL;
	variables->numOfQueries = 0;
	variables->exit = 0;
	pthread_t reading, saving;
	
	//string linklist to store names of files created
	StringType *newFileNames =NULL;
	StringType *currFileName;
	StringType *newName;
	
	//initializing semaphore
	if (sem_init(&variables->mutex, 0, 1) < 0){
		printf("Error: On semaphore init.\n");
		exit(1);
	}
	
	//creating sockets
	variables->mySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  	if (variables->mySocket < 0) {
    		printf("eek! couldn't open socket\n");
    		close(variables->mySocket);
    		exit(-1);
  	}
  	
  	//setup address
  	memset(&addr, 0, sizeof(addr));
  	addr.sin_family = AF_INET;
  	addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  	addr.sin_port = htons((unsigned short) SERVER_PORT);
	
	//connect to server 
	i = connect(variables->mySocket, 
              (struct sockaddr *) &addr,
              sizeof(addr));
  	if (i<0) {
   		printf("client could not connect!\n");
    		exit(-1);
  	}
  	
  	while (1){
  		//priting menu and getting option
  		printf("\nOptions: \na. Type search\nb. Save results\nc. Exit the program\n");
  		scanf("%c", &option);
  		printf("CHOICE IS: %c\n", option);
  		
		//condition to read pokemon based on type
  		if ('a' == option){
  			printf("Gamer has selected a. Type Search\nWhat type do you want to search for:\n");
  			scanf("%ms",&(variables->type));
  			printf("ENTERED: %s\n",variables->type);
  			pthread_create(&reading, NULL, readPokemon, (void *)variables);
  		}
  		//confiditon to save pokemon 
  		else if ('b' == option){
  			printf("Gamer has selected b. Save results\n");
			printf("Enter the name of file to be saved to:\n");
			scanf("%ms",&nameToSave);
			variables->fileToSave = fopen(nameToSave, "w");
			while(!variables->fileToSave){
					printf("INSIDE WHILE LOOOOOOOOP");
					if (!variables->fileToSave){
						printf("Unable to create the new file. Please enter the name of the file again.");
					}
					scanf("%ms",&nameToSave);
					variables->fileToSave = fopen(nameToSave, "w");
					
			}
			addString(&newName, nameToSave);
			newName->next = newFileNames;
    			newFileNames = newName;
			free(nameToSave);	
			pthread_create(&saving, NULL, write_pokemon, (void *)variables);
  		}
  		//condition to close the program
  		else if ('c' == option){
  			printf("Gamer has selected c. Exit the program\n");
  			variables->exit = 1;
  			break;
  		}
  		getchar();
  	}
  	//send the server an end message
  	send(variables->mySocket, endClient, strlen(endClient), 0);
  	
  	//outputs name of all files created and frees them
	printf("NAME OF NEW FILES THAT WERE CREATED: ");
  	currFileName = newFileNames;
  	while (currFileName != NULL){
  		StringType *tempNext;
  		printf("%s ", currFileName->fileName);
		tempNext = currFileName->next;
		free(currFileName);
		currFileName = tempNext;
	}

  	printf("\n");
  	printf("Number of qs: %d\n", variables->numOfQueries);
  	pthread_join(reading , NULL);
  	pthread_join(saving , NULL);
  	cleanup((void *)variables->pokemons);
  	/* close the socket */
	close(variables->mySocket);
	free(variables->type);
	free(variables);
	printf("CLIENT: Shutting down.\n");
  	return EXIT_SUCCESS;
}

void *readPokemon(void *arg){
	Info *vars = (Info *)arg;
	char typeBuffer[10];
	char lineBuffer[90];
	char *line;
	int bytesRcv;
	Pokemon *newPokemon;
	NodeType *newPokemonList = NULL;
	
	//need semaphore here because 2 searches will break the program
	//also don't need 2 searches going on at the same time it would just be queued
	if (sem_wait(&vars->mutex) < 0){
		printf("Error: on semaphore wait.\n");
		exit(1);
	}
	
	//send type to server
	strcpy(typeBuffer, vars->type);
	//printf("CLIENT: Sending \"%s\" to server.\n", typeBuffer);
	send(vars->mySocket, typeBuffer, strlen(typeBuffer), 0);
	
	//getting response from server intial check
	bytesRcv = recv(vars->mySocket, lineBuffer, 3, 0);
	lineBuffer[bytesRcv] = 0;
	//printf("CLIENT: Got back response \"%s\" from server.\n", lineBuffer);

	while (1){
		bytesRcv = recv(vars->mySocket, lineBuffer, 90, 0);
		lineBuffer[bytesRcv] = 0;
		//printf("CLIENT: POKEMON SENT: \"%s\" from server.\n", lineBuffer);
		if (strcmp(lineBuffer, "END") == 0){
			//printf("BREAKING LOOP");
			break;
		}else if (vars->exit == 1){
			//printf("CLOSE TIMEEEEEEEEEE\n");
			strcpy(typeBuffer, "done");
			//printf("CLIENT: sent %s\n", typeBuffer);
			send(vars->mySocket, typeBuffer, strlen(typeBuffer), 0);
			bytesRcv = recv(vars->mySocket, typeBuffer, 4, 0);
			//printf("CLIENT: server sennt: %s\n", typeBuffer);
			strcpy(typeBuffer, "done");
			send(vars->mySocket, typeBuffer, strlen(typeBuffer), 0);
			pthread_exit(NULL);
		}
		line = lineBuffer;
		line_to_pokemon(line, &newPokemon, ",");
		addPokemon(&newPokemonList, newPokemon);
		strcpy(typeBuffer, "OK");
		send(vars->mySocket, typeBuffer, strlen(typeBuffer), 0);
	}
	//printStuList(newPokemonList);
	addList(&vars->pokemons, &newPokemonList);
	
	if (sem_post(&vars->mutex) < 0){
		printf("Error: on semaphore post.\n");
		exit(1);
	}
	vars->numOfQueries++;
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
	
	//while loop goes through and prints it all into a file
	while (currNode != NULL){
		if (vars->exit == 1){
			pthread_exit(NULL);
		}
		Pokemon *tempPokemon = currNode->data;
		
		fprintf(vars->fileToSave, "%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s\n", tempPokemon->number,tempPokemon->name,tempPokemon->type1,tempPokemon->type2,tempPokemon->total,tempPokemon->hp,tempPokemon->attack,tempPokemon->defense,tempPokemon->spAtk,tempPokemon->spDef,tempPokemon->speed,tempPokemon->generation,tempPokemon->legendary);
		
		prevNode = currNode;
    		currNode = currNode->next;
	
	}
	vars->numOfQueries++;
	fclose(vars->fileToSave);
	printf("CLOSED FILE");
}

//delete this later only for testing purposes
void printStuList(NodeType volatile *list)
{
  NodeType volatile *currNode = list;
  NodeType volatile *prevNode = NULL;

  printf("FORWARD:\n");
  while (currNode != NULL) {
    printf("POKEMON NAME: %s\n",currNode->data->name);
    prevNode = currNode;
    currNode = currNode->next;
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

//adds a string to the list
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

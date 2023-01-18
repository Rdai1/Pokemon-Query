#include "server.h"

#define SERVER_PORT 80

//going to be the stuff that searches and saves and etc

int main()
{
	//variables 
	int myListenSocket;
	struct sockaddr_in  myAddr, clientAddr;
	int i, addrSize, bytesRcv;
	char *response = "OK";
	char temp[80];
	char *fileName;
	FILE *pokemon_csv_file;
	char buffer[30];
	Info *variables = (Info *) malloc(sizeof(Info));
	pthread_t reading;
	
	/* create socket */
	myListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (myListenSocket < 0) {
		printf("eek! couldn't open socket\n");
		perror("socket Error");
		exit(-1);
  	}

	/* setup my server address */
	memset(&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddr.sin_port = htons((unsigned short) SERVER_PORT);

	/* bind my listen socket */
	i = bind(myListenSocket, 
           	(struct sockaddr *) &myAddr,
          	 sizeof(myAddr));
  	if (i < 0) {
		printf("eek! couldn't bind socket\n");
		exit(-1);
	}


	/* listen */
	i = listen(myListenSocket, 5);
	if (i < 0) {
		printf("eek! couldn't listen\n");
		exit(-1);
	}
	
	//trying to open the file
	printf("Enter then name of the file containing the Pokemon descriptions:\n");
	while (1){
		scanf("%ms",&fileName);
		variables->pokemon_csv_file = fopen(fileName, "r");
		
		if (!(variables->pokemon_csv_file)) {
	    		printf("Pokemon file is not found. Please enter the name of the file again.\n");
	  	}else{
	  		printf("File to be opened: %s\n", fileName);
	  		break; 
	  	}
	}
	free(fileName);


	/* wait for connection request */
	while (1){
		addrSize = sizeof(clientAddr);

		variables->mySocket = accept(myListenSocket,
                        		(struct sockaddr *) &clientAddr,
                        		&addrSize);
		if (variables->mySocket < 0) {
			printf("eek! couldn't accept the connection\n");
			exit(-1);
		}
		printf("SERVER: Received client connection.\n");
		
		/* read message from client and do something with it */
		while (1) {
			bytesRcv = recv(variables->mySocket, buffer, sizeof(buffer), 0);
			buffer[bytesRcv] = 0;
			if((strcmp(buffer,"quit") == 0) || (strcmp(buffer,"done") == 0))
				break;
			//printf("The type that client sent: %s\n", buffer);
			variables->type = buffer;
			//respond with ok message
			//printf("SERVER: Sending \"%s\" to client\n", response);
			send(variables->mySocket, response, strlen(response), 0);
			
			//call method to read
			read_pokemon(variables);
			
		}
		printf("SERVER: Closing client connection.\n");
 		close(variables->mySocket);
		if(strcmp(buffer,"quit") == 0)
			break;
		
	}
	
	/* close sockets */
	close(myListenSocket);
	fclose(variables->pokemon_csv_file);
	free(variables);
	printf("SERVER: Shutting down.\n");
	return 0;
}

/*
 * Function: read_pokemon
 * Description: Reads csv lines from a file, creates PokemonType structures,
 *              dymamically grows the array that holds them.
 * Parameters:
 *   
 *  
 * 
 *
*/
//FILE* pokemons_csv, NodeType **pokemon, char *type

//NOTE: ONLY NEEDS TO TAKE IN TYPE BECAUSE NOT ACTUALLY ADDING TO A LINKLIST IN SERVER
//int clientSocket, char *type, FILE *pokemon_csv_file
void *read_pokemon(void *arg) {
	Info *vars = (Info *)arg;
	int num_lines = 0;
	int bytesRcv;
	char *line;
	char buffer[80];
	char temp[10];
	//printf("INSIDE READ POKEMON\n");
	
	while (fscanf(vars->pokemon_csv_file, "%m[^\n]\n", &line) != EOF) {
		//printf("Current line: %s\n", line);
		if (num_lines > 0) { /* Skip the header */
			if (isType(line, vars->type, ",") == 1){
				//printf("SERVER: sent %s\n", line);
				send(vars->mySocket, line, strlen(line), 0);
				bytesRcv = recv(vars->mySocket, buffer, sizeof(buffer), 0);
				buffer[bytesRcv] = 0;
				//printf("SERVER: client sent: %s\n", buffer);
				if((strcmp(buffer,"quit") == 0) || (strcmp(buffer,"done") == 0))
					break;
				//printf("CLIENT SENT:  %s\n", buffer);
			}
		//usleep(5000);
		}
		num_lines++;
		free(line);
	}
	//printf("finished LOOP\n");
	strcpy(temp, "END");
	//printf("SERVER: sent %s\n", temp);
	send(vars->mySocket, temp, strlen(temp), 0);
	rewind(vars->pokemon_csv_file);
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
	if (strcasecmp(strsep(&temp, separator),type) == 0){
		free(temp2);
		return 1;
	}
	free(temp2);
	return 0;
}

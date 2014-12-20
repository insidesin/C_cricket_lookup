/* ---------------------------------------------------------------------------
** File: Server.c
** Description: The server allows for cricket fans to query the statistics 
** about the bating figures for a cricket test between Australia and West Indies. 
** The statistics about the test are stored in a text file, namely Batting.txt, 
** which will be stored on the server. Users must login before being accepted 
** permission to search the files, logins can be found in the file 
** Authentication.txt. All searches are tracked on this server instance 
** and counted and displayed on the server console.
**
** Author: Jackson Powell - n8600571
** -------------------------------------------------------------------------*/

#include "Server.h"

int main(int argc, char *argv[]) {
	int sockfd, new_fd_int, i;  /* listen on sock_fd, new connection on new_fd */
	struct sockaddr_in my_addr;    /* my address information */
	struct sockaddr_in their_addr; /* connector's address information */
	struct sockaddr_in new_fd; /* connector's address information */
	socklen_t sin_size;
	
	clientsConnected = 0; //set amount of connections to 0 before accepting new ones.
	
	/* generate the socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* generate the end point */
	my_addr.sin_family = AF_INET;         /* host byte order */
	my_addr.sin_port = htons(MYPORT);     /* short, network byte order */
	my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */

	/* bind the socket to the end point */
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	/* start listnening */
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("Server is now listening ...\n");

	/* repeat: accept, send, close the connection */
	/* for every accepted connection, use a sepetate thread to serve it */
	while(TRUE) {  /* main accept() loop */
	
		sin_size = sizeof(struct sockaddr_in);
		
		//If the connections are over limit, automatically close connection
		//precisely after accepting it.
		if(clientsConnected >= CONNECTIONS_LIMIT) {
			continue;
		}
	
		if ((new_fd_int = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("accept");
			continue;
		}
		
		printf("Server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
		clientsConnected++;
		
		//Allocate new memory for passing towards thread use.
		struct new_sock *new_sock = malloc(sizeof(struct new_sock));
		new_sock->id = new_fd_int;

		pthread_t connect_tid;
		pthread_create(&connect_tid, NULL, newConnection, new_sock); //Create new newConnection Thread

		while(waitpid(-1,NULL,WNOHANG) > 0); /* clean up child processes */
	}
}

void *newConnection(void *param) {
    struct new_sock *socket = param;
	
	while(TRUE) {
		
		char name_input[MAX_INPUT_LENGTH];
		char user[MAX_INPUT_LENGTH];
		char pass[MAX_INPUT_LENGTH];
		
		int numbytes = recv(socket->id, name_input, sizeof(char*), 0);
			numbytes += recv(socket->id, user, sizeof(char*), 0);
			numbytes += recv(socket->id, pass, sizeof(char*), 0);
		
		//Retrieve name from client communication.
		if(numbytes == -1 || numbytes == 0) {
			perror("Client exited? Socket Closed");
			break;
		}
		
		char *retrievedInfo = (char *)malloc(MAX_INPUT_LENGTH * sizeof(char*));
		//Check the authentication file for proper user auths to check against.
		if (searchFile("Authentication.txt", user, retrievedInfo) != 1) {
			char *errorMessage = "\nIncorrect user/pass or no permission!\n";
			if(send(socket->id, errorMessage, strlen(errorMessage), 0) == -1)
				perror("Error sending permission error to client!");
			continue;
		}
		
		//Check the auth against the user entered information.
		if(checkAuth(retrievedInfo, user, pass) == -1) {
			char *errorMessage = "\nIncorrect user/pass or no permission!\n";
			if(send(socket->id, errorMessage, strlen(errorMessage), 0) == -1)
				perror("Error sending permission error to client!");
			continue;
		}
		
		batterInfo = (char **)malloc(MAX_INPUT_LENGTH * sizeof(char *));
		
		if (searchFile("Batting.txt", name_input, retrievedInfo) == 1) {
			
			tokenizeLine(retrievedInfo);
			
			//Format text for sending
			char displayLine[250];
			sprintf(displayLine, 
			"\n\"%s\" scored a total of %s runs.\n	\
			\n\tMore details for %s:-\n \
			\n\tCountry:\t %s \
			\n\tAverage:\t %.3f \
			\n\tHigh Score:\t %s \n",
			name_input, batterInfo[RUNS_INDEX], batterInfo[NAME_INDEX],
			batterInfo[COUNTRY_INDEX], (strtod(batterInfo[RUNS_INDEX], NULL) / 
			strtod(batterInfo[INNINGS_INDEX], NULL)), batterInfo[HISCORE_INDEX]);
			
			//Send each line to the client.
			if(send(socket->id, displayLine, strlen(displayLine), 0) == -1) 
				perror("Error sending information to client!");
			
			//Save the batter search and display it.
			saveBatterSearch();
			
		} else {
			char errorMessageLine[50];
			sprintf(errorMessageLine, "\"%s\" not found amongst the batting figures. \
			\nPlease check your spelling and try again.\n", name_input);
							
			if(send(socket->id, errorMessageLine, strlen(errorMessageLine), 0) == -1)
				perror("Error sending name errorMessage to client!");
		}
				
		free(batterInfo);
		free(retrievedInfo);
	}
	
	perror("Connection finished!");
	close(socket->id);
	free(socket);
	clientsConnected--;
}

int searchFile(char *fname, char *search, char *output) {
	FILE *fp;
	int line_num = 1;
	int i;
	
	if((fp = fopen(fname, "r")) == NULL)
		return(-1); //Return file not existant (-1)
	if(strlen(search) < 3)
		return(0); //Omit small readings
	
	char *str_check = (char *)malloc(MAX_INPUT_LENGTH * sizeof(char *));
	char case_check[strlen(search)];
	
	while(fgets(str_check, 512, fp) != NULL) { //Full line fetched.
		//Get the first letters (name)
		for(i = 0; i < strlen(search); i++) 
			case_check[i] = str_check[i];
		//If the strings match, ignoring case, copy to output.
		if(strncasecmp(case_check, search, strlen(search)) == 0) {
			strcpy(output, str_check);
			free(str_check);
			return(1);
		} 
		line_num++;
	}
	
	free(str_check);
	
	//Close the file if still open.
	if(fp)
		fclose(fp);
	
   	return(0); //Not found (0)
}

void tokenizeLine(char *line) {
    char *str_ptr = NULL;
    int num_tokens = 0;

	//Iterate through, tokenizing segments by delimiters of tab spacing.
    for(str_ptr = strtok(line, "\t"); str_ptr != NULL; 
		str_ptr = strtok(NULL, "\t")) {
		batterInfo[num_tokens] = strdup(str_ptr);
        num_tokens++;
    }
} 

void saveBatterSearch() {
	struct searched_batter searchedNames[MAX_PLAYER_SAVES];
	int i;
	for(i = 0; i < MAX_PLAYER_SAVES; i++) {
		//If it comes across an empty slot before finding the name, inserts name.
		if(strlen(searchedNames[i].name) == 0) {
			strcpy(searchedNames[i].name, batterInfo[NAME_INDEX]);
			searchedNames[i].searchCount = 1;
			break;
		//else it came across the batter and added to it's count.
		} else if(strncasecmp(searchedNames[i].name, batterInfo[NAME_INDEX], 
			strlen(batterInfo[NAME_INDEX])) == 0) {
			searchedNames[i].searchCount++;
			break;
		}
	}
	
	printf("%s has been searched %d times.\n", 
		searchedNames[i].name, searchedNames[i].searchCount);
}

int checkAuth(char *input, char *user, char *pass) {
	
	char realUser[100];
	char realPass[100];
	
	sscanf(input, "%s %s", realUser, realPass);
	
	if(strncasecmp(user, realUser, strlen(realUser)) == 0) {
		if(strncasecmp(pass, realPass, strlen(realPass)) == 0) {
			return 0;
		}
	} 
	
    return -1;
} 

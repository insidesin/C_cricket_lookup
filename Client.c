/* ---------------------------------------------------------------------------
** File: Client.c
** Description: The client connects directly to the Server instance, allowing
** the user to submit the batter name he wants to search, followed by his 
** username and password in order to gain access. Once connected, the server
** will relay the information he requires back to his connection screen.
** Server allows a max of 30 connections, so if the client does not initiate,
** the server may be down or not responding to the connection.
**
** Author: Jackson Powell - n8600571
** -------------------------------------------------------------------------*/

#include "Client.h"

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buffer[MAX_COM_SIZE];
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information */

	if (argc != 2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}

	if ((he = gethostbyname(argv[1])) == NULL) {  /* get the host info */
		herror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;      /* host byte order */
	their_addr.sin_port = htons(PORT);    /* short, network byte order */
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

	if (connect(sockfd, (struct sockaddr *)&their_addr, \
	sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
	
	//While user has not selected to quit, checks for name to be entered
	//and then sends that name to the server.
	char *name = malloc(sizeof(char*));
	char *user = malloc(sizeof(char*));
	char *pass = malloc(sizeof(char*));
	
	while(TRUE) {
		
		printf("\nEnter a player's name -- or 'q' to quit-- > ");
		scanf("%s", name);
		
		if(name[0] == 'q' || name[0] == 'Q') {
			//Finish up because user wants to close.
			printf("\nThe client has terminated.\n");
			break;
		}
		
		printf("\nEnter username and password-- > ");
		scanf("%s %s", user, pass);
			
		if ((name != NULL) && (user != NULL) && (pass != NULL)) {
			//Send the user details to the server.
			if(send(sockfd, name, sizeof(char*), 0) == -1)
				perror("Error sending player name to server!");
			if(send(sockfd, user, sizeof(char*), 0) == -1)
				perror("Error sending user server!");
			if(send(sockfd, pass, sizeof(char*), 0) == -1)
				perror("Error sending pass to server!");
				
			
			//Read any response.
			numbytes = recv(sockfd, buffer, MAX_COM_SIZE, 0);
			if(numbytes == -1 || numbytes == 0) {
				perror("Connection error. Closed Socket");
				break;
			}
			buffer[numbytes] = '\0';
			
			//And write it to the command line.
			printf("%s", buffer);
		}
		
		free(name);
		free(user);
		free(pass);
	}
	
	close(sockfd);

	return 0;
}


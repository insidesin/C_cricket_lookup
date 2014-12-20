/* ---------------------------------------------------------------------------
** File: Server.h
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

#include <arpa/inet.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <unistd.h>
#include <pthread.h>

#define MYPORT 54321    /* the port users will be connecting to */
#define BACKLOG 10     /* how many pending connections queue will hold */

#define MAX_INPUT_LENGTH 30
#define MAX_COM_SIZE 250
#define MAX_PLAYER_SAVES 52
#define CONNECTIONS_LIMIT 30
#define TRUE 1
#define FALSE 0

//Batter statistical index constants:
#define NAME_INDEX 0
#define COUNTRY_INDEX 1
#define INNINGS_INDEX 2
#define RUNS_INDEX 3
#define NUM_OUT_INDEX 4
#define HISCORE_INDEX 5
#define HS_NOT_OUT_INDEX 6

#define TOTAL_PLAYER_STATS 7

//Structure for storing searched batters.
struct searched_batter {
	char name[MAX_INPUT_LENGTH];
	int searchCount;
};

//Structure for passing through information to connection threads.
struct new_sock {
	int id;
};

//Data structures hold both the information of a searched batter
//and the amount of times they've been searched on this server instance.
char **batterInfo;

//Total amount of connections.
int clientsConnected;

/* 
 * The main thread for each connection that handles all calls
 * to other functions in order to check the client connection
 * requests and attend to them if possible.
 * @param param Passes through neccesary data structure to
 * allow the socket to maintain it's connection id, state
 * and other information.
 */
void *newConnection(void *param);

/* 
 * Opens up the file apparent in fname and searches for a string
 * that matches the search parameter and then allocates it to
 * the array passed through as the output parameter.
 * @param fname The name of the file to search within.
 * @param search The string to be searched.
 * @param output The output of the matched string, which is
 * the line of text where the search string first appears.
 * @return -1 if the file does not exist, 0 if the search was
 * not found and 1 if the search found a match.
 */
int searchFile(char *fname, char *search, char *output);

/* 
 * Splits the input string into several smaller strings and inserts
 * them into the batterInfo[] array for future reference.
 * @param input A string that contains a line of text to tokenize.
 */
void tokenizeLine(char *line);

/* 
 * Iterates through the server's already searched batters and finds the
 * user's selection and increases it's search count. If the player is not
 * found in the system, or hasn't been searched before, it adds them
 * for future reference and counts the search.
 */
void saveBatterSearch();

/* 
 * Function that both picks out the usernames from the line
 * produced by a searchFile() call as well as checks it against the
 * user's input.
 * @param input A string that contains a line of text, in this case it is
 * the searched user's credentials.
 * @param user The username the user has inputted to check against server.
 * @param pass The pass the user has inputted to check against server.
 * @return 0 if the user is accepted, -1 if not
 */
int checkAuth(char *input, char *user, char *pass);


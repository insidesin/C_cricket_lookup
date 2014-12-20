/* ---------------------------------------------------------------------------
** File: Client.h
** Description: The client connects directly to the Server instance, allowing
** the user to submit the batter name he wants to search, followed by his 
** username and password in order to gain access. Once connected, the server
** will relay the information he requires back to his connection screen.
** Server allows a max of 30 connections, so if the client does not initiate,
** the server may be down or not responding to the connection.
**
** Author: Jackson Powell - n8600571
** -------------------------------------------------------------------------*/

#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0
#define MAX_INPUT_LENGTH 20
#define MAX_COM_SIZE 250

#define PORT 54321    /* the port client will be connecting to */

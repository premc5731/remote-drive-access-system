#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <errno.h>

#include <fcntl.h>      
#include <unistd.h>     
#include <sys/stat.h>

#include <sys/socket.h>     
#include <netinet/in.h>     
#include <arpa/inet.h>      


#define HEADER_SIZE 64
#define BF_SIZE 1024
#define CMD_SIZE 1024

//====================================================================================
// Functions Decelaration
//====================================================================================

// Function to receive message from server
int RecvMsg(int sock);

// Function to read Header
int ReadHeader(int sock, char *Buffer, int max);

// Function to receive file from server
int ReceiveFileFromServer(int sock, char * filename);

// Function to send file to server
int SendFileToServer(int sock, char * filename);

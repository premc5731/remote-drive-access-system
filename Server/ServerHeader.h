#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HEADER_SIZE 64
#define MAX_PATH 1024
#define MSG_SIZE 1024
#define BF_SIZE 4096
#define RWBF_SIZE 1024

//====================================================================================
// Functions Decelaration
//====================================================================================

// Function to Send message
void SendMsg(int sock, char * msg);

// Function to list all files
int ListFiles(int sock, char * path);

// Function to list all files with their sizes
int ListFileSizes(int sock, char * path);

// Function to display present working directory
int CurrentDirectory(int sock);

// Function to store absolute path of server's directory
int SetBaseDir();

// Function to change directory with restrictions
int ChangeDirectory(int sock, char * path);

// Function to create a new directory
int MakeDirectory(int sock, char * dirname);

// Function to delete an empty directory
int RemoveDirectory(int sock, char * dirname);

// Function to delete a file
int RemoveFile(int sock, char * filename);

// Function to send a file to client
int SendFileToClient(int ClientSocket , char * Filename);

// Function to read Header
int ReadHeader(int sock, char *Buffer, int max);

// Function to recive a file from client
int ReciveFileFromClient(int sock, char * filename);

// Function to Display Manual
void Manual(int ClientSocket, char * cmd);

// Function to Display list of all commands
void Help(int ClientSocket);


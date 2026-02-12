#include "./ClientHeader.h"

////////////////////////////////////////////////////
// 
//  Commandline Argument Application
//  1st Argument : Ip address
//  2nd Argument : Port number
//  ./client    127.0.0.1   9000    
//  argv[0]     argv[1]     argv[2] 
//
//  int argc = 3
//  
////////////////////////////////////////////////////

int main(int argc , char * argv[])
{
    int sock = 0, iRet = 0, port = 0;
    char * ip = NULL;
    struct sockaddr_in ClientAddr;  // client configuration struct 
    char * token[3];
    char cmd[CMD_SIZE] = {"\0"};
    char cmdcpy[CMD_SIZE] = {'\0'};

    if(argc != 3)
    {
        printf("ERROR : Unable to proceed as invalid number of arguments\n");
        printf("Please provide below arguments : \n");
        printf("1st Argument : Ip address\n2nd Argument : Port number\n");
        return -1;
    }

    // store command line arguments into the variables
    ip = argv[1];
    port = atoi(argv[2]);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        printf("ERROR: Unable to create socket\n");
        return -1;
    }

    //printf("Socket gets created successfully with fd : %d\n",sock);

    // step 2 : connect with server

    memset(&ClientAddr, 0, sizeof(ClientAddr));

    ClientAddr.sin_family = AF_INET;
    ClientAddr.sin_port = htons(port);

    // 127.0.0.1(server ip address) -> Binary form
    inet_pton(AF_INET, ip, &ClientAddr.sin_addr);

    iRet = connect(sock, (struct sockaddr *) &ClientAddr, sizeof(ClientAddr));
    if(iRet == -1)
    {
        printf("ERROR: Unable to connect with server\n");
        return -1;
    }

    printf("Successfully Connected with server \n");

    while(1)
    {
        memset(cmd, '\0', sizeof(cmd));
        printf("Client > ");
        scanf("%[^\n]",cmd);
        getchar();

        if(strcmp(cmd, "exit") == 0)
        {
            send(sock, cmd, strlen(cmd), 0);
            break;
        }
        else if((strcmp(cmd, "cls") == 0) || strcmp(cmd, "clear") == 0)
        {
            system("clear");
            continue;
        }
        else if((strncmp(cmd, "dnf", 3) == 0))
        {
            memset(cmdcpy, '\0', sizeof(cmdcpy));
            strcpy(cmdcpy, cmd);
            token[0] = strtok(cmd, " ");
            token[1] = strtok(NULL, " ");
            token[2] = strtok(NULL, " ");

            if((strtok(NULL, " ") != NULL) || (token[1] == NULL) || (token[2] == NULL))
            {
                printf("Server > ERROR: Invalid dnf command\n");
                continue;
            }

            send(sock, cmdcpy, strlen(cmdcpy), 0);
            ReceiveFileFromServer(sock,token[2]);
            RecvMsg(sock);
            continue;
        }
        else if((strncmp(cmd, "upf", 3) == 0))
        {
            memset(cmdcpy, '\0', sizeof(cmdcpy));
            strcpy(cmdcpy, cmd);
            token[0] = strtok(cmd, " ");
            token[1] = strtok(NULL, " ");
            token[2] = strtok(NULL, " ");

            if((strtok(NULL, " ") != NULL) || (token[1] == NULL) || (token[2] == NULL))
            {
                printf("Server > ERROR: Invalid upf command\n");
                continue;
            }

            send(sock, cmdcpy, strlen(cmdcpy), 0);
            sleep(1);
            SendFileToServer(sock,token[1]);
            RecvMsg(sock);
            continue;
        }
        else if(strncmp(cmd, "cd", 2) == 0)
        {
            memset(cmdcpy, '\0', sizeof(cmdcpy));
            strcpy(cmdcpy, cmd);

            token[0] = strtok(cmd, " ");
            token[1] = strtok(NULL, " ");

            if((strtok(NULL, " ") != NULL) || (token[1] == NULL))
            {
                printf("Server > ERROR: Invalid cd command\n");
                continue;
            }
            send(sock, cmdcpy, strlen(cmdcpy), 0);
            RecvMsg(sock);
        }
        else if(strncmp(cmd, "mkdir", 5) == 0)
        {
            memset(cmdcpy, '\0', sizeof(cmdcpy));
            strcpy(cmdcpy, cmd);

            token[0] = strtok(cmd, " ");
            token[1] = strtok(NULL, " ");

            if((strtok(NULL, " ") != NULL) || (token[1] == NULL))
            {
                printf("Server > ERROR: Invalid mkdir command\n");
                continue;
            }

            send(sock, cmdcpy, strlen(cmdcpy), 0);
            RecvMsg(sock);
        }
        else if(strncmp(cmd, "rmdir", 5) == 0)
        {
            memset(cmdcpy, '\0', sizeof(cmdcpy));
            strcpy(cmdcpy, cmd);

            token[0] = strtok(cmd, " ");
            token[1] = strtok(NULL, " ");

            if((strtok(NULL, " ") != NULL) || (token[1] == NULL))
            {
                printf("Server > ERROR: Invalid rmdir command\n");
                continue;
            }

            send(sock, cmdcpy, strlen(cmdcpy), 0);
            RecvMsg(sock);
        }
        else if(strncmp(cmd, "rm", 2) == 0)
        {
            memset(cmdcpy, '\0', sizeof(cmdcpy));
            strcpy(cmdcpy, cmd);

            token[0] = strtok(cmd, " ");
            token[1] = strtok(NULL, " ");

            if((strtok(NULL, " ") != NULL) || (token[1] == NULL))
            {
                printf("Server > ERROR: Invalid rm command\n");
                continue;
            }

            send(sock, cmdcpy, strlen(cmdcpy), 0);
            RecvMsg(sock);
        }
        else if(strncmp(cmd, "man", 3) == 0)
        {
            memset(cmdcpy, '\0', sizeof(cmdcpy));
            strcpy(cmdcpy, cmd);

            token[0] = strtok(cmd, " ");
            token[1] = strtok(NULL, " ");

            if((strtok(NULL, " ") != NULL) || (token[1] == NULL))
            {
                printf("Server > ERROR: Invalid man command\n");
                continue;
            }

            send(sock, cmdcpy, strlen(cmdcpy), 0);
            RecvMsg(sock);
        }
        else
        {
            send(sock, cmd, strlen(cmd), 0);
            RecvMsg(sock);
        }
    }

    // step 4: close all resources
    close(sock);
    printf("Terminating the client application ...\n");
    return 0;
}
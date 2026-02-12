#include "./ServerHeader.h"

////////////////////////////////////////////////////
// 
//  Commandline Arguments 
//  1st Argument : Port number
//  ./server 9000
//  argv[0]  argv[1]
//
////////////////////////////////////////////////////

int main(int argc , char * argv[])
{
    int ServerSocket = 0;
    int ClientSocket = 0;
    int port;
    int iRet = 0, n = 0;
    pid_t pid = 0;
    char cmd[50] = {'\0'};
    char * token[3];

    char Filename[50] = {'\0'};

    struct sockaddr_in ServerAddr;
    struct sockaddr_in ClientAddr;

    socklen_t AddrLen = sizeof(ClientAddr);

    if((argc < 2) || (argc > 2))
    {
        printf("ERROR: Unable to proceed as invalid number of arguments \n");
        printf("Please provide the port number\n");

        return -1;
    }

    //port number of server
    port = atoi(argv[1]);

    ////////////////////////////////////////////////////
    //  step 1 : Create TCP Socket
    ////////////////////////////////////////////////////

    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(ServerSocket < 0)
    {
        printf("ERROR: Unable to create Server Socket\n");
        return -1;
    }

    ////////////////////////////////////////////////////
    //  step 2 : bind Socket to IP and Port
    ////////////////////////////////////////////////////

    memset(&ServerAddr, 0, sizeof(ServerAddr));

    // Intialise the structure

    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(port);
    ServerAddr.sin_addr.s_addr = INADDR_ANY;

    iRet = bind(ServerSocket, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));
    if(iRet == -1)
    {
        printf("ERROR: Unable to bind \n");
        close(ServerSocket);
        return -1;
    }

    ////////////////////////////////////////////////////
    //  step 3 : listen for client connections
    ////////////////////////////////////////////////////

    iRet = listen(ServerSocket, 11);
    if(iRet == -1)
    {
        printf("ERROR: Server Unable to listen the request\n");
        close(ServerSocket);
        return -1;
    }

    printf("Server is running on port : %d\n",port);

    ////////////////////////////////////////////////////
    //  loop which accepts client request continuously
    ////////////////////////////////////////////////////

    // loop to accept multiple client request
    while(1)
    {
        ////////////////////////////////////////////////////
        //  step 4 : accept the client request
        ////////////////////////////////////////////////////

        memset(&ClientAddr, 0, sizeof(ClientAddr));

        printf("Server is waiting for client request\n");
        ClientSocket = accept(ServerSocket, (struct sockaddr *)&ClientAddr, &AddrLen);

        if(ClientSocket < 0)
        {
            printf("ERROR: Unable to accept client request\n");
            printf("Error: %s\n",strerror(errno));
            continue;   // return back start of loop
        }

        printf("Client gets connected : %s\n",inet_ntoa(ClientAddr.sin_addr));

        ////////////////////////////////////////////////////
        //  step 5 : create new process to handle client request
        ////////////////////////////////////////////////////

        pid = fork();

        if(pid < 0)
        {
            printf("ERROR: Unable to create a new process for client request \n");
            close(ClientSocket);
            continue;
        }

        // New process gets created for client
        if(pid == 0)    
        {
            printf("New process gets created for client request\n");

            close(ServerSocket);

            // Save the absolute path of current directory
            SetBaseDir();

            while(1)
            {
                memset(cmd, '\0', sizeof(cmd));

                iRet = recv(ClientSocket, cmd, sizeof(cmd), 0);
                if(iRet <= 0)
                {
                    printf("ERROR: Client is terminated abnormally\n");
                    return -1;
                }

                cmd[iRet] = '\0';

                cmd[strcspn(cmd, "\r\n")] = '\0';

                //printf("cmd : %s\n",cmd);

                if(strcmp(cmd, "ls") == 0)
                {
                    ListFiles(ClientSocket,getcwd(NULL, 0));
                }
                else if(strcmp(cmd, "ls -s") == 0)
                {
                    ListFileSizes(ClientSocket, getcwd(NULL, 0));
                }
                else if(strcmp(cmd, "exit") == 0)
                {
                    break;
                }
                else if(strcmp(cmd, "pwd") == 0)
                {
                    CurrentDirectory(ClientSocket);
                }
                else if(strncmp(cmd, "man", 3) == 0)
                {
                    token[0] = strtok(cmd, " ");
                    token[1] = strtok(NULL, " ");

                    Manual(ClientSocket,token[1]);
                }
                else if(strcmp(cmd, "help") == 0)
                {

                    Help(ClientSocket);
                }
                
                else if(strncmp(cmd, "cd", 2) == 0)
                {
                    token[0] = strtok(cmd, " ");
                    token[1] = strtok(NULL, " ");

                    ChangeDirectory(ClientSocket, token[1]);
                }
                else if(strncmp(cmd, "mkdir", 5) == 0)
                {
                    token[0] = strtok(cmd, " ");
                    token[1] = strtok(NULL, " ");

                    MakeDirectory(ClientSocket, token[1]);
                }
                else if(strncmp(cmd, "rmdir", 5) == 0)
                {
                    token[0] = strtok(cmd, " ");
                    token[1] = strtok(NULL, " ");

                    RemoveDirectory(ClientSocket, token[1]);
                }
                else if(strncmp(cmd, "rm", 2) == 0)
                {
                    token[0] = strtok(cmd, " ");
                    token[1] = strtok(NULL, " ");

                    RemoveFile(ClientSocket, token[1]);
                }
                else if(strncmp(cmd, "dnf", 3) == 0)
                {
                    token[0] = strtok(cmd, " ");
                    token[1] = strtok(NULL, " ");
                    token[2] = strtok(NULL, " ");

                    SendFileToClient(ClientSocket,token[1]);
                }
                else if(strncmp(cmd, "upf", 3) == 0)
                {
                    token[0] = strtok(cmd, " ");
                    token[1] = strtok(NULL, " ");
                    token[2] = strtok(NULL, " ");

                    ReciveFileFromClient(ClientSocket,token[2]);
                }
                else
                {
                    SendMsg(ClientSocket,"Invalid command\n");
                }
            }

            printf("Child process for IP Address : %s is Terminating...\n",inet_ntoa(ClientAddr.sin_addr));
            exit(0);    // kill the child process


        }   // End of if (fork)
        else
        {
            close(ClientSocket);
        }   // End of else

    }   // End of while

    close(ServerSocket);
    
    return 0;
}   // End of main
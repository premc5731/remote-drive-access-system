#include "./ClientHeader.h"

//====================================================================================
// Functions Definitions
//====================================================================================

//====================================================================================
// Function to receive message from server
int RecvMsg(int sock)
{
    char Header[HEADER_SIZE] = {0};
    char Buffer[BF_SIZE + 1] = {'\0'};   
    long MsgSize = 0, toRead = 0;
    long received = 0, remaining = 0;
    int iRet = 0;

    iRet = recv(sock, Header, HEADER_SIZE - 1, 0);
    if (iRet <= 0)
    {
        printf("ERROR: Header receive failed\n");
        return -1;
    }

    Header[iRet] = '\0';

    sscanf(Header, "MSG %ld", &MsgSize);
    
    printf("Server > ");

    while (received < MsgSize)
    {
        remaining = MsgSize - received;

        if(remaining > BF_SIZE)
        {
            toRead = BF_SIZE;
        }
        else
        {
            toRead = remaining;
        }

        iRet = recv(sock, Buffer, toRead, 0);
        if (iRet <= 0)
        {
            printf("ERROR: Message receive failed");
            return -1;
        }

        Buffer[iRet] = '\0';   
        printf("%s", Buffer);  

        received = received + iRet;
    }

    return 0;
}

//====================================================================================
// Function to read Header
int ReadHeader(int sock, char *Buffer, int max)
{
    int i = 0, iRet = 0;
    char ch = '\0';

    while(i < max - 1)
    {
        iRet = read(sock, &ch , 1);
        if(iRet <= 0)
        {
            break;
        }

        if(ch == '\n')
        {
            break;
        }

        Buffer[i++] = ch;
    }   // End of while

    Buffer[i] = '\0';

    return i;
}

//====================================================================================
// Function to receive file from server
int ReceiveFileFromServer(int sock, char * filename)
{
    int outfd = 0, iRet = 0, toRead = 0;
    long received = 0, FileSize = 0, remaining = 0;
    char Buffer[BF_SIZE] = {'\0'};
    char Header[HEADER_SIZE] = {'\0'};

    iRet = ReadHeader(sock, Header, sizeof(Header));

    if(iRet <= 0)
    {
        printf("Client > ERROR: server gets disconnected abnormally\n");
        //close(sock);
        return -1;
    }

    if(strncmp(Header, "ERR", 3) == 0)
    {
        return -1;
    }

    sscanf(Header, "OK %ld", &FileSize);

    outfd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if(outfd < 0)
    {
        printf("Client > ERROR: Unable to create downloaded file\n");
        return -1;
    }

    while(received < FileSize)
    {
        remaining = FileSize - received;

        if(remaining > BF_SIZE)
        {
            toRead = BF_SIZE;
        }
        else
        {
            toRead = remaining;
        }

        iRet = read(sock, Buffer, toRead);
        write(outfd, Buffer, iRet);

        received = received + iRet;
    }   // End of while

    close(outfd);

    return 0;
}

//====================================================================================
// Function to send file to server
int SendFileToServer(int sock, char * filename)
{
    int fd = 0, BytesRead = 0;
    struct stat sobj;
    char Buffer[BF_SIZE];
    char Header[HEADER_SIZE];

    filename[strcspn(filename, "\r\n")] = '\0';

    fd = open(filename, O_RDONLY);

    // Unable to open file
    if(fd < 0)
    {
        write(sock, "ERR\n", 4);
        return -1;
    }

    stat(filename, &sobj);

    if (!(S_ISREG(sobj.st_mode)))
    {
        write(sock, "ERRD\n", 5);
        return -1;
    }

    // Header : "OK 1700"
    snprintf(Header, sizeof(Header), "OK %ld\n",sobj.st_size);

    // write header to client
    write(sock, Header, strlen(Header));
    sleep(1);

    //send actual file contents
    while((BytesRead = read(fd, Buffer, sizeof(Buffer))) > 0)
    {
        // Send the data to client
        write(sock, Buffer, BytesRead);
    }

    close(fd);

}
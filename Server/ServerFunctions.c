#include "./ServerHeader.h"

char BASE_DIR[MAX_PATH];
char MSG[MSG_SIZE];

//====================================================================================
// Functions Definitions
//====================================================================================

//====================================================================================
// Function to Send message
void SendMsg(int ClientSocket, char * msg)
{
    long len = 0, i = 0, iRet = 0;
    int sent = 0, remaining = 0, toSend = 0;
    char ch = '\0';
    char Header[HEADER_SIZE] = {'\0'};

    len = strlen(msg);
    snprintf(Header, HEADER_SIZE, "MSG %ld\n", len);

    send(ClientSocket ,Header, strlen(Header),0);
    sleep(1);

    while(sent < len)
    {
        remaining = len - sent;

        if(remaining > 1024)
        {
            toSend = 1024;
        }
        else
        {
            toSend = remaining;
        }

        iRet = send(ClientSocket, msg + sent, toSend, 0);
        if (iRet <= 0)
        {
            printf("ERROR: Message send failed");
            return;
        }

        sent = sent + iRet;
    }

    return;
}

//====================================================================================
// Function to list all files
int ListFiles(int ClientSocket, char * path)
{
    DIR * dp = NULL;
    struct dirent * ptr = NULL;
    char Buffer[BF_SIZE] = {'\0'};
    int offset = 0, iRet = 0;

    Buffer[offset] = '\n';
    offset = 1;

    dp = opendir(path);
    if(dp == NULL)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "Error: %s\n", strerror(errno));
        offset = offset + iRet;
        return -1;
    }

    while(ptr = readdir(dp))
    {
        if((strcmp(".",ptr->d_name) == 0) || (strcmp("..",ptr->d_name) == 0) )
        {
            continue;
        }
        if(ptr->d_type == DT_DIR)    // Directory
        {
            iRet = snprintf(Buffer + offset, BF_SIZE, "[D] ");
            offset = offset + iRet;
        }
        else if(ptr->d_type == DT_REG)  // Regular file
        {
            iRet = snprintf(Buffer + offset, BF_SIZE, "[R] ");
            offset = offset + iRet;
        }
        else                            // Other files
        {
            iRet = snprintf(Buffer + offset, BF_SIZE, "[O] ");
            offset = offset + iRet;
        }
        
        iRet = snprintf(Buffer + offset, BF_SIZE, "%s \n",ptr->d_name);
        offset = offset + iRet;
        
    }

    closedir(dp);
    if(Buffer[1] == '\0')
    {
        strcpy(Buffer, "Empty\n");
    }
    SendMsg(ClientSocket, Buffer);

    return 0;
}

//====================================================================================
// Function to list all files with their sizes
int ListFileSizes(int ClientSocket, char * path)
{
    DIR * dp = NULL;
    struct dirent * ptr = NULL;
    struct stat sobj;
    char filename[1024] = {'\0'};
    char Buffer[BF_SIZE] = {'\0'};
    int iRet = 0, offset = 0;

    Buffer[offset] = '\n';
    offset = 1;

    dp = opendir(path);
    if(dp == NULL)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "Error: %s\n", strerror(errno));
        offset = offset + iRet;
        return -1;
    }

    while(ptr = readdir(dp))
    {
        if((strcmp(".",ptr->d_name) == 0) || (strcmp("..",ptr->d_name) == 0) )
        {
            continue;
        }
        if(ptr->d_type == DT_DIR)    // Directory
        {
            iRet = snprintf(Buffer + offset, BF_SIZE, "[D] ");
            offset = offset + iRet;
        }
        else if(ptr->d_type == DT_REG)  // Regular file
        {
            iRet = snprintf(Buffer + offset, BF_SIZE, "[R] ");
            offset = offset + iRet;
        }
        else                            // Other files
        {
            iRet = snprintf(Buffer + offset, BF_SIZE, "[O] ");
            offset = offset + iRet;
        }
        
        memset(filename, '\0', sizeof(filename));
        snprintf(filename, sizeof(filename), "%s/%s", path, ptr->d_name);
        stat(filename, &sobj);

        iRet = snprintf(Buffer + offset, BF_SIZE, "[%-7ld]   ",sobj.st_size);
        offset = offset + iRet;

        iRet = snprintf(Buffer + offset, BF_SIZE,"%s \n",ptr->d_name);
        offset = offset + iRet;
        
    }

    closedir(dp);
    if(Buffer[1] == '\0')
    {
        strcpy(Buffer, "Empty\n");
    }
    SendMsg(ClientSocket, Buffer);

    return 0;
}

//====================================================================================
// Function to display present working directory
int CurrentDirectory(int ClientSocket)
{
    char pwd[1024] = {'\0'};
    if(getcwd(pwd, sizeof(pwd)) == NULL)
    {
        SendMsg(ClientSocket, "ERROR: Unable to get pwd\n");
        return -1;
    }

    pwd[strlen(pwd)] = '\n';
    SendMsg(ClientSocket, pwd);
}

//====================================================================================
// Function to store absolute path of server's directory
int SetBaseDir()
{
    char * path = NULL;
    
    path = realpath(".", BASE_DIR);

    if(path == NULL)
    {
        printf("ERROR: Unable to create base dir path\n");
        return -1;
    }
}

//====================================================================================
// Function to change directory with restrictions
int ChangeDirectory(int ClientSocket, char * path)
{
    char UserDir[MAX_PATH] = {'\0'};
    char * rpath = NULL;
    int len = 0, iRet = 0;

    rpath = realpath(path, UserDir);
    if(path == NULL)
    {
        SendMsg(ClientSocket,"ERROR: Unable to create user dir path\n");
        return -1;
    }

    len = strlen(BASE_DIR);

    // conditions to check the invalid paths 
    if((strncmp(BASE_DIR, UserDir, len) != 0) || (UserDir[len] != '/' && UserDir[len] != '\0')) 
    {
        SendMsg(ClientSocket,"ERROR: Permission Denied\n");
        return -1;
    }

    iRet = chdir(path);

    if(iRet == -1)
    {
        SendMsg(ClientSocket,"ERROR: Invalid path\n");
        return -1;
    }

    SendMsg(ClientSocket, "Success\n");
    return 0;
    
}

//====================================================================================
// Function to create a new directory
int MakeDirectory(int ClientSocket, char * dirname)
{
    int permission = 0777;
    int iRet = 0;

    // create dir only in cwd 
    // Not allowd to create dir in other than cwd
    if((dirname[0] == '/') || ((dirname[0] == '.') && (dirname[1] == '.')))   
    {
        SendMsg(ClientSocket,"ERROR: Unable to create directory in other than current directory\n");
        return -1;
    }

    iRet = mkdir(dirname, permission);

    if(iRet == -1)
    {
        SendMsg(ClientSocket,"ERROR: Inavlid arguments\n");
        return -1;
    }

    SendMsg(ClientSocket, "Success\n");
    return 0;

}

//====================================================================================
// Function to delete an empty directory
int RemoveDirectory(int ClientSocket, char * dirname)
{
    int iRet = 0;

    // remove dir only from cwd 
    // Not allowd to remove dir in other than cwd
    if((dirname[0] == '/') || ((dirname[0] == '.') && (dirname[1] == '.')))   
    {
        SendMsg(ClientSocket,"ERROR: Unable to remove directory in other than current directory\n");
        return -1;
    }

    iRet = rmdir(dirname);

    if(iRet == -1)
    {
        SendMsg(ClientSocket,"ERROR: Inavlid arguments\n");
        return -1;
    }

    SendMsg(ClientSocket, "Success\n");
    return 0;

}

//====================================================================================
// Function to delete a file
int RemoveFile(int ClientSocket, char * filename)
{
    int iRet = 0;

    // remove file from  only from cwd 
    // Not allowed to remove file in other than cwd
    if((filename[0] == '/') || ((filename[0] == '.') && (filename[1] == '.')))   
    {
        SendMsg(ClientSocket,"ERROR: Unable to delete file in other than current directory\n");
        return -1;
    }

    iRet = unlink(filename);

    if(iRet == -1)
    {
        SendMsg(ClientSocket,"ERROR: Inavlid arguments\n");
        return -1;
    }

    SendMsg(ClientSocket, "Success\n");
    return 0;
}
//====================================================================================
// Function to send a file to client
int SendFileToClient(int ClientSocket , char * Filename)
{
    int fd = 0;
    struct stat sobj;
    char Buffer[RWBF_SIZE];
    char Header[HEADER_SIZE];
    int BytesRead = 0;

    Filename[strcspn(Filename, "\r\n")] = '\0';

    // reading file in other than cwd is not allowed
    if((Filename[0] == '/') || ((Filename[0] == '.') && (Filename[1] == '.')))   
    {
        SendMsg(ClientSocket, "ERROR: Unable to read file in other than current directory\n");
        return -1;
    }


    fd = open(Filename, O_RDONLY);

    // Unable to open file
    if(fd < 0)
    {
        // Send Err message to client
        write(ClientSocket, "ERR\n", 4);
        sleep(1);
        SendMsg(ClientSocket, "ERROR: Unable to download file\n");

        return -1;
    }

    stat(Filename, &sobj);

    if (!(S_ISREG(sobj.st_mode)))
    {
        write(ClientSocket, "ERR\n", 4);
        sleep(1);
        SendMsg(ClientSocket, "ERROR: Please enter a regular file name \n");
        return -1;
    }

    // Header : "OK 1700"
    snprintf(Header, sizeof(Header), "OK %ld\n",sobj.st_size);

    // write header to client
    write(ClientSocket, Header, strlen(Header));

    //sleep to avoid message concatination
    sleep(1);

    //send actual file contents
    while((BytesRead = read(fd, Buffer, sizeof(Buffer))) > 0)
    {
        // Send the data to client
        write(ClientSocket, Buffer, BytesRead);
    }

    SendMsg(ClientSocket, "File downloaded Successfully\n");

    close(fd);
}

//====================================================================================
// Function to read Header
int ReadHeader(int ClientSocket, char *Buffer, int max)
{
    int i = 0, iRet = 0;
    char ch = '\0';

    while(i < max - 1)
    {
        iRet = read(ClientSocket, &ch , 1);
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
// Function to recive a file from client
int ReciveFileFromClient(int ClientSocket, char * filename)
{
    int iRet = 0, toRead = 0, outfd = 0;
    long received = 0, FileSize = 0, remaining = 0;
    char Buffer[RWBF_SIZE] = {'\0'};
    char Header[HEADER_SIZE] = {'\0'};

    iRet = ReadHeader(ClientSocket, Header, sizeof(Header));

    if(iRet <= 0)
    {
        SendMsg(ClientSocket, "ERROR: client gets disconnected abnormally\n");
        return -1;
    }

    if(strncmp(Header, "ERRD", 4) == 0)
    {
        SendMsg(ClientSocket,"ERROR: Please enter a regular file name \n");
        return -1;
    }

    if(strncmp(Header, "ERR", 3) == 0)
    {
        SendMsg(ClientSocket,"ERROR: Unable to upload file\n");
        return -1;
    }
    
    sscanf(Header, "OK %ld", &FileSize);

    outfd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if(outfd < 0)
    {
        SendMsg(ClientSocket, "ERROR: Unable to create uploaded file\n");
        return -1;
    }


    while(received < FileSize)
    {
        remaining = FileSize - received;

        if(remaining > 1024)
        {
            toRead = 1024;
        }
        else
        {
            toRead = remaining;
        }

        iRet = read(ClientSocket, Buffer, toRead);
        write(outfd, Buffer, iRet);

        received = received + iRet;
    }   // End of while

    close(outfd);

    SendMsg(ClientSocket, "File Uploaded Successfully\n");

    return 0;
}

//====================================================================================
// Function to Display Manual
void Manual(int ClientSocket, char * cmd)
{
    char Buffer[BF_SIZE] = {'\0'};
    int offset = 0, iRet = 0;

    Buffer[offset] = '\n';
    offset = 1;

    if(cmd == NULL)
	{   
        iRet = snprintf(Buffer + offset, BF_SIZE, "Please enter a command\n");
        offset = iRet + offset;
	}
	
	else if(strcmp(cmd,"ls") == 0)
	{
		iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to list the files \n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: ls\n");
        offset = iRet + offset;
	}
    else if(strcmp(cmd, "ls -s") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to list the files and their sizes\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: ls -s\n");
        offset = iRet + offset;
        printf("Inside ls -s\n");
    }
    else if(strcmp(cmd, "exit") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to terminate the application\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: exit\n");
        offset = iRet + offset;
    }
    else if(strcmp(cmd, "pwd") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to display the present working directory\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: pwd\n");
        offset = iRet + offset;
    }
    else if((strcmp(cmd, "cls") == 0) || (strcmp(cmd, "clear") == 0))
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to clear the screen\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: cls\n");
        offset = iRet + offset;
    }
    else if(strcmp(cmd, "help") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to display the list of commands\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: help\n");
        offset = iRet + offset;
    }
    else if(strcmp(cmd, "cd") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to change the directory\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: cd path \n");
        offset = iRet + offset;
    }
    else if(strcmp(cmd, "mkdir") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to create a new directory\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: mkdir dirname\n");
        offset = iRet + offset;
    }
    else if(strcmp(cmd, "rmdir") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to delete an empty directory\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: rmdir dirname\n");
        offset = iRet + offset;
    }
    else if(strcmp(cmd, "rm") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to delete a file\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: rm filename\n");
        offset = iRet + offset;
    }
    else if(strcmp(cmd, "dnf") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to download a file\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: dnf filename_to_download filename_to_be_downloaded_as\n");
        offset = iRet + offset;
    }
    else if(strcmp(cmd, "upf") == 0)
    {
        iRet = snprintf(Buffer + offset, BF_SIZE, "DESCRIPTION: This command is used to upload a file\n");
        offset = iRet + offset;
		iRet = snprintf(Buffer + offset, BF_SIZE, "USAGE: upf filename_to_upload filename_to_be_uploaded_as\n");
        offset = iRet + offset;
    }

    SendMsg(ClientSocket, Buffer);
}

//====================================================================================
// Function to Display list of all commands
void Help(int ClientSocket)
{
    char Buffer[BF_SIZE] = {'\0'};
    int offset = 0, iRet = 0;

    Buffer[offset] = '\n';
    offset = 1;

    iRet = snprintf(Buffer + offset, BF_SIZE, "ls: This command is used to list the files \n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "ls -s: This command is used to list the files and their sizes\n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "exit: This command is used to terminate the application\n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "pwd: This command is used to display the present working directory\n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "cls: This command is used to clear the screen\n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "man: This command is used to display the manual of command\n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "cd: This command is used to change the directory\n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "mkdir: This command is used to create a new directory\n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "rmdir: This command is used to delete an empty directory\n");
    offset = iRet + offset;
    
    iRet = snprintf(Buffer + offset, BF_SIZE, "rm: This command is used to delete a file\n");
    offset = iRet + offset;
    
    iRet = snprintf(Buffer + offset, BF_SIZE, "dnf: This command is used to download a file\n");
    offset = iRet + offset;

    iRet = snprintf(Buffer + offset, BF_SIZE, "upf: This command is used to upload a file\n");
    offset = iRet + offset;

    SendMsg(ClientSocket, Buffer);
}
#include<stdio.h>
#include<windows.h>
#include<winsock2.h>
#include<string.h>

#pragma comment(lib, "Ws2_32.lib")

WSADATA  wsdata;
WORD DLLVERSION;
SOCKADDR_IN addr;
SOCKET listener,conn;

int senddata(SOCKET sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;
    while (buflen > 0)
    {
        Sleep(100);
        int num = send(sock, pbuf, buflen, 0);
        if (num == SOCKET_ERROR)
        {
            
            printf("1st SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            if (WSAGetLastError() == WSAEWOULDBLOCK){
                printf("WSA ERROR !!\n");
            
                continue;
            }
            
            printf("2nd SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            return 0;
        }
        pbuf += num;
        buflen -= num;
    }
    return 1;
}

int sendlong(SOCKET sock, long value)
{
    value = htonl(value);
    return senddata(sock, &value, sizeof(value));
}

int sendfile(SOCKET sock, FILE *f)
{
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    rewind(f);
    if (filesize == EOF)
        return 0;
    if (!sendlong(sock, filesize))
        return 0;
    if (filesize > 0)
    {
        char buffer[1024];
        do
        {
            size_t num = min(filesize, sizeof(buffer));
            num = fread(buffer, 1, num, f);
            if (num < 1)
                return 0;
            if (!senddata(sock, buffer,num))
                return 0;
            filesize -= num;
        }
        while (filesize > 0);
    }
    return 1;
}



int readdata(SOCKET sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0)
    {
        int num = recv(sock, pbuf, buflen, 0);
        if (num == SOCKET_ERROR)
        {
            printf("1st SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            if (WSAGetLastError() == WSAEWOULDBLOCK){
                printf("WSA ERROR !!\n");
            
                continue;
            }
            
            printf("2nd SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            return 0;
        }
        else if (num == 0){
            printf("NUM==0 error!!");
        
            return 0;
        }
        pbuf += num;
        buflen -= num;
    }

    return 1;
}

int readlong(SOCKET sock, long *value)
{
    if (!readdata(sock, value, sizeof(value)))
        return 0;
    *value = ntohl(*value);
    return 1;
}

int readfile(SOCKET sock, FILE *f)
{
    long filesize;
    if (!readlong(sock, &filesize)){
        printf("Length problem \n");
        return 0;
    }
        
    if (filesize > 0)
    {
        char buffer[1024];
        do
        {
            int num = min(filesize, sizeof(buffer));
            if (!readdata(sock, buffer, num)){
                printf("Data problem\n");
                return 0;
            }
            int offset = 0;
            do
            {
                size_t written = fwrite(&buffer[offset], 1, num-offset, f);
                if (written < 1)
                    return 0;
                offset += written;
            }
            while (offset < num);
            filesize -= num;
        }
        while (filesize > 0);
    }
    return 1;
}




void filesend()
{
    char filename[24];
    printf("Enter file name : ");
    scanf(" %s",&filename);
    printf("%s....\n",filename);
    printf("length = %d",strlen(filename));
    FILE *filehandle = fopen(filename, "rb");
    if (filehandle != NULL)
    {
        send(conn,filename,sizeof(filename),0);
        char *confirm;
        recv(conn,confirm,10,0);
        printf("Confirmation was : %s",confirm);
        if(!strcmp("Y",confirm))
            sendfile(conn, filehandle);
        fclose(filehandle);
    }
}

void recievefile()
{
    char filename[24],opt[10];
    recv(conn,filename,sizeof(filename),0);
    printf("Want to recieve : %s ? (Y/N)",filename);
    printf("%s....\n",filename);
    scanf(" %s",&opt);
    send(conn,opt,10,0);
    strcpy(filename,filename);
    if(strcmp("Y",opt))
    {
        printf("Deniedd!");
        return;
    }
    printf("%s....\n",filename);
    printf("length = %d\n",strlen(filename));
    FILE *filehandle = fopen(filename, "wb");
    if (filehandle != NULL)
    {
        int ok = readfile(conn, filehandle);
        fclose(filehandle);
    
        if (ok)
        {
            printf("FIle is ok!!!");
        }
        else
        {
            printf("ERRORROROROROOROR NOT OKAY :(((\n");
            remove(filename);
        }
   }
    printf("\nOUT OF REACH>>>>>>\n");
}
void main()
{
    
    DLLVERSION = MAKEWORD(2,1);
    if(WSAStartup(DLLVERSION,&wsdata)!=0)
    {
        printf("Error!!!");
        return;
    }
    
    int addrlen = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(7089);
    addr.sin_family = AF_INET;
    listener = socket(AF_INET,SOCK_STREAM,0);
    char ip[32];
    
    printf("Started socket!!\n");

    printf("1.Send Data\n2.Recieve Data");
    int opt,result;
    scanf("%d",&opt);
    switch(opt)
    {
        case 1:
            
            result = bind(listener,(SOCKADDR*)&addr,sizeof(addr));
            if (result == SOCKET_ERROR) {
                printf("bind failed with error: %d\n", WSAGetLastError());
                return;
            }
            listen(listener,SOMAXCONN);    
            conn = accept(listener,(SOCKADDR*)&addr,&addrlen);
            if(conn == 0)
                printf("Erorr geting conecgtion!!!");
            else                
                filesend();
    
                break;
        case 2:
            scanf(" %s",&ip);
            addr.sin_addr.s_addr = inet_addr(ip);
            conn = socket(AF_INET,SOCK_STREAM,0);
            if(connect(conn,(SOCKADDR*)&addr,addrlen) != 0)
            {
                printf("conn error !!!! %ld ",WSAGetLastError());
                return;
            }
            else
                recievefile();  
            
            break;
    }
    
   
}
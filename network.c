#include<string.h>
#include<winsock2.h>
#include<gtk/gtk.h>

#include "network.h"
#include "gui.h"

int senddata(SOCKET sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;
    while (buflen > 0)
    {
        Sleep(1);
        int num = send(sock, pbuf, buflen, 0);
        if (num == SOCKET_ERROR)
        {
            
            printf("\n1st SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                printf("WSA ERROR !!\n");
                continue;
            }
            
            printf("\n2nd SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            return 0;
        }
        pbuf += num;
        buflen -= num;
    }
    gtk_widget_hide(send_screen);
    gtk_widget_show(main_screen);

    return 1;
}

int sendlong(SOCKET sock, long value)
{
    value = htonl(value);
    return senddata(sock, &value, sizeof(value));
}

int sendfile(SOCKET sock, FILE *f)
{
    while (gtk_events_pending ())
        gtk_main_iteration_do (FALSE);
  
    fseek(f, 0, SEEK_END);
    gfloat progress = 0.0;
    long filesize = ftell(f);
    long fixedsize = filesize;
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
            progress += num;
            //printf("progress : %f\n",progress);
            gtk_progress_bar_set_fraction(send_bar,progress/fixedsize);
            while (gtk_events_pending ()) 
                gtk_main_iteration_do (FALSE);
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
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
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
    while (gtk_events_pending ())
        gtk_main_iteration_do (FALSE);

    long filesize;
    gfloat progress=0.0;
    if (!readlong(sock, &filesize)){
        printf("Length problem \n");
        return 0;
    }
    long fixedsize = filesize;

        
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
            progress += num;
            //printf("progress : %f\n",progress);
            gtk_progress_bar_set_fraction(recieve_bar,progress/fixedsize);
            while (gtk_events_pending ())
                gtk_main_iteration_do (FALSE);
          
        }
        while (filesize > 0);
    }
    return 1;
}

void filesend(char filename[54])
{
    
    printf("%s....\n",filename);
    printf("length = %d",strlen(filename));
    FILE *filehandle = fopen(filename, "rb");
    if (filehandle != NULL)
    {
        send(conn,filename,54,0);
        char *confirm;
        recv(conn,confirm,10,0);
        printf("Confirmation was : %s",confirm);
        if(!strcmp("Y",confirm))
            sendfile(conn, filehandle);
        fclose(filehandle);
        closesocket(listener);
        WSACleanup();
    }
}

void recievefile()
{
    char filename[54],opt[10];
    char *basename;
    recv(conn,filename,sizeof(filename),0);
    
    strcpy(opt,"Y");
    send(conn,opt,10,0);
    printf("\nfile is %s\nbasename is %s",filename,basename); 
    basename = strrchr(filename, '\\');
    ++basename;
   
    printf("%s....\n",basename);
    printf("length = %d\n",strlen(basename));
    FILE *filehandle = fopen(basename, "wb");
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
    closesocket(conn);
    WSACleanup();
    gtk_widget_hide(recieve_screen);
    gtk_widget_show(main_screen);
}
#include<string.h>
#include<winsock2.h>
#include<gtk/gtk.h>

#include "network.h"
#include "gui.h"

int senddata(SOCKET sock, void *buf, int buflen)
{
    
    unsigned char *pbuf = (unsigned char *) buf;
    int num;
    
    while (buflen > 0)
    {
        //Sleep(1);
        num = send(sock, pbuf, buflen, 0);
        
        if (num == SOCKET_ERROR)
        {            
            printf("\n1st SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                printf("WSA ERROR !!\n");
                continue;
            }
            
            printf("\n2nd SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            showdialog(send_screen,GTK_MESSAGE_ERROR,"Socket error transfer failed!!");
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
    UPDATE_GUI();
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
        size_t num;

        do
        {
            num = min(filesize, sizeof(buffer));
            num = fread(buffer, 1, num, f);
            if (num < 1)
                return 0;
            if (!senddata(sock, buffer,num))
                return 0;
            filesize -= num;
            progress += num;
            //printf("progress : %f\n",progress);
            gtk_progress_bar_set_fraction(send_bar,progress/fixedsize);
            
            UPDATE_GUI();
                
        }while (filesize > 0);
    }
    gtk_progress_bar_set_fraction(send_bar,0);
    showdialog(send_screen,GTK_MESSAGE_INFO,"File send successfully");
    
    return 1;

}


int readdata(SOCKET sock, void *buf, int buflen)
{
    
    unsigned char *pbuf = (unsigned char *) buf;
    int num;
    
    while (buflen > 0)
    {
        num = recv(sock, pbuf, buflen, 0);
        
        if (num == SOCKET_ERROR)
        {
            printf("1st SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                printf("WSA ERROR !!\n");
            
                continue;
            }
            
            printf("2nd SOCKET_ERROR\n DESC = %ld",WSAGetLastError());
            showdialog(recieve_screen,GTK_MESSAGE_ERROR,"Socket error transfer failed!!");
            return 0;
        }
        
        else if (num == 0){
            printf("NUM==0 error!!");
            showdialog(recieve_screen,GTK_MESSAGE_ERROR,"Read zero data from server process terminated!!");
        
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
    
    UPDATE_GUI();

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
        int num,offset;
        size_t written;

        do
        {
            num = min(filesize, sizeof(buffer));
            offset = 0;

            if (!readdata(sock, buffer, num))
            {
                printf("Data problem\n");
                showdialog(recieve_screen,GTK_MESSAGE_ERROR,"Data error reading buffer failed!!");
                return 0;
            }
                        
            do
            {
                written = fwrite(&buffer[offset], 1, num-offset, f);
                if (written < 1)
                    return 0;
                offset += written;
            } while (offset < num);
            
            filesize -= num;
            progress += num;
            //printf("progress : %f\n",progress);
            gtk_progress_bar_set_fraction(recieve_bar,progress/fixedsize);
            
            UPDATE_GUI();
          
        } while (filesize > 0);
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
        closesocket(conn);
        closesocket(listener);
        WSACleanup();
        gtk_widget_hide(send_screen);
        gtk_widget_show(main_screen);
    }

}

void recievefile()
{
    
    char filename[54],opt[10],*basename;
    
    recv(conn,filename,sizeof(filename),0);
    strcpy(opt,"Y");
    send(conn,opt,10,0);
    basename = strrchr(filename, '\\');
    ++basename;
    printf("\nfile is %s\nbasename is %s",filename,basename);
    printf("length = %d\n",strlen(basename));
    FILE *filehandle = fopen(basename, "wb");
    
    if (filehandle != NULL)
    {
        int ok = readfile(conn, filehandle);
        fclose(filehandle);
    
        if (ok)
            showdialog(recieve_screen,GTK_MESSAGE_INFO,"File recieved!");
        
        
        else
        {
            printf("ERRORROROROROOROR NOT OKAY :(((\n");
            showdialog(recieve_screen,GTK_MESSAGE_ERROR,"File transfer failed.Data send is corrupted!"); 
            remove(filename);
        }
    
    }
    
    printf("\nDone\n");
    gtk_progress_bar_set_fraction(recieve_bar,0);
    closesocket(conn);
    WSACleanup();
    gtk_widget_hide(recieve_screen);
    gtk_widget_show(main_screen);

}
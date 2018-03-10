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
        // //Sleep(1);
        
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
    DEBUG("inside sendfile");
    fseek(f, 0, SEEK_END);
    
    gfloat progress = 0.0;
    long filesize = ftell(f);
    long fixedsize = filesize;
   
    rewind(f);
    
    if (filesize == EOF)
        return 0;
    
    if (!sendlong(sock, filesize))
        return 0;
    
    struct progress_args *pargs= g_slice_new(struct progress_args);
    
    if (filesize > 0)
    {
        char buffer[1024];
        size_t num;
        do
        {

            num = min(filesize, sizeof(buffer));
            num = fread(buffer, 1, num, f);
            if (num < 1)
            {
                g_slice_free(struct progress_args, pargs);
                return 0;
            }
            if (!senddata(sock, buffer,num))
            {
                g_slice_free(struct progress_args, pargs);
                return 0;
            }
            filesize -= num;
            progress += num;
            
            pargs->bar = "send_bar";
            pargs->value = progress/fixedsize;
            g_idle_add(set_progress_threaded,pargs);
        
        }while (filesize > 0);
    }
    DEBUG("Done sending ");
     //Sleep(2000);
    g_slice_free(struct progress_args, pargs);
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
            return 0;

        }
        
        else if (num == 0)
        {
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
    
    UPDATE_GUI();
    struct progress_args *pargs= g_slice_new(struct progress_args);
    
    pargs->bar = "recieve_bar";
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
                g_slice_free(struct progress_args, pargs);
                return 0;
            }
                        
            do
            {
                written = fwrite(&buffer[offset], 1, num-offset, f);
                if (written < 1)
                {
                    g_slice_free(struct progress_args, pargs);
                    return 0;
                }
                    
                offset += written;
            } while (offset < num);
            
            filesize -= num;
            progress += num;
            //printf("progress : %f\n",progress);
            //gtk_progress_bar_set_fraction(recieve_bar,progress/fixedsize);
          
            pargs->value = progress/fixedsize;
            g_idle_add(set_progress_threaded,pargs);

            
          
        } while (filesize > 0);
    }

     //Sleep(2000);
    g_slice_free(struct progress_args, pargs);
    return 1;

}


gpointer start_threaded_send(gpointer filename)
{
    printf("On start_threaded_send called\n ");
    int addrlen = sizeof(addr);
    struct dialog_args *dargs = g_slice_new(struct dialog_args);

    dargs->screen = "send_screen";
    conn = accept(listener,(SOCKADDR*)&addr,&addrlen);
        
    if(conn == 0)
    {
        dargs->dialog_type = GTK_MESSAGE_ERROR;
        dargs->message = "Error getting connection!!";
        
    }
    else
    {
        printf("filename that going to send is %s \n",filename);
        int ok = filesend((char*)filename);
        if(ok)
        {
            DEBUG("it's ok");
            dargs->dialog_type = GTK_MESSAGE_INFO;
            dargs->message = "File send successfully!!";
            
        }
            
        else
        {
            DEBUG("it's not ok");
            dargs->dialog_type = GTK_MESSAGE_ERROR;
            dargs->message = "File transferring failed or cancelled!!";
        }
            

        
    }
    g_idle_add(showdialog_threaded,dargs);
    
    return FALSE;
}



int filesend(char filename[255])
{
    int ok = 0;
    DEBUG("inside filesend");
    printf("%s....\n",filename);
    printf("filename ippo sendum = %s length = %d",filename,strlen(filename));
    FILE *filehandle = fopen(filename, "rb");
    if (filehandle != NULL)
    {
        send(conn,filename,255,0);
        DEBUG("\nsending filename is done ");
        char confirm[10];
        recv(conn,confirm,sizeof(confirm),0);
        printf("Confirmation was : %s",confirm);
        if(!strcmp("Y",confirm))
            ok = sendfile(conn, filehandle);
        fclose(filehandle);
        closesocket(conn);
        closesocket(listener);
        WSACleanup();
       
    }
    return ok;
}

void recievefile()
{
    
    char filename[255],opt[10],*basename;
    struct dialog_args *dargs = g_slice_new(struct dialog_args);
    
    dargs->screen = "recieve_screen";

    recv(conn,filename,sizeof(filename),0);
    DEBUG("recieved name");
    strcpy(opt,"Y");
    send(conn,opt,10,0);
    DEBUG("\nsending confirmation is done");
    basename = strrchr(filename, '\\');
    ++basename;
    printf("\nfile is %s\nbasename is %s\n",filename,basename);
    printf("length = %d\n",strlen(basename));
    FILE *filehandle = fopen(basename, "wb");
    
    if (filehandle != NULL)
    {
        int ok = readfile(conn, filehandle);
        fclose(filehandle);
    
        if (ok)
        {
            DEBUG("it's ok");
            dargs->dialog_type = GTK_MESSAGE_INFO;
            dargs->message = "File recieved successfully!!";
        }
            
        else
        {
            DEBUG("it's not ok");
            printf("ERRORROROROROOROR NOT OKAY :(((\n");
            dargs->dialog_type = GTK_MESSAGE_ERROR;
            dargs->message = "File transferring failed or cancelled!!";
            remove(filename);
        }
    
    }
    DEBUG("It's done recieveing");
    g_idle_add(showdialog_threaded,dargs);
    closesocket(conn);
    WSACleanup();
     //Sleep(5000);
     

}
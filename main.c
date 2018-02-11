#include<stdio.h>
#include<string.h>
#include<winsock2.h>
#include<gtk/gtk.h>

#include "network.h"
#include "main.h"
#include "gui.h"

//#define G_MODULE_EXPORT		__declspec(dllexport)

void main(int argc, char *argv[])
{
    gtk_init (&argc, &argv);
    
   /* DLLVERSION = MAKEWORD(2,1);
    if(WSAStartup(DLLVERSION,&wsdata)!=0)
    {
        printf("Error!!!");
        return;
    }
    
    int addrlen = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(7089);
    addr.sin_family = AF_INET;
    listener = socket(AF_INET,SOCK_STREAM,0);*/
    gui_init();

      
}
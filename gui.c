#include<winsock2.h>
#include<gtk/gtk.h>
#include "gui.h"
#include "network.h"


extern int confirm;

void sndclick()
{
    DLLVERSION = MAKEWORD(2,1);
    if(WSAStartup(DLLVERSION,&wsdata)!=0)
    {
        printf("Error!!!");
        return;
    }

    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(7089);
    addr.sin_family = AF_INET;
    listener = socket(AF_INET,SOCK_STREAM,0);
    gtk_widget_show(send_screen);     
    gtk_widget_hide(main_screen); 
    printf("Reached sndclikc\n");
    int result;
    result = bind(listener,(SOCKADDR*)&addr,sizeof(addr));
    if (result == SOCKET_ERROR) 
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        return;
    }
    int addrlen = sizeof(addr);
    listen(listener,SOMAXCONN);    
        
    char *filename;
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                        NULL,
                                        action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Open",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
    gint res = gtk_dialog_run (GTK_DIALOG (dialog));
    
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        gtk_widget_destroy (dialog);
        conn = accept(listener,(SOCKADDR*)&addr,&addrlen);
        
        if(conn == 0)
            printf("Erorr geting conection!!!");
        
        else
            filesend(filename);

    }
    
    
}

void rcvclk()
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

    gtk_widget_show(recieve_screen);     
    gtk_widget_hide(main_screen); 
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
    
}

void on_window_main_destroy()
{
    gtk_main_quit();
}

void onyes()
{
    printf("Clicked yes\n");
    confirm = 1;
    gtk_widget_show(main_screen);     
    gtk_widget_hide(msgbox);     
}

void onno()
{ 
    printf("Clicked no\n");
    confirm = 0;
    gtk_widget_show(main_screen);
    gtk_widget_hide(msgbox);          
}


void gui_init()
{
        builder = gtk_builder_new();
        gtk_builder_add_from_file (builder, "cender.glade", NULL);
        
        main_screen = GTK_WIDGET(gtk_builder_get_object(builder, "main_screen"));
       // gtk_builder_connect_signals(builder,NULL);
        recieve_screen = GTK_WIDGET(gtk_builder_get_object(builder, "recieve_screen"));
        send_screen = GTK_WIDGET(gtk_builder_get_object(builder, "send_screen"));
        msgbox = GTK_WIDGET(gtk_builder_get_object(builder,"msgbox"));
        send_label = GTK_WIDGET(gtk_builder_get_object(builder,"send_label"));
        recieve_label = GTK_WIDGET(gtk_builder_get_object(builder,"recieve_label"));
        send_bar = GTK_WIDGET(gtk_builder_get_object(builder,"send_bar"));
        recieve_bar = GTK_WIDGET(gtk_builder_get_object(builder,"recieve_bar"));
        send_button = GTK_WIDGET(gtk_builder_get_object(builder,"send_button"));
        recieve_button = GTK_WIDGET(gtk_builder_get_object(builder,"recieve_button"));
        yes_button = GTK_WIDGET(gtk_builder_get_object(builder,"yes_button"));
        no_button = GTK_WIDGET(gtk_builder_get_object(builder,"no_button"));

        g_signal_connect (send_button, "clicked", G_CALLBACK (sndclick), NULL);
        g_signal_connect (recieve_button, "clicked", G_CALLBACK (rcvclk), NULL);
        g_signal_connect (yes_button, "clicked", G_CALLBACK (onyes), NULL);
        g_signal_connect (no_button, "clicked", G_CALLBACK (onno), NULL);
        
        g_object_unref(builder);
    
        gtk_widget_show(main_screen);                
        gtk_main();
}

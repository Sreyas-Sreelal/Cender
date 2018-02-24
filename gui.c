#include<winsock2.h>
#include<gtk/gtk.h>
#include "gui.h"
#include "network.h"

extern int confirm;

void on_send_button_clicked()
{
    gtk_widget_hide(main_screen); 
    gtk_widget_show(send_screen);

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
        int result,addrlen;
        PHOSTENT hostinfo;
        char name[255],*ip;
           
       
        //char info[55];
        DLLVERSION = MAKEWORD(2,1);
        if(WSAStartup(DLLVERSION,&wsdata)!=0)
        {
            printf("Error!!!");
            on_send_cancel_button_clicked();
            return;
        }
    
        addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        addr.sin_port = htons(7089);
        addr.sin_family = AF_INET;
        listener = socket(AF_INET,SOCK_STREAM,0);
        printf("Reached sndclikc\n");
        result = bind(listener,(SOCKADDR*)&addr,sizeof(addr));
        gethostname(name,sizeof(name));
        hostinfo = gethostbyname(name);
        ip = inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[0]);
        printf("Ip is %s \n",ip);
    
        gtk_label_set_text(info_label,ip);
        
        UPDATE_GUI();
        
        if (result == SOCKET_ERROR) 
        {
            printf("bind failed with error: %d\n", WSAGetLastError());
            on_send_cancel_button_clicked();
            return;
        }
    
        addrlen = sizeof(addr);
        listen(listener,SOMAXCONN);    

        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        
        gtk_widget_destroy (dialog);
        conn = accept(listener,(SOCKADDR*)&addr,&addrlen);
        
        if(conn == 0)
            printf("Erorr geting conection!!!");
        
        else
            filesend(filename);

    }
    
    else
    {
        gtk_widget_destroy (dialog);
        on_send_cancel_button_clicked();
    }
    
}

void on_recieve_button_clicked()
{
   
    printf("Reached on_recieve_button_clicked\n");
    gtk_widget_hide(main_screen);
    gtk_widget_show(intermediate_rcv);
    
}

void on_start_recieving()
{
    
    gtk_widget_hide(intermediate_rcv);
    char ip[15];
    strcpy(ip,gtk_entry_get_text (ip_input));
    printf("Ip entered is %s\n length is %d",ip,strlen(ip));
    DLLVERSION = MAKEWORD(2,1);
    if(WSAStartup(DLLVERSION,&wsdata)!=0)
    {
        printf("Error!!!");
        on_recieve_cancel_button_clicked();
        return;
    }
    
    int addrlen = sizeof(addr);
    addr.sin_port = htons(7089);
    addr.sin_family = AF_INET;
    listener = socket(AF_INET,SOCK_STREAM,0);

    gtk_widget_show(recieve_screen);     
    //scanf(" %s",&ip);
    addr.sin_addr.s_addr = inet_addr(ip);
    conn = socket(AF_INET,SOCK_STREAM,0);
    
    if(connect(conn,(SOCKADDR*)&addr,addrlen) != 0)
    {
        printf("conn error !!!! %ld ",WSAGetLastError());
        on_recieve_cancel_button_clicked();
        return;
    }
    
    else
        recievefile();  

}

void on_recieve_cancel_button_clicked()
{
    
    closesocket(conn);
    WSACleanup();
    gtk_widget_hide(recieve_screen);
    gtk_widget_show(main_screen);

}

void on_send_cancel_button_clicked()
{
    
    closesocket(listener);
    closesocket(conn);
    WSACleanup();
    gtk_widget_hide(send_screen);
    gtk_widget_show(main_screen);

}

void on_window_main_destroy()
{
    
    gtk_main_quit();

}
void on_send_screen_destroy()
{

    closesocket(listener);
    WSACleanup();
    gtk_main_quit();

}
void on_intermediate_rcv_destroy()
{
    gtk_main_quit();
}

void on_recieve_screen_destroy()
{
    closesocket(conn);
    WSACleanup();
    gtk_main_quit();

}

void on_yes_button_clicked()
{
    
    printf("Clicked yes\n");
    confirm = 1;
    gtk_widget_show(main_screen);     
    gtk_widget_hide(msgbox);     

}

void on_no_button_clicked()
{ 
    
    printf("Clicked no\n");
    confirm = 0;
    gtk_widget_show(main_screen);
    gtk_widget_hide(msgbox);     

}

void gui_init()
{
    cssloader = gtk_css_provider_new ();
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "cender.glade", NULL);
    gtk_css_provider_load_from_path(cssloader,"style.css",NULL);
    printf("\nDebug\n");
    main_screen = GTK_WIDGET(gtk_builder_get_object(builder, "main_screen"));
    // gtk_builder_connect_signals(builder,NULL);
    recieve_screen = GTK_WIDGET(gtk_builder_get_object(builder, "recieve_screen"));
    send_screen = GTK_WIDGET(gtk_builder_get_object(builder, "send_screen"));
    intermediate_rcv = GTK_WIDGET(gtk_builder_get_object(builder, "intermediate_rcv"));
    msgbox = GTK_WIDGET(gtk_builder_get_object(builder,"msgbox"));
    send_label = GTK_LABEL(gtk_builder_get_object(builder,"send_label"));
    recieve_label = GTK_LABEL(gtk_builder_get_object(builder,"recieve_label"));
    info_label = GTK_LABEL(gtk_builder_get_object(builder,"info_lbl"));
    send_bar = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"send_bar"));
    recieve_bar = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"recieve_bar"));
    send_button = GTK_WIDGET(gtk_builder_get_object(builder,"send_button"));
    recieve_button = GTK_WIDGET(gtk_builder_get_object(builder,"recieve_button"));
    start_rcv_button = GTK_WIDGET(gtk_builder_get_object(builder,"start_rcv_button"));
    yes_button = GTK_WIDGET(gtk_builder_get_object(builder,"yes_button"));
    no_button = GTK_WIDGET(gtk_builder_get_object(builder,"no_button"));
    recieve_cancel_button = GTK_WIDGET(gtk_builder_get_object(builder,"recieve_cancel_button"));
    send_cancel_button = GTK_WIDGET(gtk_builder_get_object(builder,"send_cancel_button"));
    ip_input = GTK_ENTRY(gtk_builder_get_object(builder,"input_ip"));

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssloader), GTK_STYLE_PROVIDER_PRIORITY_USER);

    g_signal_connect(send_button, "clicked", G_CALLBACK (on_send_button_clicked), NULL);
    g_signal_connect(recieve_button, "clicked", G_CALLBACK (on_recieve_button_clicked), NULL);
    g_signal_connect(yes_button, "clicked", G_CALLBACK (on_yes_button_clicked), NULL);
    g_signal_connect(no_button, "clicked", G_CALLBACK (on_no_button_clicked), NULL);
    g_signal_connect(start_rcv_button, "clicked", G_CALLBACK (on_start_recieving), NULL);
    g_signal_connect(recieve_cancel_button, "clicked", G_CALLBACK (on_recieve_cancel_button_clicked), NULL);
    g_signal_connect(send_cancel_button, "clicked", G_CALLBACK (on_send_cancel_button_clicked), NULL);
        
    g_signal_connect(main_screen, "destroy", G_CALLBACK (on_window_main_destroy), NULL);
    g_signal_connect(recieve_screen, "destroy", G_CALLBACK (on_recieve_screen_destroy), NULL);
    g_signal_connect(send_screen, "destroy", G_CALLBACK (on_send_screen_destroy), NULL);
    g_signal_connect(intermediate_rcv, "destroy", G_CALLBACK (on_intermediate_rcv_destroy), NULL);
    
    g_object_unref(builder);

    gtk_widget_show_all(main_screen);                
    gtk_main();

}

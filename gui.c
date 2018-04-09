#include<winsock2.h>
#include<gtk/gtk.h>
#include "gui.h"
#include "network.h"


void fadeoutscreen(GtkWidget *which_screen) 
{
     //Sleep(3000);
    gtk_widget_hide(which_screen);
    gtk_widget_show(main_screen);
    UPDATE_GUI();
}


void showdialog(GtkWidget* parent,GtkMessageType type,const char* message)
{
    
    GtkWidget* dialog = gtk_message_dialog_new ((GtkWindow*)parent,
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     type,
                                     GTK_BUTTONS_CLOSE,
                                     message);
    gtk_dialog_run (GTK_DIALOG (dialog));
    UPDATE_GUI();
    gtk_widget_destroy (dialog);
    if(parent!=main_screen)
    {
        #ifdef DBG_MODE
            DEBUG("parent!=main_screen\n");
        #endif
        gtk_widget_hide(parent);
        gtk_widget_show(main_screen);
    }
}

gboolean showdialog_threaded(void *args)
{
    struct dialog_args *dargs = args;
    if(dargs->screen == NULL || dargs==NULL)
        return FALSE;
    #ifdef DBG_MODE
        printf("Screen is %s",dargs->screen);
    #endif
    showdialog((!strcmp(dargs->screen,"send_screen")) ? send_screen : recieve_screen,dargs->dialog_type,dargs->message);
    g_slice_free(struct dialog_args, dargs);
    return FALSE;
}

gboolean set_progress_threaded(void *args)
{
    struct progress_args *pargs = args;
    if(pargs== NULL || pargs->bar==NULL)
        return FALSE;
    #ifdef DBG_MODE
        printf("\ncalled set_progress_threaded with name as %s \n",pargs->bar);
    #endif
    gtk_progress_bar_set_fraction((!strcmp(pargs->bar,"send_bar")) ? send_bar : recieve_bar,pargs->value);
    return FALSE;
}

void on_send_button_clicked()
{
    gtk_progress_bar_set_fraction(send_bar,0);
    UPDATE_GUI();
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
        char name[255],ip[25]="Connect to ";
           
       
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
        #ifdef DBG_MODE
            DEBUG("Reached sndclikc\n"); 
        #endif
        result = bind(listener,(SOCKADDR*)&addr,sizeof(addr));
        gethostname(name,sizeof(name));
        hostinfo = gethostbyname(name);

        strcat(ip,inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[0]));
        #ifdef DBG_MODE
            printf("Ip is %s \n",ip);
        #endif
        
        if(!strcmp(ip,"127.0.0.1"))
        {
            gtk_widget_destroy(dialog);
            showdialog(send_screen,GTK_MESSAGE_ERROR,"Please connect to a network before sending file!");
            on_send_cancel_button_clicked();
            return;
        }

        gtk_label_set_text(info_label,ip);
            
        if (result == SOCKET_ERROR) 
        {
            printf("bind failed with error: %d\n", WSAGetLastError());
            showdialog(send_screen,GTK_MESSAGE_ERROR,"Server bind failed!");
            on_send_cancel_button_clicked();
            return;
        }
    
        addrlen = sizeof(addr);
        listen(listener,SOMAXCONN);    

        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        gtk_widget_destroy(dialog);
        UPDATE_GUI();
        
        g_thread_new("start_threaded_send",start_threaded_send,filename);
        
    }
    
    else
    {
        gtk_widget_destroy (dialog);
        on_send_cancel_button_clicked();
    }
    
}

void on_recieve_button_clicked()
{
    #ifdef DBG_MODE
        DEBUG("Reached on_recieve_button_clicked\n");
    #endif
    gtk_widget_hide(main_screen);
    gtk_widget_show(intermediate_rcv);
    
}

void on_start_recieving()
{
    
    gtk_widget_hide(intermediate_rcv);
    gtk_progress_bar_set_fraction(recieve_bar,0);
    char ip[15];
    strcpy(ip,gtk_entry_get_text (ip_input));
    #ifdef DBG_MODE
        printf("Ip entered is %s\n length is %d",ip,strlen(ip));
    #endif
    DLLVERSION = MAKEWORD(2,1);
    if(WSAStartup(DLLVERSION,&wsdata)!=0)
    {
        printf("Error!!!");
        showdialog(recieve_screen,GTK_MESSAGE_ERROR,"WSA startup failed!!");
        on_recieve_cancel_button_clicked();
        return;
    }
    
    int addrlen = sizeof(addr);
    addr.sin_port = htons(7089);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);

    gtk_widget_show(recieve_screen);     
    //scanf(" %s",&ip);
   
    conn = socket(AF_INET,SOCK_STREAM,0);
    
    if(connect(conn,(SOCKADDR*)&addr,addrlen) != 0)
    {
        printf("conn error !!!! %ld ",WSAGetLastError());
        showdialog(recieve_screen,GTK_MESSAGE_ERROR,"Connection error!!");
        on_recieve_cancel_button_clicked();
        return;
    }
    
    else
        //recievefile();  
        g_thread_new("start_threaded_recieve",(void*)recievefile,NULL);
    
        

}

void on_recieve_cancel_button_clicked()
{
    
    closesocket(conn);
    WSACleanup();
    fadeoutscreen(recieve_screen);

}

void on_send_cancel_button_clicked()
{
    
    closesocket(listener);
    closesocket(conn);
    WSACleanup();
    fadeoutscreen(send_screen);

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


void gui_init()
{
    cssloader = gtk_css_provider_new ();
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "cender.glade", NULL);
    gtk_css_provider_load_from_path(cssloader,"style.css",NULL);
    main_screen = GTK_WIDGET(gtk_builder_get_object(builder, "main_screen"));
    // gtk_builder_connect_signals(builder,NULL);
    recieve_screen = GTK_WIDGET(gtk_builder_get_object(builder, "recieve_screen"));
    send_screen = GTK_WIDGET(gtk_builder_get_object(builder, "send_screen"));
    intermediate_rcv = GTK_WIDGET(gtk_builder_get_object(builder, "intermediate_rcv"));
    send_label = GTK_LABEL(gtk_builder_get_object(builder,"send_label"));
    recieve_label = GTK_LABEL(gtk_builder_get_object(builder,"recieve_label"));
    info_label = GTK_LABEL(gtk_builder_get_object(builder,"info_lbl"));
    send_bar = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"send_bar"));
    recieve_bar = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"recieve_bar"));
    send_button = GTK_WIDGET(gtk_builder_get_object(builder,"send_button"));
    recieve_button = GTK_WIDGET(gtk_builder_get_object(builder,"recieve_button"));
    start_rcv_button = GTK_WIDGET(gtk_builder_get_object(builder,"start_rcv_button"));
    recieve_cancel_button = GTK_WIDGET(gtk_builder_get_object(builder,"recieve_cancel_button"));
    send_cancel_button = GTK_WIDGET(gtk_builder_get_object(builder,"send_cancel_button"));
    ip_input = GTK_ENTRY(gtk_builder_get_object(builder,"input_ip"));

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssloader), GTK_STYLE_PROVIDER_PRIORITY_USER);

    g_signal_connect(send_button, "clicked", G_CALLBACK (on_send_button_clicked), NULL);
    g_signal_connect(recieve_button, "clicked", G_CALLBACK (on_recieve_button_clicked), NULL);
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

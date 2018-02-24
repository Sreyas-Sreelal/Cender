#include<gtk/gtk.h>

#define UPDATE_GUI() while(gtk_events_pending())gtk_main_iteration_do(FALSE)

GtkBuilder *builder; 

GtkCssProvider *cssloader;

GtkWidget 
          *main_screen,
          *recieve_screen,
          *send_screen,
          *intermediate_rcv,
          *send_button,
          *recieve_button,
          *start_rcv_button,
          *yes_button,
          *no_button,
          *recieve_cancel_button,
          *send_cancel_button;

GtkProgressBar
        *send_bar,
        *recieve_bar;

GtkEntry          
        *ip_input;

GtkLabel
        *send_label,
        *recieve_label,
        *info_label;
        
       
void gui_init();
void on_window_main_destroy();
void on_send_button_clicked();
void on_recieve_button_clicked();
void on_start_recieving();
void on_send_screen_destroy();
void on_recieve_screen_destroy();
void on_intermediate_rcv_destroy();
void on_recieve_cancel_button_clicked();
void on_send_cancel_button_clicked();
void showdialog(GtkWidget*,GtkMessageType,const char* message);

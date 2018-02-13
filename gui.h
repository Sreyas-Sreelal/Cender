#include<gtk/gtk.h>

GtkBuilder *builder; 

GtkWidget 
          *main_screen,
          *recieve_screen,
          *send_screen,
          *intermediate_rcv,
          *msgbox,
          *send_button,
          *recieve_button,
          *start_rcv_button,
          *yes_button,
          *no_button;
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
void sndclick();
void rcvclk();
void onyes();
void onno();
void on_start_recieving();


#include<gtk/gtk.h>

GtkBuilder *builder; 

GtkWidget 
          *main_screen,
          *recieve_screen,
          *send_screen,
          *msgbox,
          *send_label,
          *recieve_label,
          *send_bar,
          *recieve_bar;

void gui_init();
void on_window_main_destroy();
void sndclick();
void rcvclk();
void onyes();
void onno();


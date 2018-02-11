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
          *recieve_bar,
          *send_button,
          *recieve_button,
          *yes_button,
          *no_button;

void gui_init();
void on_window_main_destroy();
void sndclick();
void rcvclk();
void onyes();
void onno();


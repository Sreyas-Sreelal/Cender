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
    gui_init();
}
gcc main.c network.c gui.c cender.res -o cender.exe `pkg-config --cflags --libs gtk+-3.0` -lws2_32
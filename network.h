#pragma once
#include<winsock2.h>
#include "main.h"
WSADATA  wsdata;
WORD DLLVERSION;
SOCKADDR_IN addr;
SOCKET listener,conn;

int senddata(SOCKET, void*,int);
int sendlong(SOCKET, long long);
int sendfile(SOCKET, FILE*);
int readdata(SOCKET, void*,int);
int readlong(SOCKET, long long*);
int readfile(SOCKET, FILE*);
int filesend(char*);
void recievefile();
gpointer start_threaded_send(gpointer);


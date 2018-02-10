#include<winsock2.h>
#include "main.h"
WSADATA  wsdata;
WORD DLLVERSION;
SOCKADDR_IN addr;
SOCKET listener,conn;

int senddata(SOCKET, void*,int);
int sendlong(SOCKET, long);
int sendfile(SOCKET, FILE*);
int readdata(SOCKET, void*,int);
int readlong(SOCKET, long*);
int readfile(SOCKET, FILE*);
void filesend();
void recievefile();


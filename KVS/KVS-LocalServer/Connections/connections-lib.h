#ifndef _CONNECTIONS_LIB_H

#define _CONNECTIONS_LIB_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

void setup_connections();
void start_connections();
//void stop_connections();

#endif
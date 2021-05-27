#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "./Connections/connections-lib.h"
#include "./Console/console-lib.h"

int main() {
	setup_connections();

	start_connections();

	UI();

	return 0;
}
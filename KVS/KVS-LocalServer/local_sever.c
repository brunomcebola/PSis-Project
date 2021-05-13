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

/*void console() {
	
	int order_number = 0;

	while (1) {
		order_number = home_screen(&order_number);

		switch (order_number) {
			case 1:
				// Create Group functions
				break;
			case 2:
				// Delete Group Functions
				break;
			case 3:
				// Show Group Info Functions
				break;
			case 4:
				// Show Application Status Functions
				break;
			default:
				printf("You didn't choose the right option, try again.\n");
				break;
		}
	}

	pthread_exit(0);  // check later if we need to return anything in this thread
}*/

int main() {
	setup_connections();

	start_connections();

	// TODO local_server console, after being started

	while (1) {
	}

	return 0;
}
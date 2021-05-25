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

void handle_options(int option) {
	return;

	char *group_id, buffer[INT_MAX];

	switch(option) {
		case 1: // create group
			fgets(buffer, INT_MAX - 1, stdin);

			group_id = calloc(strlen(buffer) + 1, sizeof(char));
			strncpy(group_id, buffer, strlen(buffer));

			create_group(group_id);

			free(group_id);
			break;

		case 2: // delete group 
			/* code */
			break;

		case 3:
			/* code */
			break;

		case 4:
			/* code */
			break;

		default:
			break;
	}
}

int main() {
	int option = 0;

	setup_connections();

	start_connections();

	while(1) {
		option = menu();

		handle_options(1);
	}

	return 0;
}
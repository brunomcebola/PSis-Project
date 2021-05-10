#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

//#include "../../Hashtable/hashtable-lib.h"
//#include "./Console/console-lib.h"

#define LOCAL_SERVER_ADRESS "/tmp/kvs_local_server_socket"

typedef struct _connection {
	struct sockaddr_un addr;
	int socket;
	pthread_t thread;
	struct _connection *next;
} connection;

int local_server_socket = -1;
connection *connections_list = NULL;

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

void *connection_handler(void *connection) {
	printf("new connection\n\n");
	while (1) {
	}
}

void *connections_listener(void *arg) {
	int sockaddr_size = sizeof(struct sockaddr_un);
	connection new_connection;

	while (1) {
		new_connection.socket = accept(local_server_socket, (struct sockaddr *)&(new_connection.addr), (socklen_t *)&sockaddr_size);
		if (new_connection.socket == -1) {
			perror("Error");
			exit(-1);  // arranjar erros
		} else {
			if (connections_list == NULL) {
				new_connection.next = NULL;
			} else {
				new_connection.next = connections_list;
			}
			connections_list = &new_connection;

			pthread_create(&(new_connection.thread), NULL, connection_handler, (void *)&new_connection);
		}
	}
}

int main() {
	// TODO connection to the authentication_server

	printf("Starting server...\n\n");

	struct sockaddr_un local_server_addr;

	local_server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (local_server_socket == -1) {
		perror("Error creating the stream socket on the local server");
		exit(-1);  // arranjar erros
	}

	local_server_addr.sun_family = AF_UNIX;
	sprintf(local_server_addr.sun_path, LOCAL_SERVER_ADRESS);
	unlink(LOCAL_SERVER_ADRESS);

	int err = bind(local_server_socket, (struct sockaddr *)&local_server_addr, sizeof(local_server_addr));
	if (err == -1) {
		perror("Error binding the local server socket\n");
		exit(-1);  // arranjar erros
	}

	listen(local_server_socket, 10);

	// start the local_sever
	pthread_t listening_thread;
	pthread_create(&listening_thread, NULL, connections_listener, NULL);

	printf("Ready to receive connections!\n\n");

	// TODO local_server console, after being started

	while (1) {
	}

	return 0;
}
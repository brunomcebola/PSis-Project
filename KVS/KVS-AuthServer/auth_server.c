#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "../../Hashtable/hashtable-lib.h"

#define AUTH_SERVER_ADDRESS "127.0.0.1"
#define AUTH_SERVER_PORT 3000
#define AUTH_CONSOLE_SERVER_PORT 3002

typedef struct {
	char id[1024];
	char secret[256];
} group_auth;

int auth_server_socket = -1;

struct sockaddr_in local_server_addr;

void* console_Auth_server(void* arg) {
	int bytes = -1;
	int opt = 1;
	char type = 'N';

	int len = sizeof(struct sockaddr_in);

	struct sockaddr_in auth_console_server_addr;

	printf("Starting console server...\n\n");

	// getting the socket ready
	if((auth_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		perror("");
		exit(-1);
	}

	// "unlink" of the inet
	if(setsockopt(auth_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("");
		exit(-1);
	}
	// setting up the server info
	auth_console_server_addr.sin_family = AF_INET;
	auth_console_server_addr.sin_addr.s_addr = INADDR_ANY;
	auth_console_server_addr.sin_port = htons(AUTH_CONSOLE_SERVER_PORT);

	if(bind(auth_server_socket, (struct sockaddr*)&auth_console_server_addr, sizeof(auth_console_server_addr)) < 0) {
		perror("");
		exit(-1);
	}

	printf("Console server up and running!\n\n");

	while(1) {
		bytes = recvfrom(auth_server_socket, &type, sizeof(char), MSG_WAITALL, (struct sockaddr*)&local_server_addr, &len);
		if(bytes == -1) {
			perror("");
			exit(-1);
		}

		switch(type) {
			case 'C': // create group
				// code
				break;
			case 'D': // delete group
				// code
				break;
			case 'S': // giving group secret (group info of console)
				// code
				break;

			default: // handling errors ??
				break;
		}
	}
	pthread_exit(NULL);
}

int setup_server() {
	int opt = 1;

	struct sockaddr_in auth_server_addr;

	printf("Starting server...\n\n");

	// getting the socket ready
	if((auth_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		perror("");
		exit(-1);
	}

	// "unlink" of the inet
	if(setsockopt(auth_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("");
		exit(-1);
	}
	// setting up the server info
	auth_server_addr.sin_family = AF_INET;
	auth_server_addr.sin_addr.s_addr = INADDR_ANY;
	auth_server_addr.sin_port = htons(AUTH_SERVER_PORT);

	if(bind(auth_server_socket, (struct sockaddr*)&auth_server_addr, sizeof(auth_server_addr)) < 0) {
		perror("");
		exit(-1);
	}

	printf("Server up and running!\n\n");
}

int main(int argc, char const* argv[]) {

	int bytes = -1; // checking predifined functions errors
	int code = -1; // error handling
	int group_id_len = 0, secret_len = 0;
	char *group_id = NULL, *secret = NULL;

	int len = sizeof(struct sockaddr_in);

	group_auth group_auth_info;

	pthread_t console_server_thread;

	setup_server();
	pthread_create(&console_server_thread, NULL, console_Auth_server, NULL);
	// key_pair **groups = create_hash_table();

	while(1) {
		bytes = recvfrom(
			auth_server_socket, &group_auth_info, sizeof(group_auth_info), MSG_WAITALL, (struct sockaddr*)&local_server_addr, &len);
		if(bytes == -1) {
			perror("");
			exit(-1);
		}

		// handling auth_server thingys
		code = 10;

		sendto(auth_server_socket, &code, sizeof(int), MSG_CONFIRM, (struct sockaddr*)&local_server_addr, len);
	}

	pthread_exit(NULL);
}

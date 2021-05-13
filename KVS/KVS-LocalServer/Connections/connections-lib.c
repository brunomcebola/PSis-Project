#include "./connections-lib.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define LOCAL_SERVER_ADRESS "/tmp/kvs_local_server_socket"
#define AUTH_SERVER_ADDRESS "127.0.0.1"
#define AUTH_SERVER_PORT 3000

typedef struct _connection {
	struct sockaddr_un addr;
	int socket;
	pthread_t thread;
	struct _connection *next;
} connection;

connection *connections_list = NULL;
pthread_t listening_thread;

int local_server_unix_socket;
int local_server_inet_socket;
struct sockaddr_in auth_server_inet_socket_addr;
struct sockaddr_un local_server_unix_socket_addr;

void *connection_handler(void *conn) {
	int bytes = -1, len = 0;
	int code = 0;
	int group_id_len = 0, secret_len = 0;
	char *group_id = NULL, *secret = NULL;

	len = sizeof(auth_server_inet_socket_addr);

	// receive info from app

	bytes = read(((connection *)conn)->socket, &group_id_len, sizeof(int));
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	group_id = calloc(group_id_len, sizeof(char));
	bytes = read(((connection *)conn)->socket, group_id, group_id_len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = read(((connection *)conn)->socket, &secret_len, sizeof(int));
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	secret = calloc(secret_len, sizeof(char));
	bytes = read(((connection *)conn)->socket, secret, secret_len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	// send info to auth server

	bytes = sendto(local_server_inet_socket, &group_id_len, sizeof(int), MSG_CONFIRM, (const struct sockaddr *)&auth_server_inet_socket_addr, len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = sendto(local_server_inet_socket, group_id, group_id_len, MSG_CONFIRM, (const struct sockaddr *)&auth_server_inet_socket_addr, len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = sendto(local_server_inet_socket, &secret_len, sizeof(int), MSG_CONFIRM, (const struct sockaddr *)&auth_server_inet_socket_addr, len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = sendto(local_server_inet_socket, secret, secret_len, MSG_CONFIRM, (const struct sockaddr *)&auth_server_inet_socket_addr, len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	// handle response from auth server

	bytes = recvfrom(local_server_inet_socket, &code, sizeof(int), MSG_WAITALL, (struct sockaddr *)&auth_server_inet_socket_addr, &len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = write(((connection *)conn)->socket, &code, sizeof(int));
	if (bytes == 0) {
		perror("");
		exit(-1);
	}

	while (1) {
	}
}

void *connections_listener(void *arg) {
	int sockaddr_size = sizeof(struct sockaddr_un);
	connection *new_connection = NULL;

	printf("Ready to receive connections!\n\n");

	while (1) {
		new_connection = calloc(1, sizeof(connection));

		new_connection->socket = accept(local_server_unix_socket, (struct sockaddr *)&(new_connection->addr), (socklen_t *)&sockaddr_size);
		if (new_connection->socket == -1) {
			perror("");
			exit(-1);
		} else {
			printf("New connection established!\n\n");

			if (connections_list == NULL) {
				new_connection->next = NULL;
			} else {
				new_connection->next = connections_list;
			}

			connections_list = new_connection;

			pthread_create(&(new_connection->thread), NULL, connection_handler, new_connection);
		}
	}
}

void start_connections() {
	listen(local_server_unix_socket, 10);

	pthread_create(&listening_thread, NULL, connections_listener, NULL);
}

void setup_connections() {
	printf("Starting server...\n\n");

	// inicialização da ligação ao servidor de autenticação

	local_server_inet_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (local_server_inet_socket == -1) {
		perror("");
		exit(-1);
	}

	auth_server_inet_socket_addr.sin_family = AF_INET;
	auth_server_inet_socket_addr.sin_port = htons(AUTH_SERVER_PORT);
	auth_server_inet_socket_addr.sin_addr.s_addr = INADDR_ANY;

	// inicialização do servidor local

	local_server_unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (local_server_unix_socket == -1) {
		perror("");
		exit(-1);
	}

	local_server_unix_socket_addr.sun_family = AF_UNIX;
	sprintf(local_server_unix_socket_addr.sun_path, LOCAL_SERVER_ADRESS);
	unlink(LOCAL_SERVER_ADRESS);

	int err = bind(local_server_unix_socket, (struct sockaddr *)&(local_server_unix_socket_addr), sizeof(local_server_unix_socket_addr));
	if (err == -1) {
		perror("");
		exit(-1);
	}

	printf("Server up and running!\n\n");

	return;
}
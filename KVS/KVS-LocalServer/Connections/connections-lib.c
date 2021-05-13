#include "./connections-lib.h"

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "../../../Hashtable/hashtable-lib.h"

#define LOCAL_SERVER_ADRESS "/tmp/kvs_local_server_socket"
#define AUTH_SERVER_ADDRESS "127.0.0.1"
#define AUTH_SERVER_PORT 3000

#define PUT 'P'
#define GET 'G'
#define DEL 'D'
#define RCB 'R'

typedef struct _connection_t {
	struct sockaddr_un addr;
	int socket;
	pthread_t thread;
	struct _connection_t *next;
} connection_t;

typedef struct _group {
	char *group_id;
	key_pair **hash_table;
	struct _group *next;
} group_t;

connection_t *connections_list = NULL;
group_t *groups_list = NULL;
pthread_t listening_thread;

int local_server_unix_socket;
int local_server_inet_socket;
struct sockaddr_in auth_server_inet_socket_addr;
struct sockaddr_un local_server_unix_socket_addr;

void put_value(void *connection, group_t *group) {
	int bytes = -1;
	int key_len = 0, value_len = 0;
	char *key = NULL, *value = NULL;

	bytes = read(((connection_t *)connection)->socket, &key_len, sizeof(int));
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	key = calloc(key_len, sizeof(char));
	bytes = read(((connection_t *)connection)->socket, key, key_len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = read(((connection_t *)connection)->socket, &value_len, sizeof(int));
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	value = calloc(value_len, sizeof(char));
	bytes = read(((connection_t *)connection)->socket, value, value_len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	put_on_hash_table(group->hash_table, key, value);
}

void *connection_handler(void *connection) {
	int bytes = -1, code = 0;
	int len = sizeof(struct sockaddr_in);
	int group_id_len = 0, secret_len = 0;
	char *group_id = NULL, *secret = NULL, operation_type = '\0';

	// receive info from app

	bytes = read(((connection_t *)connection)->socket, &group_id_len, sizeof(int));
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	group_id = calloc(group_id_len, sizeof(char));
	bytes = read(((connection_t *)connection)->socket, group_id, group_id_len);
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = read(((connection_t *)connection)->socket, &secret_len, sizeof(int));
	if (bytes == -1) {
		perror("");
		exit(-1);
	}

	secret = calloc(secret_len, sizeof(char));
	bytes = read(((connection_t *)connection)->socket, secret, secret_len);
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

	bytes = write(((connection_t *)connection)->socket, &code, sizeof(int));
	if (bytes == 0) {
		perror("");
		exit(-1);
	}

	//

	group_t *group = groups_list;

	while (group != NULL) {
		if (strcmp(group->group_id, group_id) == 0) {
			break;
		}
		group = group->next;
	}

	if (group == NULL) {
		group = calloc(1, sizeof(group_t));

		group->group_id = calloc(group_id_len, sizeof(char));
		strcpy(group->group_id, group_id);

		group->hash_table = create_hash_table();

		group->next = groups_list;

		groups_list = group;
	}

	while (1) {
		bytes = read(((connection_t *)connection)->socket, &operation_type, sizeof(char));
		if (bytes == -1) {
			perror("");
			exit(-1);
		}

		switch (operation_type) {
			case PUT:
				put_value(connection, group);
				break;

			default:
				break;
		}
	}
}

void *connections_listener(void *arg) {
	int sockaddr_size = sizeof(struct sockaddr_un);
	connection_t *connection = NULL;

	printf("Ready to receive connections!\n\n");

	while (1) {
		connection = calloc(1, sizeof(connection_t));

		connection->socket = accept(local_server_unix_socket, (struct sockaddr *)&(connection->addr), (socklen_t *)&sockaddr_size);
		if (connection->socket == -1) {
			perror("");
			exit(-1);
		} else {
			printf("New connection_t established!\n\n");

			if (connections_list == NULL) {
				connection->next = NULL;
			} else {
				connection->next = connections_list;
			}

			connections_list = connection;

			pthread_create(&(connection->thread), NULL, connection_handler, connection);
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
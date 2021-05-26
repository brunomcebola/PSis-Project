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

#include "../../configs.h"

#include "../../../Hashtable/hashtable-lib.h"

typedef struct _connection_t {
	struct sockaddr_un addr;
	int socket;
	pthread_t thread;
	struct _connection_t* next;
} connection_t;

typedef struct _group {
	char group_id[MAX_ID + 1];
	key_pair** hash_table;
	struct _group* next;
} group_t;

connection_t* connections_list = NULL;
group_t* groups_list = NULL;
pthread_t listening_thread;

int local_server_unix_socket;
int local_server_inet_socket;
struct sockaddr_un local_server_unix_socket_addr;
struct sockaddr_in apps_auth_server_inet_socket_addr;
struct sockaddr_in console_auth_server_inet_socket_addr;

void put_value(void* connection, group_t* group) {
	int bytes = -1, code = 0;
	int key_len = 0, value_len = 0;
	char *key = NULL, *value = NULL;

	bytes = read(((connection_t*)connection)->socket, &key_len, sizeof(int));
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	key = calloc(key_len, sizeof(char));
	bytes = read(((connection_t*)connection)->socket, key, key_len);
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = read(((connection_t*)connection)->socket, &value_len, sizeof(int));
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	value = calloc(value_len, sizeof(char));
	bytes = read(((connection_t*)connection)->socket, value, value_len);
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	put_on_hash_table(group->hash_table, key, value);

	code = 10;

	bytes = write(((connection_t*)connection)->socket, &code, sizeof(int));
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	free(key);
	free(value);
}

void get_value(void* connection, group_t* group) {
	int bytes = 0, len = 0;
	char *key = NULL, **value = NULL;

	bytes = read(((connection_t*)connection)->socket, &len, sizeof(int));
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	key = calloc(len, sizeof(char));
	bytes = read(((connection_t*)connection)->socket, key, len);
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	value = calloc(1, sizeof(char*));

	get_from_hash_table(group->hash_table, key, value);

	len = strlen(*value);

	bytes = write(((connection_t*)connection)->socket, &len, sizeof(int));
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = write(((connection_t*)connection)->socket, *value, len);
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	free(key);
	free(*value);
	free(value);
}

void* connection_handler(void* connection) {
	int bytes = -1, code = 0;
	int len = sizeof(struct sockaddr_in);
	int group_id_len = 0, secret_len = 0;
	char operation_type = '\0';
	access_credentials group_auth_info;
	group_t* group;

	// receive info from app
	bytes = read(((connection_t*)connection)->socket, &group_id_len, sizeof(int));
	if(bytes == -1) {
		perror("");
		exit(-1);
	} else if(group_id_len > 1024) {
		exit(-2);
	}

	bytes = read(((connection_t*)connection)->socket, group_auth_info.id, group_id_len);
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = read(((connection_t*)connection)->socket, &secret_len, sizeof(int));
	if(bytes == -1) {
		perror("");
		exit(-1);
	} else if(secret_len > 256) {
		exit(-2);
	}

	bytes = read(((connection_t*)connection)->socket, group_auth_info.secret, secret_len);
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	// send info to auth server
	bytes = sendto(local_server_inet_socket,
				   &group_auth_info,
				   sizeof(group_auth_info),
				   MSG_CONFIRM,
				   (const struct sockaddr*)&apps_auth_server_inet_socket_addr,
				   len);
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	// handle response from auth server
	bytes = recvfrom(local_server_inet_socket,
					 &code,
					 sizeof(int),
					 MSG_WAITALL,
					 (struct sockaddr*)&apps_auth_server_inet_socket_addr,
					 &len);
	if(bytes == -1) {
		perror("");
		exit(-1);
	}

	bytes = write(((connection_t*)connection)->socket, &code, sizeof(int));
	if(bytes == 0) {
		perror("");
		exit(-1);
	}

	//

	group = groups_list;

	while(group != NULL) {
		if(strcmp(group->group_id, group_auth_info.id) == 0) {
			break;
		}
		group = group->next;
	}

	if(group == NULL) {
		group = calloc(1, sizeof(group_t));

		strncpy(group->group_id, group_auth_info.id, MAX_ID);

		group->hash_table = create_hash_table();

		group->next = groups_list;

		groups_list = group;
	}

	while(1) {
		bytes = read(((connection_t*)connection)->socket, &operation_type, sizeof(char));
		if(bytes == -1) {
			perror("");
			exit(-1);
		}

		switch(operation_type) {
			case PUT:
				put_value(connection, group);
				break;

			case GET:
				get_value(connection, group);
				break;

			default:
				break;
		}
	}
}

void* connections_listener(void* arg) {
	int sockaddr_size = sizeof(struct sockaddr_un);
	connection_t* connection = NULL;

	while(1) {
		connection = calloc(1, sizeof(connection_t));

		connection->socket = accept(local_server_unix_socket,
									(struct sockaddr*)&(connection->addr),
									(socklen_t*)&sockaddr_size);
		if(connection->socket == -1) {
			perror("");
			exit(-1);
		} else {
			connection->next = connections_list;

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
	// inicialização da ligação ao servidor de autenticação
	local_server_inet_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(local_server_inet_socket == -1) {
		perror("");
		exit(-1);
	}

	apps_auth_server_inet_socket_addr.sin_family = AF_INET;
	apps_auth_server_inet_socket_addr.sin_port = htons(APPS_AUTH_SERVER_PORT);
	apps_auth_server_inet_socket_addr.sin_addr.s_addr = INADDR_ANY;

	console_auth_server_inet_socket_addr.sin_family = AF_INET;
	console_auth_server_inet_socket_addr.sin_port = htons(CONSOLE_AUTH_SERVER_PORT);
	console_auth_server_inet_socket_addr.sin_addr.s_addr = INADDR_ANY;

	// inicialização do servidor local

	local_server_unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if(local_server_unix_socket == -1) {
		perror("");
		exit(-1);
	}

	local_server_unix_socket_addr.sun_family = AF_UNIX;
	sprintf(local_server_unix_socket_addr.sun_path, LOCAL_SERVER_ADRESS);
	unlink(LOCAL_SERVER_ADRESS);

	int err = bind(local_server_unix_socket,
				   (struct sockaddr*)&(local_server_unix_socket_addr),
				   sizeof(local_server_unix_socket_addr));
	if(err == -1) {
		perror("");
		exit(-1);
	}

	return;
}

//

char* create_group(char* group_id) {
	group_t* group = groups_list;
	int bytes = -1;
	int len = sizeof(struct sockaddr_in);
	operation_packet operation;
	char* secret = NULL;

	secret = calloc(MAX_SECRET + 1, sizeof(char));

	while(group != NULL) {
		if(strcmp(group->group_id, group_id) == 0) {
			break;
		}
		group = group->next;
	}

	if(group == NULL) {
		operation.type = POST;
		strncpy(operation.id, group_id, MAX_ID);

		bytes = sendto(local_server_inet_socket,
					   &operation,
					   sizeof(operation),
					   MSG_CONFIRM,
					   (struct sockaddr*)&console_auth_server_inet_socket_addr,
					   len);
		if(bytes == -1) {
			perror("");
			exit(-1);
		}

		bytes = recvfrom(local_server_inet_socket,
						 secret,
						 MAX_SECRET + 1,
						 MSG_WAITALL,
						 (struct sockaddr*)&console_auth_server_inet_socket_addr,
						 &len);
		if(bytes == -1) {
			perror("");
			exit(-1);
		}

		group = calloc(1, sizeof(group_t));

		strncpy(group->group_id, group_id, MAX_ID);

		group->hash_table = create_hash_table();

		group->next = groups_list;

		groups_list = group;
	} else {
		// TODO: decide if returns secret or error
	}

	return secret;
}
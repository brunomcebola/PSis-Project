#include "./connections-lib.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "../../configs.h"

#include "../../../Hashtable/hashtable-lib.h"
typedef struct _group_t {
	char group_id[MAX_GROUP_ID + 1];
	pthread_rwlock_t rwlock;
	key_pair_t** hash_table;
	struct _group_t* next;
} group_t;
typedef struct _connection_t {
	int pid;
	char open_time[29];
	char close_time[29];
	int socket;
	int cb_socket;
	pthread_t thread;
	group_t* group;
	struct _connection_t* next;
} connection_t;

connection_t* connections_list = NULL;
group_t* groups_list = NULL;
pthread_t listening_thread;
pthread_rwlock_t group_list_rwlock; // TODO dar destroy na consola

int local_server_unix_socket = -1;
int cb_local_server_unix_socket = -1;
int apps_local_server_inet_socket = -1;
int console_local_server_inet_socket = -1;
struct sockaddr_un local_server_unix_socket_addr;
struct sockaddr_un cb_local_server_unix_socket_addr;
struct sockaddr_in apps_auth_server_inet_socket_addr;
struct sockaddr_in console_auth_server_inet_socket_addr;

/*******************************************************************
* 
** void setup_connections() 
*
** Description:
*		
*
** Parameters:
*  		This function has no parameters
*
** Return:
*		Returns an int indicating if the the setup was successful 
*		(SUCCESSFUL_CONNECTION) or if it failed (UNABLE_TO_CONNECT).
*		
*
** Side-effects:
*		There are no side-effects
*		TODO: wrong commentary
*******************************************************************/
int setup_connections() {
	// CONNECTIONS TO AUTH SERVER

	// creation of dgram socket to connect the requests from apps to
	// the authserver via localserver
	apps_local_server_inet_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(apps_local_server_inet_socket == -1) {
		print_error("Unable to create socket to connect apps to authentication server");
		return UNABLE_TO_CONNECT;
	}

	// setting a timeout on requests from apps to the authserver
	struct timeval tv;
	tv.tv_sec = 300; // timeout time
	if(setsockopt(apps_local_server_inet_socket, SOL_SOCKET, SO_RCVTIMEO | SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		print_error("Unable to set options on socket to connect apps to authentication server");
		return UNABLE_TO_CONNECT;
	}

	// creation of dgram socket to connect the requests from consoles to
	// the authserver via localserver
	console_local_server_inet_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(console_local_server_inet_socket == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		print_error("Unable to create socket to connect consoles to authentication server");
		return UNABLE_TO_CONNECT;
	}

	// inicializing the rwlock for the console data sharing
	if(pthread_rwlock_init(&group_list_rwlock, NULL) != 0) {
		return UNSUCCESSFUL_OPERATION;
	}

	apps_auth_server_inet_socket_addr.sin_family = AF_INET;
	apps_auth_server_inet_socket_addr.sin_port = htons(APPS_AUTH_SERVER_PORT);
	apps_auth_server_inet_socket_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER_ADDRESS);

	console_auth_server_inet_socket_addr.sin_family = AF_INET;
	console_auth_server_inet_socket_addr.sin_port = htons(CONSOLE_AUTH_SERVER_PORT);
	console_auth_server_inet_socket_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER_ADDRESS);

	// CONNECTIONS TO APPS

	// creation of unix socket to connect the requests from apps to the localserver
	local_server_unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if(local_server_unix_socket == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		print_error("Unable to create socket to connect to apps");
		return UNABLE_TO_CONNECT;
	}

	// creation of unix socket to connect the apps callbacks to the localserver
	cb_local_server_unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if(local_server_unix_socket == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		close(local_server_unix_socket);
		local_server_unix_socket = -1;
		print_error("Unable to create socket to connect to callbacks");
		return UNABLE_TO_CONNECT;
	}

	local_server_unix_socket_addr.sun_family = AF_UNIX;
	sprintf(local_server_unix_socket_addr.sun_path, LOCAL_SERVER_ADDRESS);
	unlink(LOCAL_SERVER_ADDRESS);

	cb_local_server_unix_socket_addr.sun_family = AF_UNIX;
	sprintf(cb_local_server_unix_socket_addr.sun_path, CB_LOCAL_SERVER_ADDRESS);
	unlink(CB_LOCAL_SERVER_ADDRESS);

	// bind unix socket to connect the requests from apps to the localserver
	int err;
	err = bind(local_server_unix_socket, (struct sockaddr*)&(local_server_unix_socket_addr), sizeof(struct sockaddr_un));
	if(err == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		close(local_server_unix_socket);
		local_server_unix_socket = -1;
		close(cb_local_server_unix_socket);
		cb_local_server_unix_socket = -1;
		print_error("Unable to bind socket to connect to apps");
		return UNABLE_TO_CONNECT;
	}

	// bind unix socket to connect the apps callbacks to the localserver
	err = bind(cb_local_server_unix_socket, (struct sockaddr*)&(cb_local_server_unix_socket_addr), sizeof(struct sockaddr_un));
	if(err == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		close(local_server_unix_socket);
		local_server_unix_socket = -1;
		close(cb_local_server_unix_socket);
		cb_local_server_unix_socket = -1;
		print_error("Unable to bind socket to connect to callbacks");
		return UNABLE_TO_CONNECT;
	}

	return SUCCESSFUL_CONNECTION;
}

// APPS HANDLING FUNCTIONS

void close_connection(connection_t* connection, group_t* group, int list_critical_region, int group_critical_region) {
	time_t t;
	struct tm tm;

	// groups list critical region
	if(list_critical_region) {
		pthread_rwlock_unlock(&group_list_rwlock);
	}

	// group critical region
	if(group_critical_region) {
		pthread_rwlock_unlock(&(group->rwlock));
	}

	t = time(NULL);
	localtime_r(&t, &tm);
	sprintf(connection->close_time,
			"%02d-%02d-%d %02d:%02d:%02d",
			tm.tm_mday,
			tm.tm_mon + 1,
			tm.tm_year + 1900,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec);
	close(connection->socket);
	connection->socket = -1;
	connection->group = NULL;
	pthread_exit(0);
}

/*******************************************************************
*
** void put_value(connection_t* connection, group_t* group) 
*
** Description:
*		Inserts a new key/value pair in the provided group's hashtable
*		if it doesn't exist yet. If it does exist then the value gets
*		updated.
*
** Parameters:
*  	@param connection - struct holding connections settings
*  	@param group - struct holding group infos
*
** Return:
*		Nothing is returned by the function.
*		A success/error code is sent to the client via socket write
*
** Side-effects:
*		If a write or a read from the socket is not successful then the 
*		connection is terminated (both thread and socket get closed)
*	
//	TODO: check put_on_hash_table return code
//	TODO: synchronization missing
//  TODO: callback call
*
*******************************************************************/
void put_value(connection_t* connection) {
	int bytes = -1, code = 0;
	int key_len = 0, value_len = 0;
	char *key = NULL, *value = NULL;

	bytes = read(connection->socket, &key_len, sizeof(int));
	if(bytes != sizeof(int)) {
		close_connection(connection, connection->group, 0, 1);
	}

	key = calloc(key_len, sizeof(char));
	if(key == NULL) {
		close_connection(connection, connection->group, 0, 1);
	}

	bytes = read(connection->socket, key, key_len);
	if(bytes != key_len) {
		close_connection(connection, connection->group, 0, 1);
	}

	bytes = read(connection->socket, &value_len, sizeof(int));
	if(bytes != sizeof(int)) {
		close_connection(connection, connection->group, 0, 1);
	}

	value = calloc(value_len, sizeof(char));
	if(value == NULL) {
		close_connection(connection, connection->group, 0, 1);
	}

	bytes = read(connection->socket, value, value_len);
	if(bytes != value_len) {
		close_connection(connection, connection->group, 0, 1);
	}

	code = put_on_hash_table(connection->group->hash_table, key, value);

	bytes = write(connection->socket, &code, sizeof(int));
	if(bytes != sizeof(int)) {
		close_connection(connection, connection->group, 0, 1);
	}

	free(key);
	free(value);
}

/*******************************************************************
*
** void get_value(connection_t* connection, group_t* group) 
*
** Description:
*		Gets the value corresponding to a key in the given group's 
*		hashtable.
*
** Parameters:
*  	@param connection - struct holding connections settings
*  	@param group - struct holding group infos
*
** Return:
*		Nothing is returned by the function.
*		The value is sent to the client via socket write
*
** Side-effects:
*		If a write or a read from the socket is not successful then the 
*		connection is terminated (both thread and socket get closed)
*
//	TODO: check get_from_hash_table assigned value if key nonexistent
//	TODO: synchronization missing
*	
*******************************************************************/
void get_value(connection_t* connection) {
	int bytes = 0, len = 0;
	char *key = NULL, *value = NULL;

	bytes = read(connection->socket, &len, sizeof(int));
	if(bytes != sizeof(int)) {
		close_connection(connection, connection->group, 0, 1);
	}

	key = calloc(len, sizeof(char));
	if(key == NULL) {
		close_connection(connection, connection->group, 0, 1);
	}

	bytes = read(connection->socket, key, len);
	if(bytes != len) {
		close_connection(connection, connection->group, 0, 1);
	}

	get_from_hash_table(connection->group->hash_table, key, &value);

	len = strlen(value) + 1;
	bytes = write(connection->socket, &len, sizeof(int));
	if(bytes != sizeof(int)) {
		close_connection(connection, connection->group, 0, 1);
	}

	bytes = write(connection->socket, value, len * sizeof(char));
	if(bytes != len * sizeof(char)) {
		close_connection(connection, connection->group, 0, 1);
	}

	free(key);
	free(value);
}

/*******************************************************************
*
** void delete_value(connection_t* connection, group_t* group) 
*
** Description:
*		Deletes a key/value pair from the provided group's hashtable.
*
** Parameters:
*  	@param connection - struct holding connections settings
*  	@param group - struct holding group infos
*
** Return:
*		Nothing is returned by the function.
*		A success/error code is sent to the client via socket write
*
** Side-effects:
*		If a write or a read from the socket is not successful then the 
*		connection is terminated (both thread and socket get closed)
*	
//	TODO: check delete_from_hash_table return code
//	TODO: synchronization missing
*
*******************************************************************/
void delete_value(connection_t* connection) {
	int bytes = -1, code = 0;
	char key[MAX_KEY + 1];
	connection_t* aux = connections_list;

	bytes = read(connection->socket, key, (MAX_KEY + 1) * sizeof(char));
	if(bytes != (MAX_KEY + 1) * sizeof(char)) {
		close_connection(connection, connection->group, 0, 1);
	}

	code = delete_from_hash_table(connection->group->hash_table, key);

	bytes = write(connection->socket, &code, sizeof(int));
	if(bytes != sizeof(int)) {
		close_connection(connection, connection->group, 0, 1);
	}

	while(aux != NULL) {
		if(aux->group == connection->group) {
			bytes = write(aux->cb_socket, key, (MAX_KEY + 1) * sizeof(char));
			if(bytes != (MAX_KEY + 1) * sizeof(char)) {
				close_connection(connection, connection->group, 0, 1);
			}
		}

		aux = aux->next;
	}
}

// TODO: missing code commentary
void register_callback(connection_t* connection) {
	int bytes = 0;
	int code = 0;
	char key[MAX_KEY + 1];
	char name[MAX_NAME + 1];
	sem_t* sem_id;

	// reading key
	bytes = read(connection->socket, key, (MAX_KEY + 1) * sizeof(char));
	if(bytes != (MAX_KEY + 1) * sizeof(char)) {
		close_connection(connection, connection->group, 0, 1);
	}

	// reading semaphore/pipe name
	bytes = read(connection->socket, name, (MAX_NAME + 1) * sizeof(char));
	if(bytes != (MAX_NAME + 1) * sizeof(char)) {
		close_connection(connection, connection->group, 0, 1);
	}

	code = put_sem_on_hash_table(connection->group->hash_table, key, name);

	bytes = write(connection->socket, &code, sizeof(int));
	if(bytes != sizeof(int)) {
		close_connection(connection, connection->group, 0, 1);
	}

	return;
}

// TODO: missing code commentary
// handles connections between app local auth
void* connection_handler(void* connection) {
	int bytes = -1, code = 0;
	int len = sizeof(struct sockaddr_in);
	char operation_type = '\0';

	group_t* group;
	connection_packet connection_info;

	// receive info from app
	bytes = read(((connection_t*)connection)->socket, &connection_info, sizeof(connection_packet));
	if(bytes != sizeof(connection_packet)) {
		close_connection((connection_t*)connection, NULL, 0, 0);
	}

	((connection_t*)connection)->pid = connection_info.pid;

	// TODO falta sync de coms com auth server

	// send informaton to auth server
	bytes = sendto(apps_local_server_inet_socket,
				   &(connection_info.credentials),
				   sizeof(access_packet),
				   MSG_CONFIRM,
				   (const struct sockaddr*)&apps_auth_server_inet_socket_addr,
				   len);
	if(bytes != sizeof(access_packet)) {
		close_connection((connection_t*)connection, NULL, 0, 0);
	}

	// handle response from auth server
	bytes = recvfrom(
		apps_local_server_inet_socket, &code, sizeof(int), MSG_WAITALL, (struct sockaddr*)&apps_auth_server_inet_socket_addr, &len);
	if(bytes != sizeof(int)) {
		close_connection((connection_t*)connection, NULL, 0, 0);
	}

	bytes = write(((connection_t*)connection)->socket, &code, sizeof(int));
	if(bytes != sizeof(int)) {
		close_connection((connection_t*)connection, NULL, 0, 0);
	}

	pthread_rwlock_wrlock(&group_list_rwlock);

	group = groups_list;

	while(group != NULL) {
		if(strcmp(group->group_id, connection_info.credentials.group_id) == 0) {
			break;
		}
		group = group->next;
	}

	if(group == NULL) {
		group = calloc(1, sizeof(group_t));
		if(group == NULL) {
			close_connection((connection_t*)connection, NULL, 1, 0);
		}

		strncpy(group->group_id, connection_info.credentials.group_id, MAX_GROUP_ID);

		group->hash_table = create_hash_table();

		group->next = groups_list;

		// inicializing the rwlock for each data
		if(pthread_rwlock_init(&group->rwlock, NULL) != 0) {
			close_connection((connection_t*)connection, NULL, 1, 0);
		}

		groups_list = group;
	}
	pthread_rwlock_unlock(&group_list_rwlock);

	while(1) {
		pthread_rwlock_rdlock(&group_list_rwlock);
		group = groups_list;

		while(group != NULL) {
			if(strcmp(group->group_id, connection_info.credentials.group_id) == 0) {
				break;
			}
			group = group->next;
		}

		((connection_t*)connection)->group = group;

		if(group == NULL) {
			close_connection((connection_t*)connection, NULL, 1, 0);
		}

		bytes = read(((connection_t*)connection)->socket, &operation_type, sizeof(char));
		if(bytes != sizeof(char)) {
			close_connection((connection_t*)connection, NULL, 1, 0);
		}
		pthread_rwlock_rdlock(&group->rwlock);
		pthread_rwlock_unlock(&group_list_rwlock);

		switch(operation_type) {
			case PUT:
				put_value((connection_t*)connection);
				break;

			case GET:
				get_value((connection_t*)connection);
				break;

			case DEL:
				delete_value((connection_t*)connection);
				break;

			case RCB:
				register_callback((connection_t*)connection);
				break;
		}

		pthread_rwlock_unlock(&group->rwlock);
	}
}

// TODO: missing code commentary
void* connections_listener(void* arg) {
	int sockaddr_size = sizeof(struct sockaddr_un);
	connection_t* connection = NULL;
	time_t t;
	struct tm tm;

	while(1) {
		connection = calloc(1, sizeof(connection_t));
		if(connection == NULL) {
			close_connection((connection_t*)connection, NULL, 0, 0);
		}

		connection->socket = accept(local_server_unix_socket, NULL, NULL);
		if(connection->socket != -1) {
			connection->cb_socket = accept(cb_local_server_unix_socket, NULL, NULL);
			if(connection->cb_socket == -1) {
				close(connection->socket);
				free(connection);
			} else {
				t = time(NULL);
				localtime_r(&t, &tm);
				sprintf(connection->open_time,
						"%02d-%02d-%d %02d:%02d:%02d",
						tm.tm_mday,
						tm.tm_mon + 1,
						tm.tm_year + 1900,
						tm.tm_hour,
						tm.tm_min,
						tm.tm_sec);

				connection->close_time[0] = '\0';

				connection->next = connections_list;

				connections_list = connection;

				pthread_create(&(connection->thread), NULL, connection_handler, connection);
			}

		} else {
			free(connection);
		}
	}
}

// TODO: missing code commentary
void start_connections() {
	listen(local_server_unix_socket, 10);

	listen(cb_local_server_unix_socket, 10);

	pthread_create(&listening_thread, NULL, connections_listener, NULL);
}

// CONSOLE HANDLING FUNCTIONS

// TODO: missing code commentary
void group_info(char* group_id, char** secret, int* num_pairs) {
	group_t* group = groups_list;
	int bytes = -1;
	int len = sizeof(struct sockaddr_in);
	operation_packet operation;

	while(group != NULL) {
		if(strcmp(group->group_id, group_id) == 0) {
			break;
		}
		group = group->next;
	}

	if(group != NULL) {
		operation.type = GET;
		strncpy(operation.group_id, group_id, MAX_GROUP_ID);

		bytes = sendto(console_local_server_inet_socket,
					   &operation,
					   sizeof(operation),
					   MSG_CONFIRM,
					   (struct sockaddr*)&console_auth_server_inet_socket_addr,
					   len);
		if(bytes == -1) {
			perror("");
			exit(-1);
		}

		*secret = calloc(MAX_SECRET + 1, sizeof(char));

		bytes = recvfrom(console_local_server_inet_socket,
						 *secret,
						 MAX_SECRET + 1,
						 MSG_WAITALL,
						 (struct sockaddr*)&console_auth_server_inet_socket_addr,
						 &len);

		if(bytes == -1) {
			exit(-1);
			// TODO
		}

		*num_pairs = get_number_of_entries(group->hash_table);

	} else {
		// faz sentido ser so erro mesmo, se deu merda deu merda
		// TODO: decide if returns secret or error
	}
}

// TODO: missing code commentary
char* create_group(char* group_id) {
	group_t* group = groups_list;
	int bytes = -1;
	int len = sizeof(struct sockaddr_in);
	operation_packet operation;
	char* secret = NULL;

	pthread_rwlock_wrlock(&group_list_rwlock);

	while(group != NULL) {
		if(strcmp(group->group_id, group_id) == 0) {
			break;
		}
		group = group->next;
	}

	// creates new group if it doesn't exist or gets the secret of the existing group
	operation.type = group == NULL ? POST : GET;
	strncpy(operation.group_id, group_id, MAX_GROUP_ID);

	bytes = sendto(console_local_server_inet_socket,
				   &operation,
				   sizeof(operation),
				   MSG_CONFIRM,
				   (struct sockaddr*)&console_auth_server_inet_socket_addr,
				   len);
	if(bytes == -1) {
		// TODO
	}

	secret = calloc(MAX_SECRET + 1, sizeof(char));
	if(secret == NULL) {
		// TODO
	}

	bytes = recvfrom(console_local_server_inet_socket,
					 secret,
					 MAX_SECRET + 1,
					 MSG_WAITALL,
					 (struct sockaddr*)&console_auth_server_inet_socket_addr,
					 &len);
	if(bytes == -1) {
		// TODO
	}

	if(group == NULL) {
		group = calloc(1, sizeof(group_t));

		strncpy(group->group_id, group_id, MAX_GROUP_ID);

		group->hash_table = create_hash_table();

		group->next = groups_list;

		if(pthread_rwlock_init(&group->rwlock, NULL) != 0) {
			// TODO
		}

		groups_list = group;
	} else {
		printf("\n");
		print_warning("The specified group already exists");
		printf("\n\n");
	}

	pthread_rwlock_unlock(&group_list_rwlock);

	return secret;
}

// TODO: missing code commentary
void app_status() {
	connection_t* connection = connections_list;

	if(connection == NULL) {
		printf("No app has connected so far\n\n");
	} else {
		while(connection != NULL) {
			print_success("PID", int2str(connection->pid));
			print_success("Connection establishing time", connection->open_time);
			print_success("Connection close time", connection->close_time);
			printf("\n");
			connection = connection->next;
		}
	}
}

// TODO: missing code commentary
int delete_group(char* group_id) {
	group_t* group = groups_list;
	group_t* before_group = groups_list;
	int bytes = -1;
	int len = sizeof(struct sockaddr_in);
	operation_packet operation;
	char response;

	pthread_rwlock_wrlock(&group_list_rwlock);

	while(group != NULL) {
		if(strcmp(group->group_id, group_id) == 0) {
			break;
		}
		before_group = group;
		group = group->next;
	}

	if(group != NULL) {
		operation.type = DEL;
		strncpy(operation.group_id, group_id, MAX_GROUP_ID);

		bytes = sendto(console_local_server_inet_socket,
					   &operation,
					   sizeof(operation),
					   MSG_CONFIRM,
					   (struct sockaddr*)&console_auth_server_inet_socket_addr,
					   len);
		if(bytes == -1) {
			return UNSUCCESSFUL_OPERATION;
		}

		bytes = recvfrom(console_local_server_inet_socket,
						 &response,
						 sizeof(int),
						 MSG_WAITALL,
						 (struct sockaddr*)&console_auth_server_inet_socket_addr,
						 &len);

		if(bytes == -1) {
			return UNSUCCESSFUL_OPERATION;
		}

		// removing group from the list
		if(before_group == group) {
			groups_list = group->next;
		} else {
			before_group->next = group->next;
		}

		pthread_rwlock_unlock(&group_list_rwlock);

		pthread_rwlock_destroy(&group->rwlock);
		destroy_hash_table(group->hash_table);
		free(group->hash_table);
		free(group);

	} else {
		// TODO: decide if returns secret or error
	}

	return 1;
}

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
pthread_rwlock_t group_list_rwlock;
pthread_mutex_t authentication_mutex;

int local_server_unix_socket = -1;
int cb_local_server_unix_socket = -1;
int apps_local_server_inet_socket = -1;
int console_local_server_inet_socket = -1;
struct sockaddr_un local_server_unix_socket_addr;
struct sockaddr_un cb_local_server_unix_socket_addr;
struct sockaddr_in apps_auth_server_inet_socket_addr;
struct sockaddr_in console_auth_server_inet_socket_addr;

// APPS HANDLING FUNCTIONS FOR INTERNAL USE

/*********************************************************************
*
** void close_connection(connection_t* connection, 
**				        int list_critical_region, int group_critical_region) 
*
** Description:
*	 	Clsoes the specified connection.
*
** Parameters:
*  	@param connection 					 - struct holding connections settings;
*  	@param list_critical_region  - flag indicating if function is being 
*																	 called from inside a list reserved 
*																	 area;
*  	@param group_critical_region - flag indicating if function is being 
*																	 called from inside a group reserved 
*																	 area.
*
** Return:
*		Nothing is returned by this function.
*
** Side-effects:
*		This function has no side effects.
*	
*********************************************************************/
void close_connection(connection_t* connection, int list_critical_region, int group_critical_region) {
	time_t t;
	struct tm tm;
	group_t* group = connection->group;

	t = time(NULL);
	localtime_r(&t, &tm);
	sprintf(connection->close_time, "%02d-%02d-%d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
	if(connection->socket != -1) {
		close(connection->socket);
		connection->socket = -1;
	}
	if(connection->cb_socket != -1) {
		close(connection->cb_socket);
		connection->cb_socket = -1;
	}
	connection->group = NULL;

	// groups list critical region
	if(list_critical_region) {
		pthread_rwlock_unlock(&group_list_rwlock);
	}

	// group critical region
	if(group_critical_region) {
		pthread_rwlock_unlock(&(group->rwlock));
	}

	pthread_exit(0);
}

/*********************************************************************
*
** void put_value(connection_t* connection) 
*
** Description:
*		Creates a new key/value pair on the group's hashtable associated
*		with the connection if one doesn't exist yet. If it does exist then  
*		the value gets updated. A code is sent to client informing the 
*		status (CREATED or UPDATED).
*
** Parameters:
*  	@param connection - struct holding connections settings.
*
** Return:
*		Nothing is returned by this function.
*
** Side-effects:
*		If a write or a read from the socket is not successful then the 
*		connection is terminated (both thread and socket get closed).
*		A success/error code is sent to the client via socket write.
*	
*********************************************************************/
void put_value(connection_t* connection) {
	int bytes = -1, code = 0;
	int len_bytes = 0;
	char key[MAX_KEY + 1], *value = NULL;

	// reading the key from the stream
	len_bytes = (MAX_KEY + 1) * sizeof(char);
	bytes = read(connection->socket, key, len_bytes);
	if(bytes == 0) {
		close_connection(connection, 0, 1);
	} else if(bytes != len_bytes) {
		return;
	}

	// reading the value from the stream
	bytes = read(connection->socket, &len_bytes, sizeof(int));
	if(bytes == 0) {
		close_connection(connection, 0, 1);
	} else if(bytes != sizeof(int)) {
		return;
	}

	value = calloc(len_bytes, sizeof(char));
	if(value == NULL) {
		close_connection(connection, 0, 1);
	}

	bytes = read(connection->socket, value, len_bytes);
	if(bytes == 0) {
		close_connection(connection, 0, 1);
	} else if(bytes != len_bytes) {
		return;
	}

	// create/update hash table
	code = put_on_hash_table(connection->group->hash_table, key, value);

	if(code == NO_MEMORY_AVAILABLE) {
		close_connection(connection, 0, 1);
	}

	// sending the status code to the stream
	bytes = write(connection->socket, &code, sizeof(int));
	if(bytes != sizeof(int)) {
		return;
	}

	free(value);
}

/*********************************************************************
*
** void get_value(connection_t* connection) 
*
** Description:
*		Gets the value corresponding to a key in the group's hashtable 
*		associated with the connection.
*
** Parameters:
*  	@param connection - struct holding connections settings.
*
** Return:
*		Nothing is returned by this function.
*
** Side-effects:
*		If a write or a read from the socket is not successful then the 
*		connection is terminated (both thread and socket get closed).
*		A success/error code is sent to the client via socket write.
*	
*********************************************************************/
void get_value(connection_t* connection) {
	int bytes = 0, len_bytes = 0;
	int code = 0;
	char key[MAX_KEY + 1], *value = NULL;

	// reading the key from the stream
	len_bytes = (MAX_KEY + 1) * sizeof(char);
	bytes = read(connection->socket, key, len_bytes);
	if(bytes == 0) {
		close_connection(connection, 0, 1);
	} else if(bytes != len_bytes) {
		return;
	}

	// get value from hashtable
	code = get_from_hash_table(connection->group->hash_table, key, &value);

	if(code == NO_MEMORY_AVAILABLE) {
		close_connection(connection, 0, 1);
	} else if(code == NONEXISTENT_KEY) {
		len_bytes = sizeof(char);
		value = calloc(1, sizeof(char));
		if(value == NULL) {
			close_connection(connection, 0, 1);
		}
		value[0] = '\0';
	} else {
		len_bytes = (strlen(value) + 1) * sizeof(char);
	}

	// sending the value to the stream
	bytes = write(connection->socket, &len_bytes, sizeof(int));
	if(bytes != sizeof(int)) {
		return;
	}

	bytes = write(connection->socket, value, len_bytes);
	if(bytes != len_bytes) {
		return;
	}

	free(value);
}

/*********************************************************************
*
** void delete_value(connection_t* connection, group_t* group) 
*
** Description:
*		Deletes a key/value pair from the the group's hashtable associated
*		with the connection
*
** Parameters:
*  	@param connection - struct holding connections settings
*
** Return:
*		Nothing is returned by this function.
*
** Side-effects:
*		If a write or a read from the socket is not successful then the 
*		connection is terminated (both thread and socket get closed).
*		A success/error code is sent to the client via socket write
*
*********************************************************************/
void delete_value(connection_t* connection) {
	int bytes = -1, len_bytes = 0, code = 0, status = 0;
	char key[MAX_KEY + 1];
	connection_t* aux = connections_list;

	// reading key from stream
	len_bytes = (MAX_KEY + 1) * sizeof(char);
	bytes = read(connection->socket, key, len_bytes);
	if(bytes == 0) {
		close_connection(connection, 0, 1);
	} else if(bytes != len_bytes) {
		return;
	}

	// delete key/value pair from group
	code = delete_from_hash_table(connection->group->hash_table, key);

	// sending status code to the stream
	bytes = write(connection->socket, &code, sizeof(int));
	if(bytes != sizeof(int)) {
		return;
	}

	// notify all apps connected to the group that a key/pair value
	// (send through the stream) was deleted
	status = SUCCESSFUL_OPERATION;
	while(aux != NULL) {
		if(aux->group == connection->group) {
			len_bytes = (MAX_KEY + 1) * sizeof(char);
			bytes = write(aux->cb_socket, key, len_bytes);
			if(bytes != len_bytes) {
				status = UNSUCCESSFUL_OPERATION;
				break;
			}
		}

		aux = aux->next;
	}

	// sending notification status code to the stream
	bytes = write(connection->socket, &status, sizeof(int));
	if(bytes != sizeof(int)) {
		return;
	}
}

/*********************************************************************
*
** void register_callback(connection_t* connection)
*
** Description:
*		Associates a sempahore to a key/value pair from the the group's 
*		hashtable associated with the connection
*
** Parameters:
*  	@param connection - struct holding connections settings
*
** Return:
*		Nothing is returned by this function.
*
** Side-effects:
*		If a write or a read from the socket is not successful then the 
*		connection is terminated (both thread and socket get closed).
*		A success/error code is sent to the client via socket write.
*
*********************************************************************/
void register_callback(connection_t* connection) {
	int bytes = 0, len_bytes = 0;
	int code = 0;
	char key[MAX_KEY + 1];
	char name[MAX_NAME + 1];
	sem_t* sem_id;

	// reading the key from the stream
	len_bytes = (MAX_KEY + 1) * sizeof(char);
	bytes = read(connection->socket, key, len_bytes);
	if(bytes == 0) {
		close_connection(connection, 0, 1);
	} else if(bytes != len_bytes) {
		return;
	}

	// reading name from the stream
	len_bytes = (MAX_NAME + 1) * sizeof(char);
	bytes = read(connection->socket, name, len_bytes);
	if(bytes == 0) {
		close_connection(connection, 0, 1);
	} else if(bytes != len_bytes) {
		return;
	}

	// put semaphore on hashtable
	code = put_sem_on_hash_table(connection->group->hash_table, key, name);

	if(code == NO_MEMORY_AVAILABLE) {
		close_connection(connection, 0, 1);
	}

	// sending code to the stream
	bytes = write(connection->socket, &code, sizeof(int));
	if(bytes != sizeof(int)) {
		return;
	}

	return;
}

/*********************************************************************
*
** void* connection_handler(void* connection)
*
** Description:
*		Handles every app request: authentication, put value, get value,
*		delete value and register callback.
*
** Parameters:
*  	@param connection - struct holding connections settings
*
** Return:
*		Nothing is returned by this function.
*
** Side-effects:
*		If a write or a read from the socket is not successful then the 
*		connection is terminated (both thread and socket get closed).
*		A success/error code is sent to the client via socket write.
*
*********************************************************************/
void* connection_handler(void* connection) {
	int bytes = -1, code = 0;
	int len = sizeof(struct sockaddr_in);
	char operation_type = '\0';

	group_t* group;
	connection_packet connection_info;

	// receive info from app
	bytes = read(((connection_t*)connection)->socket, &connection_info, sizeof(connection_packet));
	if(bytes != sizeof(connection_packet)) {
		close_connection((connection_t*)connection, 0, 0);
	}

	((connection_t*)connection)->pid = connection_info.pid;

	pthread_rwlock_rdlock(&group_list_rwlock);
	group = groups_list;

	while(group != NULL) {
		if(strcmp(group->group_id, connection_info.credentials.group_id) == 0) {
			break;
		}
		group = group->next;
	}

	if(group == NULL) {
		code = NONEXISTENT_GROUP;
		bytes = write(((connection_t*)connection)->socket, &code, sizeof(int));
		pthread_rwlock_unlock(&group_list_rwlock);
		close_connection((connection_t*)connection, 1, 0);
	}

	pthread_mutex_lock(&authentication_mutex);
	pthread_rwlock_unlock(&group_list_rwlock);

	// send information to auth server
	bytes = sendto(apps_local_server_inet_socket,
				   &(connection_info.credentials),
				   sizeof(access_packet),
				   MSG_CONFIRM,
				   (const struct sockaddr*)&apps_auth_server_inet_socket_addr,
				   len);
	if(bytes != sizeof(access_packet)) {
		pthread_mutex_unlock(&authentication_mutex);
		close_connection((connection_t*)connection, 0, 0);
	}

	// handle response from auth server
	bytes = recvfrom(apps_local_server_inet_socket, &code, sizeof(int), MSG_WAITALL, (struct sockaddr*)&apps_auth_server_inet_socket_addr, &len);
	if(bytes != sizeof(int)) {
		pthread_mutex_unlock(&authentication_mutex);
		close_connection((connection_t*)connection, 0, 0);
	}

	bytes = write(((connection_t*)connection)->socket, &code, sizeof(int));
	if(bytes != sizeof(int)) {
		pthread_mutex_unlock(&authentication_mutex);
		close_connection((connection_t*)connection, 0, 0);
	}

	pthread_mutex_unlock(&authentication_mutex);

	while(1) {
		bytes = read(((connection_t*)connection)->socket, &operation_type, sizeof(char));
		if(bytes != sizeof(char)) {
			close_connection((connection_t*)connection, 0, 0);
		}

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
			close_connection((connection_t*)connection, 1, 0);
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

/*********************************************************************
* 
** void* connections_listener(void* arg) 
*
** Description:
*		Initializes all the communication sockets (to apps ant to auth 
*		server). Initializes global thread locks that enable the local 
*		server to have synchronization.
*
** Parameters:
*  	@param arg - should be set to NULL.
*
** Return:
*		Nothing is returned by this function. 
*		
** Side-effects:
*		If there is an error alllocating memory to store the incoming 
*		connection info then the listening thread gets closed and no more
*		new connections are accpted.
*
*********************************************************************/
void* connections_listener(void* arg) {
	connection_t* connection = NULL;
	time_t t;
	struct tm tm;

	while(1) {
		connection = calloc(1, sizeof(connection_t));
		if(connection == NULL) {
			printf("\n\n");
			print_error("Unable to create variable to store incoming connection info");
			printf("\n\n");
			break;
		}

		connection->socket = accept(local_server_unix_socket, NULL, NULL);
		if(connection->socket != -1) {
			connection->cb_socket = accept(cb_local_server_unix_socket, NULL, NULL);
			if(connection->cb_socket != -1) {
				if(pthread_create(&(connection->thread), NULL, connection_handler, connection) != 0) {
					close(connection->socket);
					close(connection->cb_socket);
					free(connection);
					continue;
				}

				t = time(NULL);
				localtime_r(&t, &tm);
				sprintf(connection->open_time, "%02d-%02d-%d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

				connection->close_time[0] = '\0';

				connection->next = connections_list;

				connections_list = connection;
			} else {
				close(connection->socket);
				free(connection);
			}

		} else {
			free(connection);
		}
	}

	pthread_exit(NULL);
}

// CONNECTIONS HANDLING FUNCTIONS AVAILABLE IN .h

/*********************************************************************
* 
** int setup_connections() 
*
** Description:
*		Initializes all the communication sockets (to apps ant to auth 
*		server). Initializes global thread locks that enable the local 
*		server to have synchronization.
*
** Parameters:
*  	This function takes no parameters.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: UNSUCCESSFUL_OPERATION is returned. 
*		
** Side-effects:
*		This function has no side-effects.
*
*********************************************************************/
int setup_connections() {
	// CONNECTIONS TO AUTH SERVER

	// creation of dgram socket to connect the requests from apps to
	// the authserver via localserver
	apps_local_server_inet_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(apps_local_server_inet_socket == -1) {
		print_error("Unable to create socket to connect apps to authentication server");
		return UNSUCCESSFUL_OPERATION;
	}

	// setting a timeout on requests from apps to the authserver
	// TODO checkar um timeout em ação
	struct timeval tv;
	tv.tv_sec = 300; // timeout time
	if(setsockopt(apps_local_server_inet_socket, SOL_SOCKET, SO_RCVTIMEO | SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		print_error("Unable to set options on socket to connect apps to authentication server");
		return UNSUCCESSFUL_OPERATION;
	}

	// creation of dgram socket to connect the requests from consoles to
	// the authserver via localserver
	console_local_server_inet_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(console_local_server_inet_socket == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		print_error("Unable to create socket to connect consoles to authentication server");
		return UNSUCCESSFUL_OPERATION;
	}

	apps_auth_server_inet_socket_addr.sin_family = AF_INET;
	apps_auth_server_inet_socket_addr.sin_port = htons(APPS_AUTH_SERVER_PORT);
	apps_auth_server_inet_socket_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER_ADDRESS);

	console_auth_server_inet_socket_addr.sin_family = AF_INET;
	console_auth_server_inet_socket_addr.sin_port = htons(CONSOLE_AUTH_SERVER_PORT);
	console_auth_server_inet_socket_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER_ADDRESS);

	// GLOBAL LOCKS

	// inicializing the rwlock for the console data sharing
	if(pthread_rwlock_init(&group_list_rwlock, NULL) != 0) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		print_error("Unable to initialize groups list rwlock");
		return UNSUCCESSFUL_OPERATION;
	}
	// inicializing the mutex for communication with the authentication server
	if(pthread_mutex_init(&authentication_mutex, NULL) != 0) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		pthread_rwlock_destroy(&group_list_rwlock);
		printf("\n\n");
		print_error("Unable to initialize authentication mutex");
		printf("\n\n");
		return UNSUCCESSFUL_OPERATION;
	}

	// CONNECTIONS TO APPS

	// creation of unix socket to connect the requests from apps to the localserver
	local_server_unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if(local_server_unix_socket == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		pthread_rwlock_destroy(&group_list_rwlock);
		pthread_mutex_destroy(&authentication_mutex);
		printf("\n\n");
		print_error("Unable to create socket to connect to apps");
		printf("\n\n");
		return UNSUCCESSFUL_OPERATION;
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
		pthread_rwlock_destroy(&group_list_rwlock);
		pthread_mutex_destroy(&authentication_mutex);
		printf("\n\n");
		print_error("Unable to create socket to connect to callbacks");
		printf("\n\n");
		return UNSUCCESSFUL_OPERATION;
	}

	local_server_unix_socket_addr.sun_family = AF_UNIX;
	sprintf(local_server_unix_socket_addr.sun_path, LOCAL_SERVER_ADDRESS);
	if(unlink(LOCAL_SERVER_ADDRESS) == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		close(local_server_unix_socket);
		local_server_unix_socket = -1;
		pthread_rwlock_destroy(&group_list_rwlock);
		pthread_mutex_destroy(&authentication_mutex);
		printf("\n\n");
		print_error("Unable to unlick socket to connect to apps path");
		printf("\n\n");
		return UNSUCCESSFUL_OPERATION;
	}

	cb_local_server_unix_socket_addr.sun_family = AF_UNIX;
	sprintf(cb_local_server_unix_socket_addr.sun_path, CB_LOCAL_SERVER_ADDRESS);
	if(unlink(CB_LOCAL_SERVER_ADDRESS) == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		close(local_server_unix_socket);
		local_server_unix_socket = -1;
		pthread_rwlock_destroy(&group_list_rwlock);
		pthread_mutex_destroy(&authentication_mutex);
		printf("\n\n");
		print_error("Unable to unlick socket to connect to callbacks path");
		printf("\n\n");
		return UNSUCCESSFUL_OPERATION;
	}

	// bind unix socket to connect the requests from apps to the localserver
	if(bind(local_server_unix_socket, (struct sockaddr*)&(local_server_unix_socket_addr), sizeof(struct sockaddr_un)) == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		close(local_server_unix_socket);
		local_server_unix_socket = -1;
		close(cb_local_server_unix_socket);
		cb_local_server_unix_socket = -1;
		pthread_rwlock_destroy(&group_list_rwlock);
		pthread_mutex_destroy(&authentication_mutex);
		printf("\n\n");
		print_error("Unable to bind socket to connect to apps");
		printf("\n\n");
		return UNSUCCESSFUL_OPERATION;
	}

	// bind unix socket to connect the apps callbacks to the localserver
	if(bind(cb_local_server_unix_socket, (struct sockaddr*)&(cb_local_server_unix_socket_addr), sizeof(struct sockaddr_un)) == -1) {
		close(apps_local_server_inet_socket);
		apps_local_server_inet_socket = -1;
		close(console_local_server_inet_socket);
		console_local_server_inet_socket = -1;
		close(local_server_unix_socket);
		local_server_unix_socket = -1;
		close(cb_local_server_unix_socket);
		cb_local_server_unix_socket = -1;
		pthread_rwlock_destroy(&group_list_rwlock);
		pthread_mutex_destroy(&authentication_mutex);
		printf("\n\n");
		print_error("Unable to bind socket to connect to callbacks");
		printf("\n\n");
		return UNSUCCESSFUL_OPERATION;
	}

	return SUCCESSFUL_OPERATION;
}

/*********************************************************************
* 
** void start_connections() 
*
** Description:
*		Sets apps socket and callbacs socket to listening mode with a max
*		queue of connections set to MAX_QUEUE. Launches thread to handle 
*		requestes cumming from the apps.
*
** Parameters:
*  	This function takes no parameters.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: UNSUCCESSFUL_OPERATION is returned. 
*		
** Side-effects:
*		This function has no side-effects.
*
*********************************************************************/
int start_connections() {
	if(listen(local_server_unix_socket, MAX_QUEUE) == -1) {
		print_error("Unable to set apps socket to listening mode");
		return UNSUCCESSFUL_OPERATION;
	}

	if(listen(cb_local_server_unix_socket, MAX_QUEUE) == -1) {
		print_error("Unable to set callbacks socket to listening mode");
		return UNSUCCESSFUL_OPERATION;
	}

	if(pthread_create(&listening_thread, NULL, connections_listener, NULL) != 0) {
		print_error("Unable to create apps listening thread");
		return UNSUCCESSFUL_OPERATION;
	}

	return SUCCESSFUL_OPERATION;
}

// CONSOLE HANDLING FUNCTIONS AVAILABLE IN .h

/*********************************************************************
* 
** int group_info(char* group_id, char** secret, int* num_pairs) 
*
** Description:
*		Verifys if the groups exists and provides the secret of it,
*		because in the parameter its passed as a pointer to the string.
*		It also shows both the group_id and the secret as well as
*		the number of keys in this group.
*
** Parameters:
*  	@param group_id  - string that identifies the group;
*  	@param secret	 - pointer of a string that specifies secret of 
					   a group;
*	@param num_pairs - pointer of int that says how many keys there 
*					   are in this specified group.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- SENT_BROKEN_MESSAGE if there's any error related to the write
*		  function;
*		- RECEIVED_BROKEN_MESSAGE is returned if there's any error
*		  related to the read function;
*		- NO_MEMORY_AVAILABLE is returned when there's any error related
*		  to the calloc function;
*		- NON_EXISTED_GROUP is returned if the group_id provided
*		  isnt related to an existed group.
*
** Side-effects:
*		There are no side-effects 
*
*******************************************************************/
int group_info(char* group_id, char** secret, int* num_pairs) {
	group_t* group = groups_list;
	int bytes = -1, len_bytes = 0;
	int len = sizeof(struct sockaddr_in);
	operation_packet operation;

	while(group != NULL) {
		if(strcmp(group->group_id, group_id) == 0) {
			break;
		}
		group = group->next;
	}

	// it means it's the group we wanted
	if(group != NULL) {
		operation.type = GET;
		strncpy(operation.group_id, group_id, MAX_GROUP_ID);

		bytes =
			sendto(console_local_server_inet_socket, &operation, sizeof(operation), MSG_CONFIRM, (struct sockaddr*)&console_auth_server_inet_socket_addr, len);
		if(bytes == -1) {
			return SENT_BROKEN_MESSAGE;
		}

		*secret = calloc(MAX_SECRET + 1, sizeof(char));
		if(*secret == NULL) {
			return NO_MEMORY_AVAILABLE;
		}

		len_bytes = (MAX_SECRET + 1) * sizeof(char);
		bytes = recvfrom(console_local_server_inet_socket, *secret, len_bytes, MSG_WAITALL, (struct sockaddr*)&console_auth_server_inet_socket_addr, &len);
		if(bytes != len_bytes) {
			return RECEIVED_BROKEN_MESSAGE;
		}

		*num_pairs = get_number_of_entries(group->hash_table);

	} else {
		return NONEXISTENT_GROUP;
	}
	return SUCCESSFUL_OPERATION;
}

/*********************************************************************
* 
** char* create_group(char* group_id, char** secret)
*
** Description:
*		This function creates memory in the local server for the 
*		group that the console asks for. This function also
*		communicates with the authentication server so it does the 
*		same. The authentication server returns the secret, that 
*		returns \0 if there's any error.
*
** Parameters:
*  		@param group_id - string that identifies the group
*  		@param secret 	- pointer of a string that specifies secret 
*						  of a group
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned if the groups is
*		created successfully.
*
*		On error: 
*		- SENT_BROKEN_MESSAGE if there's any error related to the write
*		  function;
*		- RECEIVED_BROKEN_MESSAGE is returned if there's any error
*		  related to the read function;
*		- NO_MEMORY_AVAILABLE is returned when there's any error related
*		  to the calloc function;
*		- UNSUCCESSFUL_OPERATION is returned if the secret received
*		  from the authentication server is '\0' and if it's read
*		  write mutex can't be inicialized.	
*
** Side-effects:
*		There are no side-effects
*		
*********************************************************************/
int create_group(char* group_id, char** secret) {
	group_t* group = groups_list;
	int bytes = -1, len_bytes = 0;
	int len = sizeof(struct sockaddr_in);
	operation_packet operation;

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

	bytes = sendto(console_local_server_inet_socket, &operation, sizeof(operation), MSG_CONFIRM, (struct sockaddr*)&console_auth_server_inet_socket_addr, len);
	if(bytes != sizeof(operation)) {
		return SENT_BROKEN_MESSAGE;
	}

	(*secret) = calloc(MAX_SECRET + 1, sizeof(char));
	if((*secret) == NULL) {
		return NO_MEMORY_AVAILABLE;
	}

	len_bytes = (MAX_SECRET + 1) * sizeof(char);
	bytes = recvfrom(console_local_server_inet_socket, *secret, len_bytes, MSG_WAITALL, (struct sockaddr*)&console_auth_server_inet_socket_addr, &len);
	if(bytes != len_bytes) {
		return RECEIVED_BROKEN_MESSAGE;
	}

	if(strlen(*secret) == 0) {
		return UNSUCCESSFUL_OPERATION;
	}

	if(group == NULL) {
		group = calloc(1, sizeof(group_t));

		strncpy(group->group_id, group_id, MAX_GROUP_ID);

		group->hash_table = create_hash_table();

		group->next = groups_list;

		if(pthread_rwlock_init(&group->rwlock, NULL) != 0) {
			return UNSUCCESSFUL_OPERATION;
		}

		groups_list = group;
	} else {
		printf("\n");
		print_warning("The specified group already exists");
		printf("\n\n");
	}

	pthread_rwlock_unlock(&group_list_rwlock);

	return SUCCESSFUL_OPERATION;
}

/*********************************************************************
* 
**void app_status() 
*
** Description:
*		This function shows the console user the pid, opened time
*		and closed time (if the app has already been closed) of 
*		the application that called this function.
*
** Parameters:
*  		This function doesn't have any parameters.
*
** Return:
*		This functions doesn't return anything.
*		
*
** Side-effects:
*		There are no side-effects
*		
*********************************************************************/
void app_status() {
	connection_t* connection = connections_list;
	char* buffer = NULL;

	if(connection == NULL) {
		print_warning("No app has connected so far");
		printf("\n\n");
	} else {
		while(connection != NULL) {
			buffer = int2str(connection->pid);
			printf("\n\n");
			print_success("PID", buffer);
			printf("\n");
			print_success("Connection establishing time", connection->open_time);
			printf("\n");
			print_success("Connection close time", connection->close_time);
			printf("\n");
			connection = connection->next;
			free(buffer);
		}
	}
}

/*********************************************************************
* 
**int delete_group(char* group_id) 
*
** Description:
*		This functions searches for the group specified in the hash
*		table, to delete the data related to it. It also communicates
*		with the authentication server to do the same in it's side.
*
** Parameters:
*  		@param group_id - string that identifies the group.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned if the groups is
*		deleted successfully.
*
*		On error: 
*		- SENT_BROKEN_MESSAGE if there's any error related to the write
*		  function;
*		- RECEIVED_BROKEN_MESSAGE is returned if there's any error
*		  related to the read function;
*		- NO_MEMORY_AVAILABLE is returned when there's any error related
*		  to the calloc function;
*		- UNSUCCESSFUL_OPERATION is returned if you can't communicate to
*		  the callback functions that you want them to be deleted and 
*		  if it's read
*		  write mutex can't be inicialized.	
*		
*
** Side-effects:
*		There are no side-effects
*
*********************************************************************/
int delete_group(char* group_id) {
	group_t* group = groups_list;
	group_t* before_group = groups_list;
	int bytes = -1, len_bytes = 0;
	int len = sizeof(struct sockaddr_in);
	operation_packet operation;
	char response;
	connection_t* aux = connections_list;
	char key[MAX_KEY + 1] = "\0";

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

		bytes =
			sendto(console_local_server_inet_socket, &operation, sizeof(operation), MSG_CONFIRM, (struct sockaddr*)&console_auth_server_inet_socket_addr, len);
		if(bytes != sizeof(operation)) {
			return SENT_BROKEN_MESSAGE;
		}

		bytes = recvfrom(console_local_server_inet_socket, &response, sizeof(int), MSG_WAITALL, (struct sockaddr*)&console_auth_server_inet_socket_addr, &len);
		if(bytes != sizeof(int)) {
			return RECEIVED_BROKEN_MESSAGE;
		}

		if(response == NONEXISTENT_KEY) {
			return NONEXISTENT_GROUP;
		}

		// removing group from the list
		if(before_group == group) {
			groups_list = group->next;
		} else {
			before_group->next = group->next;
		}

		pthread_rwlock_unlock(&group_list_rwlock);

		pthread_rwlock_destroy(&(group->rwlock));
		destroy_hash_table(group->hash_table);
		free(group->hash_table);
		pthread_rwlock_destroy(&group->rwlock);

		while(aux != NULL) {
			if(aux->group == group) {
				len_bytes = (MAX_KEY + 1) * sizeof(char);
				bytes = write(aux->cb_socket, key, len_bytes);
				if(bytes != len_bytes) {
					return UNSUCCESSFUL_OPERATION;
				}
			}

			aux = aux->next;
		}

		free(group);
		
	} else {
		return NONEXISTENT_GROUP;
	}

	return SUCCESSFUL_OPERATION;
}

/*********************************************************************
* 
**int close_local()
*
** Description:
*		This function terminates all connections in and out of the
*		local server, frees all variables and kills the local server
*		itself.
*
** Parameters:
*  		This function takes no parameters
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned.
*
*		On error: UNSUCCESSFUL_OPERATION is returned.
*		
** Side-effects:
*		There are no side-effects
*
*********************************************************************/
int close_local(){
	int len_bytes = 0, bytes = 0;
	char key[MAX_KEY + 1] = "\0";

	group_t* group = groups_list;
	group_t* before_group = groups_list;

	connection_t* aux = connections_list;
	connection_t* before_aux = connections_list;

	//deleting all the groups in the local server.	
	pthread_rwlock_rwlock(&group_list_rwlock);
	while(group != NULL) {

		// warning callback functions to shutdown
		aux = connections_list;
		before_aux = connections_list;
		while(aux != NULL) {
			if(aux->group == group) {
				len_bytes = (MAX_KEY + 1) * sizeof(char);
				bytes = write(aux->cb_socket, key, len_bytes);
				if(bytes != len_bytes) {
					return UNSUCCESSFUL_OPERATION;
				}
				
				if(aux->socket != -1) {
					close(aux->socket);
					aux->socket = -1;
				}
				if(aux->cb_socket != -1) {
					close(aux->cb_socket);
					aux->cb_socket = -1;
				}

				if(before_aux == aux){
					connections_list = aux->next;
					free(aux);
					aux = connections_list;
					before_aux = connections_list;
				}
				else{
					before_aux->next = aux->next;
					free(aux);
					aux = before_aux->next;
				}
			}
			else{
				before_aux = aux;
				aux = aux->next;
			}
		}

		before_group = group;
		group = group->next;
		destroy_hash_table(before_group->hash_table);
		free(before_group->hash_table);
		pthread_rwlock_destroy(&before_group->rwlock);
		free(before_group);
	}
	pthread_rwlock_unlock(&group_list_rwlock);

	pthread_rwlock_destroy(&group_list_rwlock);
	pthread_mutex_destroy(&authentication_mutex);

	if(close(local_server_unix_socket) != 0 ){
		return UNSUCCESSFUL_OPERATION;
	}

	if(close(cb_local_server_unix_socket) != 0){
		return UNSUCCESSFUL_OPERATION;
	}

	if(close(apps_local_server_inet_socket) != 0){
		return UNSUCCESSFUL_OPERATION;
	}

	if(close(console_local_server_inet_socket) != 0){
		return UNSUCCESSFUL_OPERATION;
	}

	return SUCCESSFUL_OPERATION;

}
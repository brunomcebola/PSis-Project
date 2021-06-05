#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "../configs.h"

#include "../../Hashtable/hashtable-lib.h"

int apps_auth_server_socket = -1;
int console_auth_server_socket = -1;

pthread_rwlock_t groups_rwlock;

key_pair_t** groups;

/*************************************************************************
*
** char* generate_secret() 
*
** Description:
*		Creates a semi-random secret. The secret will have a max
* 		size based on the macro MAX_SECRET, it needs to be an even
*		number, since the secret will always be based on 1 upper
*		letter and 1 number.
*
** Parameters:
*  	There are no parameters in this function.
*
** Return:
*		On success: The string of the secret created is returned. 
*
** Side-effects:
*		This function has no side-effect.
*	
*************************************************************************/
char* generate_secret() {
	char* key = calloc(MAX_SECRET + 1, sizeof(char));
	srand(time(NULL));

	for(int i = 0; i < MAX_SECRET; i++) {
		// random upper letter
		key[i] = 'A' + (rand() % 26);
		// random number
		key[++i] = '0' + (rand() % 10);
	}

	return key;
}

/*************************************************************************
*
** void create_group(struct sockaddr_in* addr, char* group_id) 
*
** Description:
*		Stores the new group in the hash-table and gives the key to the 
*		local_server via INET Datagram Socket.
*
** Parameters:
*  	@param addr 	- struct with the information of the inet socket;
*  	@param group_id - string of the group that we want to create.
*
** Return:
*		This function doesn't return any information.
*
** Side-effects:
*		This function has no side-effect..
*	
*************************************************************************/
void create_group(struct sockaddr_in* addr, char* group_id) {
	int bytes = -1, code = 0;
	char* secret = NULL;

	//verifies if the group
	code = get_from_hash_table(groups, group_id, &secret);
	if(code == NO_MEMORY_AVAILABLE){
		secret = calloc(MAX_SECRET + 1 , sizeof(char));
		secret[0] = '\0';
	}

	if(code == NONEXISTENT_KEY) {
		secret = generate_secret();
		code = put_on_hash_table(groups, group_id, secret);
		if(code == NO_MEMORY_AVAILABLE){
			secret[0] = '\0';
		}
	}

	bytes = sendto(console_auth_server_socket, secret, MAX_SECRET + 1, MSG_CONFIRM, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if(bytes != MAX_SECRET + 1) {
		free(secret);
		return;
	}

	free(secret);

	return;
}

/*************************************************************************
*
** void delete_group(struct sockaddr_in* addr, char* group_id) 
*
** Description:
*		Deletes a specified group in an hash-table sending the feedback of
*		this operation to the local_server via INET Datagram socket.
*
** Parameters:
*  	@param addr 	- struct with the information of the inet socket;
*  	@param group_id - string of the group that we want to delete.
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect
*	
*************************************************************************/
void delete_group(struct sockaddr_in* addr, char* group_id) {
	int bytes = -1;
	int response = 0;

	response = delete_from_hash_table(groups, group_id);

	bytes = sendto(console_auth_server_socket, &response, sizeof(int), MSG_CONFIRM, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if(bytes == -1) {
		return;
	}

	return;
}

/*************************************************************************
*
** void get_group_secret(struct sockaddr_in* addr, char* group_id)  
*
** Description:
*		Searches for a secret of a specified group, giving that secret
*		as a feedback to the local_server / console, sending the secret
*		to the local_server via INET Datagram socket.
*
** Parameters:
*  	@param addr 	- struct with the information of the inet socket;
*  	@param group_id - string of the group that we want to create.
*
** Return:
*		This function doesn't return any information.
*
** Side-effects:
*		There's no side-effect in this function.
*	
*************************************************************************/
void get_group_secret(struct sockaddr_in* addr, char* group_id) {
	int bytes = -1;
	char* secret = NULL;

	get_from_hash_table(groups, group_id, &secret);

	bytes = sendto(console_auth_server_socket, secret, MAX_SECRET + 1, MSG_CONFIRM, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if(bytes == -1) {
		// TODO
	}

	return;
}

/*******************************************************************
*
**void* console_handler(void* arg) 
*
** Description:
*		Handles whichs algorithms are supposed to be used based on
*		the will of the admin. If the admin wants to delete or
*		create a group, for example, this functions calls the 
*		necessary funtions to do that.
*
** Parameters:
*  	@param arg - not used, just labeled because of pthread 
*				 definition.
*
** Return:
*		This function doesn't return any information.
*
** Side-effects:
*		There's no side-effect.
*	 
*******************************************************************/
void* console_handler(void* arg) {
	int bytes = -1;
	char operation_type = '\0';
	int len = 0;
	operation_packet operation;
	struct sockaddr_in local_server_addr;

	len = sizeof(struct sockaddr_in);

	while(1) {
		bytes =
			recvfrom(console_auth_server_socket, &operation, sizeof(operation), MSG_WAITALL, (struct sockaddr*)&local_server_addr, &len);
		if(bytes == -1) {
			return NULL;
		}

		if(operation.type == GET) {
			pthread_rwlock_rdlock(&groups_rwlock);
		} else {
			pthread_rwlock_wrlock(&groups_rwlock);
		}

		switch(operation.type) {
			case POST: // create group
				create_group(&local_server_addr, operation.group_id);
				break;
			case DEL: // delete group
				delete_group(&local_server_addr, operation.group_id);
				break;
			case GET: // giving group secret (group info of console)
				get_group_secret(&local_server_addr, operation.group_id);
				break;
		}
		pthread_rwlock_unlock(&groups_rwlock);
	}
	pthread_exit(NULL);
}

/*******************************************************************
*
** void app_handler() 
*
** Description:
*		Verifies if the all the groups trying to connect is sending
*		the right information. Basically authenticates the group.
*		Checking if the group exists and it's secret is right.
*
** Parameters:
*  	There are no parameters in this function.
*
** Return:
*		This function doesn't return any information.
*
** Side-effects:
*		There's no side-effect.
*	
*******************************************************************/
void apps_handler() {
	int bytes = -1; // checking predifined functions errors
	int code = -1; // error handling
	int len = 0;
	char* value = NULL;
	access_packet group_auth_info;
	struct sockaddr_in local_server_addr;

	len = sizeof(struct sockaddr_in);

	while(1) {
		bytes = recvfrom(
			apps_auth_server_socket, &group_auth_info, sizeof(access_packet), MSG_WAITALL, (struct sockaddr*)&local_server_addr, &len);
		if(bytes != sizeof(access_packet)) {
			continue;
		}

		pthread_rwlock_rdlock(&groups_rwlock);

		get_from_hash_table(groups, group_auth_info.group_id, &value);

		pthread_rwlock_unlock(&groups_rwlock);

		if(value) {
			code = strcmp(group_auth_info.secret, value) == 0 ? 1 : -1;
			free(value);
		} else {
			code = -2;
		}

		sendto(apps_auth_server_socket, &code, sizeof(int), MSG_CONFIRM, (struct sockaddr*)&local_server_addr, len);
		// kinda irrelevant
		if(bytes < 0) {
			continue;
		}
	}
}

/*******************************************************************
*
** int setup_server() 
*
** Description:
*		Initializes all the connections between the authentication
*		server and the local server. Connections between 
*		local-console and local-app handler.
*
** Parameters:
*  	There are no parameters in this function.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned if all the 
*		connections are done.
*
*		On error: 
*		- UNABLE_TO_CONNECT is returned if there's any error while
*		  trying to establish the sockets;
*		- UNSUCCESSFUL_OPERATION is returned if it's not possible
*		  to inicialize the read write lock mutex.
*
** Side-effects:
*		There's no side-effect .
*	
*******************************************************************/
int setup_server() {
	int opt = 1;

	struct sockaddr_in apps_auth_server_addr;
	struct sockaddr_in console_auth_server_addr;

	printf("Starting server...\n\n");

	// sockets creation
	if((apps_auth_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		print_error("Unable to create the application socket");
		return UNABLE_TO_CONNECT;
	}

	if((console_auth_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		print_error("Unable to create the console socket");
		return UNABLE_TO_CONNECT;
	}

	// "unlink" inet sockets
	if(setsockopt(apps_auth_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		print_error("Unable to connect to the application socket");
		return UNABLE_TO_CONNECT;
	}
	if(setsockopt(console_auth_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		print_error("Unable to connect to the console socket");
		return UNABLE_TO_CONNECT;
	}

	// setting up the server info
	apps_auth_server_addr.sin_family = AF_INET;
	apps_auth_server_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER_ADDRESS);
	apps_auth_server_addr.sin_port = htons(APPS_AUTH_SERVER_PORT);

	console_auth_server_addr.sin_family = AF_INET;
	console_auth_server_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER_ADDRESS);
	console_auth_server_addr.sin_port = htons(CONSOLE_AUTH_SERVER_PORT);

	if(pthread_rwlock_init(&groups_rwlock, NULL)  != 0){
		return UNSUCCESSFUL_OPERATION;
	}

	// bind the socket
	if(bind(apps_auth_server_socket, (struct sockaddr*)&apps_auth_server_addr, sizeof(struct sockaddr_in)) < 0) {
		print_error("Unable to connect to the application socket");
		return UNABLE_TO_CONNECT;
	}
	if(bind(console_auth_server_socket, (struct sockaddr*)&console_auth_server_addr, sizeof(struct sockaddr_in)) < 0) {
		print_error("Unable to connect to the console socket");
		return UNABLE_TO_CONNECT;
	}

	//create hashtable to store the groups
	groups = create_hash_table();

	printf("Starting console server...\n\n");

	return SUCCESSFUL_OPERATION;
}

int main(int argc, char const* argv[]) {
	pthread_t console_handler_thread;
	int code = -1;

	code = setup_server();

	pthread_create(&console_handler_thread, NULL, console_handler, NULL);

	apps_handler();

	return 0;
}

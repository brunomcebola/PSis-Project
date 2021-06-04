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
*  		There are no parameters in this function
*
** Return:
*		Returns a string (char *) that represents the secret of a group
*
** Side-effects:
*		The side-effect is that if the max size is an odd number this
*		algorithm breaks
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
** void create_group() 
*
** Description:
*		Stores the new group in an hash-table.
*
** Parameters:
*  		@param addr - struct with the information of the inet socket
*  		@param group_id - string of the group that we want to create
*
** Return:
*		This function doesn't return any information
*		It sends information via a datagram inet socket 
*
** Side-effects:
*		There's no side-effect
*	TODO: ERROR HANDLING
*************************************************************************/
void create_group(struct sockaddr_in* addr, char* group_id) {
	int bytes = -1;
	char* secret;

	secret = generate_secret();

	put_on_hash_table(groups, group_id, secret);

	bytes = sendto(console_auth_server_socket, secret, MAX_SECRET + 1, MSG_CONFIRM, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if(bytes == -1) {
		// TODO
	}

	free(secret);

	return;
}

/*************************************************************************
*
** void delete_group() 
*
** Description:
*		Deletes a specified group in an hash-table.
*
** Parameters:
*  		@param addr - struct with the information of the inet socket
*  		@param group_id - string of the group that we want to delete
*
** Return:
*		This function doesn't return any information
*		It sends information via a datagram inet socket 
*
** Side-effects:
*		There's no side-effect
*	TODO: ERROR HANDLING
*************************************************************************/
void delete_group(struct sockaddr_in* addr, char* group_id) {
	int bytes = -1;
	int response = 0;

	response = delete_from_hash_table(groups, group_id);

	bytes = sendto(console_auth_server_socket, &response, sizeof(int), MSG_CONFIRM, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if(bytes == -1) {
		// TODO
	}

	return;
}

/*************************************************************************
*
** void get_group_secret() 
*
** Description:
*		Searches for a secret of a specified group, giving that secret
*		as a feedback to the local_server / console
*
** Parameters:
*  		@param addr - struct with the information of the inet socket
*  		@param group_id - string of the group that we want to create
*
** Return:
*		This function doesn't return any information
*		It sends information via a datagram inet socket 
*
** Side-effects:
*		There's no side-effect in this function
*	TODO: ERROR HANDLING
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
**void* console_handler() 
*
** Description:
*		Handles whichs algorithms are supposed to be used based on
*		the will of the admin. If the admin wants to delete or
*		create a group, this functions calls the necessary funtions
*		to do that.
*
** Parameters:
*  		@param arg - not used, just labeled because of pthread 
*					definition
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*	 TODO: ERROR HANDLING
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
			perror("");
			exit(-1);
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

			default: // handling errors ??
				break;
		}
	}
	pthread_exit(NULL);
}

/*******************************************************************
*
** void app_handler() 
*
** Description:
*		Verifies if the all the group trying to connect is sending
*		the right information. Basically authenticates the group.
*
** Parameters:
*  		There are no parameters in this function
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*	
*******************************************************************/
void apps_handler() {
	int bytes = -1; // checking predifined functions errors
	int code = -1; // error handling
	int len = 0;
	char** value = NULL;
	access_packet group_auth_info;
	struct sockaddr_in local_server_addr;

	len = sizeof(struct sockaddr_in);

	while(1) {
		bytes = recvfrom(
			apps_auth_server_socket, &group_auth_info, sizeof(access_packet), MSG_WAITALL, (struct sockaddr*)&local_server_addr, &len);
		if(bytes != sizeof(access_packet)) {
			continue;
		}

		value = calloc(1, sizeof(char*));

		get_from_hash_table(groups, group_auth_info.group_id, value);

		// handling auth_server thingys
		code = *value ? strcmp(group_auth_info.secret, *value) == 0 ? 1 : -1 : -2;

		sendto(apps_auth_server_socket, &code, sizeof(int), MSG_CONFIRM, (struct sockaddr*)&local_server_addr, len);
		// kinda irrelevant
		if(bytes < 0) {
			continue;
		}
	}
}

/*******************************************************************
*
** void setup_server() 
*
** Description:
*		Initializes all the connections between the authentication
*		server and the local server. Connections between 
*		local-console and local-app handler
*
** Parameters:
*  		There are no parameters in this function
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*	
*******************************************************************/
void setup_server() {
	int opt = 1;

	struct sockaddr_in apps_auth_server_addr;
	struct sockaddr_in console_auth_server_addr;

	printf("Starting server...\n\n");

	// sockets creation
	if((apps_auth_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		perror("");
		exit(-1);
	}

	if((console_auth_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		perror("");
		exit(-1);
	}

	// "unlink" inet sockets
	if(setsockopt(apps_auth_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("");
		exit(-1);
	}
	if(setsockopt(console_auth_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("");
		exit(-1);
	}

	// setting up the server info
	apps_auth_server_addr.sin_family = AF_INET;
	apps_auth_server_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER_ADDRESS);
	apps_auth_server_addr.sin_port = htons(APPS_AUTH_SERVER_PORT);

	console_auth_server_addr.sin_family = AF_INET;
	console_auth_server_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER_ADDRESS);
	console_auth_server_addr.sin_port = htons(CONSOLE_AUTH_SERVER_PORT);

	// bind the sockets
	if(bind(apps_auth_server_socket, (struct sockaddr*)&apps_auth_server_addr, sizeof(apps_auth_server_addr)) < 0) {
		perror("");
		exit(-1);
	}
	if(bind(console_auth_server_socket, (struct sockaddr*)&console_auth_server_addr, sizeof(console_auth_server_addr)) < 0) {
		perror("");
		exit(-1);
	}

	//create hashtable to store the groups

	groups = create_hash_table();

	printf("Starting console server...\n\n");
}

int main(int argc, char const* argv[]) {
	pthread_t console_handler_thread;

	setup_server();

	pthread_create(&console_handler_thread, NULL, console_handler, NULL);

	apps_handler();

	return 0;
}

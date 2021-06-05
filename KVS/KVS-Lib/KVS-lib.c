#include "KVS-lib.h"

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "../configs.h"
typedef struct _callback_t {
	void (*callback_function)(char*);
	char name[MAX_NAME + 1];
	char key[MAX_KEY + 1];
	int active;
	pthread_t thread;
	sem_t* sem_id;
	struct _callback_t* next;
} callback_t;

int app_socket = -1;
int cb_socket = -1;

callback_t* callbacks_list = NULL;

pthread_t cb_socket_thread;

/*******************************************************************
*
** void* callback_handler(void* callback_info)
*
** Description:
*		This function waits for the value to be updated and calls the
*		callback function given by the app.
*
** Parameters:
*  	@param callback_info - structure  that has all the information about
*							the callback function
*
** Return:
*		Nothing is returned by the function.
*
** Side-effects:
*		This function has no side-effect
*	
*******************************************************************/
void* callback_handler(void* callback_info) {
	callback_t *self = NULL, *before = NULL;

	while(sem_wait(((callback_t*)callback_info)->sem_id) >= 0) {
		if(((callback_t*)callback_info)->active == 0) {
			break;
		}
		(((callback_t*)callback_info)->callback_function)(((callback_t*)callback_info)->key);
	}

	self = callbacks_list;
	before = callbacks_list;

	while(self != NULL) {
		if(strncmp(self->key, ((callback_t*)callback_info)->key, MAX_KEY) == 0) {
			break;
		}
		before = self;
		self = self->next;
	}

	if(before == self) {
		callbacks_list = self->next;
	} else {
		before->next = self->next;
	}

	sem_close(self->sem_id);
	sem_unlink(self->name);
	free(self);

	return NULL;
}

void* callback_socket_handler(void* args) {
	int bytes = 0;
	char key[MAX_KEY + 1];
	callback_t* self = NULL;

	while(1) {
		bytes = read(cb_socket, key, (MAX_KEY + 1) * sizeof(char));
		if(bytes != (MAX_KEY + 1) * sizeof(char)) {
			// TODO
		}

		if(strlen(key) != 0) {
			self = callbacks_list;

			while(self != NULL) {
				if(strncmp(self->key, key, MAX_KEY) == 0) {
					break;
				}
				self = self->next;
			}

			if(self != NULL) {
				self->active = 0;
				sem_post(self->sem_id);
			}
		} else {
			break;
		}
	}

	self = callbacks_list;

	while(self != NULL) {
		self->active = 0;
		sem_post(self->sem_id);
		self = self->next;
	}

	close_connection();

	printf("\n");
	print_warning("The connection was terminated due to group deletion");
	printf("\n");
}

//

/*******************************************************************
*
** int establish_connection(char* group_id, char* secret)
*
** Description:
*		Sends all the data to the local server to send to the authentication
*		sever to verify the groupd_id and secret. After this establishes
*		the connection between the local server and the application by a 
*		socket stream unix.
*
** Parameters:
*  		@param group_id - string that identifies the group
*  		@param secret - pointer of a string that specifies secret of a group
*
** Return:
*		Returns WRONG_PARAM if the secret has more size than it should
*		have, or if the size is 0. Returns UNABLE_TO_CONNECT if it can't
*		connect to the local_server or to the callback socket. if 
*		there's been an error in reading messages. It
*	 	returns SENT_BROKEN_MESSAGE or RECEIVED_BROKEN_MESSAGE if the
*		size of the messages isn't equal to what was sent or received
*
** Side-effects:
*		This function has no side-effect
*	
*******************************************************************/
int establish_connection(char* group_id, char* secret) {
	int bytes = 0;
	int response = 0;
	int err = 0;

	struct sockaddr_un app_addr;
	struct sockaddr_un cb_addr;
	struct sockaddr_un local_server_addr;
	struct sockaddr_un cb_local_server_addr;

	connection_packet connection_info;

	// verifies if the group id has more than MAX_GROUP_ID chars
	if(strlen(group_id) > MAX_GROUP_ID) {
		print_error("Group id can have a max of " STR(MAX_GROUP_ID) " chars");
		return WRONG_PARAM;
	}
	// verifies if a group id is specified
	else if(strlen(group_id) == 0) {
		print_error("No group id was specified");
		return WRONG_PARAM;
	}
	// verifies if the secret has more than MAX_SECRET chars
	else if(strlen(secret) > MAX_SECRET) {
		print_error("Secret can have a max of " STR(MAX_SECRET) " chars");
		return WRONG_PARAM;
	}
	// verifies if a secret is specified
	else if(strlen(secret) == 0) {
		print_error("No secret was specified");
		return WRONG_PARAM;
	}
	// connection establishing functionality
	else {
		app_socket = socket(AF_UNIX, SOCK_STREAM, 0);
		if(app_socket == -1) {
			print_error("Unable to create socket");
			return UNABLE_TO_CONNECT;
		}

		cb_socket = socket(AF_UNIX, SOCK_STREAM, 0);
		if(cb_socket == -1) {
			close(app_socket);
			app_socket = -1;
			print_error("Unable to create socket");
			return UNABLE_TO_CONNECT;
		}

		local_server_addr.sun_family = AF_UNIX;
		sprintf(local_server_addr.sun_path, LOCAL_SERVER_ADDRESS);

		cb_local_server_addr.sun_family = AF_UNIX;
		sprintf(cb_local_server_addr.sun_path, CB_LOCAL_SERVER_ADDRESS);

		err = connect(app_socket, (struct sockaddr*)&local_server_addr, sizeof(struct sockaddr_un));
		if(err == -1) {
			close(app_socket);
			app_socket = -1;
			close(cb_socket);
			cb_socket = -1;
			print_error("Unable to connect to local server");
			return UNABLE_TO_CONNECT;
		}

		err = connect(cb_socket, (struct sockaddr*)&cb_local_server_addr, sizeof(struct sockaddr_un));
		if(err == -1) {
			close(app_socket);
			app_socket = -1;
			close(cb_socket);
			cb_socket = -1;
			print_error("Unable to connect to local server");
			return UNABLE_TO_CONNECT;
		}

		connection_info.pid = getpid();
		strcpy(connection_info.credentials.group_id, group_id);
		strcpy(connection_info.credentials.secret, secret);

		bytes = write(app_socket, &connection_info, sizeof(connection_packet));
		if(bytes != sizeof(connection_packet)) {
			close(app_socket);
			app_socket = -1;
			print_error("Broken message sent to local server");
			return SENT_BROKEN_MESSAGE;
		}

		// saber se consegui conectar
		bytes = read(app_socket, &response, sizeof(int));
		if(bytes == -1) {
			close(app_socket);
			app_socket = -1;
			print_error("Local server closed the connection");
			return CLOSED_CONNECTION;
		} else if(bytes != sizeof(int)) {
			close(app_socket);
			app_socket = -1;
			print_error("Broken message received from local server");
			return RECEIVED_BROKEN_MESSAGE;
		}

		if(response == WRONG_SECRET) {
			close(app_socket);
			app_socket = -1;
			print_error("Wrong secret");
		} else if(response == NONEXISTENT_GROUP) {
			close(app_socket);
			app_socket = -1;
			print_error("The specified group doesn't exist");
		}

		pthread_create(&cb_socket_thread, NULL, callback_socket_handler, NULL);

		return response;
	}
}

/*******************************************************************
*
** int put_value(char* key, char* value)
*
** Description:
*		Sends the group_id and the secret to the local server so it
*		can be stored.
*
** Parameters:
*  		@param key - string that identifies the key
*  		@param value -  string that specifies the value of a key
*
** Return:
*		It returns SENT_BROKEN_MESSAGE or RECEIVED_BROKEN_MESSAGE if the
*		size of the messages isn't equal to what was sent or received.
*		It returns SUCCESSFULL_OPERATION if it was possible to send
*		all the necessary data to the local server.
*
** Side-effects:
*		This function has no side-effect
*	
*******************************************************************/
int put_value(char* key, char* value) {
	char type = PUT;
	int bytes = 0;
	int len = 0;
	int response = 0;

	if(app_socket == -1) {
		printf("You have not establish connection to the local_server yet\n");
		return -1; // arranjar erros
	}

	// letting the local_sever know that we are putting a value
	bytes = write(app_socket, &type, sizeof(char));
	if(bytes != sizeof(char)) {
		return SENT_BROKEN_MESSAGE;
	}

	// writing into the stream the key
	len = strlen(key) + 1;
	bytes = write(app_socket, &len, sizeof(int));
	if(bytes != sizeof(int)) {
		return SENT_BROKEN_MESSAGE;
	}

	bytes = write(app_socket, key, len * sizeof(char));
	if(bytes != len * sizeof(char)) {
		return SENT_BROKEN_MESSAGE;
	}

	// writing into the stream the value
	len = strlen(value) + 1;
	bytes = write(app_socket, &len, sizeof(int));
	if(bytes != sizeof(int)) {
		return SENT_BROKEN_MESSAGE;
	}

	bytes = write(app_socket, value, len * sizeof(char));
	if(bytes != len * sizeof(char)) {
		return SENT_BROKEN_MESSAGE;
	}

	bytes = read(app_socket, &response, sizeof(int));
	if(bytes != sizeof(int)) {
		return RECEIVED_BROKEN_MESSAGE;
	}

	return SUCCESSFUL_OPERATION;
}

/*******************************************************************
*
** int get_value(char* secret, char** value)
*
** Description:
*		Sends the group_id and expects the local server to store the
*		memory and copy the value in the char** value.
*
** Parameters:
*  		@param key - string that identifies the key
*  		@param value - pointer of a string that specifies value of a key
*
** Return:
*		It returns SENT_BROKEN_MESSAGE or RECEIVED_BROKEN_MESSAGE if the
*		size of the messages isn't equal to what was sent or received
*
** Side-effects:
*		This function has no side-effect
*	
*******************************************************************/
int get_value(char* key, char** value) {
	int bytes = 0, len = 0;
	char type = GET;

	// letting the local_sever know that we are getting a value
	bytes = write(app_socket, &type, sizeof(type));
	if(bytes = !sizeof(type)) {
		return SENT_BROKEN_MESSAGE;
	}

	// writing into the stream the key
	len = (strlen(key) + 1) * sizeof(char);
	bytes = write(app_socket, &len, sizeof(int));
	if(bytes != sizeof(int)) {
		return SENT_BROKEN_MESSAGE;
	}
	bytes = write(app_socket, key, len);
	if(bytes != len) {
		return SENT_BROKEN_MESSAGE;
	}

	// writing into the stream the value
	bytes = read(app_socket, &len, sizeof(int));
	if(bytes != sizeof(int)) {
		return RECEIVED_BROKEN_MESSAGE;
	}
	*value = calloc(len, sizeof(char));
	bytes = read(app_socket, *value, len);
	if(bytes != len) {
		return RECEIVED_BROKEN_MESSAGE;
	}

	return SUCCESSFUL_CONNECTION;
}

int delete_value(char* key) {
	char type = DEL;
	int bytes = 0;
	int response = -1;
	char s_key[MAX_KEY + 1];

	// verifies if the group id has more than MAX_GROUP_ID chars
	if(strlen(key) > MAX_KEY) {
		print_error("The key can have a max of " STR(MAX_KEY) " chars");
		return WRONG_PARAM;
	}
	// verifies if a group id is specified
	else if(strlen(key) == 0) {
		print_error("No key was specified");
		return WRONG_PARAM;
	}
	// verifies if a callback function is specified
	else {
		strncpy(s_key, key, MAX_KEY);

		// letting the local_sever know that we are deleting a value
		bytes = write(app_socket, &type, sizeof(char));
		if(bytes != sizeof(char)) {
			return SENT_BROKEN_MESSAGE;
		}

		bytes = write(app_socket, s_key, (MAX_KEY + 1) * sizeof(char));
		if(bytes != (MAX_KEY + 1) * sizeof(char)) {
			return SENT_BROKEN_MESSAGE;
		}

		bytes = read(app_socket, &response, sizeof(int));
		if(bytes != sizeof(int)) {
			return RECEIVED_BROKEN_MESSAGE;
		}
	}

	return SUCCESSFUL_CONNECTION;
}

int register_callback(char* key, void (*callback_function)(char*)) {
	char type = RCB;
	int bytes = 0;
	int response = 0;

	callback_t *callback_info = NULL, *aux = callbacks_list;

	// verifies if the group id has more than MAX_GROUP_ID chars
	if(strlen(key) > MAX_KEY) {
		print_error("The key can have a max of " STR(MAX_KEY) " chars");
		return WRONG_PARAM;
	}
	// verifies if a group id is specified
	else if(strlen(key) == 0) {
		print_error("No key was specified");
		return WRONG_PARAM;
	}
	// verifies if a callback function is specified
	else if(callback_function == NULL) {
		print_error("No callback function was specified");
		return WRONG_PARAM;
	}
	// register callback functionality
	else {
		while(aux) {
			if(strncmp(aux->key, key, MAX_KEY) == 0) {
				break;
			}
			aux = aux->next;
		}

		//creates callback if it doesn't exist yet
		if(aux == NULL) {
			callback_info = calloc(1, sizeof(callback_t));
			if(callback_info == NULL) {
				return UNSUCCESSFUL_OPERATION;
			}

			strncpy(callback_info->key, key, MAX_KEY);

			char* pid = int2str(getpid());

			strcpy(callback_info->name, pid);
			strcat(callback_info->name, "_");
			strcat(callback_info->name, callback_info->key);

			free(pid);

			callback_info->callback_function = callback_function;
			callback_info->sem_id = sem_open(callback_info->name, O_CREAT, 0600, 0);
			callback_info->active = 1;

			bytes = write(app_socket, &type, sizeof(type));
			if(bytes != sizeof(type)) {
				return SENT_BROKEN_MESSAGE;
			}

			bytes = write(app_socket, callback_info->key, (MAX_KEY + 1) * sizeof(char));
			if(bytes != (MAX_KEY + 1) * sizeof(char)) {
				return SENT_BROKEN_MESSAGE;
			}

			bytes = write(app_socket, callback_info->name, (MAX_NAME + 1) * sizeof(char));
			if(bytes != (MAX_NAME + 1) * sizeof(char)) {
				return SENT_BROKEN_MESSAGE;
			}

			bytes = read(app_socket, &response, sizeof(int));
			if(bytes != sizeof(int)) {
				return RECEIVED_BROKEN_MESSAGE;
			}

			if(response == SUCCESSFUL_OPERATION) {
				callback_info->next = callbacks_list;
				callbacks_list = callback_info;

				pthread_create(&(callback_info->thread), NULL, callback_handler, callback_info);

				return SUCCESSFUL_OPERATION;
			} else {
				sem_close(callback_info->sem_id);
				sem_unlink(callback_info->name);
				free(callback_info);

				return UNSUCCESSFUL_OPERATION;
			}

		}
		// updates callback function if one already exists
		else {
			aux->callback_function = callback_function;

			return SUCCESSFUL_OPERATION;
		}
	}
}

int close_connection() {
	close(app_socket);
	app_socket = -1;
	close(cb_socket);
	cb_socket = -1;

	return 1;
}

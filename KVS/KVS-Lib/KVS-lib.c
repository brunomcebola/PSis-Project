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
	pthread_t thread;
	sem_t* sem_id;
	struct _callback_t* next;
} callback_t;

int app_socket = -1;
int cb_socket = -1;

callback_t* callbacks_list = NULL;

pthread_t cb_socket_thread;

void* callback_handler(void* callback_info) {
	int bytes;
	char response;

	callback_t* k = (callback_t*)callback_info;

	while(sem_wait(((callback_t*)callback_info)->sem_id) >= 0) {
		(((callback_t*)callback_info)->callback_function)(((callback_t*)callback_info)->key);
	}
}

void* callback_socket_handler(void* args) {
	int bytes = 0;
	char key[MAX_KEY + 1];
	void* res;
	callback_t *self = NULL, *before = NULL;

	while(1) {
		bytes = read(cb_socket, key, (MAX_KEY + 1) * sizeof(char));
		if(bytes == (MAX_KEY + 1) * sizeof(char)) {
			// TODO
		}

		self = callbacks_list;
		before = callbacks_list;

		while(self != NULL) {
			if(strncmp(self->key, key, MAX_KEY) == 0) {
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

		printf("\n\nBANANASSSS\n\n");

		pthread_cancel(self->thread);

		printf("\n\nCUCUCUCUCUCU\n\n");

		sem_close(self->sem_id);
		sem_unlink(self->name);
		free(self);

		pthread_join(self->thread, &res);

		printf("\n\nOLAAAAA\n\n");
	}
}

//

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
		if(bytes == 0) {
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
	if(bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1); // arranjar erros
	}

	// writing into the stream the key
	len = strlen(key) + 1;
	bytes = write(app_socket, &len, sizeof(int));
	if(bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1); // arranjar erros
	}

	bytes = write(app_socket, key, len * sizeof(char));
	if(bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1); // arranjar erros
	}

	// writing into the stream the value
	len = strlen(value) + 1;
	bytes = write(app_socket, &len, sizeof(int));
	if(bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1); // arranjar erros
	}

	bytes = write(app_socket, value, len * sizeof(char));
	if(bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1); // arranjar erros
	}

	bytes = read(app_socket, &response, sizeof(int));
	if(bytes == -1) {
		perror("Error getting the reponse of the put");
	}

	return 1;
}

int get_value(char* key, char** value) {
	int bytes = 0, len = 0;
	char type = GET;

	// letting the local_sever know that we are getting a value
	bytes = write(app_socket, &type, sizeof(type));
	if(bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1); // arranjar erros
	}

	// writing into the stream the key
	len = (strlen(key) + 1) * sizeof(char);
	bytes = write(app_socket, &len, sizeof(int));
	if(bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1); // arranjar erros
	}
	bytes = write(app_socket, key, len);
	if(bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1); // arranjar erros
	}

	// writing into the stream the value
	bytes = read(app_socket, &len, sizeof(int));
	if(bytes == -1) {
		perror("Error reading the authentication side\n");
		exit(-1); // arranjar erros
	}
	*value = calloc(len, sizeof(char));
	bytes = read(app_socket, *value, len);
	if(bytes == -1) {
		perror("Error reading the authentication side\n");
		exit(-1); // arranjar erros
	}

	return 1;
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
			// TODO
		}

		bytes = write(app_socket, s_key, (MAX_KEY + 1) * sizeof(char));
		if(bytes != (MAX_KEY + 1) * sizeof(char)) {
			// TODO
		}

		bytes = read(app_socket, &response, sizeof(int));
		if(bytes != sizeof(int)) {
			// TODO
		}
	}

	return 1;
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
				// TODO
			}

			strncpy(callback_info->key, key, MAX_KEY);

			char* pid = int2str(getpid());

			strcpy(callback_info->name, pid);
			strcat(callback_info->name, "_");
			strcat(callback_info->name, callback_info->key);

			free(pid);

			callback_info->callback_function = callback_function;

			callback_info->sem_id = sem_open(callback_info->name, O_CREAT, 0600, 0);

			bytes = write(app_socket, &type, sizeof(type));
			if(bytes == 0) {
				// TODO
			}

			bytes = write(app_socket, callback_info->key, (MAX_KEY + 1) * sizeof(char));
			if(bytes == 0) {
				// TODO
			}

			bytes = write(app_socket, callback_info->name, (MAX_NAME + 1) * sizeof(char));
			if(bytes == 0) {
				// TODO
			}

			bytes = read(app_socket, &response, sizeof(int));
			if(bytes == -1) {
				// TODO
			}

			// TODO
			if(1) {
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

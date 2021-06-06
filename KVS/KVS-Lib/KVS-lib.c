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

// TODO ver prints em todo o lado

// FUNCTIONS FOR INTERNAL USE

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

/*******************************************************************
*
** void* callback_socket_handler(void* args)
*
** Description:
*		This function waits for the key whose callback needs to be 
*		terminated. If an empty is returned the all callbacks and the
*		connection it self gets closed because it means the group was
*		deleted from the local server.
*
** Parameters:
*  	@param args - should be set to NULL.
*
** Return:
*		Nothing is returned by the function.
*
** Side-effects:
*		This function has no side-effect
*	
*******************************************************************/
void* callback_socket_handler(void* args) {
	int bytes = 0, len_bytes = 0, code = 0;
	char key[MAX_KEY + 1];
	callback_t* self = NULL;

	len_bytes = (MAX_KEY + 1) * sizeof(char);

	while(1) {
		bytes = read(cb_socket, key, len_bytes);
		if(bytes != len_bytes) {
			break;
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

	code = close_connection();

	printf("\n");
	if(code == SUCCESSFUL_OPERATION) {
		if(bytes == len_bytes) {
			print_warning("The connection was terminated due to group deletion");
		} else {
			print_error("The connection was terminated due to a problem in the callback thread");
		}
	} else {
		print_error("The was an error closing the connection in the callback thread");
	}
	printf("\n");
}

// GROUP HANDLING FUNCTIONS AVAILABLE IN .h

/*********************************************************************
*
** int establish_connection(char* group_id, char* secret)
*
** Description:
*		Sends a connection_packet to the local server for it to redirect
*		to the authentication server, in order to grant access for the 
*		requesting application to manipulate a specified group (one of the
*		paramenters of the connection_packet). If the sent credentials are
*		correct then a connection is established. Otherwise, an error is
*		returned.		    
*
** Parameters:
*  	@param group_id - string that identifies the group id (which must
*											have a maximum size of MAX_GROUP_ID);
*  	@param secret 	- string that specifies the secret of the group
*											(which must have a maximum size of MAX_SECRET).
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- CONNECTION_ALREADY_EXISTS is returned if a connection already
*			exists
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server;
*		-	NONEXISTENT_GROUP is returned if the the provided group_id does
*			not match with any id of the groups on the local server;
*		-	WRONG_SECRET is returned if the provided secret does not match
*     with the one associated to the group with the provided group_id.
*
** Side-effects:
*		On success it creates a thrad to handle callback calls.
*	
*********************************************************************/
int establish_connection(char* group_id, char* secret) {
	int bytes = 0;
	int response = 0;

	struct sockaddr_un app_addr;
	struct sockaddr_un cb_addr;
	struct sockaddr_un local_server_addr;
	struct sockaddr_un cb_local_server_addr;

	connection_packet connection_info;

	// verifies if there is a connection
	if(app_socket != -1) {
		print_error("Already to the local server");
		return CONNECTION_ALREADY_EXISTS;
	}
	// verifies if the group id has more than MAX_GROUP_ID chars
	if(strlen(group_id) > MAX_GROUP_ID) {
		print_error("The group id can have a max of " STR(MAX_GROUP_ID) " chars");
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
		// creating main socket
		app_socket = socket(AF_UNIX, SOCK_STREAM, 0);
		if(app_socket == -1) {
			print_error("Unable to create socket");
			return UNABLE_TO_CONNECT;
		}

		// creating callback socket
		cb_socket = socket(AF_UNIX, SOCK_STREAM, 0);
		if(cb_socket == -1) {
			print_error("Unable to create callback socket");
			close_connection();
			return UNABLE_TO_CONNECT;
		}

		local_server_addr.sun_family = AF_UNIX;
		sprintf(local_server_addr.sun_path, LOCAL_SERVER_ADDRESS);

		cb_local_server_addr.sun_family = AF_UNIX;
		sprintf(cb_local_server_addr.sun_path, CB_LOCAL_SERVER_ADDRESS);

		// connecting main socket to local server
		if(connect(app_socket, (struct sockaddr*)&local_server_addr, sizeof(struct sockaddr_un)) == -1) {
			print_error("Unable to connect to local server");
			close_connection();
			return UNABLE_TO_CONNECT;
		}

		// connecting callback socket to local server
		if(connect(cb_socket, (struct sockaddr*)&cb_local_server_addr, sizeof(struct sockaddr_un)) == -1) {
			print_error("Unable to connect to local server callback");
			close_connection();
			return UNABLE_TO_CONNECT;
		}

		connection_info.pid = getpid();
		strcpy(connection_info.credentials.group_id, group_id);
		strcpy(connection_info.credentials.secret, secret);

		// sending connection_packet to the stream
		bytes = write(app_socket, &connection_info, sizeof(connection_packet));
		if(bytes != sizeof(connection_packet)) {
			print_error("Broken message sent to local server");
			close_connection();
			return SENT_BROKEN_MESSAGE;
		}

		// reading response from the stream
		bytes = read(app_socket, &response, sizeof(int));
		if(bytes == 0) {
			print_error("Local server closed the connection");
			close_connection();
			return CLOSED_CONNECTION;
		} else if(bytes != sizeof(int)) {
			print_error("Broken message received from local server");
			close_connection();
			return RECEIVED_BROKEN_MESSAGE;
		}

		// handling wrong secret
		if(response == WRONG_SECRET) {
			print_error("The specified secret doesn't match with the one associated to the group");
			close_connection();

			return WRONG_SECRET;
		}
		// handling nonexistent group
		else if(response == NONEXISTENT_GROUP) {
			print_error("The specified group doesn't exist");
			close_connection();

			return NONEXISTENT_GROUP;
		}
		// creates callback thread if everything okay
		else {
			if(pthread_create(&cb_socket_thread, NULL, callback_socket_handler, NULL) != 0) {
				print_error("Unable to launch callback listening thread");
				close_connection();
				return UNSUCCESSFUL_SUBOPERATION;
			}

			return SUCCESSFUL_OPERATION;
		}
	}
}

/*********************************************************************
*
** int put_value(char* key, char* value)
*
** Description:
*		If there is an established connection to a local server then the
*		provided key/value pair is added/updated in the group associated
*		to the connection.
*
** Parameters:
*  	@param key   - string that identifies the key/value pair inside
*									 the connection's group (it must have a maximum 
*									 size of MAX_Key);
*  	@param value - string containing the data to be stored int the
*									 key/value pair of the connection's group (it does
*									 not have a maximum size, but a value must be 
*									 specified).
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server.
*
** Side-effects:
*		If the local server closes the connection then the function forces
*		the connection to close also on the app side.
*		If a callback has been set for the given key (which means it is an
*		update operation) then it gets triggered upon successful edition
*	
*********************************************************************/
int put_value(char* key, char* value) {
	char type = PUT;
	char s_key[MAX_KEY + 1] = "\0";
	int bytes = 0;
	int len_bytes = 0;
	int response = 0;

	// verifies if there is a connection
	if(app_socket == -1) {
		print_error("Unable to connect to the local server");
		return UNABLE_TO_CONNECT;
	}
	// verifies if the key has more than MAX_KEY chars
	else if(strlen(key) > MAX_KEY) {
		print_error("The key can have a max of " STR(MAX_KEY) " chars");
		return WRONG_PARAM;
	}
	// verifies if a key is specified
	else if(strlen(key) == 0) {
		print_error("No key was specified");
		return WRONG_PARAM;
	}
	// verifies if a value is specified
	else if(strlen(value) == 0) {
		print_error("No value was specified");
		return WRONG_PARAM;
	}
	// put value functionality
	else {
		// informing the local server of the operation type
		bytes = write(app_socket, &type, sizeof(char));
		if(bytes != sizeof(char)) {
			print_error("Broken message sent to local server");
			return SENT_BROKEN_MESSAGE;
		}

		// sending the key to the stream
		strncpy(s_key, key, MAX_KEY);
		len_bytes = (MAX_KEY + 1) * sizeof(char);
		bytes = write(app_socket, s_key, len_bytes);
		if(bytes != len_bytes) {
			print_error("Broken message sent to local server");
			return SENT_BROKEN_MESSAGE;
		}

		// sending the value to the stream
		len_bytes = (strlen(value) + 1) * sizeof(char);
		bytes = write(app_socket, &len_bytes, sizeof(int));
		if(bytes != sizeof(int)) {
			print_error("Broken message sent to local server");
			return SENT_BROKEN_MESSAGE;
		}
		bytes = write(app_socket, value, len_bytes);
		if(bytes != len_bytes) {
			print_error("Broken message sent to local server");
			return SENT_BROKEN_MESSAGE;
		}

		// reading response from the stream
		bytes = read(app_socket, &response, sizeof(int));
		if(bytes == 0) {
			print_error("Local server closed the connection");
			close_connection();
			return CLOSED_CONNECTION;
		} else if(bytes != sizeof(int)) {
			print_error("Broken message received from local server");
			return RECEIVED_BROKEN_MESSAGE;
		}

		if(response == CREATED) {
			print_success("Success", "Key/pair value created");
		} else if(response == UPDATED) {
			print_success("Success", "Key/pair value updated");
		}

		return SUCCESSFUL_OPERATION;
	}
}

/*********************************************************************
*
** int get_value(char* key, char** value)
*
** Description:
*		If there is an established connection to a local server then it
*		tries to get the value associated with the given key from the 
*		group associated to the connection.
*
** Parameters:
*  	@param key   - string that identifies the key/value pair inside
*									 the connection's group (it must have a maximum 
*									 size of MAX_Key);
*  	@param value - string where the data of the key/value pair will
*									 be stored.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server;
*		- NO_MEMORY_AVAILABLE is returned if function cannot allocate 
*			memory to store the fetched value.
*
** Side-effects:
*		If the local server closes the connection then the function forces
*		the connection to close also on the app side.
*	
*********************************************************************/
int get_value(char* key, char** value) {
	char type = GET;
	int bytes = 0, len_bytes = 0;
	char s_key[MAX_KEY + 1] = "\0";

	// verifies if there is a connection
	if(app_socket == -1) {
		print_error("Unable to connect to the local server");
		return UNABLE_TO_CONNECT;
	}
	// verifies if the key has more than MAX_KEY chars
	else if(strlen(key) > MAX_KEY) {
		print_error("The key can have a max of " STR(MAX_KEY) " chars");
		return WRONG_PARAM;
	}
	// verifies if a key is specified
	else if(strlen(key) == 0) {
		print_error("No key was specified");
		return WRONG_PARAM;
	}
	// get value functionality
	else {
		// informing the local server of the operation type
		bytes = write(app_socket, &type, sizeof(char));
		if(bytes != sizeof(char)) {
			print_error("Broken message sent to local server");
			return SENT_BROKEN_MESSAGE;
		}

		// sending the key to the stream
		strncpy(s_key, key, MAX_KEY);
		len_bytes = (MAX_KEY + 1) * sizeof(char);
		bytes = write(app_socket, s_key, len_bytes);
		if(bytes != len_bytes) {
			print_error("Broken message sent to local server");
			return SENT_BROKEN_MESSAGE;
		}

		// reading the value from the stream
		bytes = read(app_socket, &len_bytes, sizeof(int));
		if(bytes == 0) {
			print_error("Local server closed the connection");
			close_connection();
			return CLOSED_CONNECTION;
		} else if(bytes != sizeof(int)) {
			print_error("Broken message received from local server");
			return RECEIVED_BROKEN_MESSAGE;
		}

		*value = calloc(len_bytes / sizeof(char), sizeof(char));
		if(*value == NULL) {
			print_error("Unable to allocate memory to store the value");
			close_connection();
			return NO_MEMORY_AVAILABLE;
		}

		bytes = read(app_socket, *value, len_bytes);
		if(bytes == 0) {
			print_error("Local server closed the connection");
			close_connection();
			return CLOSED_CONNECTION;
		} else if(bytes != len_bytes) {
			print_error("Broken message received from local server");
			return RECEIVED_BROKEN_MESSAGE;
		}

		if(strlen(*value) == 0) {
			print_warning("The provided key/value pair does not exist");
		} else {
			print_success("Success", "Able to obtain value associated with the provided key");
		}

		return SUCCESSFUL_OPERATION;
	}
}

/*********************************************************************
*
** int delete_value(char* key)
*
** Description:
*		If there is an established connection to a local server then the
*		provided key/value pair is delete from the group associated to the
*		connection.
*
** Parameters:
*  	@param key - string that identifies the key/value pair inside the
*								 connection's group (it must have a maximum size of
*								 MAX_Key).
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server;
*   - UNSUCCESSFUL_SUBOPERATION is returned when a callback handler
*			could not be erased.
*
** Side-effects:
*		If the local server closes the connection then the function forces
*		the connection to close also on the app side.
*		If a callback has been set for the given key then it gets deleted
*		upon successful key/value pair deletion.
*	
*********************************************************************/
int delete_value(char* key) {
	char type = DEL;
	int bytes = 0, len_bytes = 0;
	int response = -1;
	char s_key[MAX_KEY + 1];

	// verifies if there is a connection
	if(app_socket == -1) {
		print_error("Unable to connect to the local server");
		return UNABLE_TO_CONNECT;
	}
	// verifies if the group id has more than MAX_GROUP_ID chars
	else if(strlen(key) > MAX_KEY) {
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
		// informing the local server of the operation type
		bytes = write(app_socket, &type, sizeof(char));
		if(bytes != sizeof(char)) {
			return SENT_BROKEN_MESSAGE;
		}

		// sending the key to the stream
		strncpy(s_key, key, MAX_KEY);
		len_bytes = (MAX_KEY + 1) * sizeof(char);
		bytes = write(app_socket, s_key, len_bytes);
		if(bytes != len_bytes) {
			print_error("Broken message sent to local server");
			return SENT_BROKEN_MESSAGE;
		}

		// reading response from stream
		bytes = read(app_socket, &response, sizeof(int));
		if(bytes == 0) {
			print_error("Local server closed the connection");
			close_connection();
			return CLOSED_CONNECTION;
		} else if(bytes != sizeof(int)) {
			print_error("Broken message received from local server");
			return RECEIVED_BROKEN_MESSAGE;
		}

		if(response == NONEXISTENT_KEY) {
			print_warning("The provided key does not exist");
		} else {
			print_success("Success", "Able to delete key/value pair");
		}

		// reading callback management response from stream
		bytes = read(app_socket, &response, sizeof(int));
		if(bytes == 0) {
			print_error("Local server closed the connection");
			close_connection();
			return CLOSED_CONNECTION;
		} else if(bytes != sizeof(int)) {
			print_error("Broken message received from local server");
			return RECEIVED_BROKEN_MESSAGE;
		}

		if(response == SUCCESSFUL_OPERATION) {
			print_success("Success", "Able to erase all callbacks related to the key/value pair");
			return SUCCESSFUL_OPERATION;
		} else {
			print_error("Unable to erase all callbacks related to the key/value pair");
			return UNSUCCESSFUL_SUBOPERATION;
		}
	}
}

/*********************************************************************
*
** int register_callback(char* key, void (*callback_function)(char*))
*
** Description:
*		If there is an established connection to a local server then it
*		associates a callback function to a key/value pair in the group 
*		associated to the connection, if it does not exist already. If it
*		does then the callback function gets updated.
*
** Parameters:
*  	@param key - string that identifies the key/value pair inside the
*								 connection's group (it must have a maximum size of
*								 MAX_Key);
*  	@param callback_function - callback function address.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server;
*		- NO_MEMORY_AVAILABLE is returned if function cannot allocate 
*			memory to store the new callback on the list;
*		- UNSUCCESSFUL_OPERATION is returned if cannot create semaphore or
*			if local server sends an error code;
*		- UNSUCCESSFUL_SUBOPERATION is returned if cannot launch thread
*			to run the callback.
*
** Side-effects:
*		If the local server closes the connection then the function forces
*		the connection to close also on the app side.
*		If a callback has been set for the given key (which means it is an
*		update operation) then it gets triggered upon successful edition
*	
*********************************************************************/
int register_callback(char* key, void (*callback_function)(char*)) {
	char type = RCB;
	int bytes = 0, len_bytes = 0;
	int response = 0;

	callback_t *callback_info = NULL, *aux = callbacks_list;

	// verifies if there is a connection
	if(app_socket == -1) {
		print_error("Unable to connect to the local server");
		return UNABLE_TO_CONNECT;
	}
	// verifies if the group id has more than MAX_GROUP_ID chars
	else if(strlen(key) > MAX_KEY) {
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
			// create new callback info to place on callback list
			callback_info = calloc(1, sizeof(callback_t));
			if(callback_info == NULL) {
				print_error("Unable to allocate memory to store the new callback");
				close_connection();
				return NO_MEMORY_AVAILABLE;
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

			if(callback_info->sem_id == SEM_FAILED) {
				print_error("Unable to open semaphore for the new callback");
				free(callback_info);
				return UNSUCCESSFUL_OPERATION;
			}

			// informing the local server of the operation type
			bytes = write(app_socket, &type, sizeof(char));
			if(bytes != sizeof(char)) {
				print_error("Broken message sent to local server");
				return SENT_BROKEN_MESSAGE;
			}

			// sending key to the stream
			len_bytes = (MAX_KEY + 1) * sizeof(char);
			bytes = write(app_socket, callback_info->key, len_bytes);
			if(bytes != len_bytes) {
				print_error("Broken message sent to local server");
				return SENT_BROKEN_MESSAGE;
			}

			// sending name to the stream
			len_bytes = (MAX_NAME + 1) * sizeof(char);
			bytes = write(app_socket, callback_info->name, len_bytes);
			if(bytes != len_bytes) {
				print_error("Broken message sent to local server");
				return SENT_BROKEN_MESSAGE;
			}

			// reading response frmo stream
			bytes = read(app_socket, &response, sizeof(int));
			if(bytes == 0) {
				print_error("Local server closed the connection");
				close_connection();
				return CLOSED_CONNECTION;
			} else if(bytes != sizeof(int)) {
				print_error("Broken message received from local server");
				return RECEIVED_BROKEN_MESSAGE;
			}

			if(response == SUCCESSFUL_OPERATION) {
				callback_info->next = callbacks_list;
				callbacks_list = callback_info;

				if(pthread_create(&(callback_info->thread), NULL, callback_handler, callback_info) != 0) {
					print_error("Unable to launch callback thread");
					return UNSUCCESSFUL_SUBOPERATION;
				}

				print_success("Success", "Able to register callback");

				return SUCCESSFUL_OPERATION;
			} else {
				print_error("Unble to register callback");

				sem_close(callback_info->sem_id);
				sem_unlink(callback_info->name);
				free(callback_info);

				return UNSUCCESSFUL_OPERATION;
			}
		}
		// updates callback function if one already exists
		else {
			aux->callback_function = callback_function;

			print_success("Success", "Callback function updated");

			return SUCCESSFUL_OPERATION;
		}
	}
}

/*********************************************************************
*
** int close_connection()
*
** Description:
*		Closes all the communication channels between the app and the
*		local server.
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
*		This function has no side-effect.
*	
*********************************************************************/
int close_connection() {
	// verifies if both are closed
	if(app_socket == -1 && cb_socket == -1) {
		print_error("No active connection");
		return UNSUCCESSFUL_OPERATION;
	}

	// close main socket
	if(app_socket != -1) {
		if(close(app_socket) == -1) {
			print_error("Unable to close main socket");
			return UNSUCCESSFUL_OPERATION;
		}
		app_socket = -1;
	}

	// close callback socket
	if(cb_socket != -1) {
		if(close(cb_socket) == -1) {
			print_error("Unable to close calllback socket");
			return UNSUCCESSFUL_OPERATION;
		}
		cb_socket = -1;
	}

	return SUCCESSFUL_OPERATION;
}

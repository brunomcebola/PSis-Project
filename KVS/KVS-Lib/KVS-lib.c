#include "KVS-lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

/*************************************
 * Para não estar sempre a criar socket,
 * fazer list, array ou hash com "todos" os
 * sockets que forem criados,
 * basicamente fazer um array com um
 * número igual ao número de aplicações
 *************************************/

#define LOCAL_SERVER_ADRESS "/tmp/kvs_local_server_socket"
#define RESPONSE_LEN 10

#define PUT 'P'
#define GET 'G'
#define DEL 'D'
#define RCB 'R'

#define WRONG_SECRET -1

int app_socket = -1;

// TODO VER ERROS NOS RETURNS

int establish_connection(char *group_id, char *secret) {
    int type = 'C';
	int bytes = 0;
	int len = 0;
    int response = 0;

	struct sockaddr_un app_addr;
	struct sockaddr_un local_server_addr;

	app_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (app_socket == -1) {
		perror("Error creating the stream socket on the application");
		exit(-1);  // arranjar erros
	}

	app_addr.sun_family = AF_UNIX;
	sprintf(app_addr.sun_path, "/tmp/app_socket_%d", getpid());

	unlink(app_addr.sun_path);

	int err = bind(app_socket, (struct sockaddr *)&app_addr, sizeof(app_addr));
	if (err == -1) {
		perror("Error binding the application socket\n");
		exit(-1);  // arranjar erros
	}

	int connect_error = connect(app_socket, (struct sockaddr *)&local_server_addr, sizeof(local_server_addr));
	if (connect_error == -1) {
		perror("Error connecting the application\n");
		exit(-1);  // arranjar erros
	}

	len = (strlen(group_id) + 1) * sizeof(char);
	bytes = write(app_socket, &len, sizeof(int));
	if (bytes == 0) {
		perror("Error write the group_id in the application");
		exit(-1);  // arranjar erros
	}
	bytes = write(app_socket, group_id, len);
	if (bytes == 0) {
		perror("Error write the group_id in the application");
		exit(-1);  // arranjar erros
	}

    len = (strlen(secret) + 1) * sizeof(char);
	bytes = write(app_socket, &len, sizeof(int));
	if (bytes == 0) {
		perror("Error write the group_id in the application");
		exit(-1);  // arranjar erros
	}
	bytes = write(app_socket, &secret, len);
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

    //saber se consegui conectar 
    bytes = read(app_socket, &response, sizeof(int));
	if (bytes == -1) {
		perror("Error reading the authentication side\n");
		exit(-1);  // arranjar erros
	}
    switch (response)
    {
    case WRONG_SECRET:
        close(app_socket);
        app_socket = -1;
        break;
    
    default:
        break;
    }

	return 0;
}

int put_value(char *key, char *value) {
	char type = PUT;
    int bytes = 0;
	int len = 0;
	int response = 0;

	if (app_socket == -1) {
		printf("You have not establish connection to the local_server yet\n");
		return -1;	// arranjar erros
	}

	// letting the local_sever know that we are putting a value
	bytes = write(app_socket, &type, sizeof(type));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	// writing into the stream the key
	len = strlen(key) + 1;
	bytes = write(app_socket, &len, sizeof(int));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	bytes = write(app_socket, key, len * sizeof(char));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	// writing into the stream the value

	len = strlen(value) + 1;
	bytes = write(app_socket, &len, len * sizeof(char));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	bytes = write(app_socket, value, len);
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	bytes = read(app_socket, &response, sizeof(int) );
	if(bytes == -1){
		perror("Error getting the reponse of the put");
	}


	return 1;
}

int get_value(char *key, char **value) {
	int bytes = 0;
	int len = 0;
	char *response = NULL;
	char type = GET;

	// letting the local_sever know that we are getting a value
	bytes = write(app_socket, &type, sizeof(type));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	// writing into the stream the key
	len = (strlen(key) + 1) * sizeof(char);
	bytes = write(app_socket, &len, sizeof(int));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}
	bytes = write(app_socket, key, len);
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	// writing into the stream the value
	bytes = read(app_socket, &len, sizeof(int));
	if (bytes == -1) {
		perror("Error reading the authentication side\n");
		exit(-1);  // arranjar erros
	}
	response = calloc(len, sizeof(char));
	bytes = read(app_socket, response, RESPONSE_LEN);
	if (bytes == -1) {
		perror("Error reading the authentication side\n");
		exit(-1);  // arranjar erros
	}

	// tenho a impressão que isto está errado
	strcpy(*value, response);

	value = &response;

	return 1;
}

int delete_value(char *key) {
	int bytes = 0, len = 0;
	char type = DEL;

	// letting the local_sever know that we are deleting a value
	bytes = write(app_socket, &type, sizeof(type));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	// deleting the value

	len = strlen(key) + 1;
	bytes = write(app_socket, &len, sizeof(int));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	bytes = write(app_socket, key, len * sizeof(char) );
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	int response = -1;

	bytes = read(app_socket, &response, sizeof(response));
	if (bytes == -1) {
		perror("Error reading from the local server");
	}

	return 1;
}

int register_callback(char *key, void (*callback_funcation)(char *)) {
	int bytes = 0;
	char type = RCB;

	// letting the local_sever know that we want to use callback
	bytes = write(app_socket, &type, sizeof(type));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	// writing into the stream the key
	bytes = write(app_socket, key, sizeof(key));
	if (bytes == 0) {
		perror("Error write the secret in the application");
		exit(-1);  // arranjar erros
	}

	char value[RESPONSE_LEN];  // estes lens não podem ser estes temos que ir
							   // depois chegar a um consenso
	char new_value[RESPONSE_LEN];
	// writing into the stream the value
	bytes = read(app_socket, value, RESPONSE_LEN);
	if (bytes == -1) {
		perror("Error reading the authentication side\n");
		exit(-1);  // arranjar erros
	}

	while (read(app_socket, new_value, RESPONSE_LEN) > 0) {
		if (strcmp(new_value, value) != 0) {
			callback_funcation(new_value);
		}
	}

	return 1;
}

int close_connetion() {
	close(app_socket);
	// verificar erros
	return 1;
}

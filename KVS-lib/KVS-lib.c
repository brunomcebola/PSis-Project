#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "KVS-lib.h"

/*************************************
 * Para não estar sempre a criar socket,
 * fazer list, array ou hash com "todos" os
 * sockets que forem criados, 
 * basicamente fazer um array com um
 * número igual ao número de aplicações 
 *************************************/

#define LOCAL_SERVER_ADRESS "/tmp/local_server"
#define RESPONSE_LEN 10 

#define PUT 'P'
#define GET 'G'
#define DEL 'D'
#define RCB 'R'

int app_socket = -1;

int establish_connection(char * group_id, char * secret){
    //TODO VER ERROS NOS RETURNS


    struct sockaddr_un app_addr;
    struct sockaddr_un local_server_addr;

    app_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(app_socket == -1){
        perror("Error creating the stream socket on the application");
        exit(-1); // arranjar erros
    }

    app_addr.sun_family = AF_UNIX;
    sprintf(app_addr.sun_path, "/tmp/app_socket_%d", getpid());

    unlink(app_addr.sun_path);
    
    int err = bind(app_socket, (struct sockaddr *)&app_addr, sizeof(app_addr) );
    if(err == -1){
        perror("Error binding the application socket\n");
        exit(-1); // arranjar erros
    }

    int connect_error = connect(app_socket , (struct sockaddr *)&local_server_addr, sizeof(local_server_addr));

    if(connect_error == -1){
        perror("Error connecting the application\n");
        exit(-1); // arranjar erros
    }   

    int bytes;
    bytes = write(app_socket, group_id, sizeof(group_id));
    if(bytes == 0){
        perror("Error write the group_id in the application");
        exit(-1); // arranjar erros
    }
    bytes = write(app_socket, secret, sizeof(secret));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    char response[RESPONSE_LEN];
    bytes = read(app_socket, response, RESPONSE_LEN);
    if(bytes == -1){
        perror("Error reading the authentication side\n");
        exit(-1); // arranjar erros
    }

    return 0; 
}

int put_value(char * key, char * value){

    int bytes;
    char type = PUT;

    if(app_socket == -1){
        printf("You have not establish connection to the local_server yet\n");
        return -1; // arranjar erros
    }

    //letting the local_sever know that we are putting a value
    bytes = write(app_socket, &type, sizeof(type));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    //writing into the stream the key
    bytes = write(app_socket, key, sizeof(key));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    //writing into the stream the value
    bytes = write(app_socket, value, sizeof(value));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }


    return 1;
}

int get_value(char * key, char ** value){

    int bytes;
    char type = GET;

    //letting the local_sever know that we are getting a value
    bytes = write(app_socket, &type, sizeof(type));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    //writing into the stream the key
    bytes = write(app_socket, key, sizeof(key));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    char response[RESPONSE_LEN];
    //writing into the stream the value
    bytes = read(app_socket, response, RESPONSE_LEN);
    if(bytes == -1){
        perror("Error reading the authentication side\n");
        exit(-1); // arranjar erros
    }
    
    //tenho a impressão que isto está errado
    strcpy(*value, response);

    printf("The value of the key %s is %s", key, response);

    return 1;
}

int delete_value(char * key){

    int bytes;
    char type = DEL;

    //letting the local_sever know that we are deleting a value
    bytes = write(app_socket, &type, sizeof(type));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    bytes = write(app_socket, key, sizeof(key));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    return 1;
}

int register_callback(char * key, void (*callback_funcation)(char *)){
    int bytes;
    char type = RCB ;

    //letting the local_sever know that we want to use callback
    bytes = write(app_socket, &type, sizeof(type));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    //writing into the stream the key
    bytes = write(app_socket, key, sizeof(key));
    if(bytes == 0){
        perror("Error write the secret in the application");
        exit(-1); // arranjar erros
    }

    char value[RESPONSE_LEN]; // estes lens não podem ser estes temos que ir depois chegar a um consenso
    char new_value[RESPONSE_LEN];
    //writing into the stream the value
    bytes = read(app_socket, value, RESPONSE_LEN);
    if(bytes == -1){
        perror("Error reading the authentication side\n");
        exit(-1); // arranjar erros
    }

    while( read(app_socket, new_value, RESPONSE_LEN) > 0  ){
        if(strcmp(new_value, value) != 0){
            callback_funcation(new_value);
        }
    }

    return 1;
}

int close_connetion(){
    
    close(app_socket);

    return 1;
}
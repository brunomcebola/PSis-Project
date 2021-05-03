#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "KVS-lib.h"

#define LOCAL_SERVER_ADRESS "/tmp/local_server" 

int establish_connection(char * group_id, char * secret){
    //TODO VER ERROS NOS RETURNS


    struct sockaddr_un app_addr;
    struct sockaddr_un local_server_addr;

    int app_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(app_socket == -1){
        perror("Error creating the stream socket on the application");
        exit(-1); // arranjar erros
    }

    app_addr.sun_family = AF_UNIX;
    sprintf(app_addr.sun_path, "/tmp/app_socket_%d", getpid());

    unlink(app_addr.sun_path);
    
    int err = bind(app_socket, (struct sockadrr *)&app_addr, sizeof(app_addr) );
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

    char * response;
    bytes = read(app_socket, response, 10);
    if(bytes == -1){
        perror("Error reading the authentication side\n");
        exit(-1); // arranjar erros
    }

    return 1; 
}

int put_value(char * key, char * value){

    struct sockaddr_un app_addr;
    int app_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(app_socket == -1){
        perror("Error creating the stream socket on the application");
        exit(-1); // arranjar erros
    }

    app_addr.sun_family = AF_UNIX;
    sprintf(app_addr.sun_path, "/tmp/app_socket_%d", getpid());

    unlink(app_addr.sun_path);
    
    int err = bind(app_socket, (struct sockadrr *)&app_addr, sizeof(app_addr) );
    if(err == -1){
        perror("Error binding the application socket\n");
        exit(-1); // arranjar erros
    }

    int bytes;
    char type[4] = "PUT";

    //letting the local_sever know that we are putting a value
    bytes = write(app_socket, type, sizeof(type));
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
    struct sockaddr_un app_addr;
    int app_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(app_socket == -1){
        perror("Error creating the stream socket on the application");
        exit(-1); // arranjar erros
    }

    app_addr.sun_family = AF_UNIX;
    sprintf(app_addr.sun_path, "/tmp/app_socket_%d", getpid());

    unlink(app_addr.sun_path);
    
    int err = bind(app_socket, (struct sockadrr *)&app_addr, sizeof(app_addr) );
    if(err == -1){
        perror("Error binding the application socket\n");
        exit(-1); // arranjar erros
    }

    int bytes;
    char type[4] = "GET";

    //letting the local_sever know that we are putting a value
    bytes = write(app_socket, type, sizeof(type));
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

    char * response;
    //writing into the stream the value
    bytes = read(app_socket, response, 10);
    if(bytes == -1){
        perror("Error reading the authentication side\n");
        exit(-1); // arranjar erros
    }

    printf("The value of the key %s is %s", key, response);

    return 1;
}

int delete_value(char * key){
    struct sockaddr_un app_addr;
    int app_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(app_socket == -1){
        perror("Error creating the stream socket on the application");
        exit(-1); // arranjar erros
    }

    app_addr.sun_family = AF_UNIX;
    sprintf(app_addr.sun_path, "/tmp/app_socket_%d", getpid());

    unlink(app_addr.sun_path);
    
    int err = bind(app_socket, (struct sockadrr *)&app_addr, sizeof(app_addr) );
    if(err == -1){
        perror("Error binding the application socket\n");
        exit(-1); // arranjar erros
    }

    int bytes;
    char type[4] = "DEL";

    //letting the local_sever know that we are putting a value
    bytes = write(app_socket, type, sizeof(type));
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

}

int close_connetion(){
    struct sockaddr_un app_addr;
    int app_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(app_socket == -1){
        perror("Error creating the stream socket on the application");
        exit(-1); // arranjar erros
    }

    app_addr.sun_family = AF_UNIX;
    sprintf(app_addr.sun_path, "/tmp/app_socket_%d", getpid());

    unlink(app_addr.sun_path);
    
    int err = bind(app_socket, (struct sockadrr *)&app_addr, sizeof(app_addr) );
    if(err == -1){
        perror("Error binding the application socket\n");
        exit(-1); // arranjar erros
    }
    
    close(app_socket);

    return 1;
}
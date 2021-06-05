#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "../KVS/configs.h"

#include "../KVS/KVS-Lib/KVS-lib.h"

void* callback_function_by_us(char* key){
    printf("Key %s modified", key);
}

void put_value_UI(){
    char key[MAX_NAME +1];
    char *value;

    printf("-- Key:");
    scanf(" %s", key);
	getchar();

    printf("-- Value:");
    // TODO : aqui tem que ser getline() right?
    scanf(" %s", value);
	getchar(); 

    put_value(key, value);
}

void get_value_UI(){
    char key[MAX_NAME +1];
    char *value;

    printf("-- Key:");
    scanf(" %s", key);
	getchar();

    get_value(key, &value);
}

void delete_value_UI(){
    char key[MAX_NAME +1];

    printf("-- Key:");
    scanf(" %s", key);
	getchar();

    delete_value(key);
}

void register_callback_UI(){
    char key[MAX_NAME +1];

    printf("-- Key:");
    scanf(" %s", key);
	getchar();

    register_callback(key, callback_function_by_us(key) );
}


int main(){
    int option = -1;
    int code = -1;
    char secret[MAX_SECRET+1];
    char group_id[MAX_GROUP_ID +1];

	printf("---------------------------------\n");
	printf("------------ " ANSI_BOLD ANSI_CYAN "Application" ANSI_RESET " ------------\n");
	printf("---------------------------------\n");

    printf(ANSI_BOLD "\nLogin\n" ANSI_RESET);
	printf("--\n");
	printf("-- Group ID:");
    scanf(" %s", group_id);
	getchar();


    printf("-- Group Secret:");

    scanf(" %s", secret);
	getchar();

    printf("Group -> %s \n",group_id);
    printf("Secret -> %s \n",secret);
    code = establish_connection(group_id, secret);

    if(code == SENT_BROKEN_MESSAGE || code == RECEIVED_BROKEN_MESSAGE){
        printf("Message between application and local_server was corrupted\n");
    }
    if(code == UNABLE_TO_CONNECT){
        printf("Server couldn't establish connection\n");
    }

	while(option != 5) {
		option = -1;

		printf(ANSI_BOLD "\nWhat do you want to do?\n" ANSI_RESET);
		printf("--\n");
		printf("-- 1) Insert value --------------\n");
		printf("-- 2) Get value --------------\n");
		printf("-- 3) Delete value -----------\n");
		printf("-- 4) Register Callback of value ---\n");
        printf("-- 5) Close Application ---\n");

		printf("Option: ");

		scanf("%d", &option);
		getchar();

		printf("\n----\n\n");

		switch(option) {
			case 1: // create group
				put_value_UI();
				continue;

			case 2: // delete group
				get_value_UI();
				continue;

			case 3:
				delete_value_UI();
				continue;

			case 4:
				register_callback_UI();
				continue;

			case 5:
				break;

			default:
                printf("ERROR\n");
				continue;
		}
	}
    exit(0);
}
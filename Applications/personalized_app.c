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

void callback_function_by_us(char* key) {
	printf("\n\nKey %s modified\n\n", key);
}

void put_value_UI() {
	char *key = NULL, *value = NULL;
	size_t size = 0;

	print_title("PUT VALUE");

	printf("-- Key: ");
	getline(&key, &size, stdin);
	key[strcspn(key, "\n")] = '\0';
	size = 0;

	printf("-- Value: ");
	getline(&value, &size, stdin);
	value[strcspn(value, "\n")] = '\0';
	size = 0;

	printf("\n");
	put_value(key, value);
	printf("\n\n");
}

void get_value_UI() {
	char *key = NULL, *value = NULL;
	size_t size = 0;

	print_title("GET VALUE");

	printf("-- Key: ");
	getline(&key, &size, stdin);
	key[strcspn(key, "\n")] = '\0';
	size = 0;

	printf("\n");
	get_value(key, &value);
	printf("\n\n");

	printf("Value: %s\n\n", value);
}

void delete_value_UI() {
	char* key = NULL;
	size_t size = 0;

	print_title("GET VALUE");

	printf("-- Key: ");
	getline(&key, &size, stdin);
	key[strcspn(key, "\n")] = '\0';
	size = 0;

	printf("\n");
	delete_value(key);
	printf("\n\n");
}

void register_callback_UI() {
	char* key = NULL;
	size_t size = 0;

	print_title("REGISTER CALLBACK");

	printf("-- Key: ");
	getline(&key, &size, stdin);
	key[strcspn(key, "\n")] = '\0';
	size = 0;

	printf("\n");
	register_callback(key, callback_function_by_us);
	printf("\n\n");
}

int main() {
	int option = -1;
	int code = -1;
	char *group_id = NULL, *secret = NULL;
	size_t size = 0;

	printf("-------------------------------------\n");
	printf("------------ " ANSI_BOLD ANSI_CYAN "Application" ANSI_RESET " ------------\n");
	printf("-------------------------------------\n\n");

	print_title("Login");

	do {
		printf("-- Group ID: ");
		getline(&group_id, &size, stdin);
		group_id[strcspn(group_id, "\n")] = '\0';
		size = 0;

		printf("-- Group Secret: ");
		getline(&secret, &size, stdin);
		secret[strcspn(secret, "\n")] = '\0';
		size = 0;

		printf("\n");
		code = establish_connection(group_id, secret);
		printf("\n\n");

		free(group_id);
		free(secret);

	} while(code != SUCCESSFUL_OPERATION);

	while(option != 5) {
		option = -1;

		printf(ANSI_BOLD "What do you want to do?\n\n" ANSI_RESET);

		printf("-- 1) Insert value ------------------\n");
		printf("-- 2) Get value ---------------------\n");
		printf("-- 3) Delete value ------------------\n");
		printf("-- 4) Register Callback of value ----\n");
		printf("-- 5) Close Application -------------\n\n");

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
				close_connection();
				break;

			default:
				printf("ERROR\n");
				continue;
		}
	}
	exit(0);
}
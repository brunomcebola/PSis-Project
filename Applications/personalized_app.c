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
	printf("[1] Key %s modified\n\n", key);
}

void callback_function_by_us_2(char* key) {
	printf("[2] Key %s modified\n\n", key);
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

	if(strlen(value) != 0) {
		printf("Value: %s\n\n", value);
	}
}

void delete_value_UI() {
	char* key = NULL;
	size_t size = 0;

	print_title("DELETE VALUE");

	printf("-- Key: ");
	getline(&key, &size, stdin);
	key[strcspn(key, "\n")] = '\0';
	size = 0;

	printf("\n");
	delete_value(key);
	printf("\n\n");
}

void register_callback_UI() {
	char *key = NULL, *option = NULL;
	size_t size = 0;

	print_title("REGISTER CALLBACK");

	printf("-- Key: ");
	getline(&key, &size, stdin);
	key[strcspn(key, "\n")] = '\0';
	size = 0;

	printf("-- Which function do you want (1 or 2): ");
	getline(&option, &size, stdin);
	option[strcspn(option, "\n")] = '\0';
	size = 0;

	printf("\n");
	if(strlen(option) != 1) {
		print_warning("That is not an option");
	} else {
		if(atoi(option) == 0) {
			print_warning("That is not an option");
		} else {

			if(atoi(option) == 1) {
				register_callback(key, callback_function_by_us);
			} else if(atoi(option) == 2) {
				register_callback(key, callback_function_by_us_2);
			} else {
				print_warning("That is not an option");
			}
		}
	}
	printf("\n\n");

	free(key);
	free(option);
}

void login_UI() {
	char *group_id = NULL, *secret = NULL;
	size_t size = 0;
	int code = -1;

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
}

int main() {
	char* option = NULL;
	size_t size = 0;

	option = calloc(2, sizeof(char));
	strcpy(option, "5");

	printf("-------------------------------------\n");
	printf("------------ " ANSI_BOLD ANSI_CYAN "Application" ANSI_RESET " ------------\n");
	printf("-------------------------------------\n\n");

	while(atoi(option) != 6) {
		if(atoi(option) == 5) {
			login_UI();
		}

		free(option);
		option = NULL;
		size = 0;

		printf(ANSI_BOLD "What do you want to do?\n\n" ANSI_RESET);

		printf("-- 1) Insert value ------------------\n");
		printf("-- 2) Get value ---------------------\n");
		printf("-- 3) Delete value ------------------\n");
		printf("-- 4) Register Callback -------------\n");
		printf("-- 5) Logout ------------------------\n");
		printf("-- 6) Close Application -------------\n\n");

		printf("Option: ");
		getline(&option, &size, stdin);
		option[strcspn(option, "\n")] = '\0';
		size = 0;

		printf("\n");
		if(strlen(option) != 1) {
			print_warning("Option Not recognized");
			printf("\n\n----\n");
			continue;
		} else if(atoi(option) == 0) {
			print_warning("Option Not recognized");
			printf("\n\n----\n");
			continue;
		}

		printf("\n----\n\n");

		switch(atoi(option)) {
			case 1: // create group
				put_value_UI();
				break;

			case 2: // delete group
				get_value_UI();
				break;

			case 3:
				delete_value_UI();
				break;

			case 4:
				register_callback_UI();
				break;

			case 5:
				close_connection();
				sleep(1);
				break;

			case 6:
				close_connection();
				break;

			default:
				print_warning("Option Not recognized");
				break;
		}
	}
	exit(0);
}
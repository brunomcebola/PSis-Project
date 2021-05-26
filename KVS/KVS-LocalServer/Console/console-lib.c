#include "./console-lib.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "../../configs.h"

#include "../Connections/connections-lib.h"

// colors and fonts

#define ANSI_BOLD "\x1b[1m"
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_RESET "\x1b[0m"

void create_group_UI() {
	char *group_id = NULL, *secret = NULL;
	size_t size = 0;
	int retype = 0;

	printf(ANSI_BOLD "CREATE GROUP\n\n" ANSI_RESET);

	do {
		retype = 0;

		printf("Group id: ");
		getline(&group_id, &size, stdin);
		group_id[strcspn(group_id, "\n")] = '\0';

		if(strlen(group_id) == 0) {
			printf(ANSI_RED "Please provide a group id!\n\n" ANSI_RESET);
			retype = 1;
		} else if(strlen(group_id) > MAX_GROUP_ID) {
			printf(ANSI_RED "The group id can have a max of 1024 chars!\n\n" ANSI_RESET);
			retype = 1;
		}

	} while(retype);

	secret = create_group(group_id);

	//secret = strndup("o meu segredo bue fixe", 256);

	printf(ANSI_BOLD ANSI_GREEN "Secret:" ANSI_RESET ANSI_BOLD " %s\n" ANSI_RESET, secret);
	printf("\n----\n");

	free(group_id);
	free(secret);

	return;
}

int UI() {
	int option = -1;

	printf("---------------------------------\n");
	printf("------------ " ANSI_BOLD ANSI_CYAN "Console" ANSI_RESET " ------------\n");
	printf("---------------------------------\n");

	while(option != 5) {
		option = -1;

		printf(ANSI_BOLD "\nWhat do you want to do?\n" ANSI_RESET);
		printf("--\n");
		printf("-- 1) Create Group --------------\n");
		printf("-- 2) Delete Group --------------\n");
		printf("-- 3) Show Group Info -----------\n");
		printf("-- 4) Show Application Status ---\n");
		printf("-- 5) Close console -------------\n\n");

		printf("Option: ");

		scanf(" %d", &option);
		getchar();

		printf("\n----\n\n");

		switch(option) {
			case 1: // create group
				create_group_UI();
				break;

			case 2: // delete group
				/* code */
				break;

			case 3:
				/* code */
				break;

			case 4:
				/* code */
				break;

			case 5:
				break;

			default:
				break;
		}
	}
}

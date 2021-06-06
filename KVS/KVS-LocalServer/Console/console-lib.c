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

/*******************************************************************
*
** void create_group_UI() 
*
** Description:
*		Calls the algorithm that will create the group in the
*		authentication server.
*
** Parameters:
*  		There are no parameters in this function
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*	
*******************************************************************/
void create_group_UI() {
	char *group_id = NULL, *secret = NULL;
	size_t size = 0;
	int retype = 0;

	print_title("CREATE GROUP");

	do {
		retype = 0;

		printf("Group id: ");
		getline(&group_id, &size, stdin);
		group_id[strcspn(group_id, "\n")] = '\0';

		if(strlen(group_id) == 0) {
			print_error("Please provide a group id!");
			retype = 1;
		} else if(strlen(group_id) > MAX_GROUP_ID) {
			print_error("The group id can have a max of " STR(MAX_GROUP_ID) " chars!");
			retype = 1;
		}

	} while(retype);

	create_group(group_id, &secret);

	print_success("\n🔑 Secret", secret);
	printf("\n----\n");

	free(group_id);
	free(secret);

	return;
}

/*******************************************************************
*
** void delete_group_UI() 
*
** Description:
*		Calls the algorithm that will delete the group in the
*		authentication server.
*
** Parameters:
*  		There are no parameters in this function
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*	
*******************************************************************/
void delete_group_UI() {
	char* group_id = NULL;
	size_t size = 0;
	int retype = 0;
	int response;

	print_title("DELETE GROUP");

	do {
		retype = 0;

		printf("Group id: ");
		getline(&group_id, &size, stdin);
		group_id[strcspn(group_id, "\n")] = '\0';

		if(strlen(group_id) == 0) {
			print_error("Please provide a group id!");
			retype = 1;
		} else if(strlen(group_id) > MAX_GROUP_ID) {
			print_error("The group id can have a max of " STR(MAX_GROUP_ID) " chars!");
			retype = 1;
		}

	} while(retype);

	response = delete_group(group_id);

	if(response == SUCCESSFUL_OPERATION) {
		print_success("Deleted group", group_id);
	} else {
		print_error("It was not possible to delete this group");
	}
	printf("\n----\n");

	free(group_id);

	return;
}

/*******************************************************************
*
** void group_info_UI() 
*
** Description:
*		Handles the interaction between the admin and the console,
*		calling the algorithm to receive both the secret and number
*		of key/pair values.
*
** Parameters:
*  		There are no parameters in this function
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*	
*******************************************************************/
void group_info_UI() {
	char *group_id = NULL, *secret = NULL;
	int num_pairs = 0;
	size_t size = 0;
	int retype = 0;
	int code = 0;

	print_title("SHOW GROUP INFO");

	do {
		retype = 0;

		printf("Group id: ");
		getline(&group_id, &size, stdin);
		group_id[strcspn(group_id, "\n")] = '\0';

		if(strlen(group_id) == 0) {
			print_error("Please provide a group id!");
			retype = 1;
		} else if(strlen(group_id) > MAX_GROUP_ID) {
			print_error("The group id can have a max of " STR(MAX_GROUP_ID) " chars!");
			retype = 1;
		}

	} while(retype);

	code = group_info(group_id, &secret, &num_pairs);

	if(code != NONEXISTENT_GROUP) {
		print_success("Secret", secret);
		print_success("Number of key/pair values", int2str(num_pairs));
	} else {
		print_warning("The specified group does not exist");
	}

	printf("\n----\n");

	free(group_id);
	free(secret);

	return;
}

/*******************************************************************
*
** void app_status_UI() 
*
** Description:
*		Calls de algorithm to obtain the status of the application
*
** Parameters:
*  		There are no parameters in this function
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*	
*******************************************************************/
void app_status_UI() {
	print_title("SHOW APPLICATION STATUS");

	app_status();

	printf("----\n");

	return;
}

/*******************************************************************
*
** void UI() 
*
** Description:
*		Creates the interection between the admin of local server
*		and the local server/authentication server
*
** Parameters:
*  		There are no parameters in this function
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*	
*******************************************************************/
void UI() {
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
				delete_group_UI();
				break;

			case 3:
				group_info_UI();
				break;

			case 4:
				app_status_UI();
				break;

			case 5:
				// TODO close
				break;

			default:
				break;
		}
	}
}
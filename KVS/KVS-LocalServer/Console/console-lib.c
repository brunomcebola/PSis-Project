#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int menu() {
	int option;

	printf("---------------------------------\n");
	printf("------------ Console ------------\n");
	printf("---------------------------------\n\n");

	printf("What do you want to do?\n");
	printf("-- 1) Create Group --------------\n");
	printf("-- 2) Delete Group --------------\n");
	printf("-- 3) Show Group Info -----------\n");
	printf("-- 4) Show Application Status ---\n\n");

	printf("Option: ");

	scanf("%d", &option);

	return option;
}
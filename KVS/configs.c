#include "configs.h"

#include <stdio.h>
#include <stdlib.h>

void print_title(char* title) {
	printf(ANSI_BOLD "%s\n\n" ANSI_RESET, title);

	return;
}

void print_error(char* error) {
	printf(ANSI_RED "\n\nError: " ANSI_RESET "%s!\n\n", error);

	return;
}

void print_success(char* description, char* data) {
	printf(ANSI_BOLD ANSI_GREEN "%s: " ANSI_RESET ANSI_BOLD "%s\n" ANSI_RESET, description, data);

	return;
}

char* int2str(int val) {
	char str[15];

	sprintf(str, "%d", val);

	return str;
}
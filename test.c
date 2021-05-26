#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "./KVS/KVS-Lib/KVS-lib.h"

int main() {
	char *secret = "adeus", *group_id = "ola";
	char** value = calloc(1, sizeof(char*));

	int p = establish_connection(group_id, secret);

	printf("Code: %d\n", p);

	put_value("nome", "Bruno Cebola");

	get_value("nome", value);

	printf("Nome: %s\n", *value);

	while(1) {
	}

	return 0;
}
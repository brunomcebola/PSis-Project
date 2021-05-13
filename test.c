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
	char *secret = "adeus", *id = "ola";

	int p = establish_connection(id, secret);

	printf("Code: %d\n", p);

	while (1) {
	}

	return 0;
}
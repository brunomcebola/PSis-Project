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
	char *secret = "o meu segredo bue fixe!", *group_id = "hello";
	char** value = calloc(1, sizeof(char*));

	int p = establish_connection(group_id, secret);

	printf("Code: %d\n", p);

	put_value("nome1", "Bruno Cebola");

	put_value("nome2", "Rui Abrantes");

	put_value("nome3", "Vasco Rodrigues");

	put_value("nome4", "André Silva");

	return 0;
}
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "./KVS/KVS-Lib/KVS-lib.h"

int main() {
	char *secret = "L4H4Y7K3T7U2L2T4J0J7N5Q1P7B5E2D9", *group_id = "hello";
	char *nome1 = NULL, *nome2 = NULL, *nome3 = NULL, *nome4 = NULL;

	int p = establish_connection(group_id, secret);

	printf("Code: %d\n", p);

	put_value("nome1", "Bruno Cebola");

	put_value("nome2", "Rui Abrantes");

	put_value("nome3", "Vasco Rodrigues");

	put_value("nome4", "André Silva");

	get_value("nome1", &nome1);

	get_value("nome2", &nome2);

	get_value("nome3", &nome3);

	get_value("nome4", &nome4);

	printf("Nome1: %s\n", nome1);

	printf("Nome2: %s\n", nome2);

	printf("Nome3: %s\n", nome3);

	printf("Nome4: %s\n", nome4);

	//

	put_value("nome4", "Bruno Cebola");

	put_value("nome3", "Rui Abrantes");

	put_value("nome2", "Vasco Rodrigues");

	put_value("nome1", "André Silva");

	get_value("nome1", &nome1);

	get_value("nome2", &nome2);

	get_value("nome3", &nome3);

	get_value("nome4", &nome4);

	printf("Nome1: %s\n", nome1);

	printf("Nome2: %s\n", nome2);

	printf("Nome3: %s\n", nome3);

	printf("Nome4: %s\n", nome4);

	//

	delete_value("nome1");

	delete_value("nome2");

	delete_value("nome3");

	delete_value("nome4");

	//

	close_connection();

	return 0;
}
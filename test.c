#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "./KVS/configs.h"

#include "./KVS/KVS-Lib/KVS-lib.h"

void my_function(char* key) {
	printf("\n\n[%s] Chave modificada!\n\n", key);
}

int main() {
	char *secret = "X4C7M4B8T1I7W8L2G2E9I0H5M7C7M8O9", *group_id = "hello";
	char *nome1 = NULL, *nome2 = NULL, *nome3 = NULL, *nome4 = NULL;

	establish_connection(group_id, secret);

	put_value("nome1", "Bruno Cebola");

	get_value("nome1", &nome1);

	printf("Nome1: %s\n", nome1);

	//register_callback("nome1", my_function);

	//

	put_value("nome1", "André Silva");

	get_value("nome1", &nome1);

	printf("Nome1: %s\n", nome1);

	//

	delete_value("nome1");

	//

	close_connection();

	return 0;
}
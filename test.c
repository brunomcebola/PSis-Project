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
	char *secret = "S3N4V1H5M5M5J8Z9X9W8U1D2Q1T4T1T3", *group_id = "hello";
	char *nome1 = NULL, *nome2 = NULL, *nome3 = NULL, *nome4 = NULL;

	printf("%s\n", int2str(2));

	int p = establish_connection(group_id, secret);

	put_value("nome1", "Bruno Cebola");

	get_value("nome1", &nome1);

	printf("Nome1: %s\n", nome1);

	register_callback("nome1", my_function);

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
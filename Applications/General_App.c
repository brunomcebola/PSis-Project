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
	char *secret = "E1O6E4X2W9Q1Q3E1X5J6O9R1K5Y4Q4R8", *group_id = "hello";
	char* group1 = NULL,group2 = NULL, group3 = NULL, group4 = NULL;

	establish_connection(group_id, secret);

	put_value("nome1", "Bruno Cebola");

	get_value("nome1", &group1);

	printf("Nome1: %s\n", group1);

	register_callback("nome1", my_function);

	//

	put_value("nome1", "ola\nadeus");

	get_value("nome1", &group1);

	printf("Nome1: %s\n", group1);

	//

	put_value("nome2", "Bruce Lee");

	get_value("nome2", &group2);

	printf("Nome2: %s\n", group2);

	//

	put_value("nome3", "Logan Paul");

	get_value("nome3", &group3);

	printf("Nome3: %s\n", group3);

	//

	put_value("nome4", "Floyd Maywheather");

	get_value("nome4", &group4);

	printf("Nome4: %s\n", group4);

	//

	delete_value("nome1");
	delete_value("nome2");
	delete_value("nome3");
	delete_value("nome4");

	//

	get_value("nome1", &group1);
	get_value("nome2", &group2);
	get_value("nome3", &group3);
	get_value("nome4", &group4);
	printf("Nome1 -> %s\n", group1);
	printf("Nome2 -> %s\n", group2);
	printf("Nome3 -> %s\n", group3);
	printf("Nome4 -> %s\n", group4);


	//

	close_connection();

	return 0;
}
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "./Hashtable/hashtable-lib.h"
#include "./KVS/KVS-Lib/KVS-lib.h"

int main() {
	char *tester;

	key_pair **my_hash_table;

	my_hash_table = create_hash_table();

	put_on_hash_table(my_hash_table, "boas", "4321");
	put_on_hash_table(my_hash_table, "boas1", "1234");

	get_from_hash_table(my_hash_table, "boas", &tester);

	printf("My value on get is %s\n\n\n", tester);

	printf_hash_table(my_hash_table);

	delete_from_hash_table(my_hash_table, "boas");

	printf("\n\n\n");

	printf_hash_table(my_hash_table);

	char *secret = "adeus", *id = "ola";

	int p = establish_connection(id, secret);

	printf("Code: %d\n", p);

	while (1) {
	}

	return 0;
}
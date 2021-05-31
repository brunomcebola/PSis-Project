#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

<<<<<<< HEAD
#include <time.h>
#include <stdlib.h>

#include "./Hashtable/hashtable-lib.h"

char* generate_secret() {

	char * key = calloc(10, sizeof(char));
	srand(time(NULL));
	int plus_one;

	for(int i = 0; i <10 ; i = i + 2){
		// random upper letter
		key[i] = 'A' + (rand() % 26);
		// random number
		plus_one = i +1;
		key[plus_one] = '0' + (rand() % 10);
	}

	return key;
}

int main() {

	char  * test;
=======
#include "./KVS/KVS-Lib/KVS-lib.h"

int main() {
	char *secret = "o meu segredo bue fixe!", *group_id = "hello";
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
>>>>>>> bruno

	
	while(1){
		test = generate_secret();
		printf(test);
		getchar();
	}
	return 0;
}
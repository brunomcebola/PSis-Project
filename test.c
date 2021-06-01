#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

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

	
	while(1){
		test = generate_secret();
		printf(test);
		getchar();
	}
	return 0;
}
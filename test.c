#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

void *getstuff(void *arg) {
	char t = '\0';

	while (1) {
		t = getchar();
		printf("Thread: %c\n", t);
	}
    pthread_exit(0);
}

int main() {
	char t = '\0';

	pthread_t getstuff_thread;
	pthread_create(&getstuff_thread, NULL, getstuff, NULL);

	while (1) {
		t = getchar();
		printf("Main: %c\n", t);
	}

	return 0;
}
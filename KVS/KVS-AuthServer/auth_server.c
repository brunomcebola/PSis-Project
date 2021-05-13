#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define AUTH_SERVER_ADDRESS "127.0.0.1"
#define AUTH_SERVER_PORT 3000

int auth_server_socket = -1;

int main(int argc, char const *argv[]) {
	int opt = 1;
	char buffer[1024] = {0};
	char *hello = "Hello from server";
	int len;

	struct sockaddr_in auth_server_addr;
	struct sockaddr_in local_server_addr;

	len = sizeof(local_server_addr);

	printf("Starting server...\n\n");

	// Creating socket file descriptor
	if ((auth_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		perror("");
		exit(-1);
	}

	if (setsockopt(auth_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("");
		exit(-1);
	}
	auth_server_addr.sin_family = AF_INET;
	auth_server_addr.sin_addr.s_addr = INADDR_ANY;
	auth_server_addr.sin_port = htons(AUTH_SERVER_PORT);

	if (bind(auth_server_socket, (struct sockaddr *)&auth_server_addr, sizeof(auth_server_addr)) < 0) {
		perror("");
		exit(-1);
	}

	printf("Server up and running!\n\n");

	int bytes = -1;
	int group_id_len = 0, secret_len = 0;
	char *group_id = NULL, *secret = NULL;
	int code = -1;

	while (1) {
		bytes = recvfrom(auth_server_socket, &group_id_len, sizeof(int), MSG_WAITALL, (struct sockaddr *)&local_server_addr, &len);
		if (bytes == -1) {
			perror("");
			exit(-1);
		}

		group_id = calloc(group_id_len, sizeof(char));
		bytes = recvfrom(auth_server_socket, group_id, group_id_len, MSG_WAITALL, (struct sockaddr *)&local_server_addr, &len);
		if (bytes == -1) {
			perror("");
			exit(-1);
		}

		bytes = recvfrom(auth_server_socket, &secret_len, sizeof(int), MSG_WAITALL, (struct sockaddr *)&local_server_addr, &len);
		if (bytes == -1) {
			perror("");
			exit(-1);
		}

		secret = calloc(secret_len, sizeof(char));
		bytes = recvfrom(auth_server_socket, secret, secret_len, MSG_WAITALL, (struct sockaddr *)&local_server_addr, &len);
		if (bytes == -1) {
			perror("");
			exit(-1);
		}

		code = 10;

		sendto(auth_server_socket, &code, sizeof(int), MSG_CONFIRM, (struct sockaddr *)&local_server_addr, len);
	}

	return 0;
}

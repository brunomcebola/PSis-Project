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
	/*char *secret = "adeus", *id = "ola";
	char** value = calloc(1, sizeof(char*));

	int p = establish_connection(id, secret);

	printf("Code: %d\n", p);

	put_value("nome", "Bruno Cebola");

	get_value("nome", value);

	printf("Nome: %s\n", *value);

	while(1) {
	}*/

	char* group_id = NULL;

	group_id = strndup("gCPHYTR54tOit4UYL1XoKbfccsN3NGea5WGUAL9mwNhS5cUwuT8GIXI2P933rQ02myVMXOiWwhnPi0ImGqJCJm0HUaxBTYjnaHcqRWpuN80khm6athbsi5SvCBFq9FSU0FHr29037tepx0IRVxGOsjCNqTXAnkIr3OiYglMYYUsEpPLpdbcuL92aqEkNeR6m3Ck4ULcUGpY3kBK11yIyrhZJGbb3QOBkw8aogEoGC2k4R7whH2M14kUtmrKP ola", 256);

	printf("%s", group_id);

	return 0;
}
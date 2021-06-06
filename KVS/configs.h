#ifndef _CONFIGS_H

#define _CONFIGS_H

// servers configurations

#define LOCAL_SERVER_ADDRESS "/tmp/kvs_local_server_socket"
#define CB_LOCAL_SERVER_ADDRESS "/tmp/kvs_cb_local_server_socket"
#define AUTH_SERVER_ADDRESS "127.0.0.1"
#define APPS_AUTH_SERVER_PORT 3000
#define CONSOLE_AUTH_SERVER_PORT 3002

// operation codes

#define PUT 'P'
#define GET 'G'
#define DEL 'D'
#define RCB 'R'
#define POST 'O'

// commun structs

#define MAX_GROUP_ID 64
#define MAX_SECRET 32
#define MAX_KEY 64
#define MAX_NAME MAX_KEY + 16
#define MAX_QUEUE 16

typedef struct {
	char group_id[MAX_GROUP_ID + 1];
	char secret[MAX_SECRET + 1];
} access_packet;

typedef struct {
	int pid;
	access_packet credentials;
} connection_packet;

typedef struct {
	char type;
	char group_id[MAX_GROUP_ID + 1];
} operation_packet;

// colors and fonts

#define ANSI_BOLD "\x1b[1m"
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_RESET "\x1b[0m"

// MACROS

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// response codes

#define UNABLE_TO_CONNECT -1
#define CLOSED_CONNECTION -2
#define WRONG_PARAM -3
#define SENT_BROKEN_MESSAGE -4
#define RECEIVED_BROKEN_MESSAGE -5
#define NO_MEMORY_AVAILABLE -6
#define UNSUCCESSFUL_OPERATION -7
#define WRONG_SECRET -8
#define NONEXISTENT_GROUP -9
#define NONEXISTENT_KEY -11
#define CONNECTION_ALREADY_EXISTS -12
#define UNSUCCESSFUL_SUBOPERATION -13
#define NONEXISTENT_HASH_TABLE -14

#define SUCCESSFUL_OPERATION 1
#define SUCCESSFUL_CONNECTION 2
#define CREATED 3
#define UPDATED 4

// helper functions

void print_title(char* title);
void print_error(char* error);
void print_success(char* description, char* data);
char* int2str(int val);
void print_warning(char* warning);

#endif
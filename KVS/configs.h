#ifndef _CONFIGS_H

#define _CONFIGS_H

// servers configurations

#define LOCAL_SERVER_ADRESS "/tmp/kvs_local_server_socket"
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

#define MAX_GROUP_ID 1024
#define MAX_SECRET 32

typedef struct {
	char group_id[MAX_GROUP_ID + 1];
	char secret[MAX_SECRET + 1];
} access_packet;

typedef struct {
	char type;
	char group_id[MAX_GROUP_ID + 1];
} operation_packet;

#endif
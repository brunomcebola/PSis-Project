#ifndef _CONNECTIONS_LIB_H

#define _CONNECTIONS_LIB_H

// functions to handle application side requestes

int setup_connections();
void start_connections();

// functions to handle console side requestes

int create_group(char* group_id, char** secret);
int delete_group(char* group_id);
int group_info(char* group_id, char** secret, int* num_pairs);
void app_status();

#endif
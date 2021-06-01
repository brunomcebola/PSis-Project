#ifndef _CONNECTIONS_LIB_H

#define _CONNECTIONS_LIB_H

// functions to handle application side requestes

void setup_connections();
void start_connections();

// functions to handle console side requestes

char* create_group(char* group_id);
int delete_group(char * group_id);
void group_info(char* group_id, char** secret, int* num_pairs);
int show_app_status();

#endif
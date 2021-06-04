#ifndef _HASHTABLE_LIB_H

#define _HASHTABLE_LIB_H

#include <stdio.h>
#include <stdlib.h>

typedef struct _key_pair_t key_pair_t;

key_pair_t** create_hash_table();
void destroy_hash_table(key_pair_t** hash_table);
int put_on_hash_table(key_pair_t** hash_table, char* key, char* value);
// isto era suposto ter sem_t mas estava a tripar por isso pus void
int put_sem_on_hash_table(key_pair_t** hash_table, char* key, char* sem_name);
int get_from_hash_table(key_pair_t** hash_table, char* key, char** value);
int delete_from_hash_table(key_pair_t** hash_table, char* key);
int get_number_of_entries(key_pair_t** hash_table);

void printf_hash_table(key_pair_t** hash_table);

#endif
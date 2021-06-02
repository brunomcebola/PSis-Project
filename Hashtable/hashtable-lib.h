#ifndef _HASHTABLE_LIB_H

#define _HASHTABLE_LIB_H

typedef struct _key_pair key_pair;

key_pair** create_hash_table();
int destroy_hash_table(key_pair** hash_table);
int put_on_hash_table(key_pair** hash_table, char* key, char* value);
// isto era suposto ter sem_t mas estava a tripar por isso pus void
int put_sem_pipe_on_hash_table(key_pair** hash_table, char* key, void *sem_id);
int get_sem_pipe_from_hash_table(key_pair** hash_table, char* key);
int get_from_hash_table(key_pair** hash_table, char* key, char** value);
int delete_from_hash_table(key_pair** hash_table, char* key);
int get_number_of_entries(key_pair** hash_table);

void printf_hash_table(key_pair** hash_table);

#endif
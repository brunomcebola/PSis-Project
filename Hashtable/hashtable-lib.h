#ifndef _HASHTABLE_LIB_H

#define _HASHTABLE_LIB_H

typedef struct _key_pair key_pair;

key_pair** create_hash_table();
int destroy_hash_table(key_pair** hash_table);
int put_on_hash_table(key_pair** hash_table, char* key, char* value);
int get_from_hash_table(key_pair** hash_table, char* key, char** value);
int delete_from_hash_table(key_pair** hash_table, char* key);
int get_number_of_entries(key_pair** hash_table);

void printf_hash_table(key_pair** hash_table);

#endif
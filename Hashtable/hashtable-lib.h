#ifndef _KVS_LIB_H

#define _KVS_LIB_H

typedef struct _key_pair{
    char * key;
    char * value;
    struct _key_pair * next;
}key_pair;

unsigned int hash(char * key);

key_pair ** create_hash_table();
void initialize_hash_table(key_pair ** hash_table);

int put_on_hash_table(key_pair ** hash_table, char * key, char * value);
int get_from_hash_table(key_pair ** hash_table, char * key, char ** value);
int delete_from_hash_table(key_pair ** hash_table, char * value);


#endif
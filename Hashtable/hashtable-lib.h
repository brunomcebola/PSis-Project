#ifndef _KVS_LIB_H

#define _KVS_LIB_H

typedef struct _key_pair{
    char * key;
    char * value;
}key_pair;

unsigned int hash(char * key);
void initialize_hash_table();
int put_on_hash_table(key_pair * new_value);
int get_on_hash_table(key_pair * value);


#endif
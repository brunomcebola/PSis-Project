#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HASH_SIZE 11 //preference of prime number // it was 53
#define MAX_KEY_SIZE 256

#include "hashtable-lib.h"

/*
Colisions need to be handled with lists
*/

void printf_hash_table(key_pair ** hash_table){
    key_pair * hash_helper = NULL;
    for(int i = 0; i< HASH_SIZE; i++){
        hash_helper = hash_table[i];
        if(hash_helper == NULL){
            printf("----\n");
            continue;
        }
        
        do{
            printf("The key is %s, and the value is %s\n", hash_helper->key, hash_helper->value);
            hash_helper = hash_helper->next;
        }while( hash_helper != NULL );
        
    }
}

int check_duplication(key_pair * list, char * key){


    // return 1 if its a duplication
    // return 0 if its not a duplication


    // não sei se isto funciona assim FALTA TESTAR
    while(list){
        if(strcmp(list->key, key) == 0){
            return 1; // this key already exists
        }
        list = list->next;
    }

    return 0; // it means there's no repetion
}

key_pair ** create_hash_table(){
    key_pair ** hash_table;
    hash_table = calloc(HASH_SIZE , sizeof(char *));

    return hash_table;
}

unsigned int hash(char * key){

    /*******************************************
     * Every number here can be changed        *
     * depending on preference and performence *
     *******************************************/
    int key_size = strnlen(key,MAX_KEY_SIZE);
    int hash_weight = 123; // example
    unsigned int hash_value = 0;

    for(int i = 0; i < key_size; i++){
        hash_value += key[i];
        hash_value = (hash_value * hash_weight);
    }
    hash_value = hash_value % HASH_SIZE;
    return hash_value;
}

void initialize_hash_table(key_pair ** hash_table){
    // hash needs to start empy
    for(int i = 0; i < HASH_SIZE; i++)
        hash_table[i] = NULL;
}

int put_on_hash_table(key_pair ** hash_table, char * key, char * value){
    key_pair *old_head = NULL;
    key_pair *new_key = calloc(1, sizeof(key_pair));
    

    new_key->next = NULL;
    new_key->key = malloc( ( strlen(key) + 1) * sizeof(char));
    new_key->value = malloc( ( strlen(value) + 1) * sizeof(char));

    strcpy(new_key->key, key);
    strcpy(new_key->value, value);

    // return 1 sucess
    // return 0 key already existed

    int hash_position = hash(key);

    // checking if its the first time on the hashtable
    if( hash_table[hash_position] == NULL){
        //putting the value on the hash tabble
        hash_table[hash_position] = new_key;
    }
    // handling colisions
    else{
        // checking if the value already exists
        // if(check_duplication(hash_table[hash_position],key)){
        //     return 0; // this key already exists
        // }
        // putting the value on the hash table
        old_head = hash_table[hash_position];
        hash_table[hash_position] = new_key;
        new_key->next = old_head;
    }

    return 1; // it means it was a success
}

int get_from_hash_table(key_pair ** hash_table, char * key, char ** value){    
    char * new_value;
    key_pair * key_pair;

    // return 1 if it exists
    // return 0 if it doesnt exist

    int hash_position = hash(key);
    key_pair = hash_table[hash_position];
    if( key_pair == NULL){
        //putting the value on the hash tabble
        printf("Key %s doesn't exist", *key);
        return 0;
    }

    // searching for the key
    while(key_pair){
        if(strcmp(key_pair->key, key) == 0){
            break; // this key already exists
        }
        key_pair = key_pair->next;
    }

    if(key_pair){
        new_value = calloc( strlen(key_pair->value) + 1, sizeof(char));
        strcpy(new_value,key_pair->value);
        *value = new_value;
        return 1;
    }

    return 0;
}

int delete_from_hash_table(key_pair ** hash_table, char * key){
    key_pair * key_before = NULL;
    key_pair * key_pair = NULL;
    // return 1 means that it was possible to delete
    // return 0 means that it wasn't possible to delete


    int hash_position = hash(key);
    key_pair = hash_table[hash_position];
    if( key_pair == NULL){
        //putting the value on the hash tabble
        printf("Key %s doesn't exist", *key);
        return 0;
    }
    // searching for the key 
    while(key_pair){
        if(strcmp(key_pair->key, key) == 0)
            break; // we found the key we want to delete
        
        key_before = key_pair;
        key_pair = key_pair->next;
    }

    if(key_pair){
        free(key_pair->key);
        free(key_pair->value);
        if(key_before == NULL)
            hash_table[hash_position] = key_pair->next;
        else
            key_before->next = key_pair->next;
        free(key_pair);
    }

    return 0;
}
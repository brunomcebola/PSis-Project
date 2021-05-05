#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HASH_SIZE 53 //preference of prime number 
#define MAX_KEY_SIZE 256

#include "hashtable-lib.h"

key_pair * hash_table[HASH_SIZE];

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
        hash_value = (hash_value * hash_weight) % HASH_SIZE;
    }

    return hash_value;
}

void initialize_hash_table(){
    // hash needs to start empy
    for(int i = 0; i < HASH_SIZE; i++)
        hash_table[i] = NULL;
}

int put_on_hash_table(key_pair * new_value){

    // we probably will need to check if the value
    // already exists

    //return 1 sucess
    //return 0 value already existed

    int hash_position = hash((*new_value).key);
    if( hash_table[hash_position] != NULL)
        (*hash_table)[hash_position] = (*new_value);
    // handling colisions
    else{
        bool check = true;
        int new_position = hash_position + 1;
        do{
            if( hash_table[hash_position] != NULL){
                (*hash_table)[hash_position] = (*new_value);
                check = false;
            }
            else
                new_position++;
        } while(check);
    }

    return 1; // it means it was a success
}

int get_on_hash_table(key_pair * value){

}
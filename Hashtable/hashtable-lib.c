#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#define HASH_SIZE 11 // preference of prime number // it was 53
#define MAX_KEY_SIZE 256

#include "hashtable-lib.h"

typedef struct _sem_list{
	sem_t* sem_id;			// semaphore indentification
	struct _sem_list* next;
} sem_list;

typedef struct _key_pair {
	char* key;
	char* value;
	sem_list * head;
	struct _key_pair* next;
} key_pair;


/*******************************************************************
*
**unisgned int hash() 
*
** Description:
*		Gives a pre-specified weight to the key parameter, so that the
*		postion of the key in the hash table is always the same.
*		This way, the same key will always be in a random but fixed
*		position and different keys will probably be in different 
*		positions.
*
** Parameters:
*  		@param key - string that it's supposed t be stored
*
** Return:
*		Unsiged int that represent the position of a certain key
*		in the hashtable
*
** Side-effects:
*		There's no side-effect 
*		TODO: checkar melhor se realmente não tem side-effect
*******************************************************************/
unsigned int hash(char* key) {
	/*******************************************
	 * Every number here can be changed        *
	 * depending on preference and performence *
	 *******************************************/
	int key_size = strnlen(key, MAX_KEY_SIZE);
	int hash_weight = 123; // example
	unsigned int hash_value = 0;

	for(int i = 0; i < key_size; i++) {
		hash_value += key[i];
		hash_value = (hash_value * hash_weight);
	}
	hash_value = hash_value % HASH_SIZE;
	return hash_value;
}

/*******************************************************************
*
**void printf_hash_table() 
*
** Description:
*		Prints in to the stdout the information stores in the hashtable
*
** Parameters:
*  		@param hash_table - a struct that stores the needed information
*
** Return:
*		This functions doesn't return anything
*
** Side-effects:
*		There's no side-effect 
*******************************************************************/
void printf_hash_table(key_pair** hash_table) {
	key_pair* hash_helper = NULL;
	for(int i = 0; i < HASH_SIZE; i++) {
		hash_helper = hash_table[i];
		if(hash_helper == NULL) {
			printf("----\n");
			continue;
		}

		do {
			printf("The key is %s, and the value is %s\n", hash_helper->key, hash_helper->value);
			hash_helper = hash_helper->next;
		} while(hash_helper != NULL);
	}
}

/*******************************************************************
*
**key_pair** create_hash_table() 
*
** Description:
*		Allocates and inicializes the memory necessary for the hash
*		table to store information
*
** Parameters:
*  		There are no paraments in this function
*
** Return:
*		This function returns the key_pair pointer of the array that
*		represents the hash table that is supposed to store information
*
** Side-effects:
*		There's no side-effect
*******************************************************************/
key_pair** create_hash_table() {
	key_pair** hash_table = calloc(HASH_SIZE, sizeof(char*));

	// hash needs to start empy
	for(int i = 0; i < HASH_SIZE; i++) {
		hash_table[i] = NULL;
	}

	return hash_table;
}

/*******************************************************************
*
**int detroy_hash_table() 
*
** Description:
*		This function frees all the memory used in the hash table.
*		It frees the memory inside each structure and in the end
*		it frees the memory of the array representing the hash
*		table.
*
** Parameters:
*  		@param hash_table - pointer of an array that represents the
*							hashtable used to store information
*
** Return:
*		This function returns X
*
** Side-effects:
*		There's no side-effect 
*		TODO: error handling for the returns
*******************************************************************/
int destroy_hash_table(key_pair** hash_table) {
	key_pair *key_pair = NULL, *key_pair_aux = NULL;

	for(int i = 0; i < HASH_SIZE; i++) {
		key_pair = hash_table[i];

		if(key_pair == NULL) {
			continue;
		} else {
			while(key_pair) {
				key_pair_aux = key_pair;
				key_pair = key_pair->next;

				free(key_pair_aux->key);
				free(key_pair_aux->value);
				free(key_pair_aux);
			}
		}
	}

	return 1;
}

/*******************************************************************
*
**int put_on_hash_table() 
*
** Description:
*		Stores the information in the hash table, the colitions in 
*		the hash table are fixed by a list, if a certain key needs
*		to be stored in and already used position it goes to the head
*		of the list, sao the last value of the list is the first one
*		to be stored
*
** Parameters:
*  		@param hash_table - pointer of an array that represents the
*							hashtable used to store information
*		@param key - string that it's supposed t be stored and
*					represents a certain value
*		@param value - string that it's supposed t be stored being
*					the main data that's supposed to be stored
*
** Return:
*		This function returns X
*
** Side-effects:
*		There's no side-effect 
*		TODO: error handling for the returns
*******************************************************************/
int put_on_hash_table(key_pair** hash_table, char* key, char* value) {
	key_pair* old_head = NULL;
	key_pair* new_key = NULL;

	key_pair* key_pair = NULL;
	int hash_position = 0;

	hash_position = hash(key);

	// checking if key is new
	key_pair = hash_table[hash_position];
	while(key_pair) {
		if(strcmp(key_pair->key, key) == 0) {
			break; // this key already exists
		}
		key_pair = key_pair->next;
	}

	// create new key/pair value
	if(key_pair == NULL) {
		new_key = calloc(1, sizeof(key_pair));

		new_key->next = hash_table[hash_position];
		new_key->key = malloc((strlen(key) + 1) * sizeof(char));
		new_key->value = malloc((strlen(value) + 1) * sizeof(char));
		new_key->head = NULL;

		strcpy(new_key->key, key);
		strcpy(new_key->value, value);

		hash_table[hash_position] = new_key;
	}
	// update already existing key/pair value
	else {
		free(key_pair->value);
		key_pair->value = malloc((strlen(value) + 1) * sizeof(char));
		strcpy(key_pair->value, value);
	}

	return 1; // it means it was a success
}

/*******************************************************************
*
**int put_on_hash_table() 
*
** Description:
*		Stores the semaphore id information in the hash table so
*		it's easy to know which semaphore needs to be used for a
*		callback function
*
** Parameters:
*  		@param hash_table - pointer of an array that represents the
*							hashtable used to store information
*		@param key - string that it's supposed t be stored and
*					represents a certain value
*		@param sem_id - pointer of void that will represent the 
*						sem_t from the <semaphore.h> library and
*						that indicates which semaphore is used
*						for this key
*
** Return:
*		This function returns X
*
** Side-effects:
*		There's no side-effect 
*		TODO: error handling for the returns
*******************************************************************/
int put_sem_on_hash_table(key_pair** hash_table, char* key, void *sem_id){
	
	key_pair* key_pair = NULL;
	int hash_position = 0;

	sem_list* aux = NULL;

	sem_list* new_sem = calloc(1, sizeof(sem_list));
	new_sem->sem_id = (sem_t *) sem_id;

	hash_position = hash(key);

	// checking if key is new
	key_pair = hash_table[hash_position];
	while(key_pair) {
		if(strcmp(key_pair->key, key) == 0) {
			break; // this key already exists
		}
		key_pair = key_pair->next;
	}

	aux = key_pair->head;
	key_pair->head = new_sem;
	new_sem->next = aux;

	return 1;
}

/*******************************************************************
*
**int put_on_hash_table() 
*
** Description:
*		Searches for a key and storing the value of it in a variable
*		that the user of this function has. 
*
** Parameters:
*  		@param hash_table - pointer of an array that represents the
*							hashtable used to store information
*		@param key - string that it's supposed t be stored and
*					represents a certain value
*		@param value - string that wants to be stored and wants be 
*						known by the user of this function
*
** Return:
*		This function returns X
*
** Side-effects:
*		There's no side-effect 
*		TODO: error handling for the returns
*******************************************************************/
int get_from_hash_table(key_pair** hash_table, char* key, char** value) {
	char* new_value;
	key_pair* key_pair;

	// return 1 if it exists
	// return 0 if it doesnt exist

	int hash_position = hash(key);
	key_pair = hash_table[hash_position];
	if(key_pair == NULL) {
		// putting the value on the hash tabble
		printf("Key %s doesn't exist", key);
		return 0;
	}

	// searching for the key
	while(key_pair) {
		if(strcmp(key_pair->key, key) == 0) {
			break; // this key already exists
		}
		key_pair = key_pair->next;
	}

	if(key_pair) {
		new_value = calloc(strlen(key_pair->value) + 1, sizeof(char));
		strcpy(new_value, key_pair->value);
		*value = new_value;
		return 1;
	}

	return 0;
}

/*******************************************************************
*
**int delete_sem_list() 
*
** Description:
*		Deletes all the informations about the list of semaphores 
*
** Parameters:
*  		@param key_given - is the struct that holds all the important
*						information, and it's the struct that wants
*						to be deleted, by being the parameter instead
*						of the key, it's not needed to do the search
*						making it faster. 
*
** Return:
*		This function returns X
*
** Side-effects:
*		There's no side-effect 
*		TODO: error handling for the returns
*******************************************************************/
int delete_sem_list(key_pair* key_given){

	sem_list* head = key_given->head;
	sem_list* deleting_item = key_given->head;
	
	while(head != NULL) {
		head = head->next;
		free(deleting_item);
		deleting_item = head;	
	}

	return 1;
}

/*******************************************************************
*
**int delete_from_hash_table() 
*
** Description:
*		It deletes a specified key in the hash table, freeing all
*		the memory that's attached to it.
*
** Parameters:
*  		@param hash_table - pointer of an array that represents the
*							hashtable used to store information
*		@param key - string that it's supposed t be stored and
*					represents a certain value
*
** Return:
*		This function returns X
*
** Side-effects:
*		There's no side-effects in this function
*		TODO: checkar os erros handlings para a saída
*******************************************************************/
int delete_from_hash_table(key_pair** hash_table, char* key) {
	key_pair* key_before = NULL;
	key_pair* key_pair = NULL;
	// return 1 means that it was possible to delete
	// return 0 means that it wasn't possible to delete

	int hash_position = hash(key);
	key_pair = hash_table[hash_position];
	if(key_pair == NULL) {
		// putting the value on the hash tabble
		printf("Key %s doesn't exist", key);
		return 0;
	}
	// searching for the key
	while(key_pair) {
		if(strcmp(key_pair->key, key) == 0)
			break; // we found the key we want to delete

		key_before = key_pair;
		key_pair = key_pair->next;
	}

	if(key_pair) {
		free(key_pair->key);
		free(key_pair->value);
		// TODO: STILL NEEDS TO DELETE LIST OF SEMAPHORES
		if(key_before == NULL)
			hash_table[hash_position] = key_pair->next;
		else
			key_before->next = key_pair->next;
		free(key_pair);
	}

	return 0;
}

/*******************************************************************
*
**int get_number_of_entries() 
*
** Description:
*		It counts the number of all the keys stored in the hash table
*
** Parameters:
*  		@param hash_table - pointer of an array that represents the
*							hashtable used to store information
*
** Return:
*		This function returns the number of keys that a certain group
*		has stored
*
** Side-effects:
*		Depending on the size of hash table and the position of the
*		keys in the hash table, it can take a long time to count
*		all of them
*******************************************************************/
int get_number_of_entries(key_pair** hash_table) {
	key_pair* key_pair = NULL;
	int entries = 0;

	for(int i = 0; i < HASH_SIZE; i++) {
		key_pair = hash_table[i];

		if(key_pair == NULL) {
			continue;
		} else {
			while(key_pair) {
				entries++;
				key_pair = key_pair->next;
			}
		}
	}

	return entries;
}
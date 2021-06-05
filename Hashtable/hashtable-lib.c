#include "hashtable-lib.h"

#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define HASH_SIZE 11 // preference of prime number // it was 53
#define MAX_KEY_SIZE 256

#include "../KVS/configs.h"

typedef struct _sem_list_t {
	sem_t* sem_id; // semaphore indentification
	struct _sem_list_t* next;
} sem_list_t;

typedef struct _key_pair_t {
	char* key;
	char* value;
	sem_list_t* sem_head;
	struct _key_pair_t* next;
} key_pair_t;

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
*
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
void printf_hash_table(key_pair_t** hash_table) {
	key_pair_t* hash_helper = NULL;
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
*		This function returns nothing
*
** Side-effects:
*		There's no side-effect 
*******************************************************************/
void delete_sem_list(key_pair_t* key_given) {
	sem_list_t* head = key_given->sem_head;
	sem_list_t* deleting_item = key_given->sem_head;

	while(head != NULL) {
		sem_post(head->sem_id);
		head = head->next;
		free(deleting_item);
		deleting_item = head;
	}

	return;
}

/*******************************************************************
*
**key_pair_t** create_hash_table() 
*
** Description:
*		Allocates and inicializes the memory necessary for the hash
*		table to store information
*
** Parameters:
*  		There are no paraments in this function
*
** Return:
*		This function returns the key_pair_t pointer of the array that
*		represents the hash table that is supposed to store information
*
** Side-effects:
*		There's no side-effect
*******************************************************************/
key_pair_t** create_hash_table() {
	key_pair_t** hash_table = calloc(HASH_SIZE, sizeof(char*));

	// hash needs to start empy
	for(int i = 0; i < HASH_SIZE; i++) {
		hash_table[i] = NULL;
	}

	return hash_table;
}

/*******************************************************************
*
**void detroy_hash_table() 
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
*		This function doesn't return anything
*
** Side-effects:
*		There's no side-effect 
*******************************************************************/
void destroy_hash_table(key_pair_t** hash_table) {
	key_pair_t *key_pair = NULL, *key_pair_aux = NULL;

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
				delete_sem_list(key_pair_aux);
				free(key_pair_aux);
			}
		}
	}
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
*		This function returns UPDATE if the key already existed but
*		the value was different and return SUCCESSFUL_OPERATION otherwhise
*
** Side-effects:
*		If it's an update it posts the semaphores so the registers
*		callback knows that there's has been an update on the the key
*
*******************************************************************/
int put_on_hash_table(key_pair_t** hash_table, char* key, char* value) {
	key_pair_t* old_head = NULL;
	key_pair_t* new_key = NULL;

	key_pair_t* key_pair = NULL;
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
		new_key = calloc(1, sizeof(key_pair_t));

		new_key->next = NULL;

		new_key->key = calloc(strlen(key) + 1, sizeof(char));
		new_key->value = calloc(strlen(value) + 1, sizeof(char));
		new_key->sem_head = NULL;

		strcpy(new_key->key, key);
		strcpy(new_key->value, value);

		hash_table[hash_position] = new_key;

		return CREATED;
	}
	// update already existing key/pair value
	else if(strcmp(key_pair->value, value) != 0) {
		free(key_pair->value);
		key_pair->value = calloc((strlen(value) + 1), sizeof(char));
		strcpy(key_pair->value, value);

		sem_list_t* aux = key_pair->sem_head;
		while(aux != NULL) {
			sem_post(aux->sem_id);
			aux = aux->next;
		}

		return UPDATE;
	}
}

/*******************************************************************
*
**int put_sem_on_hash_table() 
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
*		@param sem_name - pointer of void that will represent the 
*						sem_t from the <semaphore.h> library and
*						that indicates which semaphore is used
*						for this key
*
** Return:
*		This function returns SUCCESSFUL_OPERATION if it was possible
*		to store the semaphore name. It returns UNSUCCESSFUL_OPERATION
*		if the calloc can't allocate or inicialize the memory
*
** Side-effects:
*		There's no side-effect 
*******************************************************************/
int put_sem_on_hash_table(key_pair_t** hash_table, char* key, char* sem_name) {
	key_pair_t* key_pair = NULL;
	int hash_position = 0;
	sem_list_t* new_sem = NULL;

	hash_position = hash(key);

	// checking if key is new
	key_pair = hash_table[hash_position];
	while(key_pair) {
		if(strcmp(key_pair->key, key) == 0) {
			break; // this key already exists
		}
		key_pair = key_pair->next;
	}

	if(key_pair) {
		new_sem = calloc(1, sizeof(sem_list_t));
		if(new_sem == NULL) {
			return UNSUCCESSFUL_OPERATION;
		}

		new_sem->sem_id = sem_open(sem_name, O_CREAT, 0600, 0);

		new_sem->next = key_pair->sem_head;

		key_pair->sem_head = new_sem;
	}
	return SUCCESSFUL_OPERATION;
}

/*******************************************************************
*
**int get_from_hash_table() 
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
*		This function returns NONEXISTENT_KEY if the value isn't in 
*		the hash table and SUCCESSFUL_OPERATION if it was possible 
*		to find the key.
*
** Side-effects:
*		There's no side-effect 
*******************************************************************/
int get_from_hash_table(key_pair_t** hash_table, char* key, char** value) {
	key_pair_t* key_pair;

	int hash_position = hash(key);
	key_pair = hash_table[hash_position];
	if(key_pair == NULL) {
		return NONEXISTENT_KEY;
	}

	// searching for the key
	while(key_pair) {
		if(strcmp(key_pair->key, key) == 0) {
			break; // this key already exists
		}
		key_pair = key_pair->next;
	}

	if(key_pair) {
		*value = calloc(strlen(key_pair->value) + 1, sizeof(char));
		strcpy(*value, key_pair->value);
		return SUCCESSFUL_OPERATION;
	}

	return NONEXISTENT_KEY;
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
*		This function returns WRONG_KEY if it wasn't
*		possible to delete the key from the hash table and returns
*		SUCCESSFUL_OPERATION if it was possible to delete the key
*		from the hash table
*
** Side-effects:
*		There's no side-effects in this function
*******************************************************************/
int delete_from_hash_table(key_pair_t** hash_table, char* key) {
	key_pair_t* key_before = NULL;
	key_pair_t* key_pair = NULL;
	// return 1 means that it was possible to delete
	// return 0 means that it wasn't possible to delete

	int hash_position = hash(key);
	key_pair = hash_table[hash_position];
	if(key_pair == NULL) {
		return WRONG_KEY;
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
		//check if the function works fine
		delete_sem_list(key_pair);
		if(key_before == NULL)
			hash_table[hash_position] = key_pair->next;
		else
			key_before->next = key_pair->next;
		free(key_pair);
		return SUCCESSFUL_OPERATION;
	}

	return WRONG_KEY;
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
int get_number_of_entries(key_pair_t** hash_table) {
	key_pair_t* key_pair = NULL;
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
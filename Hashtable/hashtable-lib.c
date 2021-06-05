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

/*********************************************************************
*
** void delete_sem_list(key_pair_t* key_given)
*
** Description:
*		Deletes all the informations about the list of semaphores.
*
** Parameters:
*  	@param key_given - structure that has the semaphore list that needs
*													to be deleted.
*
** Return:
*		This function returns nothing.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
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


/*********************************************************************
*
** unsigned int hash(char* key)
*
** Description:
*			This functions takes a string and returns its position on 
*			the hash table based on a pre-specified weight, its 
*			dimensions and the letters that the string is composed of.
*
** Parameters:
*  	@param key 	- string that identifies a specified key (which must 
*									have a maximum size of MAX_KEY).
*
** Return:
*		On success: The function returns an unsiged integer that 
*		represent the position of a certain key in the hashtable.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
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


/*********************************************************************
*
** key_pair_t** create_hash_table()
*
** Description:
*		Allocates and inicializes a pre-specified dimension for the 
*		hash-table that's needed for storing information.	
*
** Parameters:
*  	There are no parameters in this function.
*
** Return:
*		On success: A pointer of the array that represents the hash 
*		table needed.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
key_pair_t** create_hash_table() {
	key_pair_t** hash_table = calloc(HASH_SIZE, sizeof(char*));

	// hash needs to start empy
	for(int i = 0; i < HASH_SIZE; i++) {
		hash_table[i] = NULL;
	}

	return hash_table;
}


/*********************************************************************
*
** int destroy_hash_table(key_pair_t** hash_table)
*
** Description:
*			Frees the memory of all the information in the respective
*			structure and in the end frees the memory of the hash
*			table.
*
** Parameters:
*  	@param hash_table 	- pointer of the array that represents the
*							hash table that stores the information.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- NONEXISTENT_HASH_TABLE if the parameter given to the function
*		  is NULL, which means that there's isn't a hash table to be
*		  destroyed.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
int destroy_hash_table(key_pair_t** hash_table) {
	key_pair_t *key_pair = NULL, *key_pair_aux = NULL;

	if(hash_table == NULL){
		return NONEXISTENT_HASH_TABLE;
	}

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

	return SUCCESSFUL_OPERATION;
}


/*********************************************************************
*
** int put_on_hash_table(key_pair_t** hash_table, char* key, char* value)
*
** Description:
*			Stores the information on the hash table allocating memory
*			if the key didn't exists but if it did exist just changes 
*			the value.
*
** Parameters:
*  	@param hash_table 	- pointer of the array that represents the
*							hash table that stores the information;
*	@param key - string that's supposed t be stored and represents
*									a certain value;
*	@param value - string that's supposed t be stored being the main
*								data that's supposed to be stored.
*
** Return:
*		On success: CREATED is returned if it's key didn't
*		exist and UPDATED is returned if the key already existed but 
*		the value is changed.
*
*		On error: 
*		- NO_MEMORY_AVAILABLE if calloc returns an error.
*
** Side-effects:
*		If it's an update it posts the semaphores so the registers
*		callback knows that there's has been an update on the the key
*	
*********************************************************************/
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
		if(new_key == NULL){
			return NO_MEMORY_AVAILABLE;
		}


		new_key->next = NULL;

		new_key->key = calloc(strlen(key) + 1, sizeof(char));
		if(new_key->key == NULL){
			return NO_MEMORY_AVAILABLE;
		}
		new_key->value = calloc(strlen(value) + 1, sizeof(char));
		if(new_key->value == NULL){
			return NO_MEMORY_AVAILABLE;
		}
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

		return UPDATED;
	}
}


/*********************************************************************
*
** int put_sem_on_hash_table(key_pair_t** hash_table, char* key, char* sem_name)
*
** Description:
*		Stores the semaphore id in a list in a specified position of the
*		hash table. This list used for the callback funtion to signal
*		changes in the values.
*
** Parameters:
*  	@param hash_table - pointer of an array that represents the
*							hashtable used to store information;
*	@param key - string that it's supposed t be stored and
*					represents a certain value;
*	@param sem_name - pointer of void that will represent the 
*						sem_t from the <semaphore.h> library and
*						that indicates which semaphore is used
*						for this key.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- NO_MEMORY_AVAILABLE if there's any error in the calloc function.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
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
		if(new_sem == NULL){
			return NO_MEMORY_AVAILABLE;
		}

		new_sem->sem_id = sem_open(sem_name, O_CREAT, 0600, 0);

		new_sem->next = key_pair->sem_head;

		key_pair->sem_head = new_sem;
	}
	return SUCCESSFUL_OPERATION;
}


/*********************************************************************
*
** int get_from_hash_table(key_pair_t** hash_table, char* key, char** value) 
*
** Description:
*		Searches for a key and storing the value of it in a variable
*		that the user of this function has.		
*
** Parameters:
*  	@param hash_table - pointer of an array that represents the
*							hashtable used to store information;
*	@param key - string that it's supposed t be stored and
*					represents a certain value;
*	@param value - string that wants to be stored and wants be 
*						known by the user of this function.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned if it was
*		possible to find the key in the hash table.
*
*		On error: 
*		- NONEXISTENT_KEY is returned if the key doesn't already
*		  exist in the hash table given;
*		- NO_MEMORY_AVAILABLE if there's any error with the calloc.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
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
		if(*value == NULL){
			return NO_MEMORY_AVAILABLE;
		}
		strcpy(*value, key_pair->value);
		return SUCCESSFUL_OPERATION;
	}

	return NONEXISTENT_KEY;
}


/*********************************************************************
*
** int delete_from_hash_table(key_pair_t** hash_table, char* key)
*
** Description:
*		It deletes a specified key in the hash table, freeing all
*		the memory that's attached to it.		
*
** Parameters:
*  	@param hash_table - pointer of an array that represents the
*							hashtable used to store information;
*	@param key - string that it's supposed t be stored and
*					represents a certain value.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned if it was
*		possible to delete the key in the hash table.
*
*		On error: 
*		- NONEXISTENT_KEY if the key didn't exist in the hash table 
*		  given.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
int delete_from_hash_table(key_pair_t** hash_table, char* key) {
	key_pair_t* key_before = NULL;
	key_pair_t* key_pair = NULL;
	// return 1 means that it was possible to delete
	// return 0 means that it wasn't possible to delete

	int hash_position = hash(key);
	key_pair = hash_table[hash_position];
	if(key_pair == NULL) {
		return NONEXISTENT_KEY;
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

	return NONEXISTENT_KEY;
}



/*******************************************************************
*
**int get_number_of_entries() 
*
** Description:
*		It counts the number of all the keys stored in the hash table.
*
** Parameters:
*  	@param hash_table - pointer of an array that represents the
*							hashtable used to store information.
*
** Return:
*		On success: The number of entries in the hash table is returned.
*
*		On error: 
*		- NONEXISTENT_HASH_TABLE if the hash table given doesn't exist.
*
** Side-effects:
*		This function has no side-effect.		
*
*******************************************************************/
int get_number_of_entries(key_pair_t** hash_table) {
	key_pair_t* key_pair = NULL;
	int entries = 0;

	if(hash_table){
		return NONEXISTENT_HASH_TABLE;
	}

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
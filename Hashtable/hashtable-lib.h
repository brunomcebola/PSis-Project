#ifndef _HASHTABLE_LIB_H

#define _HASHTABLE_LIB_H

#include <stdio.h>
#include <stdlib.h>

typedef struct _key_pair_t key_pair_t;

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
*		On error: NULL is returned.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
key_pair_t** create_hash_table();

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
*						  hash table that stores the information.
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
int destroy_hash_table(key_pair_t** hash_table);

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
int put_on_hash_table(key_pair_t** hash_table, char* key, char* value);

/*********************************************************************
*
** int put_sem_on_hash_table(key_pair_t** hash_table, 
**																					char* key, char* sem_name)
*
** Description:
*		Stores the semaphore id in a list in a specified position of the
*		hash table. This list used for the callback funtion to signal
*		changes in the values.
*
** Parameters:
*  	@param hash_table - pointer of an array that represents the
*							          hashtable used to store information;
*	  @param key 				- string that it's supposed t be stored and
*												represents a certain value;
*	  @param sem_name 	- pointer of void that will represent the 
*												sem_t from the <semaphore.h> library and
*												that indicates which semaphore is used
*												for this key.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- NO_MEMORY_AVAILABLE if there's any error in the calloc function;
*		- UNSUCCESSFUL_OPERATION is returned if there's any error related
*		  to the sem_open() function.
*
** Side-effects:
*		This function has no side-effect.
*	
*********************************************************************/
int put_sem_on_hash_table(key_pair_t** hash_table, char* key, char* sem_name);

/*********************************************************************
*
** int get_from_hash_table(key_pair_t** hash_table, char* key, char** value) 
*
** Description:
*		Searches for a key and storing the value of it in a variable
*		that the user of this function has.		
*
** Parameters:
*  	@param hash_table 	- pointer of an array that represents the
*						  hashtable used to store information;
*	@param key 			- string that it's supposed t be stored and
*						  represents a certain value;
*	@param value 		- string that wants to be stored and wants be 
*						  known by the user of this function.
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
int get_from_hash_table(key_pair_t** hash_table, char* key, char** value);

/*********************************************************************
*
** int delete_from_hash_table(key_pair_t** hash_table, char* key)
*
** Description:
*		It deletes a specified key in the hash table, freeing all
*		the memory that's attached to it.		
*
** Parameters:
*  	@param hash_table 	- pointer of an array that represents the
*						  hashtable used to store information;
*	@param key		 	- string that it's supposed t be stored and
*						  represents a certain value.
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
int delete_from_hash_table(key_pair_t** hash_table, char* k);

/*******************************************************************
*
**int get_number_of_entries() 
*
** Description:
*		It counts the number of all the keys stored in the hash table.
*
** Parameters:
*  	@param hash_table - pointer of an array that represents the
*						hashtable used to store information.
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
int get_number_of_entries(key_pair_t** hash_table);

#endif
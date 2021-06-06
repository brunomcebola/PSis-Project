#ifndef _CONNECTIONS_LIB_H

#define _CONNECTIONS_LIB_H

// functions to start the server

/*********************************************************************
* 
** int setup_connections() 
*
** Description:
*		Initializes all the communication sockets (to apps ant to auth 
*		server). Initializes global thread locks that enable the local 
*		server to have synchronization.
*
** Parameters:
*  	This function takes no parameters.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: UNSUCCESSFUL_OPERATION is returned. 
*		
** Side-effects:
*		This function has no side-effects.
*
*********************************************************************/
int setup_connections();

/*********************************************************************
* 
** void start_connections() 
*
** Description:
*		Sets apps socket and callbacs socket to listening mode with a max
*		queue of connections set to MAX_QUEUE. Launches thread to handle 
*		requestes cumming from the apps.
*
** Parameters:
*  	This function takes no parameters.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: UNSUCCESSFUL_OPERATION is returned. 
*		
** Side-effects:
*		This function has no side-effects.
*
*********************************************************************/
int start_connections();

// functions to handle console side requestes

/*********************************************************************
* 
** int group_info(char* group_id, char** secret, int* num_pairs) 
*
** Description:
*		Verifys if the groups exists and provides the secret of it,
*		because in the parameter its passed as a pointer to the string.
*		It also shows both the group_id and the secret as well as
*		the number of keys in this group.
*
** Parameters:
*  	@param group_id  - string that identifies the group;
*  	@param secret	   - pointer of a string that specifies secret of 
*					             a group;
*	  @param num_pairs - pointer of int that says how many keys there 
*					             are in this specified group.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- SENT_BROKEN_MESSAGE if there's any error related to the write
*		  function;
*		- RECEIVED_BROKEN_MESSAGE is returned if there's any error
*		  related to the read function;
*		- NO_MEMORY_AVAILABLE is returned when there's any error related
*		  to the calloc function;
*		- NON_EXISTED_GROUP is returned if the group_id provided
*		  isnt related to an existed group.
*
** Side-effects:
*		There are no side-effects 
*
*******************************************************************/
int group_info(char* group_id, char** secret, int* num_pairs);

/*********************************************************************
* 
**void app_status() 
*
** Description:
*		This function shows the console user the pid, opened time
*		and closed time (if the app has already been closed) of 
*		the application that called this function.
*
** Parameters:
*  		This function doesn't have any parameters.
*
** Return:
*		This functions doesn't return anything.
*		
*
** Side-effects:
*		There are no side-effects
*		
*********************************************************************/
void app_status();

/*********************************************************************
* 
** char* create_group(char* group_id, char** secret)
*
** Description:
*		This function creates memory in the local server for the 
*		group that the console asks for. This function also
*		communicates with the authentication server so it does the 
*		same. The authentication server returns the secret, that 
*		returns \0 if there's any error.
*
** Parameters:
*  		@param group_id - string that identifies the group
*  		@param secret 	- pointer of a string that specifies secret 
*						  of a group
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned if the groups is
*		created successfully.
*
*		On error: 
*		- SENT_BROKEN_MESSAGE if there's any error related to the write
*		  function;
*		- RECEIVED_BROKEN_MESSAGE is returned if there's any error
*		  related to the read function;
*		- NO_MEMORY_AVAILABLE is returned when there's any error related
*		  to the calloc function;
*		- UNSUCCESSFUL_OPERATION is returned if the secret received
*		  from the authentication server is '\0' and if it's read
*		  write mutex can't be inicialized.	
*
** Side-effects:
*		There are no side-effects
*		
*********************************************************************/
int create_group(char* group_id, char** secret);

/*********************************************************************
* 
**int delete_group(char* group_id) 
*
** Description:
*		This functions searches for the group specified in the hash
*		table, to delete the data related to it. It also communicates
*		with the authentication server to do the same in it's side.
*
** Parameters:
*  		@param group_id - string that identifies the group.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned if the groups is
*		deleted successfully.
*
*		On error: 
*		- SENT_BROKEN_MESSAGE if there's any error related to the write
*		  function;
*		- RECEIVED_BROKEN_MESSAGE is returned if there's any error
*		  related to the read function;
*		- NO_MEMORY_AVAILABLE is returned when there's any error related
*		  to the calloc function;
*		- UNSUCCESSFUL_OPERATION is returned if you can't communicate to
*		  the callback functions that you want them to be deleted and 
*		  if it's read
*		  write mutex can't be inicialized.	
*		
*
** Side-effects:
*		There are no side-effects
*
*********************************************************************/
int delete_group(char* group_id);

/*********************************************************************
* 
**int close_local()
*
** Description:
*		This function terminates all connections in and out of the
*		local server, frees all variables and kills the local server
*		itself.
*
** Parameters:
*  		This function takes no parameters
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned.
*
*		On error: UNSUCCESSFUL_OPERATION is returned.
*		
** Side-effects:
*		There are no side-effects
*
*********************************************************************/
int close_local();

#endif
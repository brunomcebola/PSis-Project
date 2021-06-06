#ifndef KVS_LIB_H_

#define KVS_LIB_H_

/* FUNCTION DECLARATIONS */

/*********************************************************************
*
** int establish_connection(char* group_id, char* secret)
*
** Description:
*		Sends a connection_packet to the local server for it to redirect
*		to the authentication server, in order to grant access for the 
*		requesting application to manipulate a specified group (one of the
*		paramenters of the connection_packet). If the sent credentials are
*		correct then a connection is established. Otherwise, an error is
*		returned.		    
*
** Parameters:
*  	@param group_id - string that identifies the group id (which must
*											have a maximum size of MAX_GROUP_ID);
*  	@param secret 	- string that specifies the secret of the group
*											(which must have a maximum size of MAX_SECRET).
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- CONNECTION_ALREADY_EXISTS is returned if a connection already
*			exists
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server;
*		-	NONEXISTENT_GROUP is returned if the the provided group_id does
*			not match with any id of the groups on the local server;
*		-	WRONG_SECRET is returned if the provided secret does not match
*     with the one associated to the group with the provided group_id.
*
** Side-effects:
*		On success it creates a thrad to handle callback calls.
*	
*********************************************************************/
int establish_connection(char* group_id, char* secret);

/*********************************************************************
*
** int put_value(char* key, char* value)
*
** Description:
*		If there is an established connection to a local server then the
*		provided key/value pair is added/updated in the group associated
*		to the connection.
*
** Parameters:
*  	@param key   - string that identifies the key/value pair inside
*									 the connection's group (it must have a maximum 
*									 size of MAX_Key);
*  	@param value - string containing the data to be stored int the
*									 key/value pair of the connection's group (it does
*									 not have a maximum size, but a value must be 
*									 specified).
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server.
*
** Side-effects:
*		If the local server closes the connection then the function forces
*		the connection to close also on the app side.
*		If a callback has been set for the given key (which means it is an
*		update operation) then it gets triggered upon successful edition
*	
*********************************************************************/
int put_value(char* key, char* value);

/*********************************************************************
*
** int get_value(char* key, char** value)
*
** Description:
*		If there is an established connection to a local server then it
*		tries to get the value associated with the given key from the 
*		group associated to the connection.
*
** Parameters:
*  	@param key   - string that identifies the key/value pair inside
*									 the connection's group (it must have a maximum 
*									 size of MAX_Key);
*  	@param value - string where the data of the key/value pair will
*									 be stored.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server;
*		- NO_MEMORY_AVAILABLE is returned if function cannot allocate 
*			memory to store the fetched value.
*
** Side-effects:
*		If the local server closes the connection then the function forces
*		the connection to close also on the app side.
*	
*********************************************************************/
int get_value(char* key, char** value);

/*********************************************************************
*
** int delete_value(char* key)
*
** Description:
*		If there is an established connection to a local server then the
*		provided key/value pair is delete from the group associated to the
*		connection.
*
** Parameters:
*  	@param key - string that identifies the key/value pair inside the
*								 connection's group (it must have a maximum size of
*								 MAX_Key).
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server;
*   - UNSUCCESSFUL_SUBOPERATION is returned when a callback handler
*			could not be erased.
*
** Side-effects:
*		If the local server closes the connection then the function forces
*		the connection to close also on the app side.
*		If a callback has been set for the given key then it gets deleted
*		upon successful key/value pair deletion.
*	
*********************************************************************/
int delete_value(char* key);

/*********************************************************************
*
** int register_callback(char* key, void (*callback_function)(char*))
*
** Description:
*		If there is an established connection to a local server then it
*		associates a callback function to a key/value pair in the group 
*		associated to the connection, if it does not exist already. If it
*		does then the callback function gets updated.
*
** Parameters:
*  	@param key - string that identifies the key/value pair inside the
*								 connection's group (it must have a maximum size of
*								 MAX_Key);
*  	@param callback_function - callback function address.
*
** Return:
*		On success: SUCCESSFUL_OPERATION is returned. 
*
*		On error: 
*		- WRONG_PARAM is returned if either the secret or the  group_id 
*			does not have the correct size; 
*		- UNABLE_TO_CONNECT is return if either the connection to main 
*			socket or to the callback socket is not successful; 
*		- CLOSED_CONNECTION is returned if the local server closes the
* 		connection;
*		- SENT_BROKEN_MESSAGE is return if there is a problem sending the 
*			connection_packet to the local server; 
*		- RECEIVED_BROKEN_MESSAGE is return if there is a problem reading
*			the response from the local server;
*		- NO_MEMORY_AVAILABLE is returned if function cannot allocate 
*			memory to store the new callback on the list;
*		- UNSUCCESSFUL_OPERATION is returned if cannot create semaphore or
*			if local server sends an error code;
*		- UNSUCCESSFUL_SUBOPERATION is returned if cannot launch thread
*			to run the callback.
*
** Side-effects:
*		If the local server closes the connection then the function forces
*		the connection to close also on the app side.
*		If a callback has been set for the given key (which means it is an
*		update operation) then it gets triggered upon successful edition
*	
*********************************************************************/

int register_callback(char* key, void (*callback)(char*));

/*********************************************************************
*
** int close_connection()
*
** Description:
*		Closes all the communication channels between the app and the
*		local server.
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
*		This function has no side-effect.
*	
*********************************************************************/
int close_connection(void);

#endif
#ifndef KVS_LIB_H_

#define KVS_LIB_H_

/* FUNCTION DECLARATIONS */

/**
  ** Description:
  *   This function receives as arguments the strings containing the group name
  *   and corresponding secret, and tries to open the connection with the
  *   KVS-LocalServer.
  *
  ** Parameters:
  *   @param group_id
  *   @param secret 
  *
  ** Scenarios:
  *   If successful, all following operations on key-value pairs are done
  *   in then context of the provided group_id.
  *
  ** Return:
  *   If the secret is correct this function returns 0.
  *
  *   If any error occurs the function should return a negative number.
  *
  *   For each possible error students should define and return a different
  *   return value.
  * 
**/
int establish_connection(char* group_id, char* secret);

/*
 * Description:
 *   This function tries to assign the provided value to the provided key.
 *
 * Scenarios:
 *   - If the provided key-pair does not exist in the server, it is created.
 *
 *   - If the key-value pair already exists in the server it is updated with
 *     the provided value.
 *
 * Return:
 *   In case of success the function return 1.
 *
 *   If any error occurs the function should return a negative number.
 *
 *   For each possible error students should define and return a different
 *   return value.
 */
int put_value(char* key, char* value);

/*
 * Description:
 *   This function tries to retrieve the value associated to the provided key.
 *
 * Scenarios:
 *   - If the provided key-pair exists in the server the corresponding value
 *     is “returned” through the value argument.
 *
 * NOTES:
 *   This function should do a malloc that will store the value associated
 *   with the key.
 *
 * Return:
 *   In case of success the function return 1.
 *
 *   If any error occurs the function should return a negative number.
 *
 *   For each possible error students should define and return a different
 *   return value.
 */
int get_value(char* key, char** value);

/*
 * Description:
 *   This function tries to retrieve delete the pair key-value associated to the
 *   provided key.
 *
 * Return:
 *   In case of success the function return 1.
 *
 *   If any error occurs the function should return a negative number.
 *
 *   For each possible error students should define and return a different
 *   return value.
 */
int delete_value(char* key);

/*
 * Description:
 *   This function tries to register a callback the will later be called.
 *   The function receives the key that will be monitored.
 *   The pointer to the function that will be executed in the
 *   applications, when the value associated with the key is changed.
 *
 * Return:
 *   In case of success the function return 1.
 *
 *   If any error occurs the function should return a negative number.
 *
 *   For each possible error students should define and return a different
 *   return value.
 */
int register_callback(char* key, void (*callback)(char*));

/*
 * Description:
 *   This function closes the connection previously opened.
 *
 * Return:
 *   In case of success the function return 1.
 *
 *   If any error occurs the function should return a negative number.
 *
 *   For each possible error students should define and return a different
 *   return value.
 */
int close_connection(void);

#endif
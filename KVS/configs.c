#include "configs.h"

#include <stdio.h>
#include <stdlib.h>
/*******************************************************************
*
**void print_title() 
*
** Description:
*		Prints out a title, in form of a string, in bold
*
** Parameters:
*  		@param title - string refering to a title printed to the
*						console
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*******************************************************************/
void print_title(char* title) {
	printf(ANSI_BOLD "%s\n\n" ANSI_RESET, title);

	return;
}

/*******************************************************************
*
**void print_error() 
*
** Description:
*		Prints out the error in red so it's clear that it's an error
*
** Parameters:
*  		@param error - string that specifies the error
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*******************************************************************/
void print_error(char* error) {
	printf(ANSI_RED "\n\nError: " ANSI_RESET "%s!\n\n", error);

	return;
}

/*******************************************************************
*
** void print_warning(char* warning)
*
** Description:
*		Prints out the warning in yellow
*
** Parameters:
*  		@param warning - string that specifies the warning
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*
*******************************************************************/
void print_warning(char* warning) {
	printf(ANSI_YELLOW "Warning: " ANSI_RESET "%s!", warning);

	return;
}

/*******************************************************************
*
**void print_sucess() 
*
** Description:
*		Prints in the stdin that something was sucessful, specifying
*		which part was sucessful, in green, and the data related 
*		to that part in bold
*
** Parameters:
*  		@param description - string defining which part was succesful
*		@param data - string that specifies which that was acomplished 
*
** Return:
*		This function doesn't return any information
*
** Side-effects:
*		There's no side-effect 
*******************************************************************/
void print_success(char* description, char* data) {
	printf(ANSI_BOLD ANSI_GREEN "%s: " ANSI_RESET ANSI_BOLD "%s\n" ANSI_RESET, description, data);

	return;
}

/*******************************************************************
*
** char* int2str() 
*
** Description:
*		Converts an intenger to a string value
*
** Parameters:
*  		@param val - int that needs to be converted to string  
*
** Return:
*		The string that was converted from the intenger
*
** Side-effects:
*		It always allocates 15 bytes when probably there's no need
*		for that much
* TODO: bruno confere esta parte 
*******************************************************************/
char* int2str(int val) {
	char* str = calloc(15, sizeof(char));

	sprintf(str, "%d", val);

	return str;
}
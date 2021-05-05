#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <pthread.h>

#include "./Console/console-lib.h"
#include "./Hashtable/hashtable-lib.h"

void * console(void *arg){
    int order_number = 0;

    while(1){
        
        order_number = home_screen(&order_number);

        switch (order_number){
        case 1:
            // Create Group functions
            break;
        case 2:
            // Delete Group Functions
            break;
        case 3:
            // Show Group Info Functions
            break;
        case 4:
            // Show Application Status Functions
            break;
        default:
            printf("You didn't choose the right option, try again.\n");
            break;
        }
    }

    pthread_exit(0); // check later if we need to return anything in this thread
}

int main(){

    // start the local_sever


    //connection to the authentication_server
    

    // local_server console, after being started
    pthread_t console_thread;
    
    pthread_create(console_thread, NULL,console, NULL );


    return 0;
}
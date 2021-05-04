#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <pthread.h>

#include "./Console/console-lib.h"

void * console(void *arg){
    int order_number = 0;

    while(1){
        
        order_number = home_screen(&order_number)

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

    pthread_exit();
}

int main(){

    // Start the local_sever
    

    // local_server console, after being started

    


    return 0;
}
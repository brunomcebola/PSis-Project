#include <stdio.h>~

int main(){

    // Start the local_sever
    

    // local_server console, after being started

    int order_number = 0;

    printf("---------------------\n");
    printf("-------Console-------\n");
    printf("---------------------\n");

    printf("What do you want to do ?\n");
    printf("-- Create Group ------------- 1\n");
    printf("-- Delete Group ------------- 2\n");
    printf("-- Show Group Info ---------- 3\n");
    printf("-- Show Application Status -- 4\n");



    switch (order_number){
    case 1:
        // some code
        break;
    case 2:
        // some code
        break;
    case 3:
        // some code
        break;
    case 4:
        // some code
        break;
    default:
        printf("You didn't choose the right option, try again.\n");
        break;
    }


    return 0;
}
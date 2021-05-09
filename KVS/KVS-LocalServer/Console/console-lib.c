#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <pthread.h>


int home_screen(int * number_order){

    printf("---------------------\n");
    printf("-------Console-------\n");
    printf("---------------------\n");

    printf("What do you want to do ?\n");
    printf("-- Create Group ------------- 1\n");
    printf("-- Delete Group ------------- 2\n");
    printf("-- Show Group Info ---------- 3\n");
    printf("-- Show Application Status -- 4\n");

    scanf("%d", number_order);

    return (*number_order);
}
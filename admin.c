#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define BUFF_SIZE 200

// will display a message
void displayMessage()
{
    printf("Do you want to close the hotel? Enter Y for Yes and N for No.\n");
}
 
int main() {
    int adm_shmid,hm_shmid;
    char *shmptr,buff[BUFF_SIZE];
    // creating key to connect to admin and hotel manager
    key_t key = ftok("admin.c", 0);

    if (key == -1) {
        printf("Error in executing ftok\n");
        exit(-1);
    }
    
    // creating an shm between admin and hotel manager
    adm_shmid = shmget(key, BUFF_SIZE, 0666 | IPC_CREAT);
    if (adm_shmid == -1) {
        printf("Error in executing shmget\n");
        exit(-2);
    }
    
    // attaching admin to the shm between admin and hotel manager
    // we are type casting it to char* because we are storing a string in the shm
    shmptr = (char*)shmat(adm_shmid, NULL, 0);
    if(shmptr==(void*)-1)
    {
        perror("Error in shmPtr in attaching the memory segment\n");
        return 1;
    }

    displayMessage();
    
    // loop will keep running until the hotel closes i.e when the i/p given to admin ti 'Y'
    while (1) {
        
        
        // Prompt user to enter Y for Yes and N for No
        char decision;
        scanf("%c",&decision);
        
        // If Y, inform hotel manager to close and exit the loop
        if(decision=='Y' || decision=='y')
        {
            // copying a string close into the shm of admin and hotel manager
            strcpy(buff,"close");
            int len=strlen(buff);
            if(buff[len-1]=='\n')
            buff[len-1]='\0';
            strcpy(shmptr,buff);

            // cleanup
            // detaching admin to the shm between admin hotel manager 
            if (shmdt(shmptr) == -1)
            {
                perror("Error in shmdt\n");
                exit(-1);
            }
            
            // freeing the shm after all the processes have been detached
            if(shmctl(adm_shmid,IPC_RMID,NULL)==-1)
            {
                perror("Error in executing shmctl\n");
                exit(-1);
            }
            break;
        }
        // If N, continue the loop
        else if(decision=='N' || decision=='n')
        {
            displayMessage();

        }
    }

    return 0;
}